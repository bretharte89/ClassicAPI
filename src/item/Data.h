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

#include <cstdint>

namespace Item::Data {

// Warm the item-stats cache for `itemID`. If already cached this is a
// no-op (caller can read item info immediately). If not cached,
// triggers the engine's `SMSG_ITEM_QUERY_SINGLE_RESPONSE` round-trip
// and fires `GET_ITEM_INFO_RECEIVED` when the response arrives.
//
// Used by:
//   - Our `Script_GetItemInfo` hook (so 1.12's `GetItemInfo` becomes
//     transparent like 5.x+)
//   - `GameTooltip:SetItemByID` (so item tooltips warm the cache when
//     opened cold)
//
// Note: this is the **implicit** warmup path — fires GET_ITEM_INFO_RECEIVED
// only. For modern explicit-request semantics (fires
// ITEM_DATA_LOAD_RESULT), use `C_Item.RequestLoadItemData(ByID)` from
// Lua.
void WarmCache(uint32_t itemID);

// Hook for `Script_GetItemInfo` (`0x0048E070`). Pre-warms the cache
// from arg 1 (number or `"item:NNN..."` string), then runs the
// original. The original still returns nil this call for cache
// misses, but the query is now in flight; subsequent calls return
// valid data and `GET_ITEM_INFO_RECEIVED` fires when ready.
using Script_GetItemInfo_t = int(__fastcall *)(void *L);
extern Script_GetItemInfo_t Script_GetItemInfo_o;
int __fastcall Script_GetItemInfo_h(void *L);

} // namespace Item::Data
