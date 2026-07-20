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

// String helpers that 1.12's Lua 5.0 is missing:
//
//   - `string.match(s, pattern [, init])` — first-match extraction (5.1).
//   - `string.gmatch(s, pattern)`         — match iterator (5.1).
//   - `strsplit(sep, str [, pieces])`     — WoW global; split on any char.
//
// The two `string.*` ones reuse pattern machinery 5.0 already ships:
//   - `match` is `find` returning captures / the whole match instead of the
//     start/end indices — so we call the engine's `str_find` and transform.
//   - `gmatch` is exactly 5.0's `string.gfind` (renamed in 5.1); we register
//     it as a direct alias of the engine's `gfind` C function.
// `strsplit` is a hand-rolled port of 3.3.5's `strsplit` (see below).
//
// NOTE: only the `string.foo(s, ...)` call form works — NOT the
// `("x"):foo(...)` method sugar. WoW's Lua VM strips type-metatables for
// every value except tables/userdata (both `lua_setmetatable` and the VM's
// `luaT_gettmbyobj` hard-return "no metamethod" for strings), so strings have
// no `__index`, and there's no client-side way to add one short of hooking a
// hot VM-core function. This is the long-standing vanilla constraint.

#include "Game.h"
#include "Offsets.h"

#include <cstddef>

namespace BaseLib::StringLib {

namespace {

// `string.match(s, pattern [, init])`. Calls the engine `str_find` and
// reshapes its result: no match → nil; match with captures → the captures;
// match with no captures → the whole matched substring.
int __fastcall Script_string_match(void *L) {
    // Normalize to exactly (s, pattern, init): pad missing args with nil so
    // str_find defaults `init` to 1, and drop any stray 4th value so it can't
    // be misread as find's `plain` flag (match has no plain form).
    Game::Lua::SetTop(L, 3);
    constexpr int kBase = 3;

    auto strFind = reinterpret_cast<Game::Lua::CFunction>(Offsets::FUN_LUA_STR_FIND);
    const int nres = strFind(L); // pushes start,end,caps.. above kBase, or a nil

    if (nres <= 1) {
        // No match. str_find pushed the nil (return 1); guard the degenerate
        // nres==0 case by pushing our own nil so the shape holds.
        if (nres < 1)
            Game::Lua::PushNil(L);
        return 1;
    }

    if (nres == 2) {
        // No captures → the whole matched substring s[start..end] (1-based,
        // inclusive). An empty match has end == start-1 → an empty string.
        const int start = static_cast<int>(Game::Lua::ToNumber(L, kBase + 1));
        const int end = static_cast<int>(Game::Lua::ToNumber(L, kBase + 2));
        // str_find already coerced arg 1 to a string, so this is safe.
        const char *s = Game::Lua::ToString(L, 1);
        Game::Lua::SetTop(L, kBase); // drop start, end
        const size_t len = (end >= start) ? static_cast<size_t>(end - start + 1) : 0;
        Game::Lua::PushLString(L, s + start - 1, static_cast<unsigned int>(len));
        return 1;
    }

    // Captures present → drop start & end, hand back only the captures.
    Game::Lua::Remove(L, kBase + 1); // remove start; end → kBase+1, caps → kBase+2..
    Game::Lua::Remove(L, kBase + 1); // remove end;   caps → kBase+1..
    return nres - 2;
}

// `string.gmatch` is 5.0's `string.gfind` verbatim — register the engine
// function pointer directly rather than wrapping it.
const auto Script_string_gmatch =
    reinterpret_cast<Game::Lua::CFunction>(Offsets::FUN_LUA_STR_GFIND);

// `strsplit(sep, str [, pieces])` — the WoW global (also aliased as
// `string.split`), split `str` on ANY character in `sep` and return the
// pieces as multiple values. `pieces > 0` caps the result count, with the
// unsplit remainder as the final piece; `0` / omitted = unlimited. Ported
// from 3.3.5's `strsplit` (FUN_00816a60) with one deliberate change: we do
// NOT `lua_settop(L, 0)` up front. Popping the args would leave the source
// string unreferenced, so a GC step triggered by a `pushlstring` could free
// the very buffer we're still scanning. Keeping the args on the stack keeps
// the source GC-rooted; Lua returns the top N values (our pieces) regardless
// of the args sitting below them. Each push is guarded by `lua_checkstack`
// (this pushes an unbounded number of results), erroring like 3.3.5 on
// genuine stack exhaustion.
int __fastcall Script_strsplit(void *L) {
    const char *sep = Game::Lua::ToString(L, 1);
    const char *str = Game::Lua::ToString(L, 2);
    if (sep == nullptr || str == nullptr) {
        Game::Lua::Error(L, "Usage: strsplit(\"separators\", str [, pieces])");
        return 0; // unreachable
    }
    const int pieces =
        Game::Lua::IsNumber(L, 3) ? static_cast<int>(Game::Lua::ToNumber(L, 3)) : 0;

    const char *segStart = str;
    int count = 0;
    // Only scan for separators while unlimited (pieces == 0) or the cap
    // hasn't been reached (pieces > 1). pieces == 1 (or <= 0 but non-zero)
    // yields the whole string as a single piece — matches 3.3.5.
    if (pieces == 0 || pieces > 1) {
        for (const char *p = str; *p != '\0'; ++p) {
            bool isSep = false;
            for (const char *s = sep; *s != '\0'; ++s) {
                if (*p == *s) {
                    isSep = true;
                    break;
                }
            }
            if (!isSep)
                continue;
            ++count;
            if (Game::Lua::CheckStack(L, count) == 0) {
                Game::Lua::Error(L, "strsplit(): Stack overflow");
                return 0; // unreachable
            }
            Game::Lua::PushLString(L, segStart,
                                   static_cast<unsigned int>(p - segStart));
            segStart = p + 1;
            if (count == pieces - 1)
                break; // cap hit; remainder becomes the final piece
        }
    }
    if (Game::Lua::CheckStack(L, count + 1) == 0) {
        Game::Lua::Error(L, "strsplit(): Stack overflow");
        return 0; // unreachable
    }
    Game::Lua::PushString(L, segStart); // final piece (remainder to end)
    return count + 1;
}

void RegisterFns() {
    Game::Lua::RegisterTableFunction("string", "match", &Script_string_match);
    Game::Lua::RegisterTableFunction("string", "gmatch", Script_string_gmatch);
    Game::Lua::RegisterGlobalFunction("strsplit", &Script_strsplit);
}

// Both states are Lua 5.0 and equally missing these; RegisterTableFunction
// targets whichever state is active in each registration hook.
const Game::ModuleAutoRegister _autoreg{&RegisterFns};
const Game::GlueModuleAutoRegister _autoregGlue{&RegisterFns};

} // namespace

} // namespace BaseLib::StringLib
