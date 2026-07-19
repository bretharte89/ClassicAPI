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

#include "Custom.h"

#include "Game.h"
#include "Offsets.h"
#include "debug/Log.h"

#include <cstdint>
#include <cstring>

namespace Event::Custom {

namespace {

// Reservations registered by `AutoReserve` at static-init time, before
// any engine code runs. `RetryClaims` (triggered by the
// `Frame::RegisterEvent` hook) walks this list and claims a NULL slot
// for each unclaimed entry.
//
// MUST stay comfortably above the total number of `AutoReserve` instances
// across the codebase (file-scope + the lazily-registered TTS ones) — the
// constructor SILENTLY drops any reservation past the cap, and which ones
// overflow depends on unspecified cross-TU static-init order, so exceeding
// it makes an arbitrary subset of custom events quietly stop firing (their
// `Lookup` returns -1 and `Fire` no-ops). We hit exactly that at 32: ~40
// reservations meant a shifting handful (e.g. ITEM_DATA_LOAD_RESULT /
// GET_ITEM_INFO_RECEIVED) never claimed a slot. Grep `AutoReserve` before
// bumping the reservation count near this ceiling.
constexpr int MAX_RESERVED = 64;
struct ReservedName {
    const char *name;
    int slot;  // -1 until claimed
};
ReservedName g_reserved[MAX_RESERVED];
int g_reservedCount = 0;
bool g_writesEnabled = false;

// Engine's SStrDup at `FUN_STORM_SSTRDUP` — uses `SMemAlloc` internally
// and is exactly what the engine itself does when filling event table
// entries. Reload teardown's `SMemFree(entry.name)` validates that the
// pointer came from `SMemAlloc`, so handing it an SStrDup'd block is
// the only safe way to inject a name.
char *SStrDup(const char *s) {
    if (s == nullptr)
        return nullptr;
    using SStrDup_t = char *(__stdcall *)(const char *src, const char *file,
                                          int line);
    auto fn = reinterpret_cast<SStrDup_t>(Offsets::FUN_STORM_SSTRDUP);
    return fn(s, __FILE__, __LINE__);
}

// Walk the engine's event table from the END looking for a NULL-name
// slot. The bulk-init at `0x0051AB30` leaves ~200 NULL gaps scattered
// through the 548-slot input names array (it only writes ~350 of the
// slots explicitly; the rest stay zero-initialized from .data). UP
// and DOWN walks both find these gaps — we go DOWN for convention with
// VanillaMinimapTracking and so the dump in `_classicapi_DumpAllEvents`
// shows our events grouped at the tail, visually distinct from engine
// events.
int TryClaim(const char *eventName) {
    if (!g_writesEnabled)
        return -1;
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    if (base == nullptr || count <= 0)
        return -1;
    for (int i = count - 1; i >= 0; --i) {
        auto **namePtr = reinterpret_cast<const char **>(
            base + i * Offsets::EVENT_ENTRY_STRIDE +
            Offsets::OFF_EVENT_ENTRY_NAME);
        if (*namePtr == nullptr) {
            char *copy = SStrDup(eventName);
            if (copy == nullptr)
                return -1;
            *namePtr = copy;
            return i;
        }
    }
    return -1;
}

} // namespace

AutoReserve::AutoReserve(const char *name) {
    if (name == nullptr || g_reservedCount >= MAX_RESERVED)
        return;
    for (int i = 0; i < g_reservedCount; ++i) {
        if (std::strcmp(g_reserved[i].name, name) == 0)
            return;
    }
    g_reserved[g_reservedCount].name = name;
    g_reserved[g_reservedCount].slot = -1;
    ++g_reservedCount;
}

int Lookup(const char *name) {
    if (name == nullptr)
        return -1;
    for (int i = 0; i < g_reservedCount; ++i) {
        if (std::strcmp(g_reserved[i].name, name) == 0)
            return g_reserved[i].slot;
    }
    return -1;
}

int LookupByName(const char *name) {
    if (name == nullptr)
        return -1;
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    if (base == nullptr || count <= 0)
        return -1;
    for (int i = 0; i < count; ++i) {
        const char *entryName = *reinterpret_cast<const char *const *>(
            base + i * Offsets::EVENT_ENTRY_STRIDE +
            Offsets::OFF_EVENT_ENTRY_NAME);
        if (entryName != nullptr && std::strcmp(entryName, name) == 0)
            return i;
    }
    return -1;
}

void RetryClaims() {
    for (int i = 0; i < g_reservedCount; ++i) {
        if (g_reserved[i].slot < 0)
            g_reserved[i].slot = TryClaim(g_reserved[i].name);
    }
}

void EnableWrites() { g_writesEnabled = true; }

void PrepareForReload() {
    for (int i = 0; i < g_reservedCount; ++i)
        g_reserved[i].slot = -1;
    g_writesEnabled = false;
}

// Diagnostic Lua functions — registered via the auto-register pattern
// at the bottom of this file. Not on any hot path.
namespace {

int __fastcall Script_DumpSlot(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Debug::Log::Printf("[dumpslot] usage: _classicapi_DumpSlot(slot)");
        return 0;
    }
    const int slot = static_cast<int>(Game::Lua::ToNumber(L, 1));
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    const char *name = nullptr;
    if (base != nullptr && slot >= 0 && slot < count) {
        name = *reinterpret_cast<const char *const *>(
            base + slot * Offsets::EVENT_ENTRY_STRIDE +
            Offsets::OFF_EVENT_ENTRY_NAME);
    }
    Debug::Log::Printf("[dumpslot] base=0x%08X count=%d slot=%d name='%s'",
                       static_cast<unsigned>(reinterpret_cast<uintptr_t>(base)),
                       count, slot, name ? name : "(null)");
    return 0;
}

int __fastcall Script_DumpAllEvents(void *) {
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    Debug::Log::Printf("[dumpall] base=0x%08X count=%d",
                       static_cast<unsigned>(reinterpret_cast<uintptr_t>(base)),
                       count);
    if (base == nullptr)
        return 0;
    for (int i = 0; i < count; ++i) {
        const char *name = *reinterpret_cast<const char *const *>(
            base + i * Offsets::EVENT_ENTRY_STRIDE +
            Offsets::OFF_EVENT_ENTRY_NAME);
        if (name != nullptr && *name != '\0')
            Debug::Log::Printf("[%d] %s", i, name);
    }
    Debug::Log::Printf("[dumpall] done");
    return 0;
}

void RegisterDiagnostics() {
    Game::Lua::RegisterGlobalFunction("_classicapi_DumpSlot", &Script_DumpSlot);
    Game::Lua::RegisterGlobalFunction("_classicapi_DumpAllEvents",
                                      &Script_DumpAllEvents);
}

const Game::ModuleAutoRegister _autoreg{&RegisterDiagnostics};

} // namespace

} // namespace Event::Custom
