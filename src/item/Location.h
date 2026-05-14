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

namespace Item::Location {

// Resolves an item-location arg at Lua stack index `locIdx` to a `CGItem *`
// pointer (returned as a raw byte pointer for use with `OFF_*` offsets).
//
// Accepts:
//   - location tables: `{ equipmentSlotIndex = N }` or
//     `{ bagID = B, slotIndex = S }` — same shapes the modern
//     `ItemLocation` mixin uses.
//   - GUID strings: `"0xHHHHHHHHLLLLLLLL"` — the form
//     `C_Item.GetItemGUID` returns. The GUID is looked up by walking
//     equipment slots 1..19 + bags 0..4; bank and keyring are not
//     walked.
//
// Returns `nullptr` when the value isn't a valid location arg, the
// table doesn't carry recognizable fields, the GUID is malformed, or
// the resolved slot is empty / the GUID isn't resident. Does NOT
// raise a Lua error — callers should validate the argument type
// beforehand with `IsLocationArg` if they want a typed error.
//
// Both paths may stomp the Lua stack (the bag-form table uses
// `PackBagSlot`; the GUID path iterates bag slots via the same
// helper). Only safe to call from inside a Lua callback.
const uint8_t *Resolve(void *L, int locIdx);

// Type-check helper: returns true if the value at `idx` could be a
// valid location arg (a table or a string). Used by every C_Item.*
// function that takes a location, to validate before calling
// `Resolve`. Does not dereference the value.
bool IsLocationArg(void *L, int idx);

// Resolves a `(bagID, slotIndex)` pair directly to a `CGItem *`. Used by
// the positional-arg C_Container.* APIs that don't go through the
// ItemLocation table shape. Stomps the Lua stack — same caveat as
// `Resolve()` above.
const uint8_t *ResolveBag(void *L, int bagID, int slotIndex);

// Resolves a 1-based character-pane equipment slot directly to a
// `CGItem *`. Used by the positional-arg `GetInventoryItemDurability`
// (and any future per-slot getter) that doesn't take an ItemLocation
// table. Player-only — walks the local player's private inventory
// manager, same path the `{equipmentSlotIndex=N}` table form uses.
// Returns nullptr for out-of-range slots or empty equipment.
const uint8_t *ResolveEquipmentSlot(int slot1Based);

// GUID-walk result. `equipmentSlotIndex != 0` means the item was
// found in a character-pane equipment slot; otherwise it's in
// `bagID`/`slotIndex`. `item` always points to the found CGItem
// when the function returns true.
struct ByGUIDResult {
    int equipmentSlotIndex; // 1..19, or 0 if bag form
    int bagID;
    int slotIndex;
    const uint8_t *item;
};

// Walks the local player's inventory (equipment 1..19 + bags 0..4)
// looking for a CGItem with a matching GUID. Returns false when the
// GUID isn't found, true (and fills `*out`) on a hit. Stomps the
// Lua stack — only safe from inside a Lua callback.
bool FindByGUID(void *L, uint64_t guid, ByGUIDResult *out);

// Parses the `"0xHHHHHHHHLLLLLLLL"` GUID string format
// `C_Item.GetItemGUID` returns. Strict: requires exactly `0x` prefix
// + 16 case-insensitive hex digits. Returns false (and leaves `*out`
// untouched) on malformed input.
bool ParseGUIDString(const char *s, uint64_t *out);

} // namespace Item::Location
