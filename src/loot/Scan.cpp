// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.
//
// Hook-driven loot scanner. Walks the nearby-lootable-units set, opens
// each corpse's loot session via `FUN_CMSG_LOOT_UNIT`, then *intercepts
// the engine's loot-window response* before any visual updates happen.
// The intercept (`FUN_LOOT_CONTROLLER`) lets the original function run
// so `VAR_LOOT_SLOTS` populates and the link builder works — but a
// paired hook on `FUN_FIRE_EVENT_NO_ARGS` swallows the `LOOT_OPENED` /
// `LOOT_CLOSED` event fires while the scan is in progress. Net result:
// the engine's state machine cycles silently for each corpse and
// `LootFrame` never reacts. Other Lua addons listening to those events
// also see nothing during the scan; that's deliberate.
//
// Compared to the older polling-driven version this also collapses the
// state machine — we no longer need `WaitingOpen` / `WaitingClose` /
// per-tick `VAR_LOOT_GUID` reads, because the controller hook *is* the
// state transition.

#include "../Game.h"
#include "../Offsets.h"
#include "../event/Custom.h"
#include "../guid/Guid.h"
#include "../tick/WorldTick.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Loot::Scan {

namespace {

constexpr const char *kEventCompleted = "LOOT_SCAN_COMPLETED";
const Event::Custom::AutoReserve _reserve{kEventCompleted};

// Per-step timeout for the controller hook to fire on a queued
// CMSG_LOOT. WorldTick fires at the client frame rate (~30-60 Hz),
// so 180 ticks ≈ 3-6s wall clock. Long enough for high-latency
// servers, short enough that a single dropped response doesn't hang
// the whole scan.
constexpr int kStepTimeoutTicks = 180;

// Event IDs the engine fires through `FUN_FIRE_EVENT_NO_ARGS` for
// loot-window transitions. The hook swallows these only when the
// scanner has the suppression flag raised — normal play sees them
// fire unmodified.
constexpr int EVENT_LOOT_OPENED = 0x10B;
constexpr int EVENT_LOOT_CLOSED = 0x10D;

enum class State {
    Idle,
    WaitingForResponse, // CMSG_LOOT sent; awaiting our controller hook
};

struct SlotItem {
    uint32_t itemID;
    uint32_t count;
    std::string link; // empty if the cache record wasn't loaded
};

struct LootEntry {
    uint64_t guid;
    uint32_t coin; // copper amount; 0 if no coin
    std::vector<SlotItem> items;
};

struct ScanCtx {
    State state = State::Idle;
    void *player = nullptr;
    uint64_t currentGuid = 0;
    int ticksSinceTransition = 0;
    bool suppressEvents = false; // read by the FireEvent hook
    std::vector<uint64_t> queue;
    std::vector<LootEntry> results;
};

ScanCtx s_scan;

// === Typedefs for engine helpers ===

using ClntObjMgrEnumVisibleObjectsCallback_t = int(__fastcall *)(void *ctx,
                                                                  void *unusedEdx,
                                                                  uint64_t guid);
using ClntObjMgrEnumVisibleObjects_t =
    int(__fastcall *)(ClntObjMgrEnumVisibleObjectsCallback_t cb, void *ctx);
using ClntObjMgrObjectPtr_t = void *(__fastcall *)(uint32_t typeMask,
                                                    const char *debugMsg,
                                                    uint32_t guidLo,
                                                    uint32_t guidHi,
                                                    int debugCode);
using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
using LootUnit_t = void(__thiscall *)(void *player, void *target,
                                      char useDistanceCheck);
using CloseLootInner_t = void(__fastcall *)(int sendRelease,
                                             char earlyReturnOnGameObject,
                                             char showError);
using LootSlotLinkBuilder_t = const char *(__fastcall *)(uint32_t userFacingSlot,
                                                          char *outBuf,
                                                          int bufSize);
// `FUN_LOOT_CONTROLLER` is `__fastcall(int, undefined4, int)` — three
// args: lootStructPtr in ECX, coin in EDX, an extra unk on stack[0].
// Mis-declaring as 4-arg __fastcall would push one phantom stack slot
// the engine doesn't pop, drifting ESP by 4 bytes per call until a
// later return reads `0x00000001` as a corrupt frame pointer and the
// process crashes. We do see / forward the `coin` value because the
// engine reads it as `DAT_00b71ba0` later.
using LootController_t = void(__fastcall *)(int lootStructPtr,
                                             int coin,
                                             int unk);
using FireEventNoArgs_t = void(__fastcall *)(int eventID,
                                              int /*edx unused*/);

struct C3Vector {
    float x, y, z;
};
using GetPosition_t = const C3Vector *(__thiscall *)(const void *self,
                                                      C3Vector *outBuf);
constexpr int VTBL_GET_POSITION_OFFSET = 0x14;

constexpr float INTERACT_REACH = 1.3333333f;
constexpr float MIN_INTERACT_RANGE = 5.0f;

// === Range / lootability helpers (identical to Loot::Nearby) ===

bool IsLootableUnit(const void *unit) {
    auto *fields = *reinterpret_cast<const uint8_t *const *>(
        static_cast<const uint8_t *>(unit) + Offsets::OFF_UNIT_DESCRIPTOR);
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        fields + Offsets::OFF_UNIT_FIELD_DYNAMIC_FLAGS);
    return (flags & Offsets::UNIT_DYNFLAG_LOOTABLE) != 0;
}

