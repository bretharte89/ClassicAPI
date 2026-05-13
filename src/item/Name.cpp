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

namespace Item::Name {

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

// Reads `m_name[0]` directly off a cached record. Items only ever
// populate the first name slot — the array shape (`char *m_name[4]`
// per VanillaHelpers's `ItemStats_C`) is a client-internal layout, not
// a locale array like `Spell.dbc`. The active locale's name is what
// the server delivered in `SMSG_ITEM_QUERY_SINGLE_RESPONSE`.
int PushNameForItemID(void *L, int itemID) {
    if (itemID <= 0)
        return 0;
    const uint8_t *record = PeekItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr) {
        Item::Data::WarmCache(static_cast<uint32_t>(itemID));
        return 0;
    }
    const char *name = *reinterpret_cast<const char *const *>(
        record + Offsets::OFF_ITEMSTATS_NAME);
    if (name == nullptr || *name == '\0')
        return 0;
    Game::Lua::PushString(L, name);
    return 1;
}

} // namespace

// `C_Item.GetItemName(itemLocation)` — returns the cached display name
// of the item at the location, or nil for empty / uncached / invalid
// inputs. Reads `m_name[0]` straight off `ItemStats_C` — no engine
// round-trip, no `GetItemInfo` chaining.
static int __fastcall Script_C_Item_GetItemName(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemName(itemLocation)");
        return 0;
    }
    return PushNameForItemID(L, Item::ID::FromCGItem(Item::Location::Resolve(L, 1)));
}

// `C_Item.GetItemNameByID(itemInfo)` — returns the cached display name
// for `itemID`, firing a background cache fill on miss. Accepts the
// same input shapes as `GetItemInfoInstant` (number, `"item:NNN..."`
// string, or any string containing a parseable `item:NN` chunk).
static int __fastcall Script_C_Item_GetItemNameByID(void *L) {
    return PushNameForItemID(L, Item::Arg::ResolveItemID(L, 1));
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemName", &Script_C_Item_GetItemName);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemNameByID", &Script_C_Item_GetItemNameByID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Name
