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

// Temporary probe helpers used to derive byte offsets in the player /
// local-player structs. Intended to be deleted once the offsets are
// baked in.
//
// Bound globals:
//
//   _classicapi_DescDump(start, len) → string
//   _classicapi_PlayerDump(start, len) → string
//      Return a space-separated `0xOFFSET=0xVALUE` string of non-zero
//      u32 entries in the half-open range [start, start+len) of the
//      player's m_objectFields descriptor (DescDump) or the local
//      CGPlayer object directly (PlayerDump). Use DescDump for
//      UpdateField-broadcast state (mount, stealth visibility);
//      PlayerDump for client-only state (movement flags).
//      Useful for short ranges (~0x100 bytes); longer ranges may
//      truncate when /dump'd.
//
//   _classicapi_DescLog(label, start, len)
//   _classicapi_PlayerLog(label, start, len)
//      Same dump shape, but appends the result to `debug.log` via
//      Debug::Log::Append rather than returning it. Use for ranges
//      too long for /dump's output buffer (e.g., scanning the whole
//      CGPlayer object for movement flags).
//
//   _classicapi_SnapAll()
//      Captures a baseline snapshot of three regions on the player:
//          • m_objectFields descriptor   [0,  0x400)
//          • CGPlayer object             [0, 0x2000)
//          • [unit + OFF_CGPLAYER_INFO]  [0, 0x2000)   (the +0xE68 sub-struct)
//      Diff functions below compare the *live* memory to this snapshot.
//      Workflow: stand idle, call SnapAll, perform the action whose
//      state you want to localize (e.g. click warlock summoning portal),
//      then call DiffAll[Log] before the state clears. Re-snap any time.
//
//   _classicapi_DiffAll() → string
//   _classicapi_DiffAllLog(label)
//      Emit `OFF=OLD→NEW` for every u32 that has changed since the last
//      SnapAll, grouped into DESC / PLAYER / EXTRA sections. DiffAll
//      returns the string (use for short diffs via /dump); DiffAllLog
//      appends to debug.log (use for long diffs where /dump truncates).

#include "Game.h"
#include "Log.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Debug::Probe {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

const uint8_t *ResolvePlayer() {
    auto fn = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return static_cast<const uint8_t *>(fn("player"));
}

const uint8_t *ResolvePlayerDescriptor() {
    auto *player = ResolvePlayer();
    if (player == nullptr)
        return nullptr;
    return *reinterpret_cast<const uint8_t *const *>(
        player + Offsets::OFF_UNIT_DESCRIPTOR);
}

// [unit + OFF_CGPLAYER_INFO] — the +0xE68 sub-struct documented in
// Offsets.h. Holds PLAYER_FLAGS (+0x08) and visible-items (+0x118).
// Most other +0xE68 fields are undocumented and are the reason this
// region is a high-yield target for change-detection scans.
const uint8_t *ResolvePlayerExtra() {
    auto *player = ResolvePlayer();
    if (player == nullptr)
        return nullptr;
    return *reinterpret_cast<const uint8_t *const *>(
        player + Offsets::OFF_CGPLAYER_INFO);
}

// Writes non-zero u32 entries in [start, start+len) into `out` as
// space-separated `0xOFFSET=0xVALUE` pairs. Returns the length
// written (excluding the nul terminator). Output is truncated if it
// would exceed `outLen-1` bytes; partial output is still well-formed
// (doesn't end mid-pair).
int FormatNonZeroU32s(const uint8_t *base, int start, int len,
                     char *out, int outLen) {
    if (base == nullptr || out == nullptr || outLen <= 0)
        return 0;
    int pos = 0;
    out[0] = '\0';
    for (int off = 0; off + 4 <= len && pos < outLen - 32; off += 4) {
        const uint32_t v = *reinterpret_cast<const uint32_t *>(base + start + off);
        if (v != 0) {
            const int n = std::snprintf(out + pos, outLen - pos,
                                        "0x%X=0x%X ", start + off, v);
            if (n < 0)
                break;
            pos += n;
        }
    }
    if (pos == 0) {
        const int n = std::snprintf(out, outLen, "(all zero)");
        pos = n > 0 ? n : 0;
    }
    return pos;
}