float BoundingRadius(const void *unit) {
    auto *fields = *reinterpret_cast<const uint8_t *const *>(
        static_cast<const uint8_t *>(unit) + Offsets::OFF_UNIT_DESCRIPTOR);
    return *reinterpret_cast<const float *>(
        fields + Offsets::OFF_UNIT_FIELD_BOUNDING_RADIUS);
}

const C3Vector *GetPosition(const void *unit, C3Vector *outBuf) {
    auto vtable = *reinterpret_cast<void *const *const *>(unit);
    auto fn = reinterpret_cast<GetPosition_t>(
        *reinterpret_cast<const void *const *>(
            reinterpret_cast<const uint8_t *>(vtable) +
            VTBL_GET_POSITION_OFFSET));
    return fn(unit, outBuf);
}

bool InInteractRange(const void *player, const void *target) {
    C3Vector pBuf{};
    C3Vector tBuf{};
    const C3Vector *pPos = GetPosition(player, &pBuf);
    const C3Vector *tPos = GetPosition(target, &tBuf);
    if (pPos == nullptr || tPos == nullptr)
        return false;
    const float dx = pPos->x - tPos->x;
    const float dy = pPos->y - tPos->y;
    const float dz = pPos->z - tPos->z;
    const float distSq = dx * dx + dy * dy + dz * dz;
    float range = BoundingRadius(player) + BoundingRadius(target) + INTERACT_REACH;
    if (range < MIN_INTERACT_RANGE)
        range = MIN_INTERACT_RANGE;
    return distSq <= range * range;
}

// === Scrape ===

// Reads the post-population `VAR_LOOT_SLOTS` table into a LootEntry.
// The controller hook calls this *after* letting the original
// `FUN_LOOT_CONTROLLER` run, so the slot table is fully filled in
// and the link builder works against it.
void ScrapeCurrentLoot(LootEntry *out) {
    const uint32_t coinRaw = *reinterpret_cast<const uint32_t *>(
        Offsets::VAR_LOOT_LOOTABLE);
    out->coin = (coinRaw == 0xFFFFFFFFu) ? 0u : coinRaw;

    const bool hasCoin = coinRaw != 0;
    auto LinkBuilder = reinterpret_cast<LootSlotLinkBuilder_t>(
        Offsets::FUN_LOOT_SLOT_LINK_BUILDER);

    for (int slot = 0; slot < Offsets::LOOT_MAX_SLOTS; ++slot) {
        auto *entry = reinterpret_cast<const uint8_t *>(
            Offsets::VAR_LOOT_SLOTS + slot * Offsets::LOOT_SLOT_STRIDE);
        const uint32_t itemID = *reinterpret_cast<const uint32_t *>(entry);
        if (itemID == 0)
            continue;
        const uint32_t count = *reinterpret_cast<const uint32_t *>(entry + 8);

        SlotItem item;
        item.itemID = itemID;
        item.count = count;

        char linkBuf[0x400];
        const uint32_t slotArg = hasCoin ? static_cast<uint32_t>(slot + 1)
                                          : static_cast<uint32_t>(slot);
        const char *link = LinkBuilder(slotArg, linkBuf, sizeof linkBuf);
        if (link != nullptr)
            item.link = link;

        out->items.push_back(std::move(item));
    }
}

// === State-machine transitions ===

void StartLoot(uint64_t guid);
void TryStartNext();

void SendCloseLoot() {
    auto Close = reinterpret_cast<CloseLootInner_t>(Offsets::FUN_CLOSE_LOOT_INNER);
    Close(1, '\0', '\0');
}

void Complete() {
    s_scan.state = State::Idle;
    s_scan.currentGuid = 0;
    s_scan.suppressEvents = false;
    Event::Custom::Fire(Event::Custom::Lookup(kEventCompleted), "");
}

void TryStartNext() {
    if (s_scan.queue.empty()) {
        Complete();
        return;
    }
    uint64_t guid = s_scan.queue.back();
    s_scan.queue.pop_back();
    StartLoot(guid);
}

