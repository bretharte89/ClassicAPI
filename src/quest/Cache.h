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

namespace Quest::Cache {

// Calls the engine's quest cache `_GetRecord` at `FUN_DBCACHE_QUEST_GET_RECORD`.
//
// With `fireRequest=false`, performs only the hash-table lookup. Returns
// the cached data block (`entry+0x18` per the engine's accessor) if the
// record is fully loaded, or nullptr if not cached / not yet loaded.
//
// With `fireRequest=true`, queues a CMSG_QUEST_QUERY when the data isn't
// already loaded and registers the supplied `callback` (called with
// `userData` and a 0/1 success byte once SMSG_QUEST_QUERY_RESPONSE
// lands). When the data is already loaded, returns immediately and does
// NOT invoke the callback — same as the item cache. Caller is responsible
// for any "synthesise success=1 ourselves on cache hit" logic.
//
// Internal layout of the returned data block (offsets within the block):
//   +0x9C  inline `char title[N]` — null-terminated, locale-applied
const uint8_t *Lookup(uint32_t questID, void *callback, void *userData);

// Convenience: pure cache-state probe with no request side effect.
inline const uint8_t *Peek(uint32_t questID) {
    return Lookup(questID, nullptr, nullptr);
}

// Field offsets within the data block returned by `Lookup`.
constexpr int OFF_TITLE = 0x9C;
// Reward and choice item arrays — up to 4 reward and 6 choice
// itemIDs stored inline (4 bytes each). 0 in any slot means
// "no item in this slot". Used by `GetQuestLogItemLink` /
// `GetQuestLogItemID`.
constexpr int OFF_REWARD_ITEMS = 0x3C;
constexpr int OFF_CHOICE_ITEMS = 0x5C;
constexpr int REWARD_COUNT = 4;
constexpr int CHOICE_COUNT = 6;
// Objective arrays — four uint32[4] parallel slabs carrying
// SMSG_QUEST_QUERY_RESPONSE's RequiredNPCOrGo / Count / Item / Count
// fields verbatim. NPC-or-GO is **signed**: positive = creature
// entry, negative = gameobject entry (engine strips the sign bit
// before the cache lookup). Empty slots hold 0 — `Script_GetQuestLogLeaderBoard`
// at 0x004E0110 skips zero entries when matching its 1-based objIdx
// arg, walking the NPC/GO array first then the item array (total max 8
// objectives). Verified by decoding the four `[esi + edi*4 + 0x14XC]`
// reads and the cache-instance routing in that function:
//   - 0xC0E354 (creature cache) for positive 0x149C → "monster"
//   - 0xC0E318 (gameobject cache) for negative 0x149C → "object"
//   - 0xC0E2A0 (item cache) for 0x14BC → "item"
constexpr int OFF_REQUIRED_NPC_OR_GO = 0x149C;        // int32[4]
constexpr int OFF_REQUIRED_NPC_OR_GO_COUNT = 0x14AC;  // uint32[4]
constexpr int OFF_REQUIRED_ITEM = 0x14BC;             // uint32[4]
constexpr int OFF_REQUIRED_ITEM_COUNT = 0x14CC;       // uint32[4]
constexpr int OBJECTIVE_COUNT = 4;

} // namespace Quest::Cache
