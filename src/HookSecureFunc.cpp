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

#include <cstring>

namespace HookSecureFunc {

namespace {

// Names that must not be hooked when targeting `_G`. Hooking any of these
// would either break taint propagation (the "secure" functions) or replace
// core Lua language primitives whose behavior the engine depends on
// internally — e.g. `pairs` is called from C in the iterator path; if Lua
// `pairs` is replaced with a wrapper closure, the engine still gets the
// original via its own table-method dispatch, but Lua-side iteration goes
// through our wrapper, and the two diverging is a footgun. Modern WoW
// rejects the hook outright; we mirror that. Sorted alphabetically.
const char *const kUnhookableNames[] = {
    "getfenv", "getmetatable", "hooksecurefunc", "ipairs", "issecurevalue",
    "issecurevariable", "next", "pairs", "pcall", "pcallwithenv", "rawget",
    "rawset", "scrub", "securecall", "securecallfunction",
    "secureexecuterange", "select", "setfenv", "setmetatable", "type",
    "unpack", "wipe", "xpcall",
};

bool IsUnhookable(const char *name) {
    if (name == nullptr)
        return false;
    for (const char *blocked : kUnhookableNames) {
        if (std::strcmp(name, blocked) == 0)
            return true;
    }
    return false;
}

// Wrapper closure body — called when the hooked function is invoked.
// Stack contains the wrapper's args (`[arg1..argN]`); the closure's
// upvalues hold `(orig, callback)`.
//
// Sequence:
//   1. Push orig + copies of args, `lua_call(MULTRET)` — pops them,
//      pushes orig's results. Stack: `[args..., results...]`.
//   2. Push callback + copies of args, `lua_call(0)` — pops, pushes
//      nothing (callback returns are discarded).
//   3. Remove the original args from the stack so only results
//      remain. Return `nresults`.
//
// Modern `hooksecurefunc` semantics: original runs first, callback
// runs after with the same args (returns ignored), original's
// results propagate to the caller. No cap on return count.
int __fastcall HookWrapper(void *L) {
    const int nargs = Game::Lua::GetTop(L);

    // (1) orig(...) — pops orig + nargs copies, pushes results
    Game::Lua::PushValue(L, Game::Lua::UpvalueIndex(1));
    for (int i = 1; i <= nargs; ++i)
        Game::Lua::PushValue(L, i);
    Game::Lua::Call(L, nargs, Game::Lua::MULTRET);
    const int nresults = Game::Lua::GetTop(L) - nargs;

    // (2) callback(...) — pops callback + nargs copies, pushes 0
    Game::Lua::PushValue(L, Game::Lua::UpvalueIndex(2));
    for (int i = 1; i <= nargs; ++i)
        Game::Lua::PushValue(L, i);
    Game::Lua::Call(L, nargs, 0);

    // (3) Remove the original args from the bottom of the stack so
    //     only `[results...]` remain. lua_remove(L, 1) shifts down
    //     each call.
    for (int i = 0; i < nargs; ++i)
        Game::Lua::Remove(L, 1);

    return nresults;
}

// `hooksecurefunc(name, callback)` — hooks a global by name, target
// table is `_G`.
// `hooksecurefunc(table, name, callback)` — hooks `table[name]`.
//
// In both cases the original function runs first and its results
// propagate; `callback` runs after with the same args (returns
// discarded). The "secure" label refers to taint propagation in
// 3.x+; on 1.12 it's just a post-hook with return preservation.
int __fastcall Script_HookSecureFunc(void *L) {
    int targetIdx, nameIdx, callbackIdx;
    const int t1 = Game::Lua::Type(L, 1);

    if (t1 == Game::Lua::TYPE_TABLE) {
        // Three-arg form: (table, name, callback)
        targetIdx = 1;
        nameIdx = 2;
        callbackIdx = 3;
    } else if (t1 == Game::Lua::TYPE_STRING) {
        // Two-arg form: (name, callback) — target = _G via pseudo-
        // index. Don't push _G with `lua_pushvalue(GLOBALS_INDEX)`:
        // it crashes (the pushvalue → setobj2s chain dereferences a
        // bad TValue pointer for that pseudo-index). `lua_gettable` /
        // `lua_settable` accept GLOBALS_INDEX as the table arg
        // directly, so we use it that way without pushing.
        targetIdx = Game::Lua::GLOBALS_INDEX;
        nameIdx = 1;
        callbackIdx = 2;
    } else {
        Game::Lua::Error(L,
            "hooksecurefunc(table, name, callback) or hooksecurefunc(name, callback)");
        return 0;
    }

    if (Game::Lua::Type(L, nameIdx) != Game::Lua::TYPE_STRING) {
        Game::Lua::Error(L, "hooksecurefunc: name must be a string");
        return 0;
    }
    if (Game::Lua::Type(L, callbackIdx) != Game::Lua::TYPE_FUNCTION) {
        Game::Lua::Error(L, "hooksecurefunc: callback must be a function");
        return 0;
    }

    // Block the hook when targeting `_G` and the name is in the unhookable
    // set. We don't apply this to the three-arg form (table targets):
    // hooking `MyLib.pairs` is fine even if Lua's global `pairs` is not —
    // the blacklist is specifically about `_G[name]`. A user who passes
    // `_G` explicitly as the table target is opting in deliberately and we
    // don't second-guess it.
    if (targetIdx == Game::Lua::GLOBALS_INDEX) {
        const char *name = Game::Lua::ToString(L, nameIdx);
        if (IsUnhookable(name)) {
            Game::Lua::Error(L,
                "hooksecurefunc: function is unhookable");
            return 0;
        }
    }

    // Fetch target[name] — the original function. `gettable` uses
    // metamethods, so frame methods (resolved via the frame
    // metatable's __index) come back as callable closures, exactly
    // what we want to wrap.
    Game::Lua::PushValue(L, nameIdx);
    Game::Lua::GetTable(L, targetIdx);
    if (Game::Lua::Type(L, -1) != Game::Lua::TYPE_FUNCTION) {
        Game::Lua::Error(L, "hooksecurefunc: target field is not a function");
        return 0;
    }

    // Push callback as the second upvalue; orig is already on top
    // from the GetTable above.
    Game::Lua::PushValue(L, callbackIdx);

    // Build the wrapper as a C closure with (orig, callback) as the
    // two upvalues. PushCClosure pops the upvalues from the stack
    // and pushes the closure.
    Game::Lua::PushCClosure(L, &HookWrapper, 2);

    // Install: target[name] = wrapper. SetTable expects the value
    // on top and the key just below; push them in that order.
    Game::Lua::PushValue(L, nameIdx);
    Game::Lua::PushValue(L, -2);  // duplicate the closure
    Game::Lua::SetTable(L, targetIdx);

    return 0;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("hooksecurefunc", &Script_HookSecureFunc);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace HookSecureFunc
