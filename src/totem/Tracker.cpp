// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

// `GetTotemInfo(slot)` — the four-element totem bar, backported. Vanilla
// 1.12 tracks nothing about the shaman's totems client-side (the totem
// bar, PLAYER_TOTEM_UPDATE, and GetTotemInfo are all TBC additions), so
// this is a self-tracking subsystem — but a fully DATA-DRIVEN one:
//
//   * SLOT is read straight from the totem summon spell's `Spell.dbc`
//     effect: 87/88/89/90 = SUMMON_TOTEM_SLOT1..4 = Fire/Earth/Water/Air,
//     so `slot = effect - 86`. No hardcoded spell table; Turtle custom
//     totems self-classify.
//   * CREATURE ENTRY is the effect's `EffectMiscValue` (the summoned
//     totem's creature-template ID) — used to detect the totem's death.
//   * DURATION is the summon spell's `SpellDuration.dbc` value (via the
//     engine's own duration helper, so player duration mods apply).
//
// Detection: the player's totem cast fires SMSG_SPELL_GO, which
// `Aura::Source` already co-hooks; it calls `OnPlayerSpellGo` here (before
// its aura gate, since a summon applies no aura). We stamp the slot with
// start time + duration.
//
// Death / removal: on WorldTick we scan the object manager for a
// player-owned creature of the slot's entry. A "seen once, then gone"
// latch clears the slot early (totem killed, Totemic Recall, or manually
// destroyed) — the owner check (CreatedBy == player) means another
// shaman's same-entry totem can't keep ours alive, and the seen-latch
// tolerates the ~1-tick spawn lag before the totem appears. Duration is
// the backstop for the normal case. A recast into the same slot simply
// re-stamps it.

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"
#include "event/SignalHook.h"
#include "guid/Guid.h"
#include "item/Count.h"
#include "spell/Lookup.h"
#include "tick/WorldTick.h"
#include "unit/Identity.h"

#include <cstdint>

