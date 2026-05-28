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

// `C_Container.IsContainerItemOpenable(bag, slot)` — positional-arg
// wrapper around `Item::Openable::IsItemIDOpenable`. Same chain
// `C_Container.GetContainerItemDurability` and friends use:
// `ResolveBag` → `GetItemBySlot` → instance-block itemID → cache flag
// read.

#include "Game.h"
#include "item/ID.h"
#include "item/Location.h"
#include "item/Openable.h"

namespace Container::Openable {

namespace {

int __fastcall Script_C_Container_IsContainerItemOpenable(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L,
            "Usage: C_Container.IsContainerItemOpenable(containerIndex, slotIndex)");
        return 0;
    }
    const int bagID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const int slotIndex = static_cast<int>(Game::Lua::ToNumber(L, 2));
    const int itemID = Item::ID::FromCGItem(
        Item::Location::ResolveBag(L, bagID, slotIndex));
    Game::Lua::PushBool(L, Item::Openable::IsItemIDOpenable(
        static_cast<uint32_t>(itemID)));
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Container", "IsContainerItemOpenable",
                                     &Script_C_Container_IsContainerItemOpenable);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Container::Openable
