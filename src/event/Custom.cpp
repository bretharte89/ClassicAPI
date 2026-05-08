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
            *namePtr = eventName;
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
    // `SMemFree(entry.name)` (line 77 of `FrameScript.cpp`, per the error
    // message — the literal `0x4D` push at 0x00701A4C). It has a NULL
    // check at 0x00701A45 / `jz 0x00701A59` that skips entries with no
    // name. Our injected names are static literals in our DLL and not
    // Storm allocations, so we MUST null them out before the engine
    // walks the table — otherwise the SMem safety check trips. After
    // nulling, drop write privileges and reset the cache slots so we
    // re-claim in the rebuilt table after the next `LoadScriptFunctions`.
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    if (base != nullptr) {
        for (int i = 0; i < g_cacheCount; i++) {
            if (g_cache[i].slot >= 0) {
                auto **namePtr = reinterpret_cast<const char **>(
                    base + g_cache[i].slot * Offsets::EVENT_ENTRY_STRIDE +
                    Offsets::OFF_EVENT_ENTRY_NAME);
                *namePtr = nullptr;
            }
        }
    }
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
