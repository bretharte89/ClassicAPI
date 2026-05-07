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

// Resolves an item-location table at Lua stack index `locIdx` to a `CGItem *`
// pointer (returned as a raw byte pointer for use with `OFF_*` offsets).
//
// Accepts the same shapes the modern `ItemLocation` mixin uses:
//   { equipmentSlotIndex = N }   1-based slot index, character pane
//   { bagID = B, slotIndex = S } both required, bag/slot tuple
//
// Returns `nullptr` when the stack value isn't a table, the table doesn't
// carry a valid location, or the resolved slot is empty. Does NOT raise a
// Lua error — callers should validate the argument type beforehand if they
// want a typed error message.
//
// Walking the bag path stomps the Lua stack (a requirement of the engine's
// `PackBagSlot` helper, which reads its inputs from stack[1] and stack[2]).
// Only safe to call from inside a Lua callback.
const uint8_t *Resolve(void *L, int locIdx);

} // namespace Item::Location
