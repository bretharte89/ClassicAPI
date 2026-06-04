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
// You should have received a copy of the GNU Lesser General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"
#include "item/Arg.h"
#include "item/Data.h"
#include "item/Location.h"
#include "tick/WorldTick.h"

#include <cstdint>

namespace Item::Data {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                       const uint64_t *guid, void *callback,
                                                       void *userData, int unused);

// Two events fire on item-data arrival; they are **mutually exclusive**
// based on what triggered the cache fill:
//
//   `GET_ITEM_INFO_RECEIVED(itemID, success)` — added in 5.4. Fires for
//       **implicit / transparent** cache fills: `GetItemInfo` warmup,
//       hyperlink hover, chat-link resolution, our `SetItemByID` warmup,
//       etc. This is the event most existing addons listen to.
//
//   `ITEM_DATA_LOAD_RESULT(itemID, success)`  — added in 8.0. Fires for
//       **explicit** request paths only: `C_Item.RequestLoadItemData(ByID)`
//       and friends. The completion event for callers who explicitly
//       asked the modern Item-data API to load a specific item.
//
// Behavior verified empirically against 1.15.x: a single cache fill
// fires exactly one event, never both, depending on what initiated
// the request. We honor the split with two distinct callbacks.
//
// Names are reserved at static-init time via `Event::Custom::AutoReserve`;
// the `RebuildEventTable` hook appends them to the engine's input array
// at table-population time, so the engine itself allocates and tracks
// our entries identically to its own.
static constexpr const char *kItemDataLoadResult = "ITEM_DATA_LOAD_RESULT";
static constexpr const char *kGetItemInfoReceived = "GET_ITEM_INFO_RECEIVED";
static const Event::Custom::AutoReserve _reserveItemDataLoadResult{kItemDataLoadResult};
static const Event::Custom::AutoReserve _reserveGetItemInfoReceived{kGetItemInfoReceived};

static void FireGetItemInfoReceived(int itemID, bool success) {
    Event::Custom::FireIdSuccess(Event::Custom::Lookup(kGetItemInfoReceived),
                                 itemID, success);
}

static void FireItemDataLoadResult(int itemID, bool success) {
    Event::Custom::FireIdSuccess(Event::Custom::Lookup(kItemDataLoadResult),
                                 itemID, success);
}

// Engine callback for **implicit** cache fills (transparent warmup
// triggered by our `Script_GetItemInfo` hook or `SetItemByID`). Fires
// only the broad `GET_ITEM_INFO_RECEIVED` event — `ITEM_DATA_LOAD_RESULT`
// is reserved for explicit `RequestLoadItemData(ByID)` paths to match
// modern API semantics.
//
// Calling convention is __stdcall, 2 args / 8 bytes — matching the
// callback at 0x004FDC30 (used by 4 sites in the binary) which ends in
// `ret 8`. If the engine called us with a different convention, that
// callback would crash the engine, so stdcall is verified by induction.
static void __stdcall ItemLoadCallback_Implicit(void *userData, int success) {
    const auto itemID = static_cast<int>(reinterpret_cast<uintptr_t>(userData));
    FireGetItemInfoReceived(itemID, success != 0);
}

using ItemLoadCallback_t = void(__stdcall *)(void *userData, int success);

// Calls DBCache_ItemStats_C_GetRecord. With `callback=nullptr`,
// performs only the hash-table lookup; returns the cached record or
// nullptr. With a non-null callback, also kicks off the
// SMSG_ITEM_QUERY_SINGLE if the item isn't cached — the engine fills
// the cache asynchronously and invokes the supplied callback with the
// itemID smuggled through `userData` (the engine just stores and
// replays it without dereferencing).
//
// Disassembly of `_GetRecord` at 0x55BAAC shows it branches on the
// callback arg (stack slot +0x10), not on the 6th `bool` arg that
// VanillaHelpers documented as `requestIfMissing` — that slot is reused
// as a local immediately after entry, never read. The trigger for the
// request path is `callback != NULL`.
//
// The 6th arg IS used elsewhere though: when non-zero, the engine
// walks the entry's existing pending-callback list and skips the
// append if a node with matching `(callback, userData)` is already
// queued. We always pass `1` so that bursts of `WarmCache` calls for
// the same itemID (e.g. Bagnon enumerating the same slot repeatedly
// while the SMSG_ITEM_QUERY_SINGLE_RESPONSE is in flight) collapse to
// a single callback registration — otherwise the response fires our
// `GET_ITEM_INFO_RECEIVED` once per call.
static const uint8_t *CacheFetch(uint32_t itemID, ItemLoadCallback_t callback) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    void *cb = reinterpret_cast<void *>(callback);
    void *userData =
        callback ? reinterpret_cast<void *>(static_cast<uintptr_t>(itemID)) : nullptr;
    return fn(cache, itemID, &zeroGuid, cb, userData, 1);
}

