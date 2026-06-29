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

#include "Cache.h"

#include "Offsets.h"

namespace Quest::Cache {

// Engine cache `_GetRecord` for quest static info. Same five-arg shape as
// the item cache. `outBuf` is engine bookkeeping (8 bytes copied into the
// pending entry); we pass an 8-byte zero block to mirror the engine's own
// usage in `Script_GetQuestLogQuestText` at `0x004DFF20`.
//
//   __thiscall(this=cache, questID, &outBuf, callback, userData, unused)
using GetQuestRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t questID,
                                                       void *outBuf, void *callback,
                                                       void *userData, int unused);

const uint8_t *Lookup(uint32_t questID, void *callback, void *userData) {
    auto fn = reinterpret_cast<GetQuestRecord_t>(Offsets::FUN_DBCACHE_QUEST_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_QUEST_CACHE);
    uint64_t outBuf = 0;
    return fn(cache, questID, &outBuf, callback, userData, 0);
}

} // namespace Quest::Cache
