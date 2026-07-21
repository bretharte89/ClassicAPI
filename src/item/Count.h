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

namespace Item::Count {

// Total plain stack count of `itemID` across the player's equipped slots
// and bags 0..4 (no bank, no charge multiplier) — the "carried on me"
// count. Needs the Lua state `L`: the bag walk uses the engine's slot
// packer, which reads and clears the Lua stack, so the caller must have
// already consumed any stack arguments it still needs before calling.
int InInventory(void *L, int itemID);

} // namespace Item::Count