void DumpToLua(void *L, const uint8_t *base, int start, int len) {
    static char buf[3000];
    FormatNonZeroU32s(base, start, len, buf, sizeof(buf));
    Game::Lua::PushString(L, buf);
}

void DumpToFile(const char *label, const uint8_t *base, int start, int len) {
    static char buf[8192];
    FormatNonZeroU32s(base, start, len, buf, sizeof(buf));
    Debug::Log::Append(label, buf);
}

void ReadStartLen(void *L, int argBase, int *outStart, int *outLen,
                  int defaultLen) {
    *outStart = 0;
    *outLen = defaultLen;
    if (Game::Lua::IsNumber(L, argBase))
        *outStart = static_cast<int>(Game::Lua::ToNumber(L, argBase));
    if (Game::Lua::IsNumber(L, argBase + 1))
        *outLen = static_cast<int>(Game::Lua::ToNumber(L, argBase + 1));
}

int __fastcall Script_DescDump(void *L) {
    int start, len;
    ReadStartLen(L, 1, &start, &len, 0x200);
    DumpToLua(L, ResolvePlayerDescriptor(), start, len);
    return 1;
}

int __fastcall Script_PlayerDump(void *L) {
    int start, len;
    ReadStartLen(L, 1, &start, &len, 0x200);
    DumpToLua(L, ResolvePlayer(), start, len);
    return 1;
}

int __fastcall Script_DescLog(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: _classicapi_DescLog(label, start, len)");
        return 0;
    }
    const char *label = Game::Lua::ToString(L, 1);
    int start, len;
    ReadStartLen(L, 2, &start, &len, 0x200);
    DumpToFile(label, ResolvePlayerDescriptor(), start, len);
    return 0;
}

int __fastcall Script_PlayerLog(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: _classicapi_PlayerLog(label, start, len)");
        return 0;
    }
    const char *label = Game::Lua::ToString(L, 1);
    int start, len;
    ReadStartLen(L, 2, &start, &len, 0x200);
    DumpToFile(label, ResolvePlayer(), start, len);
    return 0;
}

// Snapshot of a fixed-size byte range, used by Snap/Diff to compare
// the live memory against a captured baseline.
struct Snap {
    static constexpr int kCap = 0x2000;
    uint8_t data[kCap];
    int start;
    int len;
    bool valid;
};

Snap descSnap;
Snap playerSnap;
Snap extraSnap;

void TakeSnap(Snap &s, const uint8_t *base, int start, int len) {
    s.valid = false;
    s.start = start;
    s.len = 0;
    if (base == nullptr || len <= 0)
        return;
    if (len > Snap::kCap)
        len = Snap::kCap;
    std::memcpy(s.data, base + start, static_cast<size_t>(len));
    s.len = len;
    s.valid = true;
}