// Resolves an item-location table at stack idx to the itemID by walking
// `Location::Resolve` -> CGItem -> instance block (+0x08) -> itemID (+0x0C).
// Returns 0 if the slot is empty or the table is malformed.
static int ResolveLocationToItemID(void *L, int idx) {
    const uint8_t *item = Item::Location::Resolve(L, idx);
    if (item == nullptr)
        return 0;
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return static_cast<int>(*reinterpret_cast<const uint32_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_ITEM_ID));
}

static int __fastcall Script_IsItemDataCachedByID(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    const bool cached =
        (itemID > 0) && (CacheFetch(static_cast<uint32_t>(itemID), nullptr) != nullptr);
    Game::Lua::PushBool(L, cached);
    return 1;
}

static int __fastcall Script_IsItemDataCached(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.IsItemDataCached(itemLocation)");
        return 0;
    }
    const int itemID = ResolveLocationToItemID(L, 1);
    const bool cached =
        (itemID > 0) && (CacheFetch(static_cast<uint32_t>(itemID), nullptr) != nullptr);
    Game::Lua::PushBool(L, cached);
    return 1;
}

// Pending explicit-request tracking. We deliberately do NOT eagerly
// create cache entries when an addon calls `C_Item.RequestLoadItemData`
// at addon-load / PLAYER_ENTERING_WORLD time — empirically (pfQuest),
// pre-creating the entry blocks the engine's natural prefetch path
// from issuing a fresh `SMSG_ITEM_QUERY_SINGLE`. The entry stays in a
// permanently-pending state because nothing ever sends the query.
//
// Instead we hook the engine's item-cache response handler at
// `FUN_ITEMSTATS_CACHE_RESPONSE` (the function that fills cache
// entries on `SMSG_ITEM_QUERY_SINGLE_RESPONSE`). After the engine
// finishes filling any entries and walking its own callback lists,
// we sweep our pending set and fire `ITEM_DATA_LOAD_RESULT(itemID, 1)`
// for any tracked items that now resolve. The engine's natural
// prefetch (for player-inventory items) takes care of the actual
// `SMSG_ITEM_QUERY_SINGLE` for us.
//
// For items the engine *doesn't* naturally prefetch (e.g. quest
// reward IDs the player has never owned), we escalate after
// `kEscalateTicks` by issuing our own `CacheFetch` with a no-op
// callback — that creates the entry and queues the query, and the
// response handler hook still fires our event when the data lands.
// `kMaxWaitTicks` is the hard timeout that gives up and fires
// `ITEM_DATA_LOAD_RESULT(itemID, nil)`.
struct Pending {
    uint32_t itemID;
    int ticksWaiting;
    bool requestIssued;
};

constexpr int kMaxPending = 64;
constexpr int kEscalateTicks = 60;
constexpr int kMaxWaitTicks = 1200;

static Pending g_pending[kMaxPending];
static int g_pendingCount = 0;

static int FindPending(uint32_t itemID) {
    for (int i = 0; i < g_pendingCount; ++i) {
        if (g_pending[i].itemID == itemID)
            return i;
    }
    return -1;
}

static void RemovePendingAt(int idx) {
    --g_pendingCount;
    if (idx != g_pendingCount)
        g_pending[idx] = g_pending[g_pendingCount];
}

// No-op callback for the escalation path — we just need a non-null
// callback pointer so the engine's `_GetRecord` queues the query.
// The response handler hook will detect the fill and fire the event;
// this callback doesn't need to do anything.
static void __stdcall ItemLoadCallback_NoOp(void * /*userData*/, int /*success*/) {}

static void RequestAndMaybeNotify(uint32_t itemID) {
    // Already loaded — fire success synchronously.
    if (CacheFetch(itemID, nullptr) != nullptr) {
        FireItemDataLoadResult(static_cast<int>(itemID), true);
        return;
    }
    // Already tracking — second request just dedups.
    if (FindPending(itemID) >= 0)
        return;
    if (g_pendingCount >= kMaxPending)
        return;
    g_pending[g_pendingCount].itemID = itemID;
    g_pending[g_pendingCount].ticksWaiting = 0;
    g_pending[g_pendingCount].requestIssued = false;
    ++g_pendingCount;
}

// Response handler hook. After the engine fills cache entries from
// `SMSG_ITEM_QUERY_SINGLE_RESPONSE` and walks the per-entry callback
// lists, we sweep `g_pending` for anything now-loaded and fire the
// event. This is the engine-aligned alternative to per-tick polling:
// we observe fills exactly when they happen, with zero added latency
// and no risk of racing the engine's own callback walk.
//
// `__thiscall` is mimicked as `__fastcall(ecx, edx_unused, ...)` per
// the convention used elsewhere in this codebase (see
// `KeyringDescChange_h`, `FrameRegisterEvent_h`).
using ItemStatsCacheResponse_t = void(__thiscall *)(void *cache, void *packetReader, int flag);
static ItemStatsCacheResponse_t ItemStatsCacheResponse_o = nullptr;

