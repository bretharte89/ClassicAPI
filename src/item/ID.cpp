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
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>

namespace Item::ID {

int FromCGItem(const uint8_t *item) {
    if (item == nullptr)
        return 0;
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return static_cast<int>(*reinterpret_cast<const uint32_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_ITEM_ID));
}

static int __fastcall Script_GetItemID(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemID(itemLocation)");
        return 0;
    }

    const int itemID = FromCGItem(Item::Location::Resolve(L, 1));
    if (itemID == 0) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::PushNumber(L, static_cast<double>(itemID));
    return 1;
}

// `C_Container.GetContainerItemID(bagIndex, slotIndex)` — modern
// positional-arg accessor for the same data `C_Item.GetItemID({bagID=B,
// slotIndex=S})` returns. Both go through `Item::Location::ResolveBag`
// → engine `PackBagSlot` → `GetItemBySlot` → CGItem → instance block
// → itemID. Returns nil for empty slots or invalid bag indices.
static int __fastcall Script_C_Container_GetContainerItemID(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: C_Container.GetContainerItemID(bagIndex, slotIndex)");
        return 0;
    }
    const int bagID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const int slotIndex = static_cast<int>(Game::Lua::ToNumber(L, 2));

    const int itemID = FromCGItem(Item::Location::ResolveBag(L, bagID, slotIndex));
    if (itemID == 0)
        return 0;
    Game::Lua::PushNumber(L, static_cast<double>(itemID));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemID", &Script_GetItemID);
    Game::Lua::RegisterTableFunction("C_Container", "GetContainerItemID",
                                     &Script_C_Container_GetContainerItemID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::ID
