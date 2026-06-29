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

#pragma once

#include "Set.h"

#include <cstdint>

namespace EquipmentSet::Locations {

// Searches every location the local player owns for an item whose
// GUID matches `targetGuid`, and returns the packed location code
// (see Set.h's `PackEquipped`/`PackBag`/`PackMainBank`/`PackBankBag`).
// Returns 0 if the GUID isn't found anywhere.
//
// Coverage — ALL locations are read via the player invMgr's flat
// GUID array at `+0x04` and (for bag contents) via each bag's own
// invMgr reached through the bag's vtable `+0x10`. None of this
// goes through the engine's `GetItemBySlot` / `PackBagSlot` path,
// so:
//   - no Lua stack is touched (safe to call from any callback)
//   - bank items resolve without the bank window ever being open
//     (the GUIDs are populated from server data at login; only
//     `GetItemBySlot` is gated by `VAR_BANK_GATE_GUID`, the
//     underlying data is always present — same trick
//     `C_Item.GetItemCount` uses)
int FindGUID(uint64_t targetGuid);

// Resolves a GUID to a `CGItem*` via the engine's GUID→CObject
// resolver (`FUN_OBJECT_RESOLVE_BY_GUID`). Returns null if the item
// isn't known to the client. Side-effect free.
const uint8_t *ResolveItemByGUID(uint64_t guid);

// Copies the current paperdoll GUIDs into `out[0..SLOT_COUNT-1]`,
// where `out[i]` is the GUID at 1-based equipment slot `i+1`.
// Empty / unequipped slots write 0. Reads from the player invMgr's
// flat GUID array directly — no Lua stack, no engine state changes.
//
// Used by `UseEquipmentSet` to seed a "virtual paperdoll" model that
// stays current across in-flight swaps. The engine's CGItem state
// lags behind packet sends (server has to ACK each swap), so any
// subsequent `FindGUID` after the first packet would return stale
// pre-swap locations. Tracking moves locally bypasses that lag.
void SnapshotPaperdoll(uint64_t *out);

} // namespace EquipmentSet::Locations
