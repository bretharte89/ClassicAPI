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

// `C_EquipmentSet.*` — backports the modern equipment-set API on top
// of a client-side persistent store. Vanilla 1.12 has no server-side
// equipment-set storage (no CMSG_EQUIPMENT_SET_SAVE opcode), so we
// persist each character's sets to
// `WTF\Account\<acct>\<realm>\<char>\ClassicAPI_EquipmentSets.txt`
// the same way `VanillaMinimapTracking` keeps its tracking config.
//
// Identity is by item GUID — `SaveEquipmentSet` snapshots the GUIDs
// currently worn, and `UseEquipmentSet` searches every player-owned
// container for those GUIDs at use time. The GUID search reads
// invMgr GUID arrays directly (same trick `C_Item.GetItemCount`
// uses), so bank items resolve without the bank window being open.
//
// Modern Classic Era's signatures use a numeric `iconFileID`; we
// accept icon paths (e.g. `"INV_Shield_06"`) instead because vanilla
// has no fileDataID system. Same string-or-default fallback semantic
// as 4.3.4's native `SaveEquipmentSet`.

#include "Data.h"
#include "Game.h"
#include "Locations.h"
#include "Offsets.h"
#include "Set.h"
#include "event/Custom.h"
#include "item/Swap.h"

#include <cstdint>
#include <cstring>

namespace EquipmentSet::Api {

namespace {

// Pulls `setID` off Lua stack[idx]. Accepts numbers only — modern
// signatures take numeric IDs, not names, for the by-ID functions.
// Returns 0 if missing/non-numeric (callers treat 0 as "no such set").
uint32_t ArgSetID(void *L, int idx) {
    if (!Game::Lua::IsNumber(L, idx))
        return 0;
    const double d = Game::Lua::ToNumber(L, idx);
    if (d < 1.0)
        return 0;
    return static_cast<uint32_t>(d);
}

int ArgInt(void *L, int idx, int fallback = 0) {
    if (!Game::Lua::IsNumber(L, idx))
        return fallback;
    return static_cast<int>(Game::Lua::ToNumber(L, idx));
}

const char *ArgString(void *L, int idx) {
    if (!Game::Lua::IsString(L, idx))
        return nullptr;
    return Game::Lua::ToString(L, idx);
}

int ResolveItemID(const uint8_t *cgItem) {
    if (cgItem == nullptr)
        return 0;
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        cgItem + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return static_cast<int>(*reinterpret_cast<const uint32_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_ITEM_ID));
}

void PushTableEntry(void *L, int key, double value) {
    Game::Lua::PushNumber(L, static_cast<double>(key));
    Game::Lua::PushNumber(L, value);
    Game::Lua::SetTable(L, -3);
}

// 1.12 has no concept of "uncomingable equipment-set support", so
// `CanUseEquipmentSets` is always true (we mirror what Classic Era
// 1.15 does — it returns true unconditionally there as well).
int __fastcall Script_CanUseEquipmentSets(void *L) {
    Game::Lua::PushBool(L, 1);
    return 1;
}

int __fastcall Script_GetNumEquipmentSets(void *L) {
    const auto &all = Data::All();
    Game::Lua::PushNumber(L, static_cast<double>(all.size()));
    return 1;
}

// Modern returns a numeric-keyed array of setIDs. Order matches the
// list's storage order (insertion order; no implicit alphabetical
// sort). Empty list returns an empty table.
int __fastcall Script_GetEquipmentSetIDs(void *L) {
    const auto &all = Data::All();
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);
    int key = 1;
    for (const auto &s : all) {
        Game::Lua::PushNumber(L, static_cast<double>(key++));
        Game::Lua::PushNumber(L, static_cast<double>(s.setID));
        Game::Lua::SetTable(L, -3);
    }
    return 1;
}

int __fastcall Script_GetEquipmentSetID(void *L) {
    const char *name = ArgString(L, 1);
    if (name == nullptr)
        return 0;
    const Set *s = Data::FindByName(name);
    if (s == nullptr)
        return 0;
    Game::Lua::PushNumber(L, static_cast<double>(s->setID));
    return 1;
}

