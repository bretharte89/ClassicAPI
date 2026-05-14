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
#include "dbc/Lookup.h"

#include <cstdint>

namespace Faction::Info {

namespace {

// __fastcall(ecx = 0-based displayed index) → factionID, or 0 for OOB.
// MSVC's free-function __fastcall puts the first int arg in ECX, matching
// the engine's calling convention for this helper.
using ResolveIndex_t = int(__fastcall *)(int idx);

ResolveIndex_t Resolver() {
    return reinterpret_cast<ResolveIndex_t>(Offsets::FUN_RESOLVE_FACTION_INDEX);
}

// Walks the displayed-faction list looking for an entry whose factionID
// matches `factionID`. Returns the 0-based displayed index, or -1 if not
// found. The displayed list is what GetFactionInfo iterates — factions
// the player has rep with, in display order — so this won't match
// factions the player has never discovered.
//
// We walk up to `[VAR_FACTION_VISIBLE_MAX_INDEX]` (inclusive) to mirror
// the resolver's own bounds-check, which is the same range
// `Script_GetFactionInfo` accepts.
int FindDisplayedIndex(int factionID) {
    auto resolver = Resolver();
    const int maxIdx = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_FACTION_VISIBLE_MAX_INDEX));
    for (int i = 0; i <= maxIdx; i++) {
        if (resolver(i) == factionID)
            return i;
    }
    return -1;
}

// Returns the Faction.dbc record pointer for `factionID`, or nullptr if
// the ID is out of range or the slot is empty.
const uint8_t *FactionRecord(int factionID) {
    if (factionID <= 0)
        return nullptr;
    return DBC::Record(Offsets::VAR_FACTION_DBC_RECORDS,
                       Offsets::VAR_FACTION_DBC_COUNT,
                       static_cast<uint32_t>(factionID));
}

// Reads `record[offset + locale*4]` as a localized C string pointer.
const char *LocalizedString(const uint8_t *record, int offset) {
    const int locale = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
    auto *strings = reinterpret_cast<const char *const *>(record + offset);
    return strings[locale];
}

// Pushes 1.0 for true and nil for false — the convention
// Script_GetFactionInfo uses for atWar/canToggleAtWar/etc. (engine
// pushes lua_pushnumber(1.0) or lua_pushnil at 0x004D65F2 /
// 0x004D6600 etc.). The engine has lua_pushboolean (we use it via
// Game::Lua::PushBoolean for C_* namespace functions); we match
// the number-or-nil shape here so encountered and unencountered
// factions return identically and addons can use a single
// comparison path.
void PushFlag(void *L, bool value) {
    if (value)
        Game::Lua::PushNumber(L, 1.0);
    else
        Game::Lua::PushNil(L);
}

} // namespace

static int __fastcall Script_GetFactionIDByIndex(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetFactionIDByIndex(factionIndex)");
        return 0;
    }
    const int idx = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
    if (idx < 0)
        return 0; // nil for non-positive indices

    // Match Script_GetFactionInfo's acceptance: we bound against
    // `[VAR_FACTION_VISIBLE_MAX_INDEX]` (the same value the resolver
    // checks internally), which is wider than `GetNumFactions()` because
    // it also covers the "Inactive" / collapsed-category rows.
    const int maxIdx = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_FACTION_VISIBLE_MAX_INDEX));
    if (idx > maxIdx)
        return 0; // nil for out-of-range

    // Resolver returns the factionID for real entries; for header /
    // category rows it returns 0 ("Other") or -1 ("Inactive"-style
    // pseudo-row). Modern WoW (Classic Era 1.15.x) normalizes both to 0
    // in the `factionID` slot of `GetFactionInfo`, so we do the same.
    // Final convention: 0 for any header, nil for OOB, real factionID
    // for real factions. Matches GetQuestIDFromLogIndex's headers-are-0
    // convention and modern WoW's GetFactionInfo[14] semantics.
    const int factionID = Resolver()(idx);
    Game::Lua::PushNumber(L, static_cast<double>(factionID > 0 ? factionID : 0));
    return 1;
}

