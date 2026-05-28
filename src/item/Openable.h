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

// Pushes the openable bool for `itemID` and returns 1, OR pushes
// nothing and returns 0 when the item is uncached / itemID is
// invalid. Caller can `return PushIsItemOpenable(...)` from a Lua
// C function; pushing 0 surfaces as `nil` to the Lua caller, which
// distinguishes "data unknown" from "definitely not openable".
//
// Reads `ItemStats_C.Flags & 0x4` from the client-side item cache.
// Fires a background `SMSG_ITEM_QUERY_SINGLE` for uncached items so
// a follow-up call after `GET_ITEM_INFO_RECEIVED` resolves correctly.
int PushIsItemOpenable(void *L, uint32_t itemID);

} // namespace Item::Openable
