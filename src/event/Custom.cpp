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

#include "Custom.h"

#include "Offsets.h"

#include <cstdint>
#include <cstring>

namespace Event::Custom {

namespace {

// Cache of registrations keyed by eventName pointer (callers always pass a
// static literal, so pointer-equality is a valid identity check). A failed
// claim caches as `slot = -1`; subsequent `Register` calls retry until it
// succeeds. This is important because the engine's event table at
// `[VAR_EVENT_TABLE_BASE_PTR]` isn't populated yet by the time our
// `LoadScriptFunctions` hook fires — count starts at 0 and grows during
// later boot phases. Retry-on-call lets us register late without needing
// to find a precise "table is ready" hook point.
struct CacheEntry {
    const char *name;
    int slot;
};
constexpr int MAX_CACHE = 16;
CacheEntry g_cache[MAX_CACHE];
int g_cacheCount = 0;
bool g_writesEnabled = false;

// Storm-allocate a copy of `s`. The engine's event table treats every
// `entry.name` as a Storm-owned pointer — its reload teardown loop calls
// `SMemFree(entry.name)` and validates the block against Storm's registered
// allocations (panics with `ERROR #124 SMem3: ...` if the pointer didn't
// come from `SMemAlloc`). So when we inject a name, we must hand it a real
// Storm allocation, not a static literal in our DLL.
//
// We never call `SMemFree` ourselves — the engine's teardown owns the
// lifetime once we've written the pointer into a slot.
char *AllocStormCopy(const char *s) {
    if (s == nullptr)
        return nullptr;
    using SMemAlloc_t = void *(__stdcall *)(size_t size, const char *file,
                                            int line, int flags);
    auto fn = reinterpret_cast<SMemAlloc_t>(Offsets::FUN_STORM_SMEM_ALLOC);
    const size_t len = std::strlen(s);
    char *buf = static_cast<char *>(fn(len + 1, __FILE__, __LINE__, 0));
    if (buf == nullptr)
        return nullptr;
    std::memcpy(buf, s, len + 1);
    return buf;
}

int TryClaim(const char *eventName) {
    if (!g_writesEnabled)
        return -1;
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    if (base == nullptr || count <= 0)
        return -1;
    // Walk the live entry range looking for the first slot whose name is
    // NULL. `Frame::RegisterEvent` skips NULL-name entries during its
    // strcmp loop (see the bail at 0x00702171), so they're safe to claim
    // without breaking any existing event.
    for (int i = 0; i < count; i++) {
        auto **namePtr = reinterpret_cast<const char **>(
            base + i * Offsets::EVENT_ENTRY_STRIDE + Offsets::OFF_EVENT_ENTRY_NAME);
        if (*namePtr == nullptr) {
            char *stormName = AllocStormCopy(eventName);
            if (stormName == nullptr)
                return -1;
            *namePtr = stormName;
            return i;
        }
    }
    return -1;
}

} // namespace

int Register(const char *eventName) {
    if (eventName == nullptr)
        return -1;
    for (int i = 0; i < g_cacheCount; i++) {
        if (g_cache[i].name == eventName) {
            if (g_cache[i].slot < 0)
                g_cache[i].slot = TryClaim(eventName);
            return g_cache[i].slot;
        }
    }
    if (g_cacheCount >= MAX_CACHE)
        return -1;
    g_cache[g_cacheCount].name = eventName;
    g_cache[g_cacheCount].slot = TryClaim(eventName);
    return g_cache[g_cacheCount++].slot;
}

void RetryAll() {
    for (int i = 0; i < g_cacheCount; i++) {
        if (g_cache[i].slot < 0)
            g_cache[i].slot = TryClaim(g_cache[i].name);
    }
}

void EnableWrites() { g_writesEnabled = true; }

void PrepareForReload() {
    // The engine's teardown at 0x00701A40 walks every entry and calls
    // `SMemFree(entry.name)`, which validates the block came from
    // `SMemAlloc`. Our injected names are Storm allocations (see
    // `AllocStormCopy`), so the engine's free path handles them
    // correctly — we don't need to touch the engine's table here.
    //
    // We DO still need to invalidate our cache: the engine's teardown
    // is followed by a full table rebuild at a fresh allocation, so
    // every cached `slot` index points into the old layout and is no
    // longer ours. Reset everything to `slot = -1` and drop the writes
    // gate; `LoadScriptFunctions_h` will re-enable writes after the
    // rebuild, and `RetryAll` (fired by every subsequent
    // `RegisterEvent` from Lua) will re-claim and re-allocate.
    for (int i = 0; i < g_cacheCount; i++)
        g_cache[i].slot = -1;
    g_writesEnabled = false;
}

void Fire_DD(int eventID, int arg1, int arg2) {
    if (eventID < 0)
        return;
    using FireEvent_DD_t = void(__cdecl *)(int eventID, const char *format, int a, int b);
    auto fn = reinterpret_cast<FireEvent_DD_t>(Offsets::FUN_FIRE_EVENT);
    fn(eventID, "%d%d", arg1, arg2);
}

} // namespace Event::Custom
