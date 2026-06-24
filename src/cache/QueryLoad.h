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

// Shared async-load dispatcher for the client query caches that use the
// generic response parser (`FUN_CACHE_QUERY_RESPONSE`) — creature,
// gameobject, and the other non-item/quest caches. Modeled on
// `Item::Data`'s pending/timeout/event machinery, generalized so several
// caches share one response-handler hook.
//
// Why shared: MinHook permits only one hook per target, and every one of
// these caches routes its `SMSG_*_QUERY_RESPONSE` through the same
// `FUN_CACHE_QUERY_RESPONSE`. So we hook it exactly once here and
// dispatch by cache instance to the right pending set + completion event.
//
// A module registers its cache once, then calls `RequestLoad`:
//
//   static const char *kEvent = "CREATURE_DATA_LOAD_RESULT";
//   static const Event::Custom::AutoReserve _r{kEvent};   // reserve the event
//   ...
//   Cache::QueryLoad::Register(cacheInstance, kEvent);     // at module init
//   Cache::QueryLoad::RequestLoad(cacheInstance, id);      // fire a load
//
// The completion event fires as `(id: number, success: bool)` — success
// when the cache fills, failure on timeout.

namespace Cache::QueryLoad {

// Register a query cache for async-load tracking. `cacheInstance` is the
// cache global (e.g. `Offsets::VAR_CREATURE_CACHE` cast to void*);
// `eventName` is a Custom event the caller has reserved via
// `Event::Custom::AutoReserve`, fired `(id, success)` on completion. Both
// must outlive the process (pass static values). Idempotent — calling
// again for the same cache is a no-op (safe across /reload).
void Register(void *cacheInstance, const char *eventName);

// True if `id` is already in the cache (pure peek, no network query).
bool IsCached(void *cacheInstance, uint32_t id);

// Issue an async load for `id`. If already cached, fires the completion
// event with success synchronously. Otherwise sends the query and tracks
// the id until the response lands (or it times out). Returns false only
// if the cache isn't registered or its pending set is full; true
// otherwise (already-cached, already-pending, or newly-tracked).
bool RequestLoad(void *cacheInstance, uint32_t id);

} // namespace Cache::QueryLoad