namespace Totem::Tracker {

namespace {

// Spell.dbc record field offsets (see spell/Info.cpp for the shared set).
constexpr int OFF_EFFECT = 0xF4;      // int32[3] — SPELL_EFFECT_*
constexpr int OFF_TOTEM_TOOL = 0xA0;  // int32[2] — Totem (required tool item IDs)
constexpr int OFF_MISC_VALUE = 0x1A8; // int32[3] — EffectMiscValue
constexpr int OFF_ICON_ID = 0x1D4;    // int32 — SpellIconID
constexpr int OFF_NAME = 0x1E0;       // char*[9] locale array

// SUMMON_TOTEM_SLOT1..4 spell effects (verified against Spell.dbc: Searing
// 87/Fire, Stoneskin 88/Earth, Healing Stream 89/Water, Windfury 90/Air).
constexpr int kEffectSummonTotemSlot1 = 87;
constexpr int kEffectSummonTotemSlot4 = 90;
constexpr int kSlots = 4;


// Re-scan the object manager for totem existence at most this often.
constexpr uint32_t kScanIntervalMs = 250;

struct Slot {
    bool active;
    bool seen;          // the totem creature has been observed in the world
    uint32_t spellID;
    uint32_t entry;     // summoned creature-template ID (for death detection)
    uint32_t startMs;   // FUN_OS_TICKCOUNT_MS (shares GetTime's epoch × 1000)
    uint32_t durationMs;
};

Slot g_slots[kSlots] = {};
uint32_t g_lastScanMs = 0;

// Per-slot totem tool item ID (Fire/Earth/Water/Air), read from the totem
// spell's `Totem[0]` field — the required TOOL the spell lists (Earth
// Totem, Fire Totem, …), NOT a consumed reagent, and NOT hardcoded. Filled
// from the player's spellbook on SPELLS_CHANGED and refreshed at cast; 0 =
// not yet resolved. `g_spellsChangedId` caches the event id.
uint32_t g_toolItem[kSlots] = {};
int g_spellsChangedId = -1;

uint32_t NowMs() {
    using TickCount_t = uint32_t(__fastcall *)();
    return reinterpret_cast<TickCount_t>(
        static_cast<uintptr_t>(Offsets::FUN_OS_TICKCOUNT_MS))();
}

// The totem tool item a spell requires (`Totem[0]`), or 0.
uint32_t TotemToolItem(const uint8_t *rec) {
    const int32_t t = *reinterpret_cast<const int32_t *>(rec + OFF_TOTEM_TOOL);
    return t > 0 ? static_cast<uint32_t>(t) : 0;
}

// Fill any still-unknown slot tool items by scanning the player's spellbook
// for totem summons and reading each one's required tool. Tools are fixed
// per element, so once a slot is resolved it never changes and the scan
// early-outs. Walks the flat learned-spell array `VAR_PLAYER_SPELLBOOK`
// (the same list `GetSpellName` reads) — the `IsPlayerSpell` bitmap
// doesn't reliably carry totems. Driven by SPELLS_CHANGED (when the
// spellbook is populated), so no polling.
void ScanTools() {
    bool anyUnknown = false;
    for (int s = 0; s < kSlots; ++s)
        if (g_toolItem[s] == 0) {
            anyUnknown = true;
            break;
        }
    if (!anyUnknown)
        return;

    auto *book = reinterpret_cast<const int32_t *>(
        static_cast<uintptr_t>(Offsets::VAR_PLAYER_SPELLBOOK));
    for (int slot = 0; slot < Offsets::SPELLBOOK_MAX_SLOTS; ++slot) {
        const int spellID = book[slot];
        if (spellID <= 0)
            continue;
        const uint8_t *rec = Spell::Lookup::RecordForID(spellID);
        if (rec == nullptr)
            continue;
        const auto *eff = reinterpret_cast<const int32_t *>(rec + OFF_EFFECT);
        for (int i = 0; i < Offsets::SPELL_RECORD_EFFECT_COUNT; ++i) {
            const int e = eff[i];
            if (e < kEffectSummonTotemSlot1 || e > kEffectSummonTotemSlot4)
                continue;
            const int s = e - kEffectSummonTotemSlot1;
            if (g_toolItem[s] == 0)
                g_toolItem[s] = TotemToolItem(rec);
            break;
        }
    }
}

// Re-resolve tool items whenever the player's spells (re)load.
// SPELLS_CHANGED is a no-arg event through the same dispatcher
// Event::SignalHook owns, so we observe it there (never suppress — return
// false). By the time it fires the spellbook is populated, so the scan
// sees the totems.
bool OnSignal(int eventID) {
    if (g_spellsChangedId < 0)
        g_spellsChangedId = Event::Custom::LookupByName("SPELLS_CHANGED");
    if (eventID == g_spellsChangedId)
        ScanTools();
    return false;
}

const Event::SignalHook::AutoSubscribe _spellsChanged{&OnSignal};

uint32_t SpellDurationMs(const uint8_t *rec) {
    using GetDuration_t = int(__fastcall *)(const uint8_t *rec, int unit,
                                            char skipMod);
    const int ms = reinterpret_cast<GetDuration_t>(
        static_cast<uintptr_t>(Offsets::FUN_GET_SPELL_DURATION))(rec, 0, 0);
    return ms > 0 ? static_cast<uint32_t>(ms) : 0;
}

// ---- object-manager existence scan (player-owned creature by entry) ------

using EnumCallback_t = int(__fastcall *)(void *ctx, void *unusedEdx,
                                         uint64_t guid);
using EnumVisibleObjects_t = int(__fastcall *)(EnumCallback_t cb, void *ctx);
using ObjectPtr_t = void *(__fastcall *)(uint32_t typeMask, const char *dbg,
                                         uint32_t guidLo, uint32_t guidHi,
                                         int dbgCode);

struct ScanCtx {
    uint64_t playerGuid;
    uint32_t entries[kSlots]; // 0 = slot not being checked
    bool present[kSlots];
};

int __fastcall ScanCallback(ScanCtx *ctx, void * /*unusedEdx*/, uint64_t guid) {
    if (!Guid::IsCreature(guid))
        return 1;
    const uint32_t entry = static_cast<uint32_t>((guid >> 24) & 0xFFFFFFu);
    if (entry == 0)
        return 1;
    int match = -1;
    for (int s = 0; s < kSlots; ++s) {
        if (ctx->entries[s] == entry) {
            match = s;
            break;
        }
    }
    if (match < 0)
        return 1;

    // Confirm it's OUR totem: resolve + check CreatedBy == player.
    auto ObjectPtr =
        reinterpret_cast<ObjectPtr_t>(Offsets::FUN_CLNT_OBJ_MGR_OBJECT_PTR);
    void *obj = ObjectPtr(Offsets::TYPEMASK_UNIT, nullptr,
                          static_cast<uint32_t>(guid),
                          static_cast<uint32_t>(guid >> 32), 0);
    if (obj == nullptr)
        return 1;
    auto *desc = *reinterpret_cast<const uint8_t *const *>(
        static_cast<const uint8_t *>(obj) + Offsets::OFF_UNIT_DESCRIPTOR);
    if (desc == nullptr)
        return 1;
    const uint64_t createdBy = *reinterpret_cast<const uint64_t *>(
        desc + Offsets::OFF_UNIT_FIELD_CREATEDBY);
    if (createdBy == ctx->playerGuid)
        ctx->present[match] = true;
    return 1; // keep scanning — other slots may match other creatures
}

void OnTick() {
    const uint32_t now = NowMs();

    // Duration backstop — always cheap, runs every tick.
    bool anyActive = false;
    for (int s = 0; s < kSlots; ++s) {
        Slot &t = g_slots[s];
        if (!t.active)
            continue;
        if (t.durationMs != 0 && now - t.startMs >= t.durationMs) {
            t.active = false;
            continue;
        }
        anyActive = true;
    }
    if (!anyActive)
        return;

    // Death / removal scan — throttled, and only while a slot is up.
    if (now - g_lastScanMs < kScanIntervalMs)
        return;
    g_lastScanMs = now;

    // The enumerator derefs the object manager unconditionally.
    if (*reinterpret_cast<void *volatile *>(Offsets::VAR_LOCAL_PLAYER_PTR) ==
        nullptr)
        return;
    const uint64_t player = Unit::Identity::PlayerGuid();
    if (player == 0)
        return;

    ScanCtx ctx{};
    ctx.playerGuid = player;
    for (int s = 0; s < kSlots; ++s)
        ctx.entries[s] = g_slots[s].active ? g_slots[s].entry : 0;

    auto Enum = reinterpret_cast<EnumVisibleObjects_t>(
        Offsets::FUN_CLNT_OBJ_MGR_ENUM_VISIBLE_OBJECTS);
    Enum(reinterpret_cast<EnumCallback_t>(&ScanCallback), &ctx);

    for (int s = 0; s < kSlots; ++s) {
        Slot &t = g_slots[s];
        if (!t.active)
            continue;
        if (ctx.present[s])
            t.seen = true;
        else if (t.seen)
            t.active = false; // was up, now gone → died / recalled early
    }
}

const Tick::WorldTick::AutoSubscribe _tick{&OnTick};

// SpellIcon.dbc path for an icon ID (records indexed by ID; path at +4).
const char *IconPath(int iconID) {
    if (iconID <= 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_ICON_COUNT));
    if (iconID > count)
        return nullptr;
    auto *const *records = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_ICON_RECORDS));
    if (records == nullptr || records[iconID] == nullptr)
        return nullptr;
    return *reinterpret_cast<const char *const *>(records[iconID] + 4);
}

