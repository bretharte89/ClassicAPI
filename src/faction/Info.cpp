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

    const int displayedIdx = FindDisplayedIndex(factionID);
    if (displayedIdx < 0)
        return 0; // not in the player's reputation list

    // Replace our argument with the displayed-list index (1-based) and
    // tail-call the engine's Script_GetFactionInfo. It reads only stack[1]
    // and pushes 11 returns; we forward the count back to the Lua caller.
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(displayedIdx + 1));

    using GetFactionInfo_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<GetFactionInfo_t>(Offsets::FUN_SCRIPT_GET_FACTION_INFO);
    return fn(L);
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetFactionIDByIndex", &Script_GetFactionIDByIndex);
    Game::Lua::RegisterGlobalFunction("GetFactionInfoByID", &Script_GetFactionInfoByID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Faction::Info
