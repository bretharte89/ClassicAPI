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

#pragma once

#include <cstdint>
#include <string>

namespace EquipmentSet {

// Character-pane slots indexed 1..19 (HEAD..TABARD); we store them as
// a fixed-size array with the slot1=index 0 convention so iteration
// is `for (int i = 0; i < SLOT_COUNT; ++i) slot = i+1;`.
constexpr int SLOT_COUNT = 19;

// Two reserved sentinels in the items[] array:
//   0 = slot was empty when set was saved
//   1 = slot is intentionally ignored (`IgnoreSlotForSave` was set at
//       save time). Matches the 4.3.4 native encoding where the engine
//       stored a (lo=1, hi=0) tuple in the same field. Real GUIDs are
//       always ≥ 2^32 in practice (the high dword has the type bits),
//       so the collision risk is theoretical only.
constexpr uint64_t GUID_EMPTY = 0;
constexpr uint64_t GUID_IGNORED = 1;

struct Set {
    uint32_t setID;
    std::string name;
    std::string icon;
    uint64_t items[SLOT_COUNT];
};

// Engine convention for SaveEquipmentSet-style "no icon given": modern
// FrameXML defaults to the question-mark icon when callers pass nil.
inline const char *DefaultIcon() { return "INV_Misc_QuestionMark"; }

// Vanilla 1.12 engine accepts equipment-set names up to 32 chars
// (1.15.x is also 32). 4.3.4 used the same buffer size.
constexpr int MAX_NAME_LEN = 31;

// Hard cap matching 4.3.4 behavior. Modern Classic Era (1.15) raised
// this to 30; we follow the 4.3.4 number so legacy addons that show
// "X / 10" don't surprise users.
constexpr int MAX_SETS = 10;

// `GetItemLocations` encoding — matches Blizzard FrameXML's
// `EquipmentManager_UnpackLocation` (constants are in
// `Constants.lua`: `ITEM_INVENTORY_LOCATION_*` and
// `ITEM_INVENTORY_BAG_BIT_OFFSET`). Verified against the Classic Era
// 1.15.x install at `_classic_/BlizzardInterfaceCode/`.
//
//   bit 20 (0x00100000)  PLAYER — local player paperdoll/bags
//   bit 21 (0x00200000)  BAGS   — inside a bag (player or bank)
//   bit 22 (0x00400000)  BANK   — main bank or bank bag
//   bits 0..7            slot   — 1-based slot within the container
//   bits 8..15           bag    — bag index (only when BAGS bit set)
//
// Composition by source — note PLAYER and BANK are **mutually
// exclusive** in the encoding (unpack uses `if player elseif bank`,
// so setting both leaves an unaccounted-for BANK bit in the slot
// remainder):
//   equipped (paperdoll)  → PLAYER         | slot
//   backpack / player bag → PLAYER | BAGS  | (bag << 8)         | slot   (bag 0..4)
//   main bank             → BANK           | slot                          (slot 1..28)
//   bank bag              → BANK   | BAGS  | ((bagID - 4) << 8) | slot   (bagID 5..10)
//
// The `bagID - 4` shift on bank bags lets unpack store an internal
// 1..6 in the field, then add `ITEM_INVENTORY_BANK_BAG_OFFSET` (= 4)
// back at unpack time to recover bag 5..10. Same scheme the engine
// uses natively.
constexpr int LOC_PLAYER = 0x00100000;
constexpr int LOC_BAGS = 0x00200000;
constexpr int LOC_BANK = 0x00400000;
constexpr int LOC_SLOT_MASK = 0xFF;
constexpr int LOC_BAG_SHIFT = 8;
constexpr int BANK_BAG_OFFSET = 4;

inline int PackEquipped(int slot) { return LOC_PLAYER | (slot & LOC_SLOT_MASK); }
inline int PackBag(int bag, int slot) {
    return LOC_PLAYER | LOC_BAGS | ((bag & 0xFF) << LOC_BAG_SHIFT) |
           (slot & LOC_SLOT_MASK);
}
inline int PackMainBank(int slot) {
    return LOC_BANK | (slot & LOC_SLOT_MASK);
}
inline int PackBankBag(int bagID, int slot) {
    const int internalBag = bagID - BANK_BAG_OFFSET; // 5..10 → 1..6
    return LOC_BANK | LOC_BAGS | ((internalBag & 0xFF) << LOC_BAG_SHIFT) |
           (slot & LOC_SLOT_MASK);
}

// Modern's `GetItemLocations` returns -1 in the slot's map entry to
// signal "this item belonged to the set but we can't find it now".
constexpr int LOC_MISSING = -1;
// Modern returns 1 for ignored slots (matches `GUID_IGNORED` value —
// coincidence, not load-bearing).
constexpr int LOC_IGNORED = 1;

// Custom event names. All three are reserved at module-init time
// via `Event::Custom::AutoReserve` in `Api.cpp`.
//
//   EQUIPMENT_SETS_CHANGED   — no payload; fires on any mutation
//                              (create / save / modify / delete /
//                              ignore-slot change). Addon UI re-reads
//                              its list / button state on this.
//   EQUIPMENT_SWAP_PENDING   — `(setID: int)`; fires at the START of
//                              `UseEquipmentSet`, right after the set
//                              exists check passes. Modern addons
//                              gate "swap-in-progress" UI state on
//                              this (grey out the set button until
//                              FINISHED arrives).
//   EQUIPMENT_SWAP_FINISHED  — `(success: bool, setID: int)`; fires
//                              at the end of `UseEquipmentSet`.
//                              `success` is false when the set
//                              doesn't exist, true otherwise (we
//                              don't probe whether each individual
//                              pickup→equip pair actually completed,
//                              matching modern's "we dispatched"
//                              definition of success).
constexpr const char *kEventName = "EQUIPMENT_SETS_CHANGED";
constexpr const char *kSwapPendingEvent = "EQUIPMENT_SWAP_PENDING";
constexpr const char *kSwapFinishedEvent = "EQUIPMENT_SWAP_FINISHED";

} // namespace EquipmentSet
