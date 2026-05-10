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
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>
#include <cstring>

namespace Item::Equipment {

namespace {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                      const uint64_t *guid, void *callback,
                                                      void *userData, int unused);

// Same item-cache peek pattern as `Item::Bag::PeekItemRecord` — we
// don't share because each caller is one-line-different and the
// helper isn't worth a header.
const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

// `OffhandHasWeapon()` — true iff the player has a one-handed
// weapon (or off-hand-only weapon) equipped in the off-hand slot.
// Returns false for empty off-hand, shields, and held items
// (tomes, orbs, librams) — even though the latter two share the
// off-hand slot.
//
// Implementation: resolves slot 17 → CGItem → itemID → cached
// ItemStats record → m_inventoryType. Falls back to false on any
// missing link in that chain (no item, item data not cached yet,
// etc.). Modern API behavior is the same — no async load fired,
// no event emitted; if the off-hand item isn't cached the function
// just returns false until something else warms the cache.
int __fastcall Script_OffhandHasWeapon(void *L) {
    auto *item = Item::Location::ResolveEquipmentSlot(Offsets::INVSLOT_OFFHAND);
    if (item == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    const int itemID = Item::ID::FromCGItem(item);
    if (itemID == 0) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    auto *record = PeekItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    const uint32_t invType = *reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_INVENTORY_TYPE);
    const bool isWeapon = (invType == Offsets::INVTYPE_WEAPON ||
                           invType == Offsets::INVTYPE_WEAPONOFFHAND);
    Game::Lua::PushBoolean(L, isWeapon ? 1 : 0);
    return 1;
}

// `C_Item.IsEquippedItem(item)` — true iff any character-pane equipment
// slot (1..19) currently holds an item matching `item`. Modern API
// accepts:
//   - itemID number → exact match against the equipped slot's itemID
//   - "item:N..." string → parse the link's first numeric field as the
//                          itemID and match the same way
//   - plain string → case-insensitive name match against the cached
//                    `m_name[0]` of each equipped item
//
// Returns false (never errors) for invalid input, an empty inventory,
// or an uncached match candidate. Walks slots in order and short-
// circuits on the first hit.
int __fastcall Script_C_Item_IsEquippedItem(void *L) {
    const auto arg = Item::Arg::Resolve(L, 1);
    if (arg.itemID <= 0 && arg.name == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    for (int slot = Offsets::EQUIPMENT_SLOT_FIRST; slot <= Offsets::EQUIPMENT_SLOT_LAST; ++slot) {
        auto *item = Item::Location::ResolveEquipmentSlot(slot);
        if (item == nullptr) continue;

        const int id = Item::ID::FromCGItem(item);
        if (id == 0) continue;

        if (arg.itemID > 0) {
            if (id == arg.itemID) {
                Game::Lua::PushBoolean(L, 1);
                return 1;
            }
            continue;
        }

        // Name-match path — peek the item-cache record and compare
        // `m_name[0]`. Uncached items short-circuit to "no match"
        // rather than firing a load; matches modern API semantics
        // (sync call, possibly stale, never blocks).
        auto *record = PeekItemRecord(static_cast<uint32_t>(id));
        if (record == nullptr) continue;
        const char *name = *reinterpret_cast<const char *const *>(
            record + Offsets::OFF_ITEMSTATS_NAME);
        if (name == nullptr) continue;
        if (_stricmp(name, arg.name) == 0) {
            Game::Lua::PushBoolean(L, 1);
            return 1;
        }
    }

    Game::Lua::PushBoolean(L, 0);
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("OffhandHasWeapon", &Script_OffhandHasWeapon);
    Game::Lua::RegisterTableFunction("C_Item", "IsEquippedItem", &Script_C_Item_IsEquippedItem);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Equipment
