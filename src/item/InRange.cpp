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

// `C_Item.IsItemInRange(item, targetUnit)` — true if the item is in
// range of the unit, false if out of range, nil when the range check
// doesn't apply. `item` is an itemID, `"item:NNN"` string, or item link
// (names aren't resolvable — vanilla has no name→ID map — and yield nil);
// `targetUnit` is a unit token ("target", "mouseover", "party1", …).
//
// An item's range comes from the spell it fires on use: we resolve the
// item to its on-use spell (`Item::Spell::OnUseSpellIDForItemID`) and run
// the same range test as `C_Spell.IsSpellInRange`, via the shared
// `Spell::Range::PlayerVsUnit` core. So items with a targeted on-use
// effect that has a range (nets, bombs, thrown/targeted quest items, …)
// get a true/false answer; items with no on-use spell, or whose on-use
// spell is rangeless (potions, food, self-buff trinkets, hearthstone),
// return nil — matching retail, which also reports nil for items without
// a range restriction.
//
// Range-only, like `IsSpellInRange`: ignores line of sight and target
// faction. Passive reader — an item not yet in the client item cache
// returns nil (no background fetch); items you'd range-check are normally
// in bags / on the action bar and already cached.

#include "Game.h"
#include "item/Arg.h"
#include "item/Spell.h"
#include "spell/Range.h"

#include <cstdint>

namespace Item::InRange {

namespace {

int __fastcall Script_C_Item_IsItemInRange(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    const int spellID =
        itemID > 0 ? Item::Spell::OnUseSpellIDForItemID(
                         static_cast<uint32_t>(itemID))
                   : 0;
    const char *unit =
        Game::Lua::IsString(L, 2) ? Game::Lua::ToString(L, 2) : nullptr;

    // `::Spell` — unqualified `Spell` would bind to `Item::Spell` here.
    const int result = ::Spell::Range::PlayerVsUnit(spellID, unit);
    if (result < 0) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::PushBool(L, result == 1);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "IsItemInRange",
                                     &Script_C_Item_IsItemInRange);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Item::InRange
