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

namespace Item::Openable {

// True iff the `ItemStats_C` record for `itemID` has the openable flag
// set (`Flags & 0x4`). Reads from the client-side item cache; returns
// false (the safe default for "I can't see openable semantics on this
// item") when the item is uncached. Item-type intrinsic — same answer
// for every instance of a given item, so container/equipment variants
// just resolve their context to an itemID and forward here.
bool IsItemIDOpenable(uint32_t itemID);

} // namespace Item::Openable