static void __fastcall ItemStatsCacheResponse_h(void *cache, void * /*edx_unused*/,
                                                 void *packetReader, int flag) {
    using Trampoline_t = void(__fastcall *)(void *cache, void *edx, void *packetReader, int flag);
    reinterpret_cast<Trampoline_t>(ItemStatsCacheResponse_o)(cache, nullptr, packetReader, flag);
    // Sweep pending — the engine just finished filling any matching
    // entries. Peek (no callback) returns non-null iff the entry's
    // loaded flag is set.
    for (int i = 0; i < g_pendingCount;) {
        const uint32_t itemID = g_pending[i].itemID;
        if (CacheFetch(itemID, nullptr) != nullptr) {
            RemovePendingAt(i);
            FireItemDataLoadResult(static_cast<int>(itemID), true);
            continue;
        }
        ++i;
    }
}

static const Game::HookAutoRegister _hookCacheResponse{
    Offsets::FUN_ITEMSTATS_CACHE_RESPONSE,
    reinterpret_cast<void *>(&ItemStatsCacheResponse_h),
    reinterpret_cast<void **>(&ItemStatsCacheResponse_o)};

// Escalation + timeout. The hook covers the happy path (engine fills
// cache → we fire). This tick is only responsible for:
//   1. Issuing a query for items the engine doesn't naturally prefetch
//      (e.g. quest rewards) — wait `kEscalateTicks` so we don't race
//      the engine's startup-time state machine, then call CacheFetch
//      with a no-op callback. The engine sends the SMSG, the response
//      handler hook fires the event when the response lands.
//   2. Giving up after `kMaxWaitTicks` for items that never resolve
//      (bad itemID, server doesn't respond, etc.) — fire failure.
static void OnWorldTick() {
    for (int i = 0; i < g_pendingCount;) {
        if (!g_pending[i].requestIssued && g_pending[i].ticksWaiting >= kEscalateTicks) {
            CacheFetch(g_pending[i].itemID, &ItemLoadCallback_NoOp);
            g_pending[i].requestIssued = true;
        }
        if (g_pending[i].ticksWaiting >= kMaxWaitTicks) {
            const uint32_t itemID = g_pending[i].itemID;
            RemovePendingAt(i);
            FireItemDataLoadResult(static_cast<int>(itemID), false);
            continue;
        }
        ++g_pending[i].ticksWaiting;
        ++i;
    }
}

static const Tick::WorldTick::AutoSubscribe _tickSub{&OnWorldTick};

// Public C++ API — see Item/Data.h. Implicit (transparent) warmup
// path used by the `Script_GetItemInfo` hook and `SetItemByID`. Fires
// the cache request only if the item isn't already cached; uses the
// implicit callback which fires only `GET_ITEM_INFO_RECEIVED` (not
// `ITEM_DATA_LOAD_RESULT`), matching modern semantics for ambient
// cache fills.
void WarmCache(uint32_t itemID) {
    CacheFetch(itemID, &ItemLoadCallback_Implicit);
}

// Hook for `Script_GetItemInfo`. Pre-warms the cache from arg 1, then
// dispatches to the original. Effect: the original still returns nil
// for cache misses (vanilla behavior), but a query is now in flight,
// so subsequent calls will return valid data and
// GET_ITEM_INFO_RECEIVED will fire.
Script_GetItemInfo_t Script_GetItemInfo_o = nullptr;

int __fastcall Script_GetItemInfo_h(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    if (itemID > 0)
        WarmCache(static_cast<uint32_t>(itemID));
    return Script_GetItemInfo_o(L);
}

static int __fastcall Script_RequestLoadItemDataByID(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    if (itemID <= 0) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    RequestAndMaybeNotify(static_cast<uint32_t>(itemID));
    Game::Lua::PushBoolean(L, 1);
    return 1;
}

static int __fastcall Script_RequestLoadItemData(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.RequestLoadItemData(itemLocation)");
        return 0;
    }
    const int itemID = ResolveLocationToItemID(L, 1);
    if (itemID <= 0) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    RequestAndMaybeNotify(static_cast<uint32_t>(itemID));
    Game::Lua::PushBoolean(L, 1);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "IsItemDataCachedByID",
                                     &Script_IsItemDataCachedByID);
    Game::Lua::RegisterTableFunction("C_Item", "IsItemDataCached", &Script_IsItemDataCached);
    Game::Lua::RegisterTableFunction("C_Item", "RequestLoadItemDataByID",
                                     &Script_RequestLoadItemDataByID);
    Game::Lua::RegisterTableFunction("C_Item", "RequestLoadItemData", &Script_RequestLoadItemData);
    // Event names are reserved at file scope via `AutoReserve`; nothing
    // to do here for events.
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

// Auto-warm the item cache on `GetItemInfo(uncached_id)` calls, so
// subsequent calls return valid data and GET_ITEM_INFO_RECEIVED
// fires when the response arrives — matches modern WoW (5.x+)
// behavior. Without this, vanilla 1.12's `GetItemInfo` returns nil
// for misses and never fires a query, forcing addons to roll their
// own warmup hacks.
static const Game::HookAutoRegister _hookreg{
    Offsets::FUN_SCRIPT_GET_ITEM_INFO,
    reinterpret_cast<void *>(&Script_GetItemInfo_h),
    reinterpret_cast<void **>(&Script_GetItemInfo_o)};

} // namespace Item::Data
