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
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>

namespace Container::FreeSlots {

namespace {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                       const uint64_t *guid, void *callback,
                                                       void *userData, int unused);

// Same `Peek the item cache without firing a query` shape as
// `Item::Info::FetchItemRecord`. We don't share that helper because it's
// file-static there; copying the four-line lookup is simpler than
// promoting it to a header for one extra caller.
const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

// Resolves the bag's metadata: total slot count + BagFamily bitfield.
//
// - bagID 0 (backpack) → (16, 0). Vanilla 1.12 backpack is fixed at 16
//   slots and unfamilied (0). 3.3.5's `Script_GetContainerNumFreeSlots`
//   hardcodes both values for the backpack path too.
// - bagID 1..4 → looks up the equipped bag at INVSLOT_BAGn, reads
//   `m_containerSlots` and `m_bagFamily` from the cached `ItemStats_C`
//   record. If no bag equipped or item not cached: (0, 0).
// - other bagIDs (bank, keyring) → (0, 0). Not supported in this
//   implementation.
struct BagInfo {
    int slotCount;
    int bagType;
};

BagInfo ResolveBagInfo(int bagID) {
    if (bagID == 0)
        return {Offsets::BACKPACK_NUM_SLOTS, 0};
    if (bagID < 1 || bagID > 4)
        return {0, 0};

    const int equipSlot = Offsets::INVSLOT_BAG1 + bagID - 1;
    auto *bagItem = Item::Location::ResolveEquipmentSlot(equipSlot);
    if (bagItem == nullptr)
        return {0, 0};

    const int itemID = Item::ID::FromCGItem(bagItem);
    if (itemID == 0)
        return {0, 0};

    auto *record = PeekItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr)
        return {0, 0};

    const int slots = static_cast<int>(*reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_CONTAINER_SLOTS));
    // Convert 1.12's raw BagFamily ID to the modern bitmask. See
    // `Offsets::OFF_ITEMSTATS_BAG_FAMILY` for the encoding-shift story
    // and `Item::Info::BagFamilyIdToBitmask` for the canonical helper —
    // we inline the one-liner here to avoid a header dance for a
    // single-instruction conversion.
    const uint32_t rawFamily = *reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_BAG_FAMILY);
    const int family =
        (rawFamily == 0) ? 0 : static_cast<int>(1u << (rawFamily - 1));
    return {slots, family};
}

// Walks the bag and counts empty slots. Each `Item::Location::ResolveBag`
// call stomps Lua stack[1]/[2] (a requirement of the engine's
// `PackBagSlot` helper), but we own the stack here — Lua just called us
// — and the caller's bagID arg has already been read into a local. So
// it's safe to repeatedly clobber the stack across iterations.
int CountFreeSlotsInBag(void *L, int bagID, int slotCount) {
    int free = 0;
    for (int slot = 1; slot <= slotCount; slot++) {
        if (Item::Location::ResolveBag(L, bagID, slot) == nullptr)
            free++;
    }
    return free;
}

// `C_Container.GetContainerNumFreeSlots(bagID)` — returns
// `(numberOfFreeSlots, bagType)`. `bagID` is 0..4 (0 = backpack,
// 1..4 = equipped bag slots). For invalid or unsupported bag IDs
// (negative bank/keyring indices, out-of-range values), returns
// `(0, 0)` — matching 3.3.5's behavior of always pushing two values
// rather than nil/nothing.
//
// `bagType` is the `BagFamily` bitmask in modern encoding (raw 1.12
// ID converted via `1 << (ID-1)`). Empirically on Turtle WoW, only
// Light Quiver actually carries a non-zero family for bags themselves
// — Soul Pouch, Enchanting Bag, etc. all return 0 because the
// vanilla-era server data doesn't populate the field. **Individual
// items** (arrows, bullets, shards, herbs) reliably return their
// family, so addons that auto-route loot work correctly via
// `C_Item.GetItemFamily(itemID)` even though `bagType` is sparse.
int __fastcall Script_C_Container_GetContainerNumFreeSlots(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L,
            "Usage: C_Container.GetContainerNumFreeSlots(bagID)");
        return 0;
    }
    const int bagID = static_cast<int>(Game::Lua::ToNumber(L, 1));

    const BagInfo info = ResolveBagInfo(bagID);
    const int free = (info.slotCount > 0) ? CountFreeSlotsInBag(L, bagID, info.slotCount) : 0;

    Game::Lua::PushNumber(L, static_cast<double>(free));
    Game::Lua::PushNumber(L, static_cast<double>(info.bagType));
    return 2;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Container", "GetContainerNumFreeSlots",
                                     &Script_C_Container_GetContainerNumFreeSlots);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Container::FreeSlots
