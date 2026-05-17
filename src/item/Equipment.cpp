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
#include "item/Swap.h"

#include <cstdint>
#include <string.h>

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

// `C_Item.IsEquippableItem(item)` — true iff `item` can be equipped
// in *some* character-pane slot. Reads `m_inventoryType` off the
// cached ItemStats record and treats any non-zero value as
// equippable (INVTYPE_NON_EQUIP = 0, everything else 1..23 fits some
// slot). Modern signature accepts itemID number, `"item:N..."` link,
// or item name; we delegate input resolution to `Item::Arg::Resolve`.
//
// Cache-miss handling: returns false without firing a load (sync
// call, matches modern semantics). Callers that want auto-warm
// behavior should run `C_Item.RequestLoadItemDataByID` first and
// re-check on `ITEM_DATA_LOAD_RESULT`.
//
// Name-only inputs aren't supported because vanilla has no item-name
// → itemID resolver (`Item::Arg::Resolve` returns just a name string,
// no itemID, and we'd need to scan the entire ItemStats cache to
// match a name back — slow, and the engine doesn't ship that map).
// Modern WoW only resolves names for items currently in inventory;
// `C_Item.IsEquippedItem` covers that exact niche already.
int __fastcall Script_C_Item_IsEquippableItem(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    if (itemID <= 0) {
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
    Game::Lua::PushBoolean(L, invType > 0 ? 1 : 0);
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

// Walks bags 0..4 looking for an item matching `arg`. On hit, sets
// `*outBag` / `*outSlot` / `*outItem` and returns true.
//
// itemID match is direct; name match peeks the item-cache record's
// `m_name[0]` and compares case-insensitively. Stomps the Lua stack
// per the helpers it calls.
bool FindItemInBags(void *L, const Item::Arg::Resolved &arg, int *outBag, int *outSlot,
                    const uint8_t **outItem) {
    for (int bag = 0; bag <= 4; ++bag) {
        const int slotCount = Item::Location::GetBagSlotCount(bag);
        for (int slot = 1; slot <= slotCount; ++slot) {
            const uint8_t *item = Item::Location::ResolveBag(L, bag, slot);
            if (item == nullptr) continue;

            const int id = Item::ID::FromCGItem(item);
            if (id == 0) continue;

            if (arg.itemID > 0) {
                if (id == arg.itemID) {
                    *outBag = bag;
                    *outSlot = slot;
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
                *outBag = bag;
                *outSlot = slot;
                *outItem = item;
                return true;
            }
        }
    }
    return false;
}

// `C_Item.EquipItemByName(itemInfo [, dstSlot])` — finds the first
// item in the player's bags matching `itemInfo` (itemID, link, or
// name) and equips it.
//
// Two paths after a shared cursor-clear:
//
// - **Explicit `dstSlot`** (1..19): cursor-free direct swap via
//   `Item::Swap::FromBag`, which calls the engine's
//   `FUN_INVENTORY_SWAP` primitive. Atomic server-side, no pickup.
//
// - **No `dstSlot`** (engine auto-picks slot from INVTYPE): cursor-
//   pickup + `AutoEquipCursorItem`, because 1.12's auto-pick logic
//   reads its slot decision off cursor state.
//
// Both paths start with `ClearCursor()` to clear any preexisting
// cursor state. If something was on the cursor, it gets returned
// to its source slot (visual lock cleared, source globals reset)
// before our work runs. Without this, the engine's swap primitive
// ends each call with a cursor-state cleanup that skips the
// item-flag clear (`FUN_00495190(0, 1)`); a held item would stay
// visually locked until a relog refreshed inventory state.
//
// Returns nothing. Silently no-ops on bad/missing input, item not
// found in bags, or engine refusing the swap (combat lockdown,
// item-locked flag, type mismatch).
int __fastcall Script_C_Item_EquipItemByName(void *L) {
    const auto arg = Item::Arg::Resolve(L, 1);
    if (arg.itemID <= 0 && arg.name == nullptr) {
        return 0;
    }

    const bool hasDstSlot = Game::Lua::IsNumber(L, 2);
    const int dstSlot = hasDstSlot ? static_cast<int>(Game::Lua::ToNumber(L, 2)) : 0;

    using ScriptFn_t = int(__fastcall *)(void *L);

    // Return any held cursor item to its slot BEFORE searching for
    // the target. FUN_INVENTORY_SWAP's end-of-call cleanup at
    // FUN_00495190(0, 1) clears LOCAL cursor globals but skips the
    // visual-lock-flag clear (`item+0x314 & ~1`) — so a held item
    // would stay visually locked until a relog refreshed state.
    // ClearCursor's FUN_00495190(1, 1) does include that clear.
    Game::Lua::SetTop(L, 0);
    reinterpret_cast<int(__fastcall *)(void *)>(
        Offsets::FUN_SCRIPT_CLEAR_CURSOR)(L);

    int bag = -1, slot = -1;
    const uint8_t *cgItem = nullptr;
    if (!FindItemInBags(L, arg, &bag, &slot, &cgItem)) {
        return 0;
    }

    if (hasDstSlot) {
        Item::Swap::FromBag(cgItem, bag, slot, dstSlot);
        return 0;
    }

    // Auto-slot path: pickup the item via Script_PickupContainerItem
    // (its state-machine handles spell-cast targeting / repair /
    // enchant-scroll / etc., none of which we want to replicate), then
    // call the engine's `CGPlayer::AutoEquipCursorItem` helper directly
    // — `Script_AutoEquipCursorItem` is a thin wrapper that just
    // resolves the local player and calls this with flag=0.
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bag));
    Game::Lua::PushNumber(L, static_cast<double>(slot));
    auto pickup = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_PICKUP_CONTAINER_ITEM);
    pickup(L);

    using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
    using AutoEquipCursor_t = void (__thiscall *)(void *player, int flag);
    auto resolve = reinterpret_cast<ResolveUnitToken_t>(
        Offsets::FUN_RESOLVE_UNIT_TOKEN);
    if (auto *player = resolve("player")) {
        auto equip = reinterpret_cast<AutoEquipCursor_t>(
            Offsets::FUN_AUTO_EQUIP_CURSOR_ITEM);
        equip(player, 0);
    }
    return 0;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("OffhandHasWeapon", &Script_OffhandHasWeapon);
    Game::Lua::RegisterTableFunction("C_Item", "IsEquippableItem",
                                     &Script_C_Item_IsEquippableItem);
    Game::Lua::RegisterTableFunction("C_Item", "IsEquippedItem", &Script_C_Item_IsEquippedItem);
    Game::Lua::RegisterTableFunction("C_Item", "EquipItemByName", &Script_C_Item_EquipItemByName);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Equipment
