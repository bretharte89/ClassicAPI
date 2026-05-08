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

namespace Talent::Info {

// `GetTalentSpellID(tabIndex, talentIndex[, rank])` — returns the
// spellID for the given talent at the requested rank, or `nil` if the
// talent isn't allocated (or any arg is out of range).
//
// 1.12's `GetTalentInfo` returns `(name, icon, tier, column, currentRank,
// maxRank)` with no spellID — addons that want to chain into the spell
// APIs (`GetSpellInfo`, `C_Spell.GetSpellDescription`,
// `GameTooltip:SetSpellByID`) currently have to maintain their own
// `(class, tab, idx) → spellID` lookup tables.
//
// `rank` is optional. When omitted, defaults to the player's currentRank
// by delegating to the engine's `Script_GetTalentInfo` and reading its
// 5th return. If currentRank is 0 (the player hasn't allocated any
// points in this talent), falls back to rank 1 so the function still
// produces the talent's canonical spellID for unallocated talents.
//
// Returns `nil` when:
//   - tab/talentIndex/rank are non-numeric or out of range
//   - the explicit `rank` exceeds the talent's allocated max (e.g. rank
//     5 on a 1-rank talent — the SpellRank slot is zero)
static int __fastcall Script_GetTalentSpellID(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: GetTalentSpellID(tabIndex, talentIndex[, rank])");
        return 0;
    }
    const int tabIndex = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const int talentIndex = static_cast<int>(Game::Lua::ToNumber(L, 2));
    if (tabIndex < 1 || talentIndex < 1)
        return 0;

    // Pre-validate tab/idx so we don't trigger `lua_error` from the
    // engine's `Script_GetTalentInfo` on out-of-range input. The engine
    // bails to its error path (lua_error) on bad tab, bad talent index,
    // or NULL player; we want all of those to return nil instead.
    const int tabCount = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_TALENT_TAB_COUNT));
    if (tabIndex > tabCount)
        return 0;

    auto *tabs = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_TALENT_TAB_INFO_ARRAY));
    if (tabs == nullptr)
        return 0;
    const uint8_t *tabInfo = tabs[tabIndex - 1];
    if (tabInfo == nullptr)
        return 0;

    const int numTalents = *reinterpret_cast<const int *>(
        tabInfo + Offsets::OFF_TABINFO_NUM_TALENTS);
    if (talentIndex > numTalents)
        return 0;

    // Resolve rank. Explicit when arg 3 is a number; else default to
    // currentRank via the engine's Script_GetTalentInfo. We pre-validated
    // tab/idx above so the engine call won't error.
    int rank;
    if (Game::Lua::IsNumber(L, 3)) {
        rank = static_cast<int>(Game::Lua::ToNumber(L, 3));
        if (rank < 1 || rank > Offsets::TALENT_MAX_RANKS)
            return 0;
    } else {
        using GetTalentInfo_t = int(__fastcall *)(void *L);
        auto fn = reinterpret_cast<GetTalentInfo_t>(
            Offsets::FUN_SCRIPT_GET_TALENT_INFO);
        const int n = fn(L);
        if (n < 5)
            return 0;
        // 5th return = currentRank. Index from top is `-(n - 4)` so we
        // don't depend on the exact return count — stock 1.12.1 emits 8
        // returns from `GetTalentInfo`; computing dynamically keeps us
        // correct against patched/extended builds too.
        rank = static_cast<int>(Game::Lua::ToNumber(L, -(n - 4)));
        if (rank < 1)
            rank = 1; // unallocated talent → fall back to rank-1 spellID
    }

    auto *talents = *reinterpret_cast<const uint8_t *const *>(
        tabInfo + Offsets::OFF_TABINFO_TALENT_ARRAY);
    if (talents == nullptr)
        return 0;
    const uint8_t *entry = talents +
        (talentIndex - 1) * Offsets::TALENT_ENTRY_STRIDE;
    const uint32_t spellID = *reinterpret_cast<const uint32_t *>(
        entry + Offsets::OFF_TALENT_SPELL_RANK + (rank - 1) * sizeof(uint32_t));
    if (spellID == 0)
        return 0; // rank slot not populated (rank > maxRank for this talent)

    Game::Lua::PushNumber(L, static_cast<double>(spellID));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetTalentSpellID", &Script_GetTalentSpellID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Talent::Info