static int __fastcall Script_GetFactionInfoByID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetFactionInfoByID(factionID)");
        return 0;
    }
    const int factionID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (factionID <= 0)
        return 0;

    // Fast path: if the faction is in the player's displayed reputation
    // list, the engine's Script_GetFactionInfo has all the rep state we
    // need (current standing, bar value, atWar flags, etc). Replace our
    // argument with the displayed-list index (1-based) and tail-call it —
    // it reads only stack[1] and pushes 11 returns; we forward the count
    // back to the Lua caller.
    const int displayedIdx = FindDisplayedIndex(factionID);
    if (displayedIdx >= 0) {
        Game::Lua::SetTop(L, 0);
        Game::Lua::PushNumber(L, static_cast<double>(displayedIdx + 1));
        using GetFactionInfo_t = int(__fastcall *)(void *L);
        auto fn = reinterpret_cast<GetFactionInfo_t>(Offsets::FUN_SCRIPT_GET_FACTION_INFO);
        return fn(L);
    }

    // Slow path: faction not in the displayed list (unencountered, or
    // simply doesn't store rep state — e.g. Steamwheedle Cartel in some
    // states). Read name + description from Faction.dbc directly and
    // synthesize Neutral defaults for the rep fields. Matches what
    // 3.3.5's GetFactionInfoByID returns for unencountered factions
    // (verified against retail dump: standingID=4, barMin=0, barMax=3000).
    const uint8_t *record = FactionRecord(factionID);
    if (record == nullptr)
        return 0;
    const char *name = LocalizedString(record, Offsets::OFF_FACTION_NAMES);
    if (name == nullptr || *name == '\0')
        return 0;
    const char *description = LocalizedString(record, Offsets::OFF_FACTION_DESCRIPTIONS);

    Game::Lua::SetTop(L, 0);
    Game::Lua::PushString(L, name);                            // 1: name
    Game::Lua::PushString(L, description ? description : "");  // 2: description
    Game::Lua::PushNumber(L, 4.0);                             // 3: standingID = Neutral
    Game::Lua::PushNumber(L, 0.0);                             // 4: barMin
    Game::Lua::PushNumber(L, 3000.0);                          // 5: barMax
    Game::Lua::PushNumber(L, 0.0);                             // 6: barValue
    PushFlag(L, false);                                         // 7: atWarWith
    PushFlag(L, false);                                         // 8: canToggleAtWar
    PushFlag(L, false);                                         // 9: isHeader
    PushFlag(L, false);                                         // 10: isCollapsed
    PushFlag(L, false);                                         // 11: hasRep
    return 11;
}

// `GetFactionParentID(factionID)` — returns the parent factionID for a
// faction in a hierarchy (e.g. Stormwind's parent is Alliance Forces;
// The Defilers's parent is Horde Forces). Returns `0` if the faction
// is top-level (no parent), or `nil` if the factionID is invalid.
//
// Reads `Faction.dbc` `ParentFactionID` at `+0x48`. Modern WoW returns
// this as the 13th value of `GetFactionInfoByID`; we expose it as its
// own getter since 1.12's `GetFactionInfo` doesn't have the slot.
static int __fastcall Script_GetFactionParentID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetFactionParentID(factionID)");
        return 0;
    }
    const int factionID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const uint8_t *record = FactionRecord(factionID);
    if (record == nullptr)
        return 0;
    const int parent = *reinterpret_cast<const int *>(
        record + Offsets::OFF_FACTION_PARENT_ID);
    Game::Lua::PushNumber(L, static_cast<double>(parent));
    return 1;
}

// Pushes a `key = value` pair into the table at stack[-3]. Matches
// the helper shape used by `charlist/Cache.cpp`'s table builders.
void SetField(void *L, const char *key, double value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushNumber(L, value);
    Game::Lua::SetTable(L, -3);
}

void SetField(void *L, const char *key, const char *value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushString(L, value != nullptr ? value : "");
    Game::Lua::SetTable(L, -3);
}

void SetField(void *L, const char *key, bool value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushBoolean(L, value ? 1 : 0);
    Game::Lua::SetTable(L, -3);
}