// Returns the 9-tuple Blizzard ships:
//   name, icon, setID, isEquipped, numItems, numEquipped,
//   numInInventory, numMissing, numIgnored
//
// `numItems` excludes ignored slots — matches modern semantics where
// an ignored slot is "not part of the set", not "part of the set but
// unfilled". `isEquipped` is the boolean equivalent of
// `numEquipped == numItems`; modern flips it true even when missing
// items exist, as long as everything currently *resolvable* is in its
// target slot. We follow that — strict-equipped semantics break in
// useful cases (e.g. you're at a bank without your bank-stored cloak).
int __fastcall Script_GetEquipmentSetInfo(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    const Set *s = Data::FindByID(setID);
    if (s == nullptr)
        return 0;

    int numItems = 0;
    int numEquipped = 0;
    int numInInventory = 0;
    int numMissing = 0;
    int numIgnored = 0;
    for (int i = 0; i < SLOT_COUNT; ++i) {
        const uint64_t g = s->items[i];
        if (g == GUID_IGNORED) {
            ++numIgnored;
            continue;
        }
        if (g == GUID_EMPTY)
            continue;
        ++numItems;
        const int loc = Locations::FindGUID(g);
        if (loc == 0) {
            ++numMissing;
        } else if ((loc & LOC_BAGS) == 0 && (loc & LOC_BANK) == 0 &&
                   (loc & LOC_SLOT_MASK) == (i + 1)) {
            ++numEquipped;
        } else {
            ++numInInventory;
        }
    }

    Game::Lua::PushString(L, s->name.c_str());
    Game::Lua::PushString(L, s->icon.c_str());
    Game::Lua::PushNumber(L, static_cast<double>(s->setID));
    Game::Lua::PushBoolean(L, (numItems > 0 && numEquipped + numMissing == numItems &&
                               numInInventory == 0));
    Game::Lua::PushNumber(L, static_cast<double>(numItems));
    Game::Lua::PushNumber(L, static_cast<double>(numEquipped));
    Game::Lua::PushNumber(L, static_cast<double>(numInInventory));
    Game::Lua::PushNumber(L, static_cast<double>(numMissing));
    Game::Lua::PushNumber(L, static_cast<double>(numIgnored));
    return 9;
}

int __fastcall Script_GetIgnoredSlots(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    const Set *s = Data::FindByID(setID);
    if (s == nullptr)
        return 0;
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);
    int key = 1;
    for (int i = 0; i < SLOT_COUNT; ++i) {
        if (s->items[i] == GUID_IGNORED) {
            Game::Lua::PushNumber(L, static_cast<double>(key++));
            Game::Lua::PushNumber(L, static_cast<double>(i + 1));
            Game::Lua::SetTable(L, -3);
        }
    }
    return 1;
}

// Modern returns a hash table { [slot] = itemID } with one entry per
// non-empty, non-ignored slot. Missing items still get an entry — but
// since we can't recover an itemID from a GUID alone after the item
// has left the client (e.g. mailed away), missing slots are omitted.
int __fastcall Script_GetItemIDs(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    const Set *s = Data::FindByID(setID);
    if (s == nullptr)
        return 0;
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);
    for (int i = 0; i < SLOT_COUNT; ++i) {
        const uint64_t g = s->items[i];
        if (g == GUID_EMPTY || g == GUID_IGNORED)
            continue;
        const uint8_t *item = Locations::ResolveItemByGUID(g);
        const int itemID = ResolveItemID(item);
        if (itemID == 0)
            continue;
        PushTableEntry(L, i + 1, static_cast<double>(itemID));
    }
    return 1;
}

// Hash table { [slot] = locationCode } — same encoding modern uses,
// see Set.h's pack helpers. Ignored slots produce `LOC_IGNORED`,
// missing items produce `LOC_MISSING`, found items produce the
// packed `LOC_PLAYER|...|slot` code. Empty (uninitialized) slots
// don't appear in the table.
int __fastcall Script_GetItemLocations(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    const Set *s = Data::FindByID(setID);
    if (s == nullptr)
        return 0;

    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);
    for (int i = 0; i < SLOT_COUNT; ++i) {
        const uint64_t g = s->items[i];
        if (g == GUID_EMPTY)
            continue;
        int loc;
        if (g == GUID_IGNORED) {
            loc = LOC_IGNORED;
        } else {
            loc = Locations::FindGUID(g);
            if (loc == 0)
                loc = LOC_MISSING;
        }
        PushTableEntry(L, i + 1, static_cast<double>(loc));
    }
    return 1;
}

int __fastcall Script_CreateEquipmentSet(void *L) {
    const char *name = ArgString(L, 1);
    if (name == nullptr || name[0] == '\0') {
        Game::Lua::Error(L, "Usage: C_EquipmentSet.CreateEquipmentSet(name [, icon])");
        return 0;
    }
    const char *icon = ArgString(L, 2);
    const uint32_t setID = Data::Create(name, icon);
    if (setID == 0)
        return 0;
    Game::Lua::PushNumber(L, static_cast<double>(setID));
    return 1;
}

