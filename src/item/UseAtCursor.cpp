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

// `C_Item.UseAtCursor(itemInfo)` — use the named item at the player's
// current cursor world position. For items whose on-use spell is
// ground-target (Iron Grenade, demolition charges, etc.), bypasses
// the manual click on the AoE reticle that the engine would
// otherwise require.
//
// Mirrors `C_Item.UseItemByName`'s bag-walk + `FUN_ITEM_USE` dispatch
// — the engine's item-use path eventually hands the on-use spell to
// the same cast dispatcher that ground-target spells go through, so
// placement-mode activation works identically. After firing the
// item, `Spell::AtCursor::Resolve()` commits the placement at cursor.
//
// Returns `true` when the cursor-placement leg landed; `false` for
// items that aren't ground-target (the item still fires, just with
// no implicit target), unparseable input, item-not-in-bags, etc.

#include "Game.h"
#include "Offsets.h"
#include "item/Arg.h"
#include "item/Location.h"
#include "spell/AtCursor.h"

#include <cstdint>

namespace Item::UseAtCursor {

namespace {

using UseItem_t = unsigned(__thiscall *)(const void *item, const uint64_t *targetGuid,
                                          int flag);

int __fastcall Script_C_Item_UseAtCursor(void *L) {
    const auto arg = Item::Arg::Resolve(L, 1);
    if (arg.itemID <= 0 && arg.name == nullptr) {
        Game::Lua::PushBool(L, false);
        return 1;
    }

    Item::Location::ByGUIDResult found;
    if (!Item::Location::FindByArgInBags(L, arg, &found)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }

    const uint64_t zeroTarget = 0;
    auto useItem = reinterpret_cast<UseItem_t>(Offsets::FUN_ITEM_USE);
    useItem(found.item, &zeroTarget, 0);

    const bool placed = Spell::AtCursor::Resolve();
    Game::Lua::PushBool(L, placed);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "UseAtCursor",
                                     &Script_C_Item_UseAtCursor);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Item::UseAtCursor
