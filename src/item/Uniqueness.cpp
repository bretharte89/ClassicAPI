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

// `C_Item.GetItemUniqueness(itemLocation)` — uniqueness cap. Reads
// `ItemStats_C.m_maxCount` from the item's cached record; that's the
// only uniqueness signal vanilla's client carries. `1` = unique
// ("Held In Off-hand" trinkets, special quest items, etc.); `0` =
// unlimited; higher values cap the inventory count.
//
// Modern WoW returns a 3-tuple `(maxEquipped, category, equippedCount)`
// keyed off the post-TBC category system (e.g. "Unique-Equipped:
// Brewfest Mug"). Vanilla has no category system, so we surface
// `m_maxCount` as the first return and leave the category nil and
// equipped count `0` — matching modern's "field present but inapplicable"
// convention.

#include "Game.h"
#include "Offsets.h"
#include "item/Data.h"
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>

namespace Item::Uniqueness {

namespace {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                       const uint64_t *guid,
                                                       void *callback, void *userData,
                                                       int unused);

const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(
        static_cast<uintptr_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD));
    auto *cache = reinterpret_cast<void *>(
        static_cast<uintptr_t>(Offsets::VAR_ITEMDB_CACHE));
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

} // namespace

// Pushes the 3-tuple `(maxAllowed, nil, 0)` derived from the
// ItemStats_C record, or nil + early-return on cache miss.
int PushFromItemID(void *L, uint32_t itemID) {
    const uint8_t *record = PeekItemRecord(itemID);
    if (record == nullptr) {
        Item::Data::WarmCache(itemID);
        Game::Lua::PushNil(L);
        return 1;
    }
    const int maxCount = *reinterpret_cast<const int *>(
        record + Offsets::OFF_ITEMSTATS_MAX_COUNT);
    Game::Lua::PushNumber(L, static_cast<double>(maxCount));
    Game::Lua::PushNil(L);              // category (vanilla has none)
    Game::Lua::PushNumber(L, 0.0);      // equippedCount (not tracked)
    return 3;
}

static int __fastcall Script_GetItemUniqueness(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::PushNil(L);
        return 1;
    }
    auto *item = Item::Location::Resolve(L, 1);
    if (item == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    const int itemID = Item::ID::FromCGItem(item);
    if (itemID <= 0) {
        Game::Lua::PushNil(L);
        return 1;
    }
    return PushFromItemID(L, static_cast<uint32_t>(itemID));
}

static int __fastcall Script_GetItemUniquenessByID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::PushNil(L);
        return 1;
    }
    const int itemID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (itemID <= 0) {
        Game::Lua::PushNil(L);
        return 1;
    }
    return PushFromItemID(L, static_cast<uint32_t>(itemID));
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemUniqueness",
                                     &Script_GetItemUniqueness);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemUniquenessByID",
                                     &Script_GetItemUniquenessByID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Uniqueness
