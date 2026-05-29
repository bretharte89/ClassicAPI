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

#include "Location.h"

#include "../Game.h"
#include "../Offsets.h"
#include "../guid/Guid.h"
#include "ID.h"

namespace Item::Location {

namespace {

using GetItemBySlot_t = void *(__thiscall *)(void *thisInvMgr, int slot);
using PackBagSlot_t = int(__fastcall *)(void *L, void **outInvMgr, int *outLinearSlot,
                                         int *outUnused);
using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                      const uint64_t *guid, void *callback,
                                                      void *userData, int unused);

void *ResolveActivePlayerInvMgr() {
    auto ResolveUnitToken =
        reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<uint8_t *>(ResolveUnitToken("player"));
    if (player == nullptr)
        return nullptr;
    return player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
}

// Reads `loc.fieldName` and returns it as an int. Returns false via the
// boolean result if the field is missing or non-numeric. Always leaves the
// Lua stack as it found it.
bool TryReadIntField(void *L, int locIdx, const char *fieldName, int *out) {
    Game::Lua::PushString(L, fieldName);
    Game::Lua::GetTable(L, locIdx); // pops key, pushes value
    const bool ok = Game::Lua::IsNumber(L, -1);
    if (ok)
        *out = static_cast<int>(Game::Lua::ToNumber(L, -1));
    Game::Lua::SetTop(L, -2); // pop value (or nil)
    return ok;
}

const uint8_t *ResolveBagSlot(void *L, int bagID, int slotIndex) {
    // PackBagSlot reads bagID/slotIndex from Lua stack[1] and stack[2] (it's
    // designed to be called from a Lua-callback context like
    // Script_GetContainerItemInfo). Replace the stack so positions 1 and 2
    // hold the raw values for PackBagSlot's internal lua_tonumber reads.
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bagID));
    Game::Lua::PushNumber(L, static_cast<double>(slotIndex));

    void *invMgr = nullptr;
    int linearSlot = 0;
    int unused = 0;
    auto PackBagSlot = reinterpret_cast<PackBagSlot_t>(Offsets::FUN_PACK_BAG_SLOT);
    if (!PackBagSlot(L, &invMgr, &linearSlot, &unused) || invMgr == nullptr)
        return nullptr;

    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    return static_cast<const uint8_t *>(GetItemBySlot(invMgr, linearSlot));
}

// --- GUID-walk helpers ---------------------------------------------------

const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

uint64_t ReadCGItemGUID(const uint8_t *item) {
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return *reinterpret_cast<const uint64_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_GUID);
}

} // namespace

const uint8_t *ResolveEquipmentSlot(int slot1Based) {
    void *invMgr = ResolveActivePlayerInvMgr();
    if (invMgr == nullptr)
        return nullptr;
    // GetItemBySlot expects 0-based linearized slot indices. The built-in
    // Lua functions all do `dec eax` on the slot argument before calling —
    // see helper at 0x004C8520.
    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    return static_cast<const uint8_t *>(GetItemBySlot(invMgr, slot1Based - 1));
}

