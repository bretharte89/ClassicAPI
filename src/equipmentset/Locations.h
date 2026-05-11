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

#include "Set.h"

#include <cstdint>

namespace EquipmentSet::Locations {

// Searches every location the local player owns for an item whose
// GUID matches `targetGuid`, and returns the packed location code
// (see Set.h's `PackEquipped`/`PackBag`/`PackMainBank`/`PackBankBag`).
// Returns 0 if the GUID isn't found anywhere.
//
// Coverage:
//   - paperdoll (slots 1..19)         — always available
//   - backpack (bag 0, slots 1..16)   — always available
//   - player bags 1..4                — always available
//   - main bank (24 slots)            — always (raw GUID array read)
//   - bank bag slots (bank bag items) — always (raw GUID array read)
//   - bank bag CONTENTS (bags 5..10)  — only while bank window is open;
//                                        the engine's bag invMgr is
//                                        populated lazily and reading
//                                        before that returns nothing
//
// `L` is required because the engine's `PackBagSlot` reads bag/slot
// args off the Lua stack; we shape the stack inside this function.
// Callers must own the stack; we leave it cleared on return.
int FindGUID(void *L, uint64_t targetGuid);

// Resolves a GUID to a `CGItem*` via the engine's GUID→CObject
// resolver (`FUN_OBJECT_RESOLVE_BY_GUID`). Returns null if the item
// isn't known to the client. Side-effect free.
const uint8_t *ResolveItemByGUID(uint64_t guid);

} // namespace EquipmentSet::Locations
