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

#include <cstddef>
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

// Builds a basic itemID-only hyperlink (`"|cffRRGGBB|Hitem:N:0:0:0|h[Name]|h|r"`)
// into `out` from the cached ItemStats record for `itemID`. No
// enchant / random-suffix decoration — use this when the caller has
// only an itemID and no live CGItem (mail inbox attachments, cursor
// drag-source items, vendor roster previews). Returns true on
// success, false if the item isn't cached or the buffer is too small.
//
// `outSize` should be at least 256 to comfortably fit the longest
// possible link (`item:` payload + colored prefix + bracketed name).
bool BasicFromItemID(uint32_t itemID, char *out, size_t outSize);

} // namespace Item::Link