// Resolves the local player's CGPlayer-side info sub-struct at
// `[player + 0xE68]`. Returns nullptr if "player" isn't resolvable
// or the sub-struct is uninitialized (pre-login / glue).
const uint8_t *PlayerInfo() {
    using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
    auto resolve = reinterpret_cast<ResolveUnitToken_t>(
        Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<const uint8_t *>(resolve("player"));
    if (player == nullptr)
        return nullptr;
    return *reinterpret_cast<const uint8_t *const *>(
        player + Offsets::OFF_CGPLAYER_INFO);
}

// Returns the player's RepListID for the currently-watched faction,
// or -1 if no faction is watched. RepListID is the rep-slot index in
// `VAR_PLAYER_REP_SLOTS`, NOT a factionID — the resolver below
// translates it.
int WatchedRepListID(const uint8_t *playerInfo) {
    return *reinterpret_cast<const int *>(
        playerInfo + Offsets::OFF_CGPLAYER_INFO_WATCHED_REP_LIST_ID);
}

// Returns the rep-slot pointer for `repListID`, or nullptr if out of
// range. Slot layout is documented at `VAR_PLAYER_REP_SLOTS` in
// `Offsets.h`.
const uint8_t *RepSlot(int repListID) {
    if (repListID < 0 || repListID >= Offsets::MAX_REP_SLOTS)
        return nullptr;
    return reinterpret_cast<const uint8_t *>(
        static_cast<uintptr_t>(Offsets::VAR_PLAYER_REP_SLOTS) +
        static_cast<uintptr_t>(repListID) * Offsets::REP_SLOT_STRIDE);
}

// `C_Reputation.GetFactionStandings()` — returns a flat
// `{ [factionID] = currentStanding }` table covering every faction
// in the player's reputation list (i.e. every populated rep slot).
//
// `currentStanding` is `base + delta` from the rep slot — same value
// `FUN_REPUTATION_GET_STANDING` returns, same as the `barValue` /
// `currentStanding` field in `GetFactionInfo` / `GetWatchedFactionData`.
//
// Unlike a displayed-list walk (`GetNumFactions` + `GetFactionInfo`),
// this skips the header rows entirely and doesn't depend on the
// player having opened the reputation pane recently — it reads
// straight out of the per-faction rep-slot array, which the engine
// keeps populated for every faction the player has rep with.
//
// Always returns a table (possibly empty); never nil.
static int __fastcall Script_C_Reputation_GetFactionStandings(void *L) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);

    for (int i = 0; i < Offsets::MAX_REP_SLOTS; i++) {
        const uint8_t *slot = RepSlot(i);
        if (slot == nullptr)
            continue;
        const int factionID = *reinterpret_cast<const int *>(
            slot + Offsets::OFF_REP_SLOT_FACTION_ID);
        if (factionID <= 0)
            continue;
        const int base = *reinterpret_cast<const int *>(
            slot + Offsets::OFF_REP_SLOT_BASE_STANDING);
        const int delta = *reinterpret_cast<const int *>(
            slot + Offsets::OFF_REP_SLOT_DELTA_STANDING);
        Game::Lua::PushNumber(L, static_cast<double>(factionID));
        Game::Lua::PushNumber(L, static_cast<double>(base + delta));
        Game::Lua::SetTable(L, -3);
    }
    return 1;
}

