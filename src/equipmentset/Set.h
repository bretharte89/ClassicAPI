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

// `GetItemLocations` encoding — matches Classic Era 1.15.x's
// `EquipmentManager_UnpackLocation` so addon code that decodes the
// returned values works unchanged.
//
//   bit 8  (0x100)  PLAYER — the local player's storage (always 1 here
//                            since we don't expose other players' sets)
//   bit 9  (0x200)  BANK   — main bank or a bank bag
//   bit 10 (0x400)  BAGS   — inside a bag (player or bank)
//   bits 0..7       slot   — 1-based slot within the container
//   bits 16..23     bag    — bag ID (0..4 player, 5..10 bank, ignored
//                            when BAGS bit is clear)
//
// Composition by source:
//   equipped (paperdoll)  → PLAYER                       | slot
//   backpack / player bag → PLAYER | BAGS                | (bag<<16) | slot
//   main bank             → PLAYER | BANK                | slot
//   bank bag              → PLAYER | BANK | BAGS         | (bag<<16) | slot
constexpr int LOC_PLAYER = 0x100;
constexpr int LOC_BANK = 0x200;
constexpr int LOC_BAGS = 0x400;
constexpr int LOC_SLOT_MASK = 0xFF;
constexpr int LOC_BAG_SHIFT = 16;

inline int PackEquipped(int slot) { return LOC_PLAYER | (slot & LOC_SLOT_MASK); }
inline int PackBag(int bag, int slot) {
    return LOC_PLAYER | LOC_BAGS | ((bag & 0xFF) << LOC_BAG_SHIFT) |
           (slot & LOC_SLOT_MASK);
}
inline int PackMainBank(int slot) {
    return LOC_PLAYER | LOC_BANK | (slot & LOC_SLOT_MASK);
}
inline int PackBankBag(int bag, int slot) {
    return LOC_PLAYER | LOC_BANK | LOC_BAGS | ((bag & 0xFF) << LOC_BAG_SHIFT) |
           (slot & LOC_SLOT_MASK);
}

// Modern's `GetItemLocations` returns -1 in the slot's map entry to
// signal "this item belonged to the set but we can't find it now".
constexpr int LOC_MISSING = -1;
// Modern returns 1 for ignored slots (matches `GUID_IGNORED` value —
// coincidence, not load-bearing).
constexpr int LOC_IGNORED = 1;

// Custom event name. Modern fires `EQUIPMENT_SETS_CHANGED` with no
// payload on any mutation (create / save / modify / delete / ignore-
// slot change). We reserve the slot via `Event::Custom::AutoReserve`
// in `Api.cpp`.
constexpr const char *kEventName = "EQUIPMENT_SETS_CHANGED";

} // namespace EquipmentSet
