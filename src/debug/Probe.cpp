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

#include "Game.h"
#include "Log.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdio>

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

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("_classicapi_DescDump", &Script_DescDump);
    Game::Lua::RegisterGlobalFunction("_classicapi_PlayerDump", &Script_PlayerDump);
    Game::Lua::RegisterGlobalFunction("_classicapi_DescLog", &Script_DescLog);
    Game::Lua::RegisterGlobalFunction("_classicapi_PlayerLog", &Script_PlayerLog);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Debug::Probe