// Pushes the empty tail (no active totem) after `haveTotem` has been
// pushed by the caller. Returns the total count including that first value.
int PushEmptyTail(void *L) {
    Game::Lua::PushString(L, "");    // totemName
    Game::Lua::PushNumber(L, 0.0);   // startTime
    Game::Lua::PushNumber(L, 0.0);   // duration
    Game::Lua::PushNil(L);           // icon
    Game::Lua::PushNumber(L, 1.0);   // modRate
    Game::Lua::PushNumber(L, 0.0);   // spellID
    return 7;
}

// `GetTotemInfo(slot)` → `haveTotem, totemName, startTime, duration, icon,
// modRate, spellID`. Slots: 1 Fire, 2 Earth, 3 Water, 4 Air. `haveTotem`
// is TOOL presence (the documented meaning) — whether the player carries
// the slot's totem tool item (Earth/Fire/Water/Air Totem) — NOT whether a
// totem is summoned; the active totem is signaled by a non-empty
// `totemName` / non-zero `startTime`.
int __fastcall Script_GetTotemInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetTotemInfo(slot)");
        return 0;
    }
    const int slot = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (slot < 1 || slot > kSlots) {
        Game::Lua::PushBool(L, false);
        return PushEmptyTail(L);
    }

    // Tool presence — independent of the summoned state. The tool item
    // (resolved from the totem spell's Totem[0] on SPELLS_CHANGED, not
    // hardcoded) is checked against the bag. The bag walk reads the Lua
    // stack, so it runs before we push any return values (`slot` captured).
    const uint32_t tool = g_toolItem[slot - 1];
    const bool haveTotem =
        tool != 0 && Item::Count::InInventory(L, static_cast<int>(tool)) > 0;

    const Slot &t = g_slots[slot - 1];
    if (!t.active) {
        Game::Lua::PushBool(L, haveTotem);
        return PushEmptyTail(L);
    }

    const uint8_t *rec = Spell::Lookup::RecordForID(static_cast<int>(t.spellID));
    const char *name = "";
    const char *icon = nullptr;
    if (rec != nullptr) {
        const int locale =
            *reinterpret_cast<const int *>(static_cast<uintptr_t>(
                Offsets::VAR_LOCALE_INDEX));
        const char *n = *reinterpret_cast<const char *const *>(
            rec + OFF_NAME + locale * 4);
        if (n != nullptr)
            name = n;
        icon = IconPath(*reinterpret_cast<const int *>(rec + OFF_ICON_ID));
    }

    Game::Lua::PushBool(L, haveTotem);
    Game::Lua::PushString(L, name);
    Game::Lua::PushNumber(L, static_cast<double>(t.startMs) * 0.001);
    Game::Lua::PushNumber(L, static_cast<double>(t.durationMs) / 1000.0);
    if (icon != nullptr)
        Game::Lua::PushString(L, icon);
    else
        Game::Lua::PushNil(L);
    Game::Lua::PushNumber(L, 1.0); // modRate — no per-totem haste in 1.12
    Game::Lua::PushNumber(L, static_cast<double>(t.spellID));
    return 7;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetTotemInfo", &Script_GetTotemInfo);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

