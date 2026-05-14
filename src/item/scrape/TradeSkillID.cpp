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

namespace Item::TradeSkillID {

namespace {

// Resolves a tradeskill index to its spell record in Spell.dbc.
// Returns nullptr when index is OOR, the entry pointer is null, or
// the entry's spellID is out of range / unmapped. Shared with the
// reagent companion.
const uint8_t *ResolveSpellRecord(int tradeSkillIndex0) {
    const int count = static_cast<int>(*reinterpret_cast<const uint32_t *>(
        Offsets::VAR_TRADESKILL_COUNT));
    if (tradeSkillIndex0 < 0 || tradeSkillIndex0 >= count)
        return nullptr;

    // `VAR_TRADESKILL_ENTRIES` is a pointer slot (`MOV ECX, [imm32]`
    // in the engine), not the array — the entry-pointer array is
    // heap-allocated and re-pointed each time the tradeskill window
    // opens.
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

} // namespace

// `GetTradeSkillItemID(index)` — itemID companion to
// `GetTradeSkillItemLink`. Returns the recipe's produced-item itemID
// (the `CreatedItem` field on the spell record). nil when the
// tradeskill UI is closed, the index is OOR, or the recipe doesn't
// produce a finished item (e.g. enchanting scrolls have a 0 here in
// vanilla — their effect targets equipment directly).
static int __fastcall Script_GetTradeSkillItemID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetTradeSkillItemID(index)");
        return 0;
    }
    const int index = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;

    auto *record = ResolveSpellRecord(index);
    if (record == nullptr)
        return 0;

    const int itemID = static_cast<int>(*reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_SPELL_RECORD_CREATED_ITEM));
    if (itemID == 0)
        return 0;

    Game::Lua::PushNumber(L, static_cast<double>(itemID));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetTradeSkillItemID", &Script_GetTradeSkillItemID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::TradeSkillID