int __fastcall Script_SaveEquipmentSet(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    if (setID == 0) {
        Game::Lua::Error(L, "Usage: C_EquipmentSet.SaveEquipmentSet(setID [, icon])");
        return 0;
    }
    const char *icon = ArgString(L, 2);
    Data::SaveExisting(setID, icon);
    return 0;
}

int __fastcall Script_ModifyEquipmentSet(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    const char *newName = ArgString(L, 2);
    if (setID == 0 || newName == nullptr || newName[0] == '\0') {
        Game::Lua::Error(L,
                         "Usage: C_EquipmentSet.ModifyEquipmentSet(setID, newName)");
        return 0;
    }
    Data::Rename(setID, newName);
    return 0;
}

int __fastcall Script_DeleteEquipmentSet(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    if (setID == 0) {
        Game::Lua::Error(L, "Usage: C_EquipmentSet.DeleteEquipmentSet(setID)");
        return 0;
    }
    Data::Delete(setID);
    return 0;
}

int __fastcall Script_IgnoreSlotForSave(void *L) {
    const int slot = ArgInt(L, 1);
    Data::IgnoreSlot(slot);
    return 0;
}

int __fastcall Script_UnignoreSlotForSave(void *L) {
    const int slot = ArgInt(L, 1);
    Data::UnignoreSlot(slot);
    return 0;
}

int __fastcall Script_IsSlotIgnoredForSave(void *L) {
    const int slot = ArgInt(L, 1);
    Game::Lua::PushBool(L, Data::IsSlotIgnored(slot));
    return 1;
}

int __fastcall Script_ClearIgnoredSlotsForSave(void *L) {
    (void)L;
    Data::ClearIgnoredSlots();
    return 0;
}

// Returns true if any item in the set currently sits in the
// client-side "in-transaction" lock state (a pending swap, mail-
// attach, trade-attach, or cursor pickup that our equip dispatch
// would race with). Reads the per-CGItem instance flag at
// `OFF_ITEM_CLIENT_LOCK` bit 0 — same bit `C_Item.IsLocked` checks
// and the engine's own pickup/unlock paths manipulate.
int __fastcall Script_EquipmentSetContainsLockedItems(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    const Set *s = Data::FindByID(setID);
    if (s == nullptr)
        return 0;
    bool any = false;
    for (int i = 0; i < SLOT_COUNT && !any; ++i) {
        const uint64_t g = s->items[i];
        if (g == GUID_EMPTY || g == GUID_IGNORED)
            continue;
        const uint8_t *item = Locations::ResolveItemByGUID(g);
        if (item == nullptr)
            continue;
        const uint32_t flags = *reinterpret_cast<const uint32_t *>(
            item + Offsets::OFF_ITEM_CLIENT_LOCK);
        if (flags & Offsets::ITEM_CLIENT_LOCK_BIT)
            any = true;
    }
    Game::Lua::PushBool(L, any);
    return 1;
}

