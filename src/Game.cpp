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
#include "MinHook.h"
#include "Offsets.h"

namespace Game {

namespace Lua {
// Each entry binds a typed function pointer in `Game::Lua::` to the
// corresponding raw VA in `Offsets`. The X-macro keeps the column-aligned
// list visually scannable and removes 27 lines of identical cast boilerplate.
#define CLASSICAPI_LUA_BINDINGS(F)              \
    F(IsNumber,    lua_isnumber,    LUA_IS_NUMBER)    \
    F(IsString,    lua_isstring,    LUA_IS_STRING)    \
    F(ToNumber,    lua_tonumber,    LUA_TO_NUMBER)    \
    F(ToBoolean,   lua_toboolean,   LUA_TO_BOOLEAN)   \
    F(ToString,    lua_tostring,    LUA_TO_STRING)    \
    F(StrLen,      lua_strlen,      LUA_STR_LEN)      \
    F(PushNumber,  lua_pushnumber,  LUA_PUSH_NUMBER)  \
    F(PushNil,     lua_pushnil,     LUA_PUSH_NIL)     \
    F(PushBoolean, lua_pushboolean, LUA_PUSH_BOOLEAN) \
    F(PushString,  lua_pushstring,  LUA_PUSH_STRING)  \
    F(PushLString, lua_pushlstring, LUA_PUSH_LSTRING) \
    F(PushValue,   lua_pushvalue,   LUA_PUSH_VALUE)   \
    F(PushCClosure,lua_pushcclosure,LUA_PUSH_CCLOSURE)\
    F(NewTable,    lua_newtable,    LUA_NEW_TABLE)    \
    F(GetTable,    lua_gettable,    LUA_GET_TABLE)    \
    F(RawGet,      lua_rawget,      LUA_RAW_GET)      \
    F(SetTable,    lua_settable,    LUA_SET_TABLE)    \
    F(RawSet,      lua_rawset,      LUA_RAW_SET)      \
    F(Insert,      lua_insert,      LUA_INSERT)       \
    F(Remove,      lua_remove,      LUA_REMOVE)       \
    F(GetTop,      lua_gettop,      LUA_GET_TOP)      \
    F(SetTop,      lua_settop,      LUA_SET_TOP)      \
    F(Call,        lua_call,        LUA_CALL)         \
    F(PCall,       lua_pcall,       LUA_PCALL)        \
    F(Next,        lua_next,        LUA_NEXT)         \
    F(Type,        lua_type,        LUA_TYPE)         \
    F(Error,       lua_error,       LUA_ERROR)

#define CLASSICAPI_BIND_LUA(Name, Typedef, Offset) \
    const Typedef##_t Name = reinterpret_cast<Typedef##_t>(Offsets::Offset);
CLASSICAPI_LUA_BINDINGS(CLASSICAPI_BIND_LUA)
#undef CLASSICAPI_BIND_LUA
#undef CLASSICAPI_LUA_BINDINGS

namespace {
using FrameScript_RegisterFunction_t = void(__fastcall *)(const char *name, CFunction func);
using RegisterFrameMethods_t = void(__fastcall *)(const FrameMethodEntry *table, int count,
                                                  void *context);
} // namespace

void *State() {
    return *reinterpret_cast<void **>(static_cast<uintptr_t>(Offsets::VAR_LUA_STATE));
}

namespace {
using FrameScriptPushObject_t = void(__fastcall *)(void *L, int idx, int unused);
using FrameScriptGetObject_t = void *(__fastcall *)(void *L, int idx);
} // namespace

void *ResolveObject(void *L, int idx) {
    auto PushObject = reinterpret_cast<FrameScriptPushObject_t>(
        Offsets::FUN_FRAMESCRIPT_PUSH_OBJECT);
    auto GetObject = reinterpret_cast<FrameScriptGetObject_t>(
        Offsets::FUN_FRAMESCRIPT_GET_OBJECT);
    PushObject(L, idx, 0);
    void *result = GetObject(L, -1);
    SetTop(L, -2);
    return result;
}

void RegisterGlobalFunction(const char *name, CFunction func) {
    auto fn = reinterpret_cast<FrameScript_RegisterFunction_t>(
        Offsets::FUN_FRAMESCRIPT_REGISTER_FUNCTION);
    fn(name, func);
}

// Aliased to the same engine entry; the engine reads VAR_LUA_STATE
// internally to decide which state to write to, and the glue hook
// runs while that pointer is set to the glue state. Wrapped as a
// named function (rather than `using RegisterGlueFunction = ...`) so
// callers express intent at the call site.
void RegisterGlueFunction(const char *name, CFunction func) {
    auto fn = reinterpret_cast<FrameScript_RegisterFunction_t>(
        Offsets::FUN_FRAMESCRIPT_REGISTER_FUNCTION);
    fn(name, func);
}

void RegisterFrameMethods(void *context, const FrameMethodEntry *table, int count) {
    auto fn = reinterpret_cast<RegisterFrameMethods_t>(Offsets::FUN_REGISTER_FRAME_METHODS);
    fn(table, count, context);
}

// Looks up `_G[name]`. If absent, creates a fresh table and binds it.
// Leaves the resulting table on top of the stack.
namespace {
void EnsureGlobalTable(void *L, const char *name) {
    PushString(L, name);
    GetTable(L, GLOBALS_INDEX);
    if (Type(L, -1) == TYPE_TABLE)
        return;
    SetTop(L, -2);                 // pop the non-table.        []
    NewTable(L);                   //                           [tbl]
    PushValue(L, -1);              //                           [tbl, tbl]
    PushString(L, name);           //                           [tbl, tbl, name]
    Insert(L, -2);                 //                           [tbl, name, tbl]
    SetTable(L, GLOBALS_INDEX);    // _G[name] = tbl; pops k+v. [tbl]
}
} // namespace

// Registers `func` at `_G[tableName][methodName]`. If the namespace
// doesn't already exist, creates an empty table for it.
void RegisterTableFunction(const char *tableName, const char *methodName, CFunction func) {
    void *L = State();
    if (L == nullptr)
        return;
    EnsureGlobalTable(L, tableName);     // [tbl]
    PushString(L, methodName);           // [tbl, methodName]
    PushCClosure(L, func, 0);            // [tbl, methodName, closure]
    SetTable(L, -3);                     // tbl[m]=c; pops k+v. [tbl]
    SetTop(L, -2);                       // pop tbl. []
}

void RegisterIntegerEnum(const char *parent, const char *sub,
                         const EnumIntegerEntry *entries, int count) {
    void *L = State();
    if (L == nullptr)
        return;
    EnsureGlobalTable(L, parent); // [parentTbl]
    PushString(L, sub);           // [parentTbl, subName]
    NewTable(L);                  // [parentTbl, subName, subTbl]
    for (int i = 0; i < count; ++i) {
        PushString(L, entries[i].key);
        PushNumber(L, static_cast<double>(entries[i].value));
        SetTable(L, -3); // subTbl[key] = value
    }
    SetTable(L, -3); // parentTbl[sub] = subTbl
    SetTop(L, -2);   // pop parentTbl
}

void SetFieldNumber(void *L, const char *key, double value) {
    PushString(L, key);
    PushNumber(L, value);
    SetTable(L, -3);
}

void SetFieldString(void *L, const char *key, const char *value) {
    PushString(L, key);
    PushString(L, value != nullptr ? value : "");
    SetTable(L, -3);
}

void SetFieldBool(void *L, const char *key, bool value) {
    PushString(L, key);
    PushBoolean(L, static_cast<int>(value));
    SetTable(L, -3);
}

void SetGlobalNumber(void *L, const char *name, double value) {
    PushString(L, name);
    PushNumber(L, value);
    RawSet(L, GLOBALS_INDEX);
}
} // namespace Lua

namespace {
ModuleAutoRegister *g_moduleHead = nullptr;
GlueModuleAutoRegister *g_glueModuleHead = nullptr;
HookAutoRegister *g_hookHead = nullptr;
} // namespace

ModuleAutoRegister::ModuleAutoRegister(Fn f) : fn(f), next(g_moduleHead) {
    g_moduleHead = this;
}

void RunModuleRegistrations() {
    for (auto *node = g_moduleHead; node != nullptr; node = node->next)
        node->fn();
}

GlueModuleAutoRegister::GlueModuleAutoRegister(Fn f)
    : fn(f), next(g_glueModuleHead) {
    g_glueModuleHead = this;
}

void RunGlueModuleRegistrations() {
    for (auto *node = g_glueModuleHead; node != nullptr; node = node->next)
        node->fn();
}

HookAutoRegister::HookAutoRegister(uintptr_t target, void *hook, void **original)
    : target(target), hook(hook), original(original), next(g_hookHead) {
    g_hookHead = this;
}

bool RunHookRegistrations() {
    for (auto *node = g_hookHead; node != nullptr; node = node->next) {
        auto *targetPtr = reinterpret_cast<LPVOID>(node->target);
        if (MH_CreateHook(targetPtr, node->hook,
                          reinterpret_cast<LPVOID *>(node->original)) != MH_OK)
            return false;
        if (MH_EnableHook(targetPtr) != MH_OK)
            return false;
    }
    return true;
}

} // namespace Game