void StartLoot(uint64_t guid) {
    auto ObjectPtr = reinterpret_cast<ClntObjMgrObjectPtr_t>(
        Offsets::FUN_CLNT_OBJ_MGR_OBJECT_PTR);
    void *target = ObjectPtr(Offsets::TYPEMASK_UNIT, nullptr,
                             static_cast<uint32_t>(guid),
                             static_cast<uint32_t>(guid >> 32), 0);
    if (target == nullptr) {
        TryStartNext();
        return;
    }
    auto LootUnit = reinterpret_cast<LootUnit_t>(Offsets::FUN_CMSG_LOOT_UNIT);
    LootUnit(s_scan.player, target, /*useDistanceCheck=*/0);
    s_scan.currentGuid = guid;
    s_scan.state = State::WaitingForResponse;
    s_scan.ticksSinceTransition = 0;
}

// === Hooks ===

LootController_t s_lootController_o = nullptr;
FireEventNoArgs_t s_fireEventNoArgs_o = nullptr;

// `LOOT_OPENED` / `LOOT_CLOSED` get suppressed only while a scan is
// actively interleaving open/close cycles, signaled by
// `s_scan.suppressEvents`. The flag is raised by the controller hook
// just around the original-loot-controller call and the
// `SendCloseLoot` that follows, then dropped before
// `TryStartNext`-initiated CMSG_LOOTs go out. Every other engine
// event fire — and normal play's loot events — pass through.
void __fastcall FireEventNoArgs_h(int eventID, int /*unusedEdx*/) {
    if (s_scan.suppressEvents &&
        (eventID == EVENT_LOOT_OPENED || eventID == EVENT_LOOT_CLOSED)) {
        return;
    }
    s_fireEventNoArgs_o(eventID, 0);
}

// Intercepts the engine's loot-window state controller. Two paths:
//
//   1. **Scan-driven open** — the incoming `lootStructPtr` carries a
//      GUID matching our expected one. Let the original run (so the
//      slot table populates), then scrape, send a release (which
//      will recursively re-enter via `FUN_0048F200` → close-path
//      `FUN_LOOT_CONTROLLER(NULL, …)`), and advance the scan. Both
//      the inner open and the recursive close fire their respective
//      events through our `FireEventNoArgs_h`, which swallows them
//      while `suppressEvents` is on.
//
//   2. **Anything else** — pass through unmodified. Normal loot
//      interactions outside a scan are unaffected; the inner-close
//      call generated by our scan's `SendCloseLoot` re-enters here
//      with `arg1 == 0` and is also a passthrough (its
//      suppression-flag check happens at the FireEvent hook layer).
void __fastcall LootController_h(int lootStructPtr, int coin, int unk) {
    const bool scanOpenCall =
        s_scan.state == State::WaitingForResponse &&
        lootStructPtr != 0 &&
        [&] {
            // Read the GUID off the response struct (`+8` deref to a
            // `{uint32 lo; uint32 hi}` block — same shape the original
            // copies into `VAR_LOOT_GUID_LO/HI`).
            auto *guidPtr = *reinterpret_cast<const uint32_t *const *>(
                reinterpret_cast<const uint8_t *>(
                    static_cast<intptr_t>(lootStructPtr)) + 8);
            if (guidPtr == nullptr) return false;
            const uint64_t guid =
                static_cast<uint64_t>(guidPtr[0]) |
                (static_cast<uint64_t>(guidPtr[1]) << 32);
            return guid == s_scan.currentGuid;
        }();

    if (scanOpenCall) {
        s_scan.suppressEvents = true;
        s_lootController_o(lootStructPtr, coin, unk);

        LootEntry entry;
        entry.guid = s_scan.currentGuid;
        ScrapeCurrentLoot(&entry);
        s_scan.results.push_back(std::move(entry));

        // Sends `CMSG_LOOT_RELEASE` and recursively calls this hook
        // with `arg1 == 0`. That recursive call falls into the
        // passthrough branch but still sees `suppressEvents == true`
        // at the FireEvent layer, so the `LOOT_CLOSED` fire inside
        // gets swallowed.
        SendCloseLoot();

        s_scan.suppressEvents = false;
        TryStartNext();
        return;
    }

    s_lootController_o(lootStructPtr, coin, unk);
}

const Game::HookAutoRegister _hookController{
    Offsets::FUN_LOOT_CONTROLLER,
    reinterpret_cast<void *>(&LootController_h),
    reinterpret_cast<void **>(&s_lootController_o)};

const Game::HookAutoRegister _hookFireEvent{
    Offsets::FUN_FIRE_EVENT_NO_ARGS,
    reinterpret_cast<void *>(&FireEventNoArgs_h),
    reinterpret_cast<void **>(&s_fireEventNoArgs_o)};