// Mirrors `Item::Bag::ResolveBagInfo`'s slot-count derivation: backpack
// is fixed at 16, equipped bags read `m_containerSlots` off the cache
// record. Returns 0 for empty bag slots or out-of-range bagIDs.
int GetBagSlotCount(int bagID) {
    if (bagID == 0)
        return Offsets::BACKPACK_NUM_SLOTS;
    if (bagID < 1 || bagID > 4)
        return 0;
    auto *bagItem = ResolveEquipmentSlot(Offsets::INVSLOT_BAG1 + bagID - 1);
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

const uint8_t *ResolveBag(void *L, int bagID, int slotIndex) {
    return ResolveBagSlot(L, bagID, slotIndex);
}

bool ParseGUIDString(const char *s, uint64_t *out) {
    return Guid::Parse(s, out);
}

const uint8_t *ResolveByGUID(uint64_t guid) {
    if (guid == 0)
        return nullptr;
    using ResolveByGUID_t = void *(__fastcall *)(int, const char *, uint32_t,
                                                  uint32_t, int);
    auto fn = reinterpret_cast<ResolveByGUID_t>(Offsets::FUN_OBJECT_RESOLVE_BY_GUID);
    return static_cast<const uint8_t *>(fn(Offsets::OBJ_TYPE_ITEM, "ItemMgr",
                                            static_cast<uint32_t>(guid),
                                            static_cast<uint32_t>(guid >> 32),
                                            0x172));
}

bool FindByItemID(void *L, int itemID, ByGUIDResult *out) {
    if (itemID <= 0)
        return false;

    for (int slot = Offsets::EQUIPMENT_SLOT_FIRST;
         slot <= Offsets::EQUIPMENT_SLOT_LAST; ++slot) {
        auto *item = ResolveEquipmentSlot(slot);
        if (item == nullptr)
            continue;
        if (Item::ID::FromCGItem(item) == itemID) {
            out->equipmentSlotIndex = slot;
            out->bagID = 0;
            out->slotIndex = 0;
            out->item = item;
            return true;
        }
    }

    for (int bagID = 0; bagID <= 4; ++bagID) {
        const int slotCount = GetBagSlotCount(bagID);
        for (int slotIndex = 1; slotIndex <= slotCount; ++slotIndex) {
            auto *item = ResolveBag(L, bagID, slotIndex);
            if (item == nullptr)
                continue;
            if (Item::ID::FromCGItem(item) == itemID) {
                out->equipmentSlotIndex = 0;
                out->bagID = bagID;
                out->slotIndex = slotIndex;
                out->item = item;
                return true;
            }
        }
    }

    return false;
}

bool FindByGUID(void *L, uint64_t guid, ByGUIDResult *out) {
    if (guid == 0)
        return false;

    // Equipment slots first — no Lua stack interaction, so cheaper.
    for (int slot = Offsets::EQUIPMENT_SLOT_FIRST;
         slot <= Offsets::EQUIPMENT_SLOT_LAST; ++slot) {
        auto *item = ResolveEquipmentSlot(slot);
        if (item == nullptr)
            continue;
        if (ReadCGItemGUID(item) == guid) {
            out->equipmentSlotIndex = slot;
            out->bagID = 0;
            out->slotIndex = 0;
            out->item = item;
            return true;
        }
    }

    // Bag slots — `ResolveBag` stomps the Lua stack on each call. Caller
    // beware: any data the caller stashed on the stack will be gone after
    // this returns true.
    for (int bagID = 0; bagID <= 4; ++bagID) {
        const int slotCount = GetBagSlotCount(bagID);
        for (int slotIndex = 1; slotIndex <= slotCount; ++slotIndex) {
            auto *item = ResolveBag(L, bagID, slotIndex);
            if (item == nullptr)
                continue;
            if (ReadCGItemGUID(item) == guid) {
                out->equipmentSlotIndex = 0;
                out->bagID = bagID;
                out->slotIndex = slotIndex;
                out->item = item;
                return true;
            }
        }
    }

    return false;
}

bool IsLocationArg(void *L, int idx) {
    const int t = Game::Lua::Type(L, idx);
    return t == Game::Lua::TYPE_TABLE || t == Game::Lua::TYPE_STRING;
}

const uint8_t *Resolve(void *L, int locIdx) {
    const int t = Game::Lua::Type(L, locIdx);

    if (t == Game::Lua::TYPE_STRING) {
        uint64_t guid = 0;
        if (!ParseGUIDString(Game::Lua::ToString(L, locIdx), &guid))
            return nullptr;
        ByGUIDResult found;
        if (!FindByGUID(L, guid, &found))
            return nullptr;
        return found.item;
    }

    if (t != Game::Lua::TYPE_TABLE)
        return nullptr;

    int eqSlot = 0;
    if (TryReadIntField(L, locIdx, "equipmentSlotIndex", &eqSlot))
        return ResolveEquipmentSlot(eqSlot);

    int bagID = 0, slotIndex = 0;
    if (TryReadIntField(L, locIdx, "bagID", &bagID) &&
        TryReadIntField(L, locIdx, "slotIndex", &slotIndex))
        return ResolveBagSlot(L, bagID, slotIndex);

    return nullptr;
}

} // namespace Item::Location