// `C_Reputation.GetWatchedFactionData()` — returns the modern-style
// table describing the faction shown above the XP bar (or nil when no
// faction is watched). Vanilla's `GetWatchedFactionInfo` returns the
// same data as a 5-tuple without the factionID, which is the field
// modern callers rely on most.
//
// Fields populated (a subset of the modern schema — only what 1.12's
// engine actually has):
//   factionID                 number  — Faction.dbc record ID
//   name                      string  — locale-applied
//   description               string  — locale-applied (may be "")
//   reaction                  number  — 1=Hated .. 8=Exalted
//   currentReactionThreshold  number  — band min (rep value)
//   nextReactionThreshold     number  — band max (rep value)
//   currentStanding           number  — base + delta
//   atWarWith                 boolean — rep slot flags bit 1
//   canToggleAtWar            boolean — rep slot flags bit 4
//   isWatched                 boolean — always true (it's THE watched)
//   isHeader                  boolean — always false (factions only)
//
// Fields the modern API also exposes but 1.12 has no source for
// (isChild, isHeaderWithRep, isCollapsed, hasBonusRepGain,
// canSetInactive, isAccountWide) are omitted rather than synthesized
// — addons can check `data.isAccountWide ~= nil` if they ever need
// to feature-detect.
static int __fastcall Script_C_Reputation_GetWatchedFactionData(void *L) {
    const uint8_t *playerInfo = PlayerInfo();
    if (playerInfo == nullptr)
        return 0;
    const int repListID = WatchedRepListID(playerInfo);
    const uint8_t *slot = RepSlot(repListID);
    if (slot == nullptr)
        return 0;
    const int factionID = *reinterpret_cast<const int *>(
        slot + Offsets::OFF_REP_SLOT_FACTION_ID);
    if (factionID <= 0)
        return 0;
    const uint8_t *record = FactionRecord(factionID);
    if (record == nullptr)
        return 0;

    // Reaction band — engine returns 0..7, Lua-side convention is 1..8.
    using ReactionBand_t = int(__fastcall *)(int factionID);
    auto reactionFn = reinterpret_cast<ReactionBand_t>(
        Offsets::FUN_REPUTATION_GET_REACTION_BAND);
    const int band = reactionFn(factionID);
    const int reaction = band + 1;

    using GetStanding_t = int(__fastcall *)(int factionID);
    auto standingFn = reinterpret_cast<GetStanding_t>(
        Offsets::FUN_REPUTATION_GET_STANDING);
    const int currentStanding = standingFn(factionID);

    const int barMin = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_REACTION_MIN_TABLE) +
        static_cast<uintptr_t>(band) * 4);
    const int barMax = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_REACTION_MAX_TABLE) +
        static_cast<uintptr_t>(band) * 4);

    const uint8_t flags = *(slot + Offsets::OFF_REP_SLOT_FLAGS);
    const bool atWar = (flags & Offsets::REP_SLOT_FLAG_AT_WAR) != 0;
    const bool canToggleAtWar =
        (flags & Offsets::REP_SLOT_FLAG_CAN_TOGGLE_AT_WAR) != 0;

    const char *name = LocalizedString(record, Offsets::OFF_FACTION_NAMES);
    const char *description = LocalizedString(record, Offsets::OFF_FACTION_DESCRIPTIONS);

    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);
    SetField(L, "factionID", static_cast<double>(factionID));
    SetField(L, "name", name);
    SetField(L, "description", description);
    SetField(L, "reaction", static_cast<double>(reaction));
    SetField(L, "currentReactionThreshold", static_cast<double>(barMin));
    SetField(L, "nextReactionThreshold", static_cast<double>(barMax));
    SetField(L, "currentStanding", static_cast<double>(currentStanding));
    SetField(L, "atWarWith", atWar);
    SetField(L, "canToggleAtWar", canToggleAtWar);
    SetField(L, "isWatched", true);
    SetField(L, "isHeader", false);
    return 1;
}

// `C_Reputation.SetWatchedFactionByID(factionID)` — sets the faction
// shown above the XP bar by ID rather than by displayed-list index.
// Modern API; vanilla 1.12 only exposes `SetWatchedFactionIndex(idx)`,
// which forces addons to walk the index list themselves.
//
// Calls the engine's inner watched-faction setter at
// `FUN_PLAYER_SET_WATCHED_FACTION` directly, bypassing the
// engine's index-based wrapper (which round-trips through the
// resolver and rejects unencountered factions). Passing factionID
// 0 clears the watched faction.
//
// Negative IDs are silently ignored.
static int __fastcall Script_C_Reputation_SetWatchedFactionByID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L,
            "Usage: C_Reputation.SetWatchedFactionByID(factionID)");
        return 0;
    }
    const int factionID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (factionID < 0)
        return 0;

    using SetWatched_t = void(__fastcall *)(int factionID);
    auto fn = reinterpret_cast<SetWatched_t>(
        Offsets::FUN_PLAYER_SET_WATCHED_FACTION);
    fn(factionID);
    return 0;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetFactionIDByIndex", &Script_GetFactionIDByIndex);
    Game::Lua::RegisterGlobalFunction("GetFactionInfoByID", &Script_GetFactionInfoByID);
    Game::Lua::RegisterGlobalFunction("GetFactionParentID", &Script_GetFactionParentID);
    Game::Lua::RegisterTableFunction("C_Reputation", "SetWatchedFactionByID",
                                     &Script_C_Reputation_SetWatchedFactionByID);
    Game::Lua::RegisterTableFunction("C_Reputation", "GetWatchedFactionData",
                                     &Script_C_Reputation_GetWatchedFactionData);
    Game::Lua::RegisterTableFunction("C_Reputation", "GetFactionStandings",
                                     &Script_C_Reputation_GetFactionStandings);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Faction::Info
