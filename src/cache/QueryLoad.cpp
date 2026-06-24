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

#include "cache/QueryLoad.h"

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"
#include "tick/WorldTick.h"

#include <cstdint>

namespace Cache::QueryLoad {

namespace {

// Generic cache `_GetRecord` (FUN_CACHE_GET_RECORD). callback=nullptr →
// pure hash-table peek (returns the loaded data block or nullptr, no
// query). Non-null callback → also fires the network query on a miss and
// stores `(callback, userData)` on the entry's pending list. dedup=1 so
// repeat fetches for the same id before the response lands don't queue
// the callback twice.
using GetCacheRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t id,
                                                       const uint64_t *guid, void *callback,
                                                       void *userData, char dedup);

// Generic response parser we hook (FUN_CACHE_QUERY_RESPONSE).
using CacheResponse_t = void(__thiscall *)(void *cache, void *packetReader, int flag);

// Engine callback shape (__stdcall, ret 8) — see Item::Data. We only
// pass it to make `_GetRecord` send the query (the trigger is
// `callback != NULL`); the actual completion is observed via the
// response hook + tick sweep, so the callback itself does nothing.
void __stdcall NoOpCallback(void * /*userData*/, int /*success*/) {}

const uint8_t *CacheFetch(void *cache, uint32_t id, void *callback) {
    auto fn = reinterpret_cast<GetCacheRecord_t>(Offsets::FUN_CACHE_GET_RECORD);
    const uint64_t zeroGuid = 0;
    void *userData =
        callback ? reinterpret_cast<void *>(static_cast<uintptr_t>(id)) : nullptr;
    return fn(cache, id, &zeroGuid, callback, userData, 1);
}

struct Pending {
    uint32_t id;
    int ticksWaiting;
};

constexpr int kMaxCaches = 8;
constexpr int kMaxPending = 256;
constexpr int kMaxWaitTicks = 1200; // ~ a few minutes of WorldTicks → give up, fire failure

struct Registered {
    void *cache;
    const char *eventName;
    Pending pending[kMaxPending];
    int pendingCount;
};

Registered g_caches[kMaxCaches];
int g_cacheCount = 0;

Registered *Find(void *cache) {
    for (int i = 0; i < g_cacheCount; ++i) {
        if (g_caches[i].cache == cache)
            return &g_caches[i];
    }
    return nullptr;
}

void Fire(const Registered *r, uint32_t id, bool success) {
    Event::Custom::FireIdSuccess(Event::Custom::Lookup(r->eventName),
                                 static_cast<int>(id), success);
}

int FindPending(const Registered *r, uint32_t id) {
    for (int i = 0; i < r->pendingCount; ++i) {
        if (r->pending[i].id == id)
            return i;
    }
    return -1;
}

void RemovePendingAt(Registered *r, int idx) {
    --r->pendingCount;
    if (idx != r->pendingCount)
        r->pending[idx] = r->pending[r->pendingCount];
}

// Fire + remove any pending id whose entry is now loaded.
void Sweep(Registered *r) {
    for (int i = 0; i < r->pendingCount;) {
        if (CacheFetch(r->cache, r->pending[i].id, nullptr) != nullptr) {
            const uint32_t id = r->pending[i].id;
            RemovePendingAt(r, i);
            Fire(r, id, true);
            continue;
        }
        ++i;
    }
}

// Response-handler hook. The engine fills entries from
// `SMSG_*_QUERY_RESPONSE` and walks each entry's callback list inside the
// original; afterward we sweep the pending set for the cache that just
// got a response and fire the completion event for anything now-loaded.
// `__thiscall` mimicked as `__fastcall(ecx, edx_unused, …)` per the
// codebase convention.
CacheResponse_t g_origResponse = nullptr;

void __fastcall Response_h(void *cache, void * /*edx*/, void *packetReader, int flag) {
    using Trampoline_t = void(__fastcall *)(void *cache, void *edx, void *packetReader, int flag);
    reinterpret_cast<Trampoline_t>(g_origResponse)(cache, nullptr, packetReader, flag);

    // Only act on caches we track; other caches route through this same
    // parser and are simply ignored.
    Registered *r = Find(cache);
    if (r != nullptr)
        Sweep(r);
}

const Game::HookAutoRegister _hookResponse{
    Offsets::FUN_CACHE_QUERY_RESPONSE, reinterpret_cast<void *>(&Response_h),
    reinterpret_cast<void **>(&g_origResponse)};

// Timeout + safety net. The hook covers the happy path; this only fires
// failure for ids that never resolve (lost query / server silent) after
// `kMaxWaitTicks`, and re-checks loaded as a backstop in case a fill
// arrived through a path the hook didn't observe.
void OnWorldTick() {
    for (int c = 0; c < g_cacheCount; ++c) {
        Registered *r = &g_caches[c];
        for (int i = 0; i < r->pendingCount;) {
            if (CacheFetch(r->cache, r->pending[i].id, nullptr) != nullptr) {
                const uint32_t id = r->pending[i].id;
                RemovePendingAt(r, i);
                Fire(r, id, true);
                continue;
            }
            if (r->pending[i].ticksWaiting >= kMaxWaitTicks) {
                const uint32_t id = r->pending[i].id;
                RemovePendingAt(r, i);
                Fire(r, id, false);
                continue;
            }
            ++r->pending[i].ticksWaiting;
            ++i;
        }
    }
}

const Tick::WorldTick::AutoSubscribe _tickSub{&OnWorldTick};

} // namespace

void Register(void *cacheInstance, const char *eventName) {
    if (cacheInstance == nullptr || eventName == nullptr)
        return;
    if (Find(cacheInstance) != nullptr) // idempotent (safe across /reload)
        return;
    if (g_cacheCount >= kMaxCaches)
        return;
    g_caches[g_cacheCount].cache = cacheInstance;
    g_caches[g_cacheCount].eventName = eventName;
    g_caches[g_cacheCount].pendingCount = 0;
    ++g_cacheCount;
}

bool IsCached(void *cacheInstance, uint32_t id) {
    return CacheFetch(cacheInstance, id, nullptr) != nullptr;
}

bool RequestLoad(void *cacheInstance, uint32_t id) {
    Registered *r = Find(cacheInstance);
    if (r == nullptr || id == 0)
        return false;

    // Already cached — fire success synchronously (modern RequestLoad
    // contract: the completion event fires even on a cache hit).
    if (CacheFetch(cacheInstance, id, nullptr) != nullptr) {
        Fire(r, id, true);
        return true;
    }
    // Already in flight — one completion event per id.
    if (FindPending(r, id) >= 0)
        return true;
    if (r->pendingCount >= kMaxPending)
        return false;

    // Send the query (non-null callback is the trigger), then track until
    // the response hook / tick fires the event.
    CacheFetch(cacheInstance, id, reinterpret_cast<void *>(&NoOpCallback));
    r->pending[r->pendingCount].id = id;
    r->pending[r->pendingCount].ticksWaiting = 0;
    ++r->pendingCount;
    return true;
}

} // namespace Cache::QueryLoad
