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
#include "item/Spell.h"

#include <cstdint>

namespace Item::Hearthstone {

namespace {

// Asks the engine how many slots `bagID` has by invoking the existing
// `Script_GetContainerNumSlots` Lua C function. Delegates instead of
// hardcoding so custom servers shipping larger-than-vanilla bags
// (e.g., 36-slot custom containers) work transparently. Returns 0
// for empty bag slots (engine pushes 0 for "no bag equipped").
//
// Stomps the Lua stack — caller must own it. Same stack-clobber
// pattern as `Item::Location::ResolveBag`.
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

// Tests whether `itemID` is a hearthstone — either the vanilla one
// (6948) or a custom-server variant that reuses Hearthstone's
// on-use spell (8690). The itemID check is the fast path; the
// on-use-spell check requires the item's ItemStats record to be in
// the local cache, which is true for any item currently in the
// player's bags (the engine pre-fills the cache during bag sync).
bool IsHearthstoneItem(uint32_t itemID) {
    if (itemID == Offsets::HEARTHSTONE_ITEM_ID)
        return true;
    return Item::Spell::OnUseSpellIDForItemID(itemID) ==
           Offsets::HEARTHSTONE_SPELL_ID;
}

// Walks the player's bags 0..4 looking for a hearthstone-equivalent
// item. Returns `(bagID, slotIndex, itemID)` in the out params, or
// `(-1, -1, 0)` if none found. Stops on first match — we don't care
// about duplicates.
//
// The `outItemID` lets `PlayerHasHearthstone` return the actual
// matched itemID (which for a custom-server hearthstone won't be
// `HEARTHSTONE_ITEM_ID`).
//
// Each `ResolveBag` / `GetContainerSlotCount` call clobbers Lua
// stack[1]/[2], but we own the stack inside our callback context.
bool FindHearthstone(void *L, int *outBag, int *outSlot, int *outItemID) {
    for (int bag = 0; bag <= 4; bag++) {
        const int slotCount = GetContainerSlotCount(L, bag);
        for (int slot = 1; slot <= slotCount; slot++) {
            const uint8_t *item = Item::Location::ResolveBag(L, bag, slot);
            if (item == nullptr)
                continue;
            const int itemID = Item::ID::FromCGItem(item);
            if (itemID > 0 && IsHearthstoneItem(static_cast<uint32_t>(itemID))) {
                *outBag = bag;
                *outSlot = slot;
                *outItemID = itemID;
                return true;
            }
        }
    }
    *outBag = -1;
    *outSlot = -1;
    *outItemID = 0;
    return false;
}

// `C_Container.PlayerHasHearthstone()` — returns the itemID of any
// hearthstone-equivalent item in the player's bags (0..4), or `nil`
// if none found. "Hearthstone-equivalent" = itemID 6948 (vanilla) OR
// any item whose on-use spell is 8690 (catches custom-server
// hearthstone reskins; see `IsHearthstoneItem` above).
//
// Returns the **actual** matched itemID, not a hardcoded constant —
// matches modern WoW's behavior where multiple hearthstone-toy
// itemIDs are recognized and the specific one found gets returned.
//
// Walks the bags via `Item::Location::ResolveBag` — same path
// `C_Container.GetContainerItemID` uses internally. Stops on first
// match.
int __fastcall Script_C_Container_PlayerHasHearthstone(void *L) {
    int bag = -1, slot = -1, itemID = 0;
    if (!FindHearthstone(L, &bag, &slot, &itemID))
        return 0;
    Game::Lua::PushNumber(L, static_cast<double>(itemID));
    return 1;
}

// `C_Container.UseHearthstone()` — finds and uses the hearthstone in
// the player's bags. Returns `true` on success (hearthstone found and
// the use call dispatched), `false` if no hearthstone is in bags.
//
// "Used" matches modern semantics — we don't probe whether the cast
// actually started (cooldown, in-combat, moving, etc. would fail
// downstream). The return is "did we have one to try with."
//
// Implementation: locate the hearthstone, then invoke the engine's
// existing `Script_UseContainerItem` Lua C function with `(bagID,
// slot)` on the stack. That path runs the same secure-action flow a
// regular `UseContainerItem(bag, slot)` Lua call would, so any spell-
// queue / GCD / movement-cancel handling is unchanged.
int __fastcall Script_C_Container_UseHearthstone(void *L) {
    int bag = -1, slot = -1, itemID = 0;
    if (!FindHearthstone(L, &bag, &slot, &itemID)) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    // Set up the stack so Script_UseContainerItem reads (bag, slot)
    // from idx 1 / idx 2. Our function takes no Lua args, so anything
    // on the stack is junk we can clear. Note: ResolveBag inside
    // FindHearthstone already clobbered the stack — we just need to
    // shape it correctly for the dispatch.
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bag));
    Game::Lua::PushNumber(L, static_cast<double>(slot));

    using ScriptFn_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_USE_CONTAINER_ITEM);
    fn(L);

    // Reset the stack so our boolean push lands at idx 1.
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushBoolean(L, 1);
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Container", "PlayerHasHearthstone",
                                     &Script_C_Container_PlayerHasHearthstone);
    Game::Lua::RegisterTableFunction("C_Container", "UseHearthstone",
                                     &Script_C_Container_UseHearthstone);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Hearthstone
