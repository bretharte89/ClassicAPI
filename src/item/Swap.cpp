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

#include "item/Swap.h"

#include "Offsets.h"
#include "item/Location.h"

#include <cstdint>

namespace Item::Swap {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
using SwapFn_t = void(__thiscall *)(
    void *player,
    uint32_t srcItemLo, uint32_t srcItemHi,
    uint32_t srcContainerLo, uint32_t srcContainerHi,
    uint32_t srcLinearSlot,
    uint32_t dstContainerLo, uint32_t dstContainerHi,
    uint32_t dstLinearSlot,
    int flag);

void *ResolvePlayer() {
    auto fn = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return fn("player");
}

// Reads the 64-bit GUID stored at instance-block offset 0 of any
// CGObject-derived pointer (CGItem, CGPlayer, CGContainer all use
// the same +0x8 → instance block → +0x00 GUID layout).
bool ReadGuid(const void *cgObject, uint32_t *lo, uint32_t *hi) {
    if (cgObject == nullptr)
        return false;
    auto *base = static_cast<const uint8_t *>(cgObject);
    auto *inst = *reinterpret_cast<const uint8_t *const *>(
        base + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (inst == nullptr)
        return false;
    *lo = *reinterpret_cast<const uint32_t *>(inst + Offsets::OFF_INSTANCE_BLOCK_GUID);
    *hi = *reinterpret_cast<const uint32_t *>(inst + Offsets::OFF_INSTANCE_BLOCK_GUID + 4);
    return true;
}

// Bag-container linear-slot range — the four equipped-bag slots
// the engine treats as "directly in the player's invMgr" for the
// purposes of opcode 0x10D's same-container check. Just here so
// the magic +19 in FromBag is explained at the call site.
constexpr int FIRST_BAG_CONTAINER_LINEAR = 19;

// Backpack contents (bag 0) live at linear slots 23..38 in the
// player invMgr — i.e. 1-based bag-0 slot S maps to linear 22+S.
constexpr int BACKPACK_LINEAR_OFFSET = 22;

// Common send path. All paperdoll-side state (player ptr, player
// GUID) is resolved here; caller supplies the source side
// (already-encoded linear slot + the container GUID it lives in).
bool SendSwap(const void *srcItem,
              uint32_t srcContainerLo, uint32_t srcContainerHi,
              uint32_t srcLinearSlot,
              int dstPaperdollSlot) {
    if (dstPaperdollSlot < 1 || dstPaperdollSlot > 19)
        return false;
    if (srcItem == nullptr)
        return false;

    void *player = ResolvePlayer();
    if (player == nullptr)
        return false;

    uint32_t playerLo = 0, playerHi = 0;
    if (!ReadGuid(player, &playerLo, &playerHi))
        return false;

    uint32_t itemLo = 0, itemHi = 0;
    if (!ReadGuid(srcItem, &itemLo, &itemHi))
        return false;

    auto fn = reinterpret_cast<SwapFn_t>(Offsets::FUN_INVENTORY_SWAP);
    fn(player,
       itemLo, itemHi,
       srcContainerLo, srcContainerHi, srcLinearSlot,
       playerLo, playerHi,
       static_cast<uint32_t>(dstPaperdollSlot - 1),
       0);
    return true;
}

} // namespace

bool FromBag(const void *cgItem, int bagID, int slotInBag, int dstPaperdollSlot) {
    if (slotInBag < 1)
        return false;

    if (bagID == 0) {
        // Backpack: container is the player, linear slot is in the
        // 23..38 range of the player invMgr.
        void *player = ResolvePlayer();
        if (player == nullptr)
            return false;
        uint32_t playerLo = 0, playerHi = 0;
        if (!ReadGuid(player, &playerLo, &playerHi))
            return false;
        return SendSwap(cgItem,
                        playerLo, playerHi,
                        static_cast<uint32_t>(BACKPACK_LINEAR_OFFSET + slotInBag),
                        dstPaperdollSlot);
    }

    if (bagID >= 1 && bagID <= 4) {
        // Equipped bag: container is the bag's CGContainer (a CGItem),
        // linear slot is 0-based within that bag.
        const uint8_t *bag = Item::Location::ResolveEquipmentSlot(
            FIRST_BAG_CONTAINER_LINEAR + bagID);
        if (bag == nullptr)
            return false;
        uint32_t bagLo = 0, bagHi = 0;
        if (!ReadGuid(bag, &bagLo, &bagHi))
            return false;
        return SendSwap(cgItem,
                        bagLo, bagHi,
                        static_cast<uint32_t>(slotInBag - 1),
                        dstPaperdollSlot);
    }

    return false;
}

bool FromPaperdoll(const void *cgItem, int srcPaperdollSlot, int dstPaperdollSlot) {
    if (srcPaperdollSlot < 1 || srcPaperdollSlot > 19)
        return false;

    void *player = ResolvePlayer();
    if (player == nullptr)
        return false;
    uint32_t playerLo = 0, playerHi = 0;
    if (!ReadGuid(player, &playerLo, &playerHi))
        return false;
    return SendSwap(cgItem,
                    playerLo, playerHi,
                    static_cast<uint32_t>(srcPaperdollSlot - 1),
                    dstPaperdollSlot);
}

} // namespace Item::Swap
