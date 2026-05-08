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

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"

#include <cstdint>

namespace Quest::Data {

// Engine cache `_GetRecord` for quest static info, mirroring the
// `Item::Data` shape but with one extra "context" arg that the engine
// uses for its own bookkeeping (we pass an 8-byte zero block; see
// `Script_GetQuestLogQuestText` at `0x004DFF20` for the same usage).
//
//   __thiscall(this=cache, questID, &outBuf, callback, userData, unused)
//
// `outBuf` provides 8 bytes that get copied into the pending entry at
// +0x10/+0x14 — engine bookkeeping, not exposed to the callback. The
// callback receives `userData` (arg4) as its first argument.
using GetQuestRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t questID,
                                                       void *outBuf, void *callback,
                                                       void *userData, int unused);

static constexpr const char *kQuestDataLoadResult = "QUEST_DATA_LOAD_RESULT";

// Same `__stdcall(userData, success)` shape as `Item::Data::ItemLoadCallback`.
// The engine's invocation pattern at 0x00562EEB et al. is:
//   mov ecx, [entry+0x18]   ; userData (= arg4 we passed to _GetRecord)
//   push 1                  ; success (1/0)
//   push ecx                ; userData
//   call [entry+0x08]       ; this callback
// `ret 8` cleans both args. We encode the questID into `userData` at
// request time so the callback knows which quest completed.
static void __stdcall QuestLoadCallback(void *userData, int success) {
    const int slot = Event::Custom::Register(kQuestDataLoadResult);
    if (slot < 0)
        return;
    const auto questID =
        static_cast<int>(reinterpret_cast<uintptr_t>(userData));
    Event::Custom::Fire_DD(slot, questID, success ? 1 : 0);
}

// Calls the quest cache `_GetRecord`. With `fireRequest=false`, performs
// only the hash-table lookup; returns the cached record (`entry+0x18`) if
// the data is fully loaded (engine checks `[entry+0x18F8]`), or nullptr
// otherwise. With `fireRequest=true`, queues a CMSG_QUEST_QUERY when the
// data isn't already loaded — engine fills the cache asynchronously and
// invokes our callback when the SMSG_QUEST_QUERY_RESPONSE lands.
//
// **Cache-hit-already-loaded path does NOT invoke the callback** — the
// engine returns `entry+0x18` directly. Callers that want notification
// regardless of cache state must fire `QUEST_DATA_LOAD_RESULT` themselves
// for that branch (see `RequestAndMaybeNotify`).
static const uint8_t *CacheFetch(uint32_t questID, bool fireRequest) {
    auto fn = reinterpret_cast<GetQuestRecord_t>(Offsets::FUN_DBCACHE_QUEST_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_QUEST_CACHE);
    uint64_t outBuf = 0; // 8-byte zero block (matches engine's own pattern)
    void *callback = fireRequest ? reinterpret_cast<void *>(&QuestLoadCallback) : nullptr;
    void *userData =
        fireRequest ? reinterpret_cast<void *>(static_cast<uintptr_t>(questID)) : nullptr;
    return fn(cache, questID, &outBuf, callback, userData, 0);
}

static void RequestAndMaybeNotify(uint32_t questID) {
    const bool wasCached = (CacheFetch(questID, false) != nullptr);
    CacheFetch(questID, true);
    if (wasCached) {
        const int slot = Event::Custom::Register(kQuestDataLoadResult);
        if (slot >= 0)
            Event::Custom::Fire_DD(slot, static_cast<int>(questID), 1);
    }
}

static int __fastcall Script_RequestLoadQuestByID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: C_QuestLog.RequestLoadQuestByID(questID)");
        return 0;
    }
    const int questID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (questID <= 0)
        return 0;
    RequestAndMaybeNotify(static_cast<uint32_t>(questID));
    return 0;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_QuestLog", "RequestLoadQuestByID",
                                     &Script_RequestLoadQuestByID);

    // Seed the event-name cache so `Event::Custom::RetryAll` (driven by
    // our hook on `Frame::RegisterEvent`) can claim a slot once the
    // engine's event table is populated. Same pattern as Item::Data.
    Event::Custom::Register(kQuestDataLoadResult);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Quest::Data
