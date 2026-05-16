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
const lua_isnumber_t IsNumber = reinterpret_cast<lua_isnumber_t>(Offsets::LUA_IS_NUMBER);
const lua_isstring_t IsString = reinterpret_cast<lua_isstring_t>(Offsets::LUA_IS_STRING);
const lua_tonumber_t ToNumber = reinterpret_cast<lua_tonumber_t>(Offsets::LUA_TO_NUMBER);
const lua_toboolean_t ToBoolean = reinterpret_cast<lua_toboolean_t>(Offsets::LUA_TO_BOOLEAN);
const lua_tostring_t ToString = reinterpret_cast<lua_tostring_t>(Offsets::LUA_TO_STRING);
const lua_strlen_t StrLen = reinterpret_cast<lua_strlen_t>(Offsets::LUA_STR_LEN);
const lua_pushnumber_t PushNumber = reinterpret_cast<lua_pushnumber_t>(Offsets::LUA_PUSH_NUMBER);
const lua_pushnil_t PushNil = reinterpret_cast<lua_pushnil_t>(Offsets::LUA_PUSH_NIL);
const lua_pushboolean_t PushBoolean = reinterpret_cast<lua_pushboolean_t>(Offsets::LUA_PUSH_BOOLEAN);
const lua_pushstring_t PushString = reinterpret_cast<lua_pushstring_t>(Offsets::LUA_PUSH_STRING);
const lua_pushlstring_t PushLString = reinterpret_cast<lua_pushlstring_t>(Offsets::LUA_PUSH_LSTRING);
const lua_pushvalue_t PushValue = reinterpret_cast<lua_pushvalue_t>(Offsets::LUA_PUSH_VALUE);
const lua_pushcclosure_t PushCClosure = reinterpret_cast<lua_pushcclosure_t>(Offsets::LUA_PUSH_CCLOSURE);
const lua_newtable_t NewTable = reinterpret_cast<lua_newtable_t>(Offsets::LUA_NEW_TABLE);
const lua_gettable_t GetTable = reinterpret_cast<lua_gettable_t>(Offsets::LUA_GET_TABLE);
const lua_rawget_t RawGet = reinterpret_cast<lua_rawget_t>(Offsets::LUA_RAW_GET);
const lua_settable_t SetTable = reinterpret_cast<lua_settable_t>(Offsets::LUA_SET_TABLE);
const lua_rawset_t RawSet = reinterpret_cast<lua_rawset_t>(Offsets::LUA_RAW_SET);
const lua_insert_t Insert = reinterpret_cast<lua_insert_t>(Offsets::LUA_INSERT);
const lua_remove_t Remove = reinterpret_cast<lua_remove_t>(Offsets::LUA_REMOVE);
const lua_gettop_t GetTop = reinterpret_cast<lua_gettop_t>(Offsets::LUA_GET_TOP);
const lua_settop_t SetTop = reinterpret_cast<lua_settop_t>(Offsets::LUA_SET_TOP);
const lua_call_t Call = reinterpret_cast<lua_call_t>(Offsets::LUA_CALL);
const lua_pcall_t PCall = reinterpret_cast<lua_pcall_t>(Offsets::LUA_PCALL);
const lua_next_t Next = reinterpret_cast<lua_next_t>(Offsets::LUA_NEXT);
const lua_type_t Type = reinterpret_cast<lua_type_t>(Offsets::LUA_TYPE);
const lua_error_t Error = reinterpret_cast<lua_error_t>(Offsets::LUA_ERROR);

namespace {
using FrameScript_RegisterFunction_t = void(__fastcall *)(const char *name, CFunction func);
using RegisterFrameMethods_t = void(__fastcall *)(const FrameMethodEntry *table, int count,
                                                  void *context);
} // namespace

void *State() {
    return *reinterpret_cast<void **>(static_cast<uintptr_t>(Offsets::VAR_LUA_STATE));
}

void RegisterGlobalFunction(const char *name, CFunction func) {
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
//
// The implementation deliberately avoids the `lua_pushvalue` + `lua_insert`
// combo we initially tried: that path was producing stack states that ended
// up with `_G[name]` aliased to one of the Lua standard library tables
// (`bit`, `table`, etc.), with our intended field write never landing.
// Re-fetching the namespace from globals after creating it is cheap and
// sidesteps the issue entirely.
namespace {
void EnsureGlobalTable(void *L, const char *name) {
    PushString(L, name);
    GetTable(L, GLOBALS_INDEX);
    if (Type(L, -1) == TYPE_TABLE)
        return;
    SetTop(L, -2); // pop the non-table
    PushString(L, name);
    NewTable(L);
    SetTable(L, GLOBALS_INDEX);
    PushString(L, name);
    GetTable(L, GLOBALS_INDEX);
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
    PushBoolean(L, value ? 1 : 0);
    SetTable(L, -3);
}
} // namespace Lua

namespace {
ModuleAutoRegister *g_moduleHead = nullptr;
HookAutoRegister *g_hookHead = nullptr;
} // namespace

ModuleAutoRegister::ModuleAutoRegister(Fn f) : fn(f), next(g_moduleHead) {
    g_moduleHead = this;
}

void RunModuleRegistrations() {
    for (auto *node = g_moduleHead; node != nullptr; node = node->next)
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
