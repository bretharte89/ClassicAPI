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

namespace Item::TradeSkillReagentID {

namespace {

// Same lookup as `TradeSkillID.cpp`; duplicated rather than promoted
// to a shared header because it's a single tiny chain and shared
// state would just trade a header dance for a function-call hop.
const uint8_t *ResolveSpellRecord(int tradeSkillIndex0) {
    const int count = static_cast<int>(*reinterpret_cast<const uint32_t *>(
        Offsets::VAR_TRADESKILL_COUNT));
    if (tradeSkillIndex0 < 0 || tradeSkillIndex0 >= count)
        return nullptr;

    auto **entries = *reinterpret_cast<const uint8_t ***>(Offsets::VAR_TRADESKILL_ENTRIES);
    if (entries == nullptr)
        return nullptr;
    auto *entry = entries[tradeSkillIndex0];
    if (entry == nullptr)
        return nullptr;

    const int spellID = static_cast<int>(*reinterpret_cast<const uint32_t *>(entry));
    if (spellID <= 0)
        return nullptr;

    const int spellCount = static_cast<int>(*reinterpret_cast<const uint32_t *>(
        Offsets::VAR_SPELL_RECORD_COUNT));
    if (spellID > spellCount)
        return nullptr;

    auto *records = *reinterpret_cast<const uint8_t *const *const *>(Offsets::VAR_SPELL_RECORDS);
    return records[spellID];
}

// Walks the recipe's reagent itemID array `record + 0xA8`, packed
// densely at the start. Returns the itemID of the Nth (1-based)
// non-zero reagent, or 0 if we hit a 0 before reaching N (the
// engine's `Script_GetTradeSkillReagentItemLink` bails the same way
// — there's no "skip empty slots" behavior).
int NthReagentItemID(const uint8_t *spellRecord, int reagentIndex1) {
    if (reagentIndex1 < 1)
        return 0;
    auto *reagents = reinterpret_cast<const uint32_t *>(
        spellRecord + Offsets::OFF_SPELL_RECORD_REAGENTS);
    int found = 0;
    for (int i = 0; i < Offsets::SPELL_RECIPE_MAX_REAGENTS; ++i) {
        const int itemID = static_cast<int>(reagents[i]);
        if (itemID == 0)
            return 0;
        if (++found == reagentIndex1)
            return itemID;
    }
    return 0;
}

} // namespace

// `GetTradeSkillReagentItemID(index, reagentIndex)` — itemID
// companion to `GetTradeSkillReagentItemLink`. nil for OOR indices
// or when there's no Nth reagent.
static int __fastcall Script_GetTradeSkillReagentItemID(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: GetTradeSkillReagentItemID(index, reagentIndex)");
        return 0;
    }
    const int index = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
    const int reagentIndex = static_cast<int>(Game::Lua::ToNumber(L, 2));

    auto *record = ResolveSpellRecord(index);
    if (record == nullptr)
        return 0;

    const int itemID = NthReagentItemID(record, reagentIndex);
    if (itemID == 0)
        return 0;

    Game::Lua::PushNumber(L, static_cast<double>(itemID));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetTradeSkillReagentItemID",
                                      &Script_GetTradeSkillReagentItemID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::TradeSkillReagentID
