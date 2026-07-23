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
// One-shot latch so a genuinely full event table (no NULL slot left to
// claim) is logged once per session instead of silently dropping the
// event — the failure mode that would otherwise rot unnoticed. Re-armed on
// `/reload` (the table is rebuilt fresh). `Event::Capacity` raises the
// FrameXML table to >=700 to keep this from ever triggering, but the log is
// the backstop if that hook is ever absent or the pool is somehow exhausted.
bool g_warnedTableFull = false;

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

// Storm allocator, matching what the engine uses for the event table itself
// (so the engine's reload teardown frees our buffer the same way). Both are
// `__stdcall`, 4 args (`ret 0x10`). SMemAlloc's flag bit 8 zero-fills.
void *SMemAlloc(unsigned size) {
    using SMemAlloc_t = void *(__stdcall *)(unsigned size, const char *file,
                                            int line, int flags);
    auto fn = reinterpret_cast<SMemAlloc_t>(Offsets::FUN_STORM_SMEMALLOC);
    return fn(size, __FILE__, __LINE__, 8 /* zero-fill */);
}
void SMemFree(void *ptr) {
    using SMemFree_t = int(__stdcall *)(void *ptr, const char *file, int line,
                                        int flags);
    auto fn = reinterpret_cast<SMemFree_t>(Offsets::FUN_STORM_SMEMFREE);
    fn(ptr, __FILE__, __LINE__, 0);
}

// Walk the engine's event table from the LOW end looking for a NULL-name
// slot. The bulk-init at `0x0051AB30` leaves ~200 NULL gaps scattered
// through the 548-slot input names array (it only writes ~350 of the
// slots explicitly; the rest stay zero-initialized from .data) — a big
// contiguous hole around 42..106 alone dwarfs our reservation count.
//
// We go LOW (ascending) deliberately: SuperWoW and nampower own the HIGH
// range (fixed event IDs 549..655, and they destructively clamp the table
// to 700), and other claim-based DLLs (VanillaMinimapTracking) also walk
// from the tail — so the top is the contested end. Claiming from the low
// engine gaps keeps us entirely out of their way and independent of their
// clamp / hook order. `EnsureCapacity` (run once at load) guarantees enough
// NULL slots exist down here before we ever get here; the skip-occupied
// invariant keeps this safe against future engine additions regardless of
// direction.
int TryClaim(const char *eventName) {
    if (!g_writesEnabled)
        return -1;
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    if (base == nullptr || count <= 0)
        return -1;
    for (int i = 0; i < count; ++i) {
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
    // Writes are enabled and the table is valid, but every slot is taken —
    // real capacity exhaustion. Warn once so it's diagnosable.
    if (!g_warnedTableFull) {
        g_warnedTableFull = true;
        Debug::Log::Printf(
            "[event] table full (%d slots, no NULL left) — cannot claim '%s'"
            " and any further custom events. Raise the FrameXML event count"
            " (Event::Capacity) or reduce reservations.",
            count, eventName ? eventName : "?");
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

void EnsureCapacity() {
    // Read the live table fresh — never cache; another DLL may have resized
    // it (SuperWoW/nampower clamp to 700, another extender may have grown it).
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    if (base == nullptr || count <= 0 || g_reservedCount <= 0)
        return;

    // Count the NULL-name slots available to claim.
    int nulls = 0;
    for (int i = 0; i < count; ++i)
        if (*reinterpret_cast<const char *const *>(
                base + i * Offsets::EVENT_ENTRY_STRIDE +
                Offsets::OFF_EVENT_ENTRY_NAME) == nullptr)
            ++nulls;

    // Enough room already (the common case — the engine's low gaps are huge).
    // If another DLL already enlarged the table, we simply see its NULLs here
    // and don't grow. Cooperative, no coordination needed.
    if (nulls >= g_reservedCount)
        return;

    // We're short — expand. But growing moves the buffer, which invalidates
    // the entries' self-referential list anchors, so it's ONLY safe while
    // every chain is empty (no frame registered yet). We run before the first
    // RegisterEvent, so that holds — but VERIFY it defensively: a populated
    // chain means someone registered early, and growing would corrupt their
    // subscriptions. If so, abort and let claiming use whatever NULLs exist
    // (the one-shot log fires if that's not enough).
    for (int i = 0; i < count; ++i) {
        const uint32_t head = *reinterpret_cast<const uint32_t *>(
            base + i * Offsets::EVENT_ENTRY_STRIDE + Offsets::OFF_EVENT_ENTRY_HEAD);
        const bool empty = (head == 0) || (head & 1);  // 0 / low-bit sentinel
        if (!empty) {
            Debug::Log::Printf(
                "[event] %d NULL slots < %d reserved, but chains are live — "
                "skipping grow to avoid corrupting registrations",
                nulls, g_reservedCount);
            return;
        }
    }

    // Grow by the deficit plus a margin (so we don't claim the very last
    // slots and starve other claim-based DLLs). Pool-minimal.
    constexpr int kMargin = 16;
    const int newCount = count + (g_reservedCount - nulls) + kMargin;
    auto *newBuf = static_cast<uint8_t *>(
        SMemAlloc(static_cast<unsigned>(newCount) * Offsets::EVENT_ENTRY_STRIDE));
    if (newBuf == nullptr)
        return;  // alloc failed — keep the old table; claiming logs if short

    // Rebuild every entry at its NEW address: copy the name pointer (strings
    // are shared, NOT duplicated), and re-init the anchor as an empty sentinel
    // pointing at its own new location. Safe because all chains are empty.
    for (int i = 0; i < newCount; ++i) {
        auto *e = reinterpret_cast<uint32_t *>(
            newBuf + i * Offsets::EVENT_ENTRY_STRIDE);
        if (i < count)  // new entries stay zero-filled (null name)
            e[0] = *reinterpret_cast<const uint32_t *>(
                base + i * Offsets::EVENT_ENTRY_STRIDE);
        const uint32_t anchor = static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(&e[2]));  // entry+0x08
        e[2] = anchor;       // self-referential list anchor
        e[3] = anchor | 1;   // empty-chain sentinel head (+0x0C)
    }

    // Publish base BEFORE count, so the global never advertises more entries
    // than the buffer holds. Then free the old buffer (buffer only — the names
    // live on in newBuf).
    *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR) = newBuf;
    *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_CAP) = newCount;
    *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT) = newCount;
    SMemFree(base);

    Debug::Log::Printf("[event] grew table %d -> %d (%d NULL < %d reserved)",
                       count, newCount, nulls, g_reservedCount);
}

void PrepareForReload() {
    for (int i = 0; i < g_reservedCount; ++i)
        g_reserved[i].slot = -1;
    g_writesEnabled = false;
    g_warnedTableFull = false;  // fresh table after the rebuild — re-arm
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
