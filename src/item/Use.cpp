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

// `C_Item.UseItemByName(itemInfo)` — finds the first item in the
// player's bags matching `itemInfo` (itemID, link, or name) and uses
// it via the engine's `CGItem::UseItem` primitive.
//
// Same structure 3.3.5's `Script_UseItemByName` uses: locate the item
// directly, then hand it to the engine's by-item-pointer use call.
// We skip `Script_UseContainerItem` — that function is a state-machine
// dispatcher for cursor modes (repair vendor, spell-cast targeting,
// drop-on-bag, ...) that don't apply to an addon-issued call from a
// clean cursor state. The default-path primitive `FUN_ITEM_USE`
// (`0x005D8D00`) handles every item-type-specific branch internally:
// food / potion / hearthstone / scroll / quiver all route through
// the same entry.
//
// Target arg: we pass nullptr / a zero GUID. The engine substitutes
// the player itself for self-use items (hearthstone, food, potion);
// for target-required items (scrolls, traps) the engine reads
// cursor-target state at call time, which a Lua-side caller can
// pre-set with `TargetUnit("...")`.
//
// Returns nothing. Silently no-ops on bad input, item not found in
// bags, item locked, on cooldown, etc.

#include "Game.h"
#include "Offsets.h"
#include "item/Arg.h"
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>
#include <string.h>

namespace Item::Use {

namespace {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                      const uint64_t *guid, void *callback,
                                                      void *userData, int unused);

using UseItem_t = unsigned(__thiscall *)(const void *item, const uint64_t *targetGuid,
                                          int flag);

const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

// Returns the matching CGItem* in `*outItem` on hit; mirrors the
// `Item::Equipment::FindItemInBags` walk but skips bag/slot output
// since `FUN_ITEM_USE` operates directly on the CGItem pointer.
bool FindItemInBags(void *L, const Item::Arg::Resolved &arg, const uint8_t **outItem) {
    for (int bag = 0; bag <= 4; ++bag) {
        const int slotCount = Item::Location::GetBagSlotCount(bag);
        for (int slot = 1; slot <= slotCount; ++slot) {
            const uint8_t *item = Item::Location::ResolveBag(L, bag, slot);
            if (item == nullptr) continue;

            const int id = Item::ID::FromCGItem(item);
            if (id == 0) continue;

            if (arg.itemID > 0) {
                if (id == arg.itemID) {
                    *outItem = item;
                    return true;
                }
                continue;
            }

            auto *record = PeekItemRecord(static_cast<uint32_t>(id));
            if (record == nullptr) continue;
            const char *name = *reinterpret_cast<const char *const *>(
                record + Offsets::OFF_ITEMSTATS_NAME);
            if (name == nullptr) continue;
            if (_stricmp(name, arg.name) == 0) {
                *outItem = item;
                return true;
            }
        }
    }
    return false;
}

int __fastcall Script_C_Item_UseItemByName(void *L) {
    const auto arg = Item::Arg::Resolve(L, 1);
    if (arg.itemID <= 0 && arg.name == nullptr) {
        return 0;
    }

    const uint8_t *cgItem = nullptr;
    if (!FindItemInBags(L, arg, &cgItem)) {
        return 0;
    }

    const uint64_t zeroTarget = 0;
    auto useItem = reinterpret_cast<UseItem_t>(Offsets::FUN_ITEM_USE);
    useItem(cgItem, &zeroTarget, 0);
    return 0;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "UseItemByName", &Script_C_Item_UseItemByName);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Use