// === Tick (timeout only) ===

void OnTick() {
    if (s_scan.state == State::Idle)
        return;
    ++s_scan.ticksSinceTransition;
    if (s_scan.ticksSinceTransition >= kStepTimeoutTicks) {
        // Controller hook never fired — engine guards rejected the
        // send (player state / OOR), or the server dropped the
        // packet. Skip this corpse and advance.
        s_scan.suppressEvents = false;
        TryStartNext();
    }
}

const Tick::WorldTick::AutoSubscribe _tick{&OnTick};

// === Enumeration ===

struct EnumCtx {
    const void *player;
    std::vector<uint64_t> *out;
};

int __fastcall EnumCallback(EnumCtx *ctx, void * /*unusedEdx*/, uint64_t guid) {
    auto ObjectPtr = reinterpret_cast<ClntObjMgrObjectPtr_t>(
        Offsets::FUN_CLNT_OBJ_MGR_OBJECT_PTR);
    void *obj = ObjectPtr(Offsets::TYPEMASK_UNIT, nullptr,
                          static_cast<uint32_t>(guid),
                          static_cast<uint32_t>(guid >> 32), 0);
    if (obj == nullptr)
        return 1;
    if (!IsLootableUnit(obj))
        return 1;
    if (!InInteractRange(ctx->player, obj))
        return 1;
    ctx->out->push_back(guid);
    return 1;
}

void BuildQueue(void *player) {
    s_scan.queue.clear();
    auto Enum = reinterpret_cast<ClntObjMgrEnumVisibleObjects_t>(
        Offsets::FUN_CLNT_OBJ_MGR_ENUM_VISIBLE_OBJECTS);
    EnumCtx ctx{player, &s_scan.queue};
    Enum(reinterpret_cast<ClntObjMgrEnumVisibleObjectsCallback_t>(&EnumCallback),
         &ctx);
}

// === Lua bindings ===

int __fastcall Script_ScanNearbyLoot(void *L) {
    if (s_scan.state != State::Idle) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    if (*reinterpret_cast<void *volatile *>(Offsets::VAR_LOCAL_PLAYER_PTR) == nullptr) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    auto Resolve = reinterpret_cast<ResolveUnitToken_t>(
        Offsets::FUN_RESOLVE_UNIT_TOKEN);
    void *player = Resolve("player");
    if (player == nullptr) {
        Game::Lua::PushBool(L, false);
        return 1;
    }

    s_scan.results.clear();
    s_scan.player = player;
    BuildQueue(player);

    if (s_scan.queue.empty()) {
        Complete();
        Game::Lua::PushBool(L, true);
        return 1;
    }

    TryStartNext();
    Game::Lua::PushBool(L, true);
    return 1;
}

int __fastcall Script_IsScanInProgress(void *L) {
    Game::Lua::PushBool(L, s_scan.state != State::Idle);
    return 1;
}

int __fastcall Script_GetLastScanResults(void *L) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);

    int outIdx = 1;
    for (const auto &entry : s_scan.results) {
        Game::Lua::PushNumber(L, static_cast<double>(outIdx++));
        Game::Lua::NewTable(L);

        char guidBuf[Guid::STRING_SIZE];
        Game::Lua::SetFieldString(L, "guid",
                                  Guid::FormatAsString(entry.guid, guidBuf,
                                                       sizeof guidBuf));
        Game::Lua::SetFieldNumber(L, "coin", static_cast<double>(entry.coin));

        Game::Lua::PushString(L, "items");
        Game::Lua::NewTable(L);
        int itemIdx = 1;
        for (const auto &item : entry.items) {
            Game::Lua::PushNumber(L, static_cast<double>(itemIdx++));
            Game::Lua::NewTable(L);
            Game::Lua::SetFieldNumber(L, "itemID", static_cast<double>(item.itemID));
            Game::Lua::SetFieldNumber(L, "count", static_cast<double>(item.count));
            if (!item.link.empty())
                Game::Lua::SetFieldString(L, "link", item.link.c_str());
            Game::Lua::SetTable(L, -3);
        }
        Game::Lua::SetTable(L, -3);

        Game::Lua::SetTable(L, -3);
    }
    return 1;
}

void Register() {
    Game::Lua::RegisterTableFunction("C_Loot", "ScanNearbyLoot",
                                     &Script_ScanNearbyLoot);
    Game::Lua::RegisterTableFunction("C_Loot", "IsScanInProgress",
                                     &Script_IsScanInProgress);
    Game::Lua::RegisterTableFunction("C_Loot", "GetLastScanResults",
                                     &Script_GetLastScanResults);
}

} // namespace

static const Game::ModuleAutoRegister _autoreg{&Register};

} // namespace Loot::Scan
