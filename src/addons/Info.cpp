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

// `C_AddOns.*` convenience wrappers. Modern WoW splits the legacy
// `GetAddOnInfo(arg) → 7-tuple` into a half-dozen single-field getters;
// addons backporting from later expansions call them directly and break
// without these wrappers.
//
// Implementation: the engine has dedicated per-field accessors
// (`GetTitleByName`, `GetNotesByName`, etc.) that the original
// `Script_GetAddOnInfo` calls internally. We dispatch to those directly,
// bypassing the 7-field push/drop dance and the `lua_error` that
// `Script_GetAddOnInfo` raises on out-of-range numeric indices.

#include "Game.h"
#include "Offsets.h"

#include <cstring>

namespace AddOns::Info {

namespace {

using ScriptFn_t = int(__fastcall *)(void *L);
using GetByIndex_t = void *(__fastcall *)(int idx0Based);
using FieldByName_t = const char *(__fastcall *)(const char *name);

// Resolves the user's arg (number = 1-based index, string = name)
// to a `const char *` pointing to the addon's name, or nullptr if
// the addon doesn't exist.
//
// - Numeric arg: `GetByIndex(idx-1)` returns the AddOnEntry pointer
//   (or NULL for out-of-range). The entry's first 12 bytes are an
//   inline null-terminated name buffer, so the entry pointer doubles
//   as a `const char *` for the addon's name. Same trick
//   `Script_GetAddOnInfo` uses for ret1.
// - String arg: validate existence via `GetTitleByName` (returns
//   non-NULL only if the addon is registered AND has a `## Title:`
//   in its .toc — every real addon has one in practice). Return the
//   input string as the canonical name.
//
// Returned pointer is a borrowed reference — either into the engine's
// registry (numeric path) or into the Lua-managed string buffer at
// `L[1]` (string path). Don't outlive the call frame.
const char *ResolveAddOnName(void *L) {
    const int t = Game::Lua::Type(L, 1);
    if (t == Game::Lua::TYPE_NUMBER) {
        const int idx = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
        auto fn = reinterpret_cast<GetByIndex_t>(Offsets::FUN_ADDON_GET_BY_INDEX);
        return static_cast<const char *>(fn(idx));
    }
    if (t == Game::Lua::TYPE_STRING) {
        const char *name = Game::Lua::ToString(L, 1);
        if (name == nullptr)
            return nullptr;
        auto fn = reinterpret_cast<FieldByName_t>(
            Offsets::FUN_ADDON_GET_TITLE_BY_NAME);
        if (fn(name) == nullptr)
            return nullptr; // unknown addon
        return name;
    }
    return nullptr;
}

// Pushes either the string returned by `accessor(name)` or nil if the
// accessor returns NULL. `lua_pushstring` already tail-calls
// `lua_pushnil` on a NULL string, but we keep an explicit branch so
// the early-out for "addon doesn't exist" is obvious.
int PushFieldOrNil(void *L, FieldByName_t accessor) {
    const char *name = ResolveAddOnName(L);
    if (name == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::PushString(L, accessor(name));
    return 1;
}

// `C_AddOns.GetAddOnName(indexOrName)` — the addon's directory name.
// For numeric input, that's the engine's canonical casing (whatever
// the .toc folder is on disk). For string input, the input is
// returned verbatim once existence is confirmed.
int __fastcall Script_GetAddOnName(void *L) {
    const char *name = ResolveAddOnName(L);
    Game::Lua::PushString(L, name); // tail-calls PushNil for NULL
    return 1;
}

// `C_AddOns.GetAddOnTitle(indexOrName)` — the `## Title:` from the
// .toc, with color-code escapes applied. nil for missing addons or
// addons without a title.
int __fastcall Script_GetAddOnTitle(void *L) {
    return PushFieldOrNil(L, reinterpret_cast<FieldByName_t>(
                                 Offsets::FUN_ADDON_GET_TITLE_BY_NAME));
}

// `C_AddOns.GetAddOnNotes(indexOrName)` — the `## Notes:` from the
// .toc. nil for missing addons or addons without notes.
int __fastcall Script_GetAddOnNotes(void *L) {
    return PushFieldOrNil(L, reinterpret_cast<FieldByName_t>(
                                 Offsets::FUN_ADDON_GET_NOTES_BY_NAME));
}

// Calls `Script_GetAddOnInfo` with the user's arg already at idx 1
// and returns the count of values it pushed. Used by the still-
// dispatching wrappers below (Loadable, Security) — replicating
// those engine accessors would require more disassembly than the
// wrappers are worth, so we keep paying the 7-field push cost for
// these two.
int CallGetAddOnInfo(void *L) {
    auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_ADDON_INFO);
    return fn(L);
}

// Picks the `returnPos`-th return (1-based) of `Script_GetAddOnInfo`
// and leaves it as the topmost value on the Lua stack. Lua takes
// the topmost N values when a C function returns N, so trailing
// junk below the desired value doesn't matter — we just truncate.
//
// Gates on `ResolveAddOnName` first to avoid `Script_GetAddOnInfo`'s
// `lua_error` path on out-of-range numeric input.
int ReturnNthFromGetAddOnInfo(void *L, int returnPos) {
    if (ResolveAddOnName(L) == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::SetTop(L, 1);
    const int pushed = CallGetAddOnInfo(L);
    if (pushed < returnPos) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::SetTop(L, 1 + returnPos);
    return 1;
}

// `C_AddOns.IsAddOnLoadable(indexOrName [, character, demandLoaded])`
// — returns `(loadable, reason)`. Modern signature has two optional
// args (`character`, `demandLoaded`) that we currently ignore;
// vanilla 1.12 has no per-character addon enable state and no
// demand-load distinction the engine exposes.
//
// `loadable` is a real boolean (modern semantics), not the engine's
// 1/nil. `reason` is a status string like `"INSECURE"`, `"BANNED"`,
// `"DISABLED"`, etc. — drawn from the addon-status table at
// `0x00853660` — or `nil` when the addon is loadable.
int __fastcall Script_IsAddOnLoadable(void *L) {
    if (ResolveAddOnName(L) == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        Game::Lua::PushNil(L);
        return 2;
    }
    Game::Lua::SetTop(L, 1);
    const int pushed = CallGetAddOnInfo(L);

    // Snapshot ret5 (loadable flag, idx 6) and ret6 (reason string,
    // idx 7) before we tear down the stack. The reason pointer
    // returned by `lua_tostring` is owned by the Lua string object
    // — we copy into a stack buffer so it survives the SetTop(0)
    // below.
    bool loadable = false;
    char reasonBuf[128] = {0};
    if (pushed >= 5) {
        loadable = (Game::Lua::Type(L, 6) != Game::Lua::TYPE_NIL);
    }
    if (pushed >= 6) {
        const char *reason = Game::Lua::ToString(L, 7);
        if (reason != nullptr) {
            const size_t len = std::strlen(reason);
            const size_t copyLen = (len < sizeof(reasonBuf) - 1)
                                       ? len
                                       : sizeof(reasonBuf) - 1;
            std::memcpy(reasonBuf, reason, copyLen);
            reasonBuf[copyLen] = '\0';
        }
    }

    Game::Lua::SetTop(L, 0);
    Game::Lua::PushBoolean(L, loadable ? 1 : 0);
    Game::Lua::PushString(L, reasonBuf[0] != 0 ? reasonBuf : nullptr);
    return 2;
}

// `C_AddOns.GetAddOnSecurity(indexOrName)` — 7th return ("SECURE"
// = Blizzard-signed, "INSECURE" = user addon). Engine derives this
// from a small status table indexed by a per-entry security value;
// also kept on the dispatch path for now.
int __fastcall Script_GetAddOnSecurity(void *L) {
    return ReturnNthFromGetAddOnInfo(L, 7);
}

// `C_AddOns.DoesAddOnExist(indexOrName)` — true iff the engine's
// addon registry has a matching entry. Same `ResolveAddOnName`
// gate everything else uses; non-NULL return means the addon is
// registered.
int __fastcall Script_DoesAddOnExist(void *L) {
    Game::Lua::PushBoolean(L, ResolveAddOnName(L) != nullptr ? 1 : 0);
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_AddOns", "GetAddOnName",
                                     &Script_GetAddOnName);
    Game::Lua::RegisterTableFunction("C_AddOns", "GetAddOnTitle",
                                     &Script_GetAddOnTitle);
    Game::Lua::RegisterTableFunction("C_AddOns", "GetAddOnNotes",
                                     &Script_GetAddOnNotes);
    Game::Lua::RegisterTableFunction("C_AddOns", "IsAddOnLoadable",
                                     &Script_IsAddOnLoadable);
    Game::Lua::RegisterTableFunction("C_AddOns", "GetAddOnSecurity",
                                     &Script_GetAddOnSecurity);
    Game::Lua::RegisterTableFunction("C_AddOns", "DoesAddOnExist",
                                     &Script_DoesAddOnExist);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace AddOns::Info
