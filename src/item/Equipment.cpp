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

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("OffhandHasWeapon", &Script_OffhandHasWeapon);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Equipment