// Emit one `=== <label> ===` header line + each changed u32 as
// `0xOFFSET=0xOLD->0xNEW`. Stops cleanly before overrunning the output
// buffer (leaves room for the final nul + one safety margin). Returns
// bytes written (excluding nul).
int FormatDiff(const Snap &s, const uint8_t *base, const char *label,
               char *out, int outLen) {
    if (out == nullptr || outLen <= 0)
        return 0;
    int pos = 0;
    const int header = std::snprintf(out + pos, outLen - pos, "%s: ", label);
    if (header > 0)
        pos += header;
    if (!s.valid) {
        const int n = std::snprintf(out + pos, outLen - pos, "(no snapshot) ");
        if (n > 0)
            pos += n;
        return pos;
    }
    if (base == nullptr) {
        const int n = std::snprintf(out + pos, outLen - pos, "(no base) ");
        if (n > 0)
            pos += n;
        return pos;
    }
    int changes = 0;
    for (int off = 0; off + 4 <= s.len && pos < outLen - 48; off += 4) {
        const uint32_t oldV = *reinterpret_cast<const uint32_t *>(s.data + off);
        const uint32_t newV = *reinterpret_cast<const uint32_t *>(
            base + s.start + off);
        if (oldV == newV)
            continue;
        const int n = std::snprintf(out + pos, outLen - pos,
                                    "0x%X=0x%X->0x%X ",
                                    s.start + off, oldV, newV);
        if (n < 0)
            break;
        pos += n;
        changes++;
    }
    if (changes == 0) {
        const int n = std::snprintf(out + pos, outLen - pos, "(unchanged) ");
        if (n > 0)
            pos += n;
    }
    return pos;
}

int __fastcall Script_SnapAll(void *) {
    TakeSnap(descSnap, ResolvePlayerDescriptor(), 0, 0x400);
    TakeSnap(playerSnap, ResolvePlayer(), 0, 0x2000);
    TakeSnap(extraSnap, ResolvePlayerExtra(), 0, 0x2000);
    return 0;
}

int __fastcall Script_DiffAll(void *L) {
    static char buf[3500];
    int pos = 0;
    pos += FormatDiff(descSnap, ResolvePlayerDescriptor(), "DESC",
                      buf + pos, sizeof(buf) - pos);
    pos += FormatDiff(playerSnap, ResolvePlayer(), "PLAYER",
                      buf + pos, sizeof(buf) - pos);
    pos += FormatDiff(extraSnap, ResolvePlayerExtra(), "EXTRA",
                      buf + pos, sizeof(buf) - pos);
    if (pos >= 0 && pos < static_cast<int>(sizeof(buf)))
        buf[pos] = '\0';
    else
        buf[sizeof(buf) - 1] = '\0';
    Game::Lua::PushString(L, buf);
    return 1;
}

int __fastcall Script_DiffAllLog(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: _classicapi_DiffAllLog(label)");
        return 0;
    }
    const char *label = Game::Lua::ToString(L, 1);
    static char buf[16384];
    int pos = 0;
    pos += FormatDiff(descSnap, ResolvePlayerDescriptor(), "DESC",
                      buf + pos, sizeof(buf) - pos);
    if (pos < static_cast<int>(sizeof(buf)) - 2) {
        buf[pos++] = '\n';
    }
    pos += FormatDiff(playerSnap, ResolvePlayer(), "PLAYER",
                      buf + pos, sizeof(buf) - pos);
    if (pos < static_cast<int>(sizeof(buf)) - 2) {
        buf[pos++] = '\n';
    }
    pos += FormatDiff(extraSnap, ResolvePlayerExtra(), "EXTRA",
                      buf + pos, sizeof(buf) - pos);
    if (pos >= 0 && pos < static_cast<int>(sizeof(buf)))
        buf[pos] = '\0';
    else
        buf[sizeof(buf) - 1] = '\0';
    Debug::Log::Append(label, buf);
    return 0;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("_classicapi_DescDump", &Script_DescDump);
    Game::Lua::RegisterGlobalFunction("_classicapi_PlayerDump", &Script_PlayerDump);
    Game::Lua::RegisterGlobalFunction("_classicapi_DescLog", &Script_DescLog);
    Game::Lua::RegisterGlobalFunction("_classicapi_PlayerLog", &Script_PlayerLog);
    Game::Lua::RegisterGlobalFunction("_classicapi_SnapAll", &Script_SnapAll);
    Game::Lua::RegisterGlobalFunction("_classicapi_DiffAll", &Script_DiffAll);
    Game::Lua::RegisterGlobalFunction("_classicapi_DiffAllLog", &Script_DiffAllLog);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Debug::Probe
