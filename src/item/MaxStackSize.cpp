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
#include "item/Arg.h"
#include "item/Data.h"
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>

namespace Item::MaxStackSize {

namespace {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                      const uint64_t *guid, void *callback,
                                                      void *userData, int unused);

const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

int PushMaxStackSizeForItemID(void *L, int itemID) {
    if (itemID <= 0)
        return 0;
    const uint8_t *record = PeekItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr) {
        Item::Data::WarmCache(static_cast<uint32_t>(itemID));
        return 0;
    }
    const uint32_t maxStack = *reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_STACKABLE);
    Game::Lua::PushNumber(L, static_cast<double>(maxStack));
    return 1;
}

} // namespace

// `C_Item.GetItemMaxStackSize(itemLocation)` — max stack size for
// the item type at the given location. Returns the type's stack
// cap (e.g. 20 for Linen Cloth) regardless of how many are
// actually in the slot; for the current stack count of a specific
// item, use `C_Item.GetStackCount`. Non-stackable items return 1.
// nil for empty / uncached / invalid locations.
static int __fastcall Script_C_Item_GetItemMaxStackSize(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemMaxStackSize(itemLocation)");
        return 0;
    }
    return PushMaxStackSizeForItemID(L, Item::ID::FromCGItem(Item::Location::Resolve(L, 1)));
}

// `C_Item.GetItemMaxStackSizeByID(itemInfo)` — same data, but
// keyed on itemID (or `"item:NNN..."` link string). For uncached
// items, returns nil and fires a background cache fill — re-call
// after `GET_ITEM_INFO_RECEIVED` for the value.
static int __fastcall Script_C_Item_GetItemMaxStackSizeByID(void *L) {
    return PushMaxStackSizeForItemID(L, Item::Arg::ResolveItemID(L, 1));
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemMaxStackSize",
                                     &Script_C_Item_GetItemMaxStackSize);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemMaxStackSizeByID",
                                     &Script_C_Item_GetItemMaxStackSizeByID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::MaxStackSize
