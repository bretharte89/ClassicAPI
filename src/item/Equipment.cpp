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

// Asks the engine how many slots `bagID` has via the existing
// `Script_GetContainerNumSlots` Lua C function. Same dispatch trick
// `Item::Hearthstone` uses; delegates the "what's a valid bag, how
// big is it" decision to the engine. Returns 0 for empty bag slots
// (engine pushes 0 for "no bag equipped").
//
// Stomps the Lua stack — caller must own it.
int GetContainerSlotCount(void *L, int bagID) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bagID));
    using ScriptFn_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_CONTAINER_NUM_SLOTS);
    fn(L);
    if (!Game::Lua::IsNumber(L, -1))
        return 0;
    return static_cast<int>(Game::Lua::ToNumber(L, -1));
}

// Walks bags 0..4 looking for an item matching `arg`. On hit, sets
// `*outBag` / `*outSlot` and returns true.
//
// itemID match is direct; name match peeks the item-cache record's
// `m_name[0]` and compares case-insensitively. Stomps the Lua stack
// per the helpers it calls.
bool FindItemInBags(void *L, const Item::Arg::Resolved &arg, int *outBag, int *outSlot) {
    for (int bag = 0; bag <= 4; ++bag) {
        const int slotCount = GetContainerSlotCount(L, bag);
        for (int slot = 1; slot <= slotCount; ++slot) {
            const uint8_t *item = Item::Location::ResolveBag(L, bag, slot);
            if (item == nullptr) continue;

            const int id = Item::ID::FromCGItem(item);
            if (id == 0) continue;

            if (arg.itemID > 0) {
                if (id == arg.itemID) {
                    *outBag = bag;
                    *outSlot = slot;
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
                *outBag = bag;
                *outSlot = slot;
                return true;
            }
        }
    }
    return false;
}

// `C_Item.EquipItemByName(itemInfo [, dstSlot])` — finds the first
// item in the player's bags matching `itemInfo` (itemID, link, or
// name) and equips it. With `dstSlot` (1..19 character-pane index),
// equips to that slot specifically; without, the engine auto-picks.
//
// Returns nothing. Silently no-ops on:
//   - bad/missing input
//   - item not in bags (already-equipped items aren't moved — modern
//     API behavior)
//   - the engine refusing the equip (combat, item locked, type
//     mismatch with `dstSlot`, etc.)
//
// Implementation dispatches to `Script_PickupContainerItem` then
// `Script_AutoEquipCursorItem` / `Script_EquipCursorItem`. Same
// pattern `Item::Hearthstone` uses for `UseContainerItem` — delegates
// to the engine's secure-action path so combat/lockdown/lock states
// are handled the same way they are for direct Lua calls.
int __fastcall Script_C_Item_EquipItemByName(void *L) {
    const auto arg = Item::Arg::Resolve(L, 1);
    if (arg.itemID <= 0 && arg.name == nullptr) {
        return 0;
    }

    const bool hasDstSlot = Game::Lua::IsNumber(L, 2);
    const int dstSlot = hasDstSlot ? static_cast<int>(Game::Lua::ToNumber(L, 2)) : 0;

    // Refuse the swap if the player's cursor is holding something —
    // the dispatch path below uses PickupContainerItem internally,
    // which would clobber the held item by swapping it into the
    // source bag slot. Better to no-op than to silently swap
    // someone's hearthstone into wherever the source weapon was.
    using ScriptFn_t = int(__fastcall *)(void *L);
    Game::Lua::SetTop(L, 0);
    auto cursorHasItem = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_CURSOR_HAS_ITEM);
    cursorHasItem(L);
    if (Game::Lua::ToBoolean(L, -1) != 0) {
        Game::Lua::SetTop(L, 0);
        return 0;
    }
    Game::Lua::SetTop(L, 0);

    int bag = -1, slot = -1;
    if (!FindItemInBags(L, arg, &bag, &slot)) {
        return 0;
    }

    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bag));
    Game::Lua::PushNumber(L, static_cast<double>(slot));
    auto pickup = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_PICKUP_CONTAINER_ITEM);
    pickup(L);

    Game::Lua::SetTop(L, 0);
    if (hasDstSlot) {
        Game::Lua::PushNumber(L, static_cast<double>(dstSlot));
        auto equip = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_EQUIP_CURSOR_ITEM);
        equip(L);
    } else {
        auto equip = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_AUTO_EQUIP_CURSOR_ITEM);
        equip(L);
    }
    return 0;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("OffhandHasWeapon", &Script_OffhandHasWeapon);
    Game::Lua::RegisterTableFunction("C_Item", "IsEquippedItem", &Script_C_Item_IsEquippedItem);
    Game::Lua::RegisterTableFunction("C_Item", "EquipItemByName", &Script_C_Item_EquipItemByName);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Equipment
