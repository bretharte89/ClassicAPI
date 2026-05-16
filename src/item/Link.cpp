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

#include "Link.h"

#include "Game.h"
#include "Offsets.h"
#include "item/Location.h"

#include <cstdint>

namespace Item::Link {

namespace {
using BuildItemLink_t = const char *(__fastcall *)(const void *cgItem);
} // namespace

const char *FromCGItem(const uint8_t *cgItem) {
    if (cgItem == nullptr)
        return nullptr;
    auto fn = reinterpret_cast<BuildItemLink_t>(Offsets::FUN_GAMETOOLTIP_BUILD_ITEM_LINK);
    return fn(cgItem);
}

namespace {

// `C_Item.GetItemLink(itemLocation)` — returns the fully-decorated
// per-instance hyperlink for the item at the location, matching what
// `GetContainerItemLink(bag, slot)` or `GetInventoryItemLink("player",
// slot)` would return for the same slot. Enchant ID, random-suffix,
// and any other per-instance data the server attached to the CGItem
// are preserved.
//
// Accepts table form (`{equipmentSlotIndex=N}` or `{bagID=B,
// slotIndex=S}`) and GUID string form (the value
// `C_Item.GetItemGUID` returns). `Item::Location::Resolve` handles
// all three input shapes and returns a CGItem pointer for whichever
// matches; we forward that to `FromCGItem`, which calls the engine's
// own link builder. No Lua-stack-stomping, no engine Script dispatch.
int __fastcall Script_C_Item_GetItemLink(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemLink(itemLocation)");
        return 0;
    }

    const uint8_t *cgItem = Item::Location::Resolve(L, 1);
    const char *link = FromCGItem(cgItem);
    if (link == nullptr || *link == '\0')
        return 0;

    Game::Lua::PushString(L, link);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemLink", &Script_C_Item_GetItemLink);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Item::Link
