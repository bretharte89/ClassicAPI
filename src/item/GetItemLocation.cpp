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

namespace Item::GetItemLocation {

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

// Mirrors `Item::Bag::ResolveBagInfo`'s slot-count derivation: backpack is
// fixed at 16, equipped bags read `m_containerSlots` off the cache record.
// Returns 0 for empty bag slots or out-of-range bagIDs — those iterations
// get skipped.
int SlotCountForBag(int bagID) {
    if (bagID == 0)
        return Offsets::BACKPACK_NUM_SLOTS;
    if (bagID < 1 || bagID > 4)
        return 0;
    auto *bagItem = Item::Location::ResolveEquipmentSlot(
        Offsets::INVSLOT_BAG1 + bagID - 1);
    if (bagItem == nullptr)
        return 0;
    const int bagItemID = Item::ID::FromCGItem(bagItem);
    if (bagItemID == 0)
        return 0;
    auto *record = PeekItemRecord(static_cast<uint32_t>(bagItemID));
    if (record == nullptr)
        return 0;
    return static_cast<int>(*reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_CONTAINER_SLOTS));
}

// Parses the `"0xHHHHHHHHLLLLLLLL"` form `C_Item.GetItemGUID` produces.
// Strict — exactly 18 chars (`0x` prefix + 16 hex), case-insensitive
// hex. Anything else fails; we leave malformed input as a nil return
// rather than raising, matching modern's "guid not found → nil".
bool ParseGUID(const char *s, uint64_t *out) {
    if (s == nullptr)
        return false;
    if (s[0] != '0' || (s[1] != 'x' && s[1] != 'X'))
        return false;
    uint64_t v = 0;
    for (int i = 0; i < 16; ++i) {
        const char c = s[2 + i];
        int d;
        if (c >= '0' && c <= '9')      d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else                            return false;
        v = (v << 4) | static_cast<uint64_t>(d);
    }
    if (s[18] != '\0')
        return false;
    *out = v;
    return true;
}

uint64_t ReadGUID(const uint8_t *item) {
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return *reinterpret_cast<const uint64_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_GUID);
}

// Equipment-slot search: walks 1..19, returns the 1-based slot on a
// match or 0 if not found. No Lua-stack interaction — safe to run
// before bag iteration (which does stomp the stack).
int FindInEquipmentSlots(uint64_t target) {
    for (int slot = Offsets::EQUIPMENT_SLOT_FIRST;
         slot <= Offsets::EQUIPMENT_SLOT_LAST; ++slot) {
        auto *item = Item::Location::ResolveEquipmentSlot(slot);
        if (item == nullptr)
            continue;
        if (ReadGUID(item) == target)
            return slot;
    }
    return 0;
}

// Bag search: walks backpack + equipped bags 1..4. Stomps the Lua
// stack (via `Item::Location::ResolveBag`'s `PackBagSlot` call) on
// every iteration — caller must rebuild whatever it needs on the
// stack afterwards.
bool FindInBags(void *L, uint64_t target, int *outBagID, int *outSlotIndex) {
    for (int bagID = 0; bagID <= 4; ++bagID) {
        const int slotCount = SlotCountForBag(bagID);
        for (int slot = 1; slot <= slotCount; ++slot) {
            auto *item = Item::Location::ResolveBag(L, bagID, slot);
            if (item == nullptr)
                continue;
            if (ReadGUID(item) == target) {
                *outBagID = bagID;
                *outSlotIndex = slot;
                return true;
            }
        }
    }
    return false;
}

void PushEquipmentLocation(void *L, int slot) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);
    Game::Lua::PushString(L, "equipmentSlotIndex");
    Game::Lua::PushNumber(L, static_cast<double>(slot));
    Game::Lua::SetTable(L, -3);
}

void PushBagLocation(void *L, int bagID, int slotIndex) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);
    Game::Lua::PushString(L, "bagID");
    Game::Lua::PushNumber(L, static_cast<double>(bagID));
    Game::Lua::SetTable(L, -3);
    Game::Lua::PushString(L, "slotIndex");
    Game::Lua::PushNumber(L, static_cast<double>(slotIndex));
    Game::Lua::SetTable(L, -3);
}

} // namespace

// `C_Item.GetItemLocation(itemGUID)` — reverse lookup from a per-
// instance item GUID (in the `"0xHHHHHHHHLLLLLLLL"` form
// `C_Item.GetItemGUID` returns) to an `itemLocation` table that
// identifies the slot currently holding it. Returns `nil` for
// unknown / non-resident GUIDs.
//
// Modern WoW returns an `ItemLocation` mixin object; we return a
// plain table with the same field shape the rest of the
// `C_Item.*` API accepts as input (`{equipmentSlotIndex=N}` or
// `{bagID=B, slotIndex=S}`), so you can pipe the result straight
// into `C_Item.GetItemQuality`, `C_Item.GetItemLink`, etc.
//
// Walked scope: character-pane equipment (slots 1..19) + backpack
// (bagID 0) + the four equipped bags (bagIDs 1..4). Bank and
// keyring are NOT walked — same scope as the existing
// `C_Container.*` APIs. Worst case is ~80 slot lookups; fine for
// sporadic use, not appropriate for per-frame polling.
static int __fastcall Script_C_Item_GetItemLocation(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_STRING) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemLocation(itemGUID)");
        return 0;
    }
    uint64_t target = 0;
    if (!ParseGUID(Game::Lua::ToString(L, 1), &target) || target == 0)
        return 0;

    if (const int slot = FindInEquipmentSlots(target); slot != 0) {
        PushEquipmentLocation(L, slot);
        return 1;
    }

    int bagID = 0;
    int slotIndex = 0;
    if (FindInBags(L, target, &bagID, &slotIndex)) {
        PushBagLocation(L, bagID, slotIndex);
        return 1;
    }

    return 0;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemLocation",
                                     &Script_C_Item_GetItemLocation);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::GetItemLocation
