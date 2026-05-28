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

// `C_Item.IsItemOpenable` / `C_Item.IsItemOpenableByID` — backport of
// the modern WoW accessor that says "right-clicking this item triggers
// an open/loot interaction" (clams, sacks, simple chests, quest
// boxes). The flag is intrinsic to the item type, read from the
// client-side ItemSparse cache.
//
// Vanilla 1.12's tooltip builder gates the `<Right Click to Open>`
// string (`ITEM_OPENABLE`) on `ItemStats_C.Flags & 0x4` (see
// `Offsets::ITEMSTATS_FLAG_OPENABLE`). We surface the same bit.

#include "Game.h"
#include "Offsets.h"
#include "item/Arg.h"
#include "item/Data.h"
#include "item/ID.h"
#include "item/Location.h"
#include "item/Openable.h"

#include <cstdint>

namespace Item::Openable {

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

} // namespace

bool IsItemIDOpenable(uint32_t itemID) {
    if (itemID == 0)
        return false;
    const uint8_t *record = PeekItemRecord(itemID);
    if (record == nullptr) {
        // Background fill so a follow-up call (after the cache warms)
        // returns the real value. Same pattern Quality / Info use.
        Item::Data::WarmCache(itemID);
        return false;
    }
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_FLAGS);
    return (flags & Offsets::ITEMSTATS_FLAG_OPENABLE) != 0;
}

namespace {

int __fastcall Script_C_Item_IsItemOpenable(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.IsItemOpenable(itemLocation)");
        return 0;
    }
    const int itemID = Item::ID::FromCGItem(Item::Location::Resolve(L, 1));
    Game::Lua::PushBool(L, IsItemIDOpenable(static_cast<uint32_t>(itemID)));
    return 1;
}

int __fastcall Script_C_Item_IsItemOpenableByID(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    Game::Lua::PushBool(L, IsItemIDOpenable(static_cast<uint32_t>(itemID)));
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "IsItemOpenable",
                                     &Script_C_Item_IsItemOpenable);
    Game::Lua::RegisterTableFunction("C_Item", "IsItemOpenableByID",
                                     &Script_C_Item_IsItemOpenableByID);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Item::Openable
