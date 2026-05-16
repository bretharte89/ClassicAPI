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

namespace Item::Link {

// Builds the fully-decorated per-instance item hyperlink for `cgItem`
// via the engine's internal link builder at `FUN_GAMETOOLTIP_BUILD_ITEM_LINK`
// (`0x0052AE00`). The returned string sits in the engine's long-lived
// scratch buffer; safe to `PushString` immediately (Lua copies the
// content). Returns nullptr if `cgItem` is null or the engine can't
// resolve the instance block. The link includes enchant ID, random
// suffix factor, unique ID, and any random-suffix-decorated name —
// same output `GetInventoryItemLink` / `GetContainerItemLink` would
// produce for the same slot.
const char *FromCGItem(const uint8_t *cgItem);

} // namespace Item::Link
