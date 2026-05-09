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
#include <cstdlib>
#include <cstring>

namespace Item::Count {

namespace {

// Same Lua-arg parser as `Item::Info::ResolveItemID` — accepts a number
// or a string containing `"item:NNN"` (matches the bare shorthand and
// full chat links). Returns 0 for unparseable input. We don't promote
// the Info.cpp version to a header for a single extra caller; if a
// fourth module wants this, factor out then.
int ParseItemArg(void *L, int luaIdx) {
    if (Game::Lua::IsNumber(L, luaIdx))
        return static_cast<int>(Game::Lua::ToNumber(L, luaIdx));
    if (Game::Lua::Type(L, luaIdx) != Game::Lua::TYPE_STRING)
        return 0;
    const char *s = Game::Lua::ToString(L, luaIdx);
    if (s == nullptr)
        return 0;
    if (const char *m = std::strstr(s, "item:"))
        return std::atoi(m + 5);
    return std::atoi(s);
}

// Asks the engine how many slots `bagID` has by invoking the existing
// `Script_GetContainerNumSlots` Lua C function. Same dispatch trick
// `Item::Hearthstone` uses; delegates the whole "what's a valid bag,
// how big is it" decision to the engine. Returns 0 for empty bag
// slots (no bag equipped → engine pushes 0).
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

// Reads ITEM_FIELD_STACK_COUNT off a CGItem's m_objectFields
// descriptor at +0x20. Returns 0 if the descriptor is null. Stack
// counts are always positive — non-stackable items have count = 1.
int GetStackCount(const uint8_t *cgItem) {
    if (cgItem == nullptr)
        return 0;
    auto *descriptor = *reinterpret_cast<const uint8_t *const *>(
        cgItem + Offsets::OFF_ITEM_DESCRIPTOR);
    if (descriptor == nullptr)
        return 0;
    return static_cast<int>(*reinterpret_cast<const uint32_t *>(
        descriptor + Offsets::OFF_DESCRIPTOR_STACK_COUNT));
}

// Walks `bagID` and accumulates stack counts for items matching
// `targetItemID`. Skips empty slots. Each `ResolveBag` call clobbers
// Lua stack[1]/[2]; we own the stack inside the callback.
int CountInBag(void *L, int bagID, int targetItemID) {
    int total = 0;
    const int slotCount = GetContainerSlotCount(L, bagID);
    for (int slot = 1; slot <= slotCount; slot++) {
        const uint8_t *item = Item::Location::ResolveBag(L, bagID, slot);
        if (item == nullptr)
            continue;
        if (Item::ID::FromCGItem(item) != targetItemID)
            continue;
        total += GetStackCount(item);
    }
    return total;
}

// Direct GUID-array bypass for bank slots — sidesteps the gate at
// `0x006228C1` that `GetItemBySlot` applies to slots 39..68 when no
// bank window is open. The slot data in `invMgr+0x04` is populated at
// login (verified empirically — see `Offsets::OFF_INVMGR_GUID_ARRAY`),
// so we can read GUIDs directly and resolve them via the engine's own
// object resolver `FUN_OBJECT_RESOLVE_BY_GUID` (the same function
// `GetItemBySlot` would call internally if the gate let us through).

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
using ResolveObjectByGuid_t = void *(__fastcall *)(int type,
                                                    const char *debugName,
                                                    uint32_t guidLo,
                                                    uint32_t guidHi,
                                                    int priority);

const uint8_t *PlayerInvMgr() {
    auto ResolveUnitToken =
        reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<uint8_t *>(ResolveUnitToken("player"));
    if (player == nullptr)
        return nullptr;
    return player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
}

const uint8_t *ResolveByGuid(int type, uint64_t guid) {
    if (guid == 0)
        return nullptr;
    auto fn = reinterpret_cast<ResolveObjectByGuid_t>(
        Offsets::FUN_OBJECT_RESOLVE_BY_GUID);
    return static_cast<const uint8_t *>(
        fn(type, "ItemMgr",
           static_cast<uint32_t>(guid),
           static_cast<uint32_t>(guid >> 32),
           0x172));
}

// Walks linear slots `[firstSlot, lastSlot]` of `invMgr`'s GUID array
// at `+0x04`, resolves each non-zero GUID as a TYPE_ITEM, sums stack
// counts of items matching `targetItemID`. Used for both the main bank
// (slots 39..62 in player invMgr) and individual bank-bag invMgrs
// (slots 0..N-1).
int CountInGuidArray(const uint8_t *invMgr, int firstSlot, int lastSlot,
                      int targetItemID) {
    if (invMgr == nullptr)
        return 0;
    auto *guidArray = *reinterpret_cast<const uint64_t *const *>(
        invMgr + Offsets::OFF_INVMGR_GUID_ARRAY);
    if (guidArray == nullptr)
        return 0;
    int total = 0;
    for (int slot = firstSlot; slot <= lastSlot; slot++) {
        const uint8_t *item = ResolveByGuid(Offsets::OBJ_TYPE_ITEM, guidArray[slot]);
        if (item == nullptr)
            continue;
        if (Item::ID::FromCGItem(item) != targetItemID)
            continue;
        total += GetStackCount(item);
    }
    return total;
}

// For bank bag CONTENTS: the equipped bank bags themselves live at
// linear slots 63..68 in the player invMgr. Each is a CGContainer with
// its own invMgr accessed via `vtable[+0x10]` (`CGContainer::GetInvMgr`
// or similar). We resolve the bag, get its invMgr, and walk that.
//
// Layout: bag invMgr+0x00 = max slot count (the bag's size); bag
// invMgr+0x04 = its GUID array. Same shape as the player invMgr.
int CountInBankBags(int targetItemID) {
    auto *playerInvMgr = PlayerInvMgr();
    if (playerInvMgr == nullptr)
        return 0;
    auto *playerGuidArray = *reinterpret_cast<const uint64_t *const *>(
        playerInvMgr + Offsets::OFF_INVMGR_GUID_ARRAY);
    if (playerGuidArray == nullptr)
        return 0;

    int total = 0;
    using GetBagInvMgr_t = void *(__thiscall *)(void *bag);
    for (int slot = Offsets::INVMGR_BANK_BAG_FIRST_SLOT;
         slot <= Offsets::INVMGR_BANK_BAG_LAST_SLOT; slot++) {
        const uint8_t *bag = ResolveByGuid(Offsets::OBJ_TYPE_CONTAINER,
                                           playerGuidArray[slot]);
        if (bag == nullptr)
            continue;
        // vtable +0x10 returns the bag's own invMgr pointer
        auto *vtable = *reinterpret_cast<const uint8_t *const *>(bag);
        auto getInvMgr = reinterpret_cast<GetBagInvMgr_t>(
            *reinterpret_cast<const uintptr_t *>(vtable + 0x10));
        auto *bagInvMgr = static_cast<const uint8_t *>(
            getInvMgr(const_cast<uint8_t *>(bag)));
        if (bagInvMgr == nullptr)
            continue;
        const int bagSlots = static_cast<int>(*reinterpret_cast<const uint32_t *>(bagInvMgr));
        if (bagSlots <= 0)
            continue;
        total += CountInGuidArray(bagInvMgr, 0, bagSlots - 1, targetItemID);
    }
    return total;
}

// `C_Item.GetItemCount(itemInfo [, includeBank [, includeUses]])` —
// returns the player's total count of `itemInfo` across bags
// (and optionally bank). `itemInfo` is a number or `"item:NNN"`
// string. Item names are NOT accepted (vanilla itself has no
// name → ID resolver).
//
// Bag walks:
//   - bags 0..4 always (backpack + 4 equipped bag slots) — live walk
//     via `Item::Location::ResolveBag`.
//   - if `includeBank=true`: main bank (linear slots 39..62 in player
//     invMgr) + each equipped bank bag's contents (slots 63..68 in
//     player invMgr). Bank walks **bypass `GetItemBySlot`** by reading
//     the GUID array at `invMgr+0x04` directly and resolving via
//     `FUN_OBJECT_RESOLVE_BY_GUID`. This works without the bank window
//     ever being opened — the GUIDs are populated from server data at
//     login. The engine gates `GetItemBySlot` for bank slots via a
//     banker-GUID check at `VAR_BANK_GATE_GUID`, but the underlying
//     data is always present.
//
// `includeUses` (charges-multiplier mode) is **accepted but currently
// ignored**. Modern semantics: when true, multiplies each match by
// the item's spell-charges count (so a stack of 5 wands × 50 charges
// each → 250). Implementing this needs the `ITEM_FIELD_SPELL_CHARGES`
// descriptor offset, which hasn't been verified empirically yet.
// Most callers don't use this flag; for now, both `true` and `false`
// produce identical results (stack counts only).
//
// Equipped items are NOT counted, matching modern API behavior.
// Anyone equipping a wand to count its charges should use
// `GetInventoryItemID` + a separate charges read.
int __fastcall Script_C_Item_GetItemCount(void *L) {
    const int itemID = ParseItemArg(L, 1);
    if (itemID <= 0) {
        Game::Lua::PushNumber(L, 0);
        return 1;
    }
    // `includeBank` is arg 2. ToBoolean handles missing/nil cleanly
    // (returns 0). We read it before the bag walk because the walk
    // clobbers the Lua stack via SetTop(0) inside ResolveBag.
    const bool includeBank = Game::Lua::ToBoolean(L, 2) != 0;

    int total = 0;
    for (int bag = 0; bag <= 4; bag++)
        total += CountInBag(L, bag, itemID);
    if (includeBank) {
        // Direct GUID-array reads — bypass the bank gate so this works
        // without the bank window having ever been opened in the
        // session.
        total += CountInGuidArray(PlayerInvMgr(),
                                   Offsets::INVMGR_BANK_MAIN_FIRST_SLOT,
                                   Offsets::INVMGR_BANK_MAIN_LAST_SLOT,
                                   itemID);
        total += CountInBankBags(itemID);
    }

    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(total));
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemCount",
                                     &Script_C_Item_GetItemCount);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Count
