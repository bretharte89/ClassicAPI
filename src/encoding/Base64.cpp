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

#include <cstdint>
#include <vector>

namespace Encoding::Base64 {

namespace {

constexpr const char kAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// `C_EncodingUtil.EncodeBase64(data) → string` — standard RFC 4648
// base64 with `+`/`/` (not URL-safe) and `=` padding. Output length is
// `((len + 2) / 3) * 4`.
int __fastcall Script_EncodeBase64(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: C_EncodingUtil.EncodeBase64(data)");
        return 0;
    }
    const auto *src = reinterpret_cast<const uint8_t *>(
        Game::Lua::ToString(L, 1));
    const unsigned int len = Game::Lua::StrLen(L, 1);
    if (src == nullptr || len == 0) {
        Game::Lua::PushString(L, "");
        return 1;
    }

    const unsigned int outLen = ((len + 2) / 3) * 4;
    std::vector<char> out(outLen);
    unsigned int o = 0;
    unsigned int i = 0;
    for (; i + 3 <= len; i += 3) {
        const uint32_t v = (static_cast<uint32_t>(src[i]) << 16) |
                           (static_cast<uint32_t>(src[i + 1]) << 8) |
                           static_cast<uint32_t>(src[i + 2]);
        out[o++] = kAlphabet[(v >> 18) & 0x3F];
        out[o++] = kAlphabet[(v >> 12) & 0x3F];
        out[o++] = kAlphabet[(v >> 6) & 0x3F];
        out[o++] = kAlphabet[v & 0x3F];
    }
    const unsigned int rem = len - i;
    if (rem == 1) {
        const uint32_t v = static_cast<uint32_t>(src[i]) << 16;
        out[o++] = kAlphabet[(v >> 18) & 0x3F];
        out[o++] = kAlphabet[(v >> 12) & 0x3F];
        out[o++] = '=';
        out[o++] = '=';
    } else if (rem == 2) {
        const uint32_t v = (static_cast<uint32_t>(src[i]) << 16) |
                           (static_cast<uint32_t>(src[i + 1]) << 8);
        out[o++] = kAlphabet[(v >> 18) & 0x3F];
        out[o++] = kAlphabet[(v >> 12) & 0x3F];
        out[o++] = kAlphabet[(v >> 6) & 0x3F];
        out[o++] = '=';
    }
    Game::Lua::PushLString(L, out.data(), outLen);
    return 1;
}

// `C_EncodingUtil.DecodeBase64(b64) → string | nil` — accepts the
// standard alphabet plus the URL-safe variant (`-_`). Padding (`=`) is
// required when the encoded length is not already a multiple of 4 —
// modern WoW's implementation rejects unpadded input and we match.
// Returns nil on any non-alphabet character or length-not-multiple-of-4.
int __fastcall Script_DecodeBase64(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: C_EncodingUtil.DecodeBase64(b64)");
        return 0;
    }
    const char *src = Game::Lua::ToString(L, 1);
    const unsigned int len = Game::Lua::StrLen(L, 1);
    if (src == nullptr || len == 0) {
        Game::Lua::PushString(L, "");
        return 1;
    }
    if ((len & 3) != 0)
        return 0;

    auto decode = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return 26 + (c - 'a');
        if (c >= '0' && c <= '9') return 52 + (c - '0');
        if (c == '+' || c == '-') return 62;
        if (c == '/' || c == '_') return 63;
        return -1;
    };

    std::vector<char> out;
    out.reserve((len / 4) * 3);
    for (unsigned int i = 0; i < len; i += 4) {
        const char c0 = src[i];
        const char c1 = src[i + 1];
        const char c2 = src[i + 2];
        const char c3 = src[i + 3];
        const int v0 = decode(c0);
        const int v1 = decode(c1);
        if (v0 < 0 || v1 < 0)
            return 0;

        // Padding only allowed in the final quad's c2/c3 slots.
        const bool isLastQuad = (i + 4 == len);
        if (c2 == '=') {
            if (!isLastQuad || c3 != '=')
                return 0;
            out.push_back(static_cast<char>((v0 << 2) | (v1 >> 4)));
            break;
        }
        const int v2 = decode(c2);
        if (v2 < 0)
            return 0;
        if (c3 == '=') {
            if (!isLastQuad)
                return 0;
            out.push_back(static_cast<char>((v0 << 2) | (v1 >> 4)));
            out.push_back(static_cast<char>((v1 << 4) | (v2 >> 2)));
            break;
        }
        const int v3 = decode(c3);
        if (v3 < 0)
            return 0;
        out.push_back(static_cast<char>((v0 << 2) | (v1 >> 4)));
        out.push_back(static_cast<char>((v1 << 4) | (v2 >> 2)));
        out.push_back(static_cast<char>((v2 << 6) | v3));
    }

    Game::Lua::PushLString(L, out.data(),
                           static_cast<unsigned int>(out.size()));
    return 1;
}

void Register() {
    Game::Lua::RegisterTableFunction("C_EncodingUtil", "EncodeBase64",
                                     &Script_EncodeBase64);
    Game::Lua::RegisterTableFunction("C_EncodingUtil", "DecodeBase64",
                                     &Script_DecodeBase64);
}

} // namespace

static const Game::ModuleAutoRegister _autoreg{&Register};

} // namespace Encoding::Base64
