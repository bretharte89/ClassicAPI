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

#include <vector>

namespace Encoding::Compress {

namespace {

// zlib one-shot helpers, statically linked into WoW.exe (1.2.2).
// Both are `__fastcall`: ECX = dest, EDX = destLen ptr, the remaining
// args on the stack. Callee cleans (RET 0xC).
using compress2_t = int(__fastcall *)(void *dest, unsigned long *destLen,
                                      const void *source,
                                      unsigned long sourceLen, int level);
using uncompress_t = int(__fastcall *)(void *dest, unsigned long *destLen,
                                       const void *source,
                                       unsigned long sourceLen);

// zlib 1.2.x's `compressBound` is inlined in the header; we replicate
// the formula here so we don't have to chase the symbol.
unsigned long CompressBound(unsigned long sourceLen) {
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
           (sourceLen >> 25) + 13;
}

// `C_EncodingUtil.CompressString(data) → string` — deflate-compresses
// `data` (any binary string) at zlib's default level (6) and returns
// the compressed stream. Round-trips through `DecompressString`.
//
// Output is the zlib-format stream (2-byte header + raw deflate +
// adler32 trailer) as produced by `compress2`. Same format `_GAMES_*`
// addon-side libraries like `LibDeflate` produce when using
// `CompressZlib`.
int __fastcall Script_CompressString(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: C_EncodingUtil.CompressString(data)");
        return 0;
    }
    const char *src = Game::Lua::ToString(L, 1);
    const unsigned int srcLen = Game::Lua::StrLen(L, 1);
    if (src == nullptr || srcLen == 0) {
        Game::Lua::PushString(L, "");
        return 1;
    }

    const unsigned long bound = CompressBound(srcLen);
    std::vector<char> out(bound);
    unsigned long outLen = bound;

    auto compress2 = reinterpret_cast<compress2_t>(Offsets::FUN_ZLIB_COMPRESS2);
    const int rc = compress2(out.data(), &outLen, src, srcLen,
                             /*Z_DEFAULT_COMPRESSION*/ -1);
    if (rc != 0)
        return 0; // Z_MEM_ERROR / Z_BUF_ERROR

    Game::Lua::PushLString(L, out.data(),
                           static_cast<unsigned int>(outLen));
    return 1;
}

// `C_EncodingUtil.DecompressString(data) → string | nil` — inflates a
// zlib-format stream produced by `CompressString` (or any compatible
// encoder). Returns nil on malformed input (`Z_DATA_ERROR`) or any
// other zlib failure.
//
// Output sizing: we don't know the decompressed length up front, so
// we start with a buffer 4x the input size and grow exponentially on
// `Z_BUF_ERROR`. zlib's one-shot `uncompress` can't stream a partial
// result, so a single resize-and-retry loop is the simplest correct
// approach for arbitrary-size payloads.
int __fastcall Script_DecompressString(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: C_EncodingUtil.DecompressString(data)");
        return 0;
    }
    const char *src = Game::Lua::ToString(L, 1);
    const unsigned int srcLen = Game::Lua::StrLen(L, 1);
    if (src == nullptr || srcLen == 0) {
        Game::Lua::PushString(L, "");
        return 1;
    }

    auto uncompress = reinterpret_cast<uncompress_t>(Offsets::FUN_ZLIB_UNCOMPRESS);

    unsigned long capacity = static_cast<unsigned long>(srcLen) * 4 + 32;
    // Hard cap to keep a corrupt header that claims gigabytes from
    // looping forever. 64 MiB matches modern's documented inflate
    // ceiling (large enough for any legitimate addon import string).
    constexpr unsigned long kCapacityCeiling = 64UL * 1024UL * 1024UL;

    std::vector<char> out;
    for (;;) {
        if (capacity > kCapacityCeiling)
            return 0;
        out.resize(capacity);
        unsigned long outLen = capacity;
        const int rc = uncompress(out.data(), &outLen, src, srcLen);
        if (rc == 0 /*Z_OK*/) {
            Game::Lua::PushLString(L, out.data(),
                                   static_cast<unsigned int>(outLen));
            return 1;
        }
        if (rc != -5 /*Z_BUF_ERROR*/)
            return 0; // Z_DATA_ERROR, Z_MEM_ERROR, etc.
        // Output buffer too small — grow and retry.
        capacity *= 2;
    }
}

void Register() {
    Game::Lua::RegisterTableFunction("C_EncodingUtil", "CompressString",
                                     &Script_CompressString);
    Game::Lua::RegisterTableFunction("C_EncodingUtil", "DecompressString",
                                     &Script_DecompressString);
}

} // namespace

static const Game::ModuleAutoRegister _autoreg{&Register};

} // namespace Encoding::Compress
