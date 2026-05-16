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

#pragma once

#include <cstdint>

namespace Game {

using FrameScript_Initialize_t = bool(__fastcall *)();
using LoadScriptFunctions_t = void(__fastcall *)();

namespace Lua {
using CFunction = int(__fastcall *)(void *L);

// Lua 5.0 pseudo-index used to read/write entries on the globals table.
constexpr int GLOBALS_INDEX = -10001;
// Lua 5.0 pseudo-index for the registry table — a Lua-state-local
// table protected from script code, used by C modules to anchor
// values across calls (e.g. timer callbacks pinned against GC).
constexpr int REGISTRY_INDEX = -10000;
// LUA_UPVALUEINDEX(i) — pseudo-index for accessing the i-th upvalue
// of a C closure. Lua 5.0 layout: `LUA_GLOBALSINDEX - i`.
constexpr int UpvalueIndex(int i) { return GLOBALS_INDEX - i; }

// `lua_call` / `lua_pcall` nresults sentinel meaning "all".
constexpr int MULTRET = -1;

// Type tag values returned by `Type()` (lua_type).
constexpr int TYPE_NIL = 0;
constexpr int TYPE_BOOLEAN = 1;
constexpr int TYPE_LIGHTUSERDATA = 2;
constexpr int TYPE_NUMBER = 3;
constexpr int TYPE_STRING = 4;
constexpr int TYPE_TABLE = 5;
constexpr int TYPE_FUNCTION = 6;

using lua_isnumber_t = bool(__fastcall *)(void *L, int index);
using lua_isstring_t = bool(__fastcall *)(void *L, int index);
using lua_tonumber_t = double(__fastcall *)(void *L, int index);
using lua_toboolean_t = int(__fastcall *)(void *L, int index);
using lua_tostring_t = const char *(__fastcall *)(void *L, int index);
using lua_strlen_t = unsigned int(__fastcall *)(void *L, int index);
using lua_pushnumber_t = void(__fastcall *)(void *L, double n);
using lua_pushnil_t = void(__fastcall *)(void *L);
using lua_pushboolean_t = void(__fastcall *)(void *L, int b);
using lua_pushstring_t = void(__fastcall *)(void *L, const char *s);
using lua_pushlstring_t = void(__fastcall *)(void *L, const char *s, unsigned int len);
using lua_pushvalue_t = void(__fastcall *)(void *L, int idx);
using lua_pushcclosure_t = void(__fastcall *)(void *L, CFunction fn, int upvals);
using lua_newtable_t = void(__fastcall *)(void *L);
using lua_gettable_t = void(__fastcall *)(void *L, int idx);
using lua_rawget_t = void(__fastcall *)(void *L, int idx);
using lua_settable_t = void(__fastcall *)(void *L, int idx);
using lua_rawset_t = void(__fastcall *)(void *L, int idx);
using lua_insert_t = void(__fastcall *)(void *L, int idx);
using lua_remove_t = void(__fastcall *)(void *L, int idx);
using lua_gettop_t = int(__fastcall *)(void *L);
using lua_settop_t = void(__fastcall *)(void *L, int idx);
using lua_call_t = void(__fastcall *)(void *L, int nargs, int nresults);
using lua_pcall_t = int(__fastcall *)(void *L, int nargs, int nresults, int errfunc);
using lua_next_t = int(__fastcall *)(void *L, int idx);
using lua_type_t = int(__fastcall *)(void *L, int index);
using lua_error_t = void(__cdecl *)(void *L, const char *);

extern const lua_isnumber_t IsNumber;
extern const lua_isstring_t IsString;
extern const lua_tonumber_t ToNumber;
extern const lua_toboolean_t ToBoolean;
extern const lua_tostring_t ToString;
extern const lua_strlen_t StrLen;
extern const lua_pushnumber_t PushNumber;
extern const lua_pushnil_t PushNil;
extern const lua_pushboolean_t PushBoolean;
extern const lua_pushstring_t PushString;
extern const lua_pushlstring_t PushLString;
extern const lua_pushvalue_t PushValue;
extern const lua_pushcclosure_t PushCClosure;
extern const lua_newtable_t NewTable;
extern const lua_gettable_t GetTable;
extern const lua_rawget_t RawGet;
extern const lua_settable_t SetTable;
extern const lua_rawset_t RawSet;
extern const lua_insert_t Insert;
extern const lua_remove_t Remove;
extern const lua_gettop_t GetTop;
extern const lua_settop_t SetTop;
extern const lua_call_t Call;
extern const lua_pcall_t PCall;
extern const lua_next_t Next;
extern const lua_type_t Type;
extern const lua_error_t Error;

// Returns the global `lua_State *` (read on demand from the engine's global).
// Callable outside a Lua callback, e.g. during LoadScriptFunctions setup.
void *State();

// Registers a single global Lua function (e.g. `GetSpellInfo`). The function
// must use the WoW Lua C function ABI: `int __fastcall(void *L)`.
void RegisterGlobalFunction(const char *name, CFunction func);

// Frame-method registration entry: { name, func } pairs walked by the engine's
// per-frame-type method-table iterator. Layout matches what the engine expects
// natively — name first, then function pointer.
struct FrameMethodEntry {
    const char *name;
    CFunction func;
};

// Registers a batch of methods on a per-frame-type registry (e.g.
// GameTooltipMethodRegistry for `tooltip:Foo()` calls). `context` is the
// registry address — see Offsets::VAR_*_METHOD_REGISTRY.
void RegisterFrameMethods(void *context, const FrameMethodEntry *table, int count);

// Registers `func` at `_G[tableName][methodName]`, creating the namespace
// table if it doesn't already exist. This is how modern WoW C_*-style APIs
// are bound — the engine has no built-in support for table-bound Lua
// functions, so we manipulate the globals table directly via the Lua C API.
void RegisterTableFunction(const char *tableName, const char *methodName,
                           CFunction func);

// Key/value pair for `RegisterIntegerEnum`. `key` becomes a field name
// (PascalCase, matching Blizzard's `Enum.*` naming) and `value` is the
// integer the enum field maps to.
struct EnumIntegerEntry {
    const char *key;
    int value;
};

// Registers `_G[parent][sub] = { entries }` as an integer-valued enum
// table, creating `_G[parent]` if needed. Used for Blizzard-style
// `Enum.AddOnSecurityStatus = { Secure=0, Insecure=1, ... }` shapes.
void RegisterIntegerEnum(const char *parent, const char *sub,
                         const EnumIntegerEntry *entries, int count);

// Set `t[key] = value` on the table currently at stack[-1] (the most
// common shape used when populating a struct-style table mid-build).
// Pushes the key + value, calls `SetTable(L, -3)`, which pops both and
// leaves the table on the stack — so it's safe to chain. NULL string
// values are coerced to `""` so callers don't have to gate each push.
void SetFieldNumber(void *L, const char *key, double value);
void SetFieldString(void *L, const char *key, const char *value);
void SetFieldBool(void *L, const char *key, bool value);
} // namespace Lua

// Self-registration for API modules. Each module .cpp declares a file-scope
// `static const Game::ModuleAutoRegister _r{&RegisterLuaFunctions};`, which
// chains itself onto a global list at DLL-load time. `RunModuleRegistrations`
// is called once from the LoadScriptFunctions post-hook to fire them all,
// so DllMain.cpp doesn't need to know the modules exist.
//
// Order is unspecified (LIFO of static-init order across TUs). Modules must
// not depend on each other's registration side effects.
struct ModuleAutoRegister {
    using Fn = void (*)();
    explicit ModuleAutoRegister(Fn fn);
    Fn fn;
    ModuleAutoRegister *next;
};

void RunModuleRegistrations();

// Declarative MinHook registration. Each feature module declares a
// file-scope `static const Game::HookAutoRegister _hookreg{target,
// &hook_fn, reinterpret_cast<void**>(&original_fn)};` and DllMain's
// `RunHookRegistrations` walks the list once after `MH_Initialize`,
// installing each hook with `MH_CreateHook` + `MH_EnableHook`.
//
// Same lifetime rules as ModuleAutoRegister: constructors chain onto
// a static-init list before DllMain runs, the linker keeps the OBJ
// because the constructor has side effects, and order across TUs is
// undefined but doesn't matter here (hooks are independent).
//
// Use only for feature hooks. The three core engine-init hooks in
// DllMain (FrameScript_Initialize / LoadScriptFunctions /
// Frame::RegisterEvent) have inline logic that interleaves with the
// hook chain and stays in DllMain.
struct HookAutoRegister {
    HookAutoRegister(uintptr_t target, void *hook, void **original);
    uintptr_t target;
    void *hook;
    void **original;
    HookAutoRegister *next;
};

// Installs every registered hook. Returns `false` and stops on first
// failure — caller (DllMain) should propagate by returning FALSE.
bool RunHookRegistrations();

} // namespace Game
