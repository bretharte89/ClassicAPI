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

// Engine's per-instance item-name builder: `__thiscall(CGItem *this,
// char *outBuf, uint outSize)`. Reads the instance's random-suffix ID off
// the descriptor (+0x98) and writes the decorated display name
// ("Ethereum Torque of the Sorcerer"), or the base name when there's no
// suffix. Same builder the item-link builder uses for the bracketed name,
// so this matches `GetItemLink`'s name and modern `C_Item.GetItemName`.
using BuildInstanceName_t = void(__thiscall *)(const void *cgItem, char *out,
                                               unsigned outSize);

// Pushes the suffix-decorated display name for a resolved `CGItem`.
// Returns 0 (pushes nothing) for a null item or an empty build result,
// so the caller can fall back to the base-name-by-itemID path.
int PushInstanceName(void *L, const uint8_t *cgItem) {
    if (cgItem == nullptr)
        return 0;
    char buf[128];
    buf[0] = '\0';
    auto fn = reinterpret_cast<BuildInstanceName_t>(
        Offsets::FUN_ITEM_BUILD_INSTANCE_NAME);
    fn(cgItem, buf, sizeof(buf));
    if (buf[0] == '\0')
        return 0;
    Game::Lua::PushString(L, buf);
    return 1;
}

} // namespace

// `C_Item.GetItemName(itemLocation)` — returns the display name of the
// item at the location, or nil for empty / uncached / invalid inputs.
// The location points at a real item instance, so the name carries any
// random-suffix decoration ("... of the Sorcerer") — built off the
// CGItem via the engine's own name builder, matching modern WoW and
// `C_Item.GetItemLink`'s bracketed name. Falls back to the base
// `ItemStats_C` name (and warms the cache) if the instance build yields
// nothing.
static int __fastcall Script_C_Item_GetItemName(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemName(itemLocation)");
        return 0;
    }
    const uint8_t *cgItem = Item::Location::Resolve(L, 1);
    if (int pushed = PushInstanceName(L, cgItem))
        return pushed;
    return PushNameForItemID(L, Item::ID::FromCGItem(cgItem));
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