// Walks the set and dispatches one atomic server-side swap per item
// that isn't already in its target slot. Items currently in the main
// bank or bank bags are skipped silently — vanilla rejects equip
// from bank slots server-side, so the user needs to retrieve them
// first.
//
// Cycle handling: with cursor-based pickup+equip, swap cycles (e.g.
// rings A↔B wanting each other's slots) needed a temp-bag-slot
// shuffle. We now call the engine's own `FUN_INVENTORY_SWAP` per
// item, which sends opcode 0x10D / 0x10C through the engine's
// packet pipeline — an atomic server-side swap. A two-cycle resolves
// in one call (server swaps both slots from one packet); the cursor
// is never touched. See TODO #59 for the primitive's decoding and
// TODO #73 for the cycle-resolution rationale.
int __fastcall Script_UseEquipmentSet(void *L) {
    const uint32_t setID = ArgSetID(L, 1);
    const Set *s = Data::FindByID(setID);
    if (s == nullptr) {
        const int evt = Event::Custom::Lookup(kSwapFinishedEvent);
        if (evt >= 0)
            Event::Custom::Fire(evt, "%d%d", 0, static_cast<int>(setID));
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    // Fire PENDING right after the set-exists check, before any of
    // the swap work. Addon UI can use this to gate swap-in-progress
    // visuals.
    const int pendingEvt = Event::Custom::Lookup(kSwapPendingEvent);
    if (pendingEvt >= 0)
        Event::Custom::Fire(pendingEvt, "%d", static_cast<int>(setID));

    // If the cursor is holding anything, return it to its source slot
    // and unlock it server-side BEFORE our swap chain runs. The
    // engine's FUN_INVENTORY_SWAP ends each call with a cursor-state
    // cleanup that clears the LOCAL cursor view, but never sends the
    // server-side cancel-pickup packet — so a held item would stay
    // server-locked until relog. ClearCursor sends the right packet.
    // No-op when the cursor is idle (still fires one CURSOR_UPDATE,
    // negligible).
    Game::Lua::SetTop(L, 0);
    reinterpret_cast<int(__fastcall *)(void *)>(
        Offsets::FUN_SCRIPT_CLEAR_CURSOR)(L);

    // Snapshot the set's items into a local — Locations::FindGUID
    // can shuffle the Lua stack, and we've already passed validation.
    uint64_t guids[SLOT_COUNT];
    std::memcpy(guids, s->items, sizeof(guids));

    for (int i = 0; i < SLOT_COUNT; ++i) {
        const int targetSlot = i + 1;
        const uint64_t g = guids[i];
        if (g == GUID_EMPTY || g == GUID_IGNORED)
            continue;

        const int loc = Locations::FindGUID(g);
        if (loc == 0)
            continue; // missing — nothing to do

        // Already in its target slot? Skip.
        if ((loc & LOC_BAGS) == 0 && (loc & LOC_BANK) == 0 &&
            (loc & LOC_SLOT_MASK) == targetSlot)
            continue;

        // In bank (main or bank-bag) — server rejects equip-from-bank.
        if (loc & LOC_BANK)
            continue;

        const uint8_t *item = Locations::ResolveItemByGUID(g);
        if (item == nullptr)
            continue;

        if (loc & LOC_BAGS) {
            const int srcBag = (loc >> LOC_BAG_SHIFT) & 0xFF;
            const int srcSlot = loc & LOC_SLOT_MASK;
            Item::Swap::FromBag(item, srcBag, srcSlot, targetSlot);
        } else {
            const int srcSlot = loc & LOC_SLOT_MASK;
            Item::Swap::FromPaperdoll(item, srcSlot, targetSlot);
        }
    }

    const int evt = Event::Custom::Lookup(kSwapFinishedEvent);
    if (evt >= 0)
        Event::Custom::Fire(evt, "%d%d", 1, static_cast<int>(setID));

    Game::Lua::PushBoolean(L, 1);
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "CanUseEquipmentSets",
                                     &Script_CanUseEquipmentSets);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "GetNumEquipmentSets",
                                     &Script_GetNumEquipmentSets);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "GetEquipmentSetIDs",
                                     &Script_GetEquipmentSetIDs);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "GetEquipmentSetID",
                                     &Script_GetEquipmentSetID);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "GetEquipmentSetInfo",
                                     &Script_GetEquipmentSetInfo);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "GetIgnoredSlots",
                                     &Script_GetIgnoredSlots);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "GetItemIDs",
                                     &Script_GetItemIDs);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "GetItemLocations",
                                     &Script_GetItemLocations);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "CreateEquipmentSet",
                                     &Script_CreateEquipmentSet);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "SaveEquipmentSet",
                                     &Script_SaveEquipmentSet);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "ModifyEquipmentSet",
                                     &Script_ModifyEquipmentSet);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "DeleteEquipmentSet",
                                     &Script_DeleteEquipmentSet);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "IgnoreSlotForSave",
                                     &Script_IgnoreSlotForSave);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "UnignoreSlotForSave",
                                     &Script_UnignoreSlotForSave);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "IsSlotIgnoredForSave",
                                     &Script_IsSlotIgnoredForSave);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "ClearIgnoredSlotsForSave",
                                     &Script_ClearIgnoredSlotsForSave);
    Game::Lua::RegisterTableFunction("C_EquipmentSet",
                                     "EquipmentSetContainsLockedItems",
                                     &Script_EquipmentSetContainsLockedItems);
    Game::Lua::RegisterTableFunction("C_EquipmentSet", "UseEquipmentSet",
                                     &Script_UseEquipmentSet);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};
static const Event::Custom::AutoReserve _reserve{kEventName};
static const Event::Custom::AutoReserve _reservePending{kSwapPendingEvent};
static const Event::Custom::AutoReserve _reserveFinished{kSwapFinishedEvent};

} // namespace EquipmentSet::Api
