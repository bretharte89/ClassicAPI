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

namespace Cursor::Info {

// True if a GUID-bearing item is currently on the cursor —
// `VAR_CURSOR_ITEM_GUID != 0`, the same globals `GetCursorInfo`'s bag-item
// path reads and the exact signal the engine's item-pickup path guards on.
// Used to gate cursor pickups (don't clobber a held item).
bool HasItem();

} // namespace Cursor::Info