void OnPlayerSpellGo(uint32_t spellID) {
    if (spellID == 0)
        return;
    const uint8_t *rec = Spell::Lookup::RecordForID(static_cast<int>(spellID));
    if (rec == nullptr)
        return;

    const auto *effects = reinterpret_cast<const int32_t *>(rec + OFF_EFFECT);
    const auto *misc = reinterpret_cast<const int32_t *>(rec + OFF_MISC_VALUE);
    for (int i = 0; i < Offsets::SPELL_RECORD_EFFECT_COUNT; ++i) {
        const int eff = effects[i];
        if (eff < kEffectSummonTotemSlot1 || eff > kEffectSummonTotemSlot4)
            continue;
        const int idx = eff - kEffectSummonTotemSlot1; // 0..3 (Fire..Air)
        Slot &t = g_slots[idx];
        t.active = true;
        t.seen = false;
        t.spellID = spellID;
        t.entry = static_cast<uint32_t>(misc[i]);
        t.startMs = NowMs();
        t.durationMs = SpellDurationMs(rec);
        // The cast spell IS a totem of this element, so it names the slot's
        // tool item directly — capture it (covers the slot even before the
        // spellbook scan runs).
        if (g_toolItem[idx] == 0)
            g_toolItem[idx] = TotemToolItem(rec);
        return; // one totem-slot effect per spell
    }
}

} // namespace Totem::Tracker
