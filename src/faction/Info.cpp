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
// the ID is out of range or the slot is empty. Records are 1-based
// (records[0] is unused).
const uint8_t *FactionRecord(int factionID) {
    if (factionID <= 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_FACTION_DBC_COUNT));
    if (factionID > count)
        return nullptr;
    auto *records = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_FACTION_DBC_RECORDS));
    if (records == nullptr)
        return nullptr;
    return records[factionID];
}

// Reads `record[offset + locale*4]` as a localized C string pointer.
const char *LocalizedString(const uint8_t *record, int offset) {
    const int locale = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
    auto *strings = reinterpret_cast<const char *const *>(record + offset);
    return strings[locale];
}

// Pushes a 1.12-style Lua "boolean": number 1.0 for true, nil for false.
// Mirrors what Script_GetFactionInfo does for atWar/canToggleAtWar/etc.
// (engine pushes lua_pushnumber(1.0) or lua_pushnil at 0x004D65F2 /
// 0x004D6600 etc.) — we match so encountered and unencountered factions
// have identical return shapes.
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

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetFactionIDByIndex", &Script_GetFactionIDByIndex);
    Game::Lua::RegisterGlobalFunction("GetFactionInfoByID", &Script_GetFactionInfoByID);
    Game::Lua::RegisterGlobalFunction("GetFactionParentID", &Script_GetFactionParentID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Faction::Info
