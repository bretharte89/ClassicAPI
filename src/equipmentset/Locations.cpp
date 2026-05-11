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

#include "Locations.h"

#include "Game.h"
#include "Offsets.h"
#include "item/Location.h"

#include <cstdint>

namespace EquipmentSet::Locations {

namespace {

void *ResolvePlayer() {
    using ResolveUnitToken_t = void *(__fastcall *)(const char *);
    auto fn = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return fn("player");
}

uint8_t *ResolvePlayerInvMgr() {
    auto *player = static_cast<uint8_t *>(ResolvePlayer());
    if (player == nullptr)
        return nullptr;
    return player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
}

// Reads the GUID at a given linear slot in the player invMgr's flat
// array. The array covers paperdoll → backpack → main bank → bank bag
// slots. Bag contents (player or bank) live in separate per-bag
// invMgrs and aren't reachable through this array.
uint64_t ReadFlatGUID(int linearSlot) {
    auto *invMgr = ResolvePlayerInvMgr();
    if (invMgr == nullptr)
        return 0;
    auto *guids = *reinterpret_cast<uint64_t **>(
        invMgr + Offsets::OFF_INVMGR_GUID_ARRAY);
    if (guids == nullptr)
        return 0;
    return guids[linearSlot];
}

// Reads the GUID off a `CGItem *` (instance block at +0x08, GUID at
// the start of that block). Used to verify bag walks — `ResolveBag`
// returns a CGItem and we then check its GUID against our target.
uint64_t ReadCGItemGUID(const uint8_t *item) {
    if (item == nullptr)
        return 0;
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return *reinterpret_cast<const uint64_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_GUID);
}

// Engine helper for "what's the slot count of bag N" — same primitive
// the hearthstone finder uses. Returns 0 for empty bag slots.
int BagSlotCount(void *L, int bagID) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bagID));
    using ScriptFn_t = int(__fastcall *)(void *);
    auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_CONTAINER_NUM_SLOTS);
    fn(L);
    if (!Game::Lua::IsNumber(L, -1))
        return 0;
    return static_cast<int>(Game::Lua::ToNumber(L, -1));
}

// Scans a single bag's contents for `targetGuid`. `bagID` is one of
// 0..4 (player) or 5..10 (bank). Returns the 1-based slot index of
// the match, or 0 if not found. Bank bag walks may legitimately
// return 0 when the bank is closed — the engine's `GetItemBySlot`
// gate suppresses the data, and we can't probe it without bypassing
// engine-internal state.
int WalkBagForGUID(void *L, int bagID, uint64_t targetGuid) {
    const int count = BagSlotCount(L, bagID);
    for (int slot = 1; slot <= count; ++slot) {
        const uint8_t *item = Item::Location::ResolveBag(L, bagID, slot);
        if (item == nullptr)
            continue;
        if (ReadCGItemGUID(item) == targetGuid)
            return slot;
    }
    return 0;
}

} // namespace

int FindGUID(void *L, uint64_t targetGuid) {
    if (targetGuid == GUID_EMPTY || targetGuid == GUID_IGNORED)
        return 0;

    // Paperdoll first — by far the most common hit since the user
    // already has their gear on, especially right after `SaveEquipmentSet`.
    for (int slot = 1; slot <= SLOT_COUNT; ++slot) {
        if (ReadFlatGUID(slot - 1) == targetGuid)
            return PackEquipped(slot);
    }

    // Backpack: linear slots 23..38 = bag 0 slots 1..16.
    for (int s = Offsets::INVMGR_BANK_MAIN_FIRST_SLOT - 16;
         s < Offsets::INVMGR_BANK_MAIN_FIRST_SLOT; ++s) {
        if (ReadFlatGUID(s) == targetGuid) {
            const int slot1 = s - (Offsets::INVMGR_BANK_MAIN_FIRST_SLOT - 16) + 1;
            return PackBag(0, slot1);
        }
    }

    // Main bank: linear slots 39..62 → bank slots 1..24.
    for (int s = Offsets::INVMGR_BANK_MAIN_FIRST_SLOT;
         s <= Offsets::INVMGR_BANK_MAIN_LAST_SLOT; ++s) {
        if (ReadFlatGUID(s) == targetGuid) {
            const int slot1 = s - Offsets::INVMGR_BANK_MAIN_FIRST_SLOT + 1;
            return PackMainBank(slot1);
        }
    }

    // Player bag contents (bags 1..4). Walks each bag's own invMgr
    // via the engine's `PackBagSlot` path.
    for (int bag = 1; bag <= 4; ++bag) {
        const int slot = WalkBagForGUID(L, bag, targetGuid);
        if (slot > 0)
            return PackBag(bag, slot);
    }

    // Bank bag contents (bags 5..10) — only resolves while bank is open.
    for (int bag = 5; bag <= 10; ++bag) {
        const int slot = WalkBagForGUID(L, bag, targetGuid);
        if (slot > 0)
            return PackBankBag(bag, slot);
    }

    return 0;
}

const uint8_t *ResolveItemByGUID(uint64_t guid) {
    if (guid == GUID_EMPTY || guid == GUID_IGNORED)
        return nullptr;
    using ResolveByGUID_t = void *(__fastcall *)(int type, const char *debugName,
                                                  uint32_t guidLo, uint32_t guidHi,
                                                  int priority);
    auto fn = reinterpret_cast<ResolveByGUID_t>(Offsets::FUN_OBJECT_RESOLVE_BY_GUID);
    auto *obj = fn(Offsets::OBJ_TYPE_ITEM, "ItemMgr",
                   static_cast<uint32_t>(guid),
                   static_cast<uint32_t>(guid >> 32),
                   0x172);
    return static_cast<const uint8_t *>(obj);
}

} // namespace EquipmentSet::Locations
