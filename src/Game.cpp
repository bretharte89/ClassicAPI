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

#include <cstdio>

namespace Game {

namespace Lua {
const lua_isnumber_t IsNumber = reinterpret_cast<lua_isnumber_t>(Offsets::LUA_IS_NUMBER);
const lua_isstring_t IsString = reinterpret_cast<lua_isstring_t>(Offsets::LUA_IS_STRING);
const lua_tonumber_t ToNumber = reinterpret_cast<lua_tonumber_t>(Offsets::LUA_TO_NUMBER);
const lua_toboolean_t ToBoolean = reinterpret_cast<lua_toboolean_t>(Offsets::LUA_TO_BOOLEAN);
const lua_tostring_t ToString = reinterpret_cast<lua_tostring_t>(Offsets::LUA_TO_STRING);
const lua_pushnumber_t PushNumber = reinterpret_cast<lua_pushnumber_t>(Offsets::LUA_PUSH_NUMBER);
const lua_pushnil_t PushNil = reinterpret_cast<lua_pushnil_t>(Offsets::LUA_PUSH_NIL);
const lua_pushboolean_t PushBoolean = reinterpret_cast<lua_pushboolean_t>(Offsets::LUA_PUSH_BOOLEAN);
const lua_pushstring_t PushString = reinterpret_cast<lua_pushstring_t>(Offsets::LUA_PUSH_STRING);
const lua_pushvalue_t PushValue = reinterpret_cast<lua_pushvalue_t>(Offsets::LUA_PUSH_VALUE);
const lua_pushcclosure_t PushCClosure = reinterpret_cast<lua_pushcclosure_t>(Offsets::LUA_PUSH_CCLOSURE);
const lua_newtable_t NewTable = reinterpret_cast<lua_newtable_t>(Offsets::LUA_NEW_TABLE);
const lua_gettable_t GetTable = reinterpret_cast<lua_gettable_t>(Offsets::LUA_GET_TABLE);
const lua_rawget_t RawGet = reinterpret_cast<lua_rawget_t>(Offsets::LUA_RAW_GET);
const lua_settable_t SetTable = reinterpret_cast<lua_settable_t>(Offsets::LUA_SET_TABLE);
const lua_rawset_t RawSet = reinterpret_cast<lua_rawset_t>(Offsets::LUA_RAW_SET);
const lua_insert_t Insert = reinterpret_cast<lua_insert_t>(Offsets::LUA_INSERT);
const lua_settop_t SetTop = reinterpret_cast<lua_settop_t>(Offsets::LUA_SET_TOP);
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

// Registers `func` at `_G[tableName][methodName]`. If the namespace doesn't
// already exist, creates an empty table for it. If it exists as a table,
// adds the field non-destructively (so multiple modules can share a
// namespace, e.g. several C_Item.* functions).
//
// The implementation deliberately avoids the `lua_pushvalue` + `lua_insert`
// combo we initially tried: that path was producing stack states that ended
// up with `_G[tableName]` aliased to one of the Lua standard library tables
// (`bit`, `table`, etc.), with our intended field write never landing.
// Re-fetching the namespace from globals after creating it is cheap and
// sidesteps the issue entirely.
void RegisterTableFunction(const char *tableName, const char *methodName, CFunction func) {
    void *L = State();
    if (L == nullptr)
        return;

    // Check if _G[tableName] already exists as a table.
    PushString(L, tableName);
    GetTable(L, GLOBALS_INDEX);
    const bool alreadyExists = (Type(L, -1) == TYPE_TABLE);
    SetTop(L, -2); // pop the lookup result

    if (!alreadyExists) {
        PushString(L, tableName);
        NewTable(L);
        SetTable(L, GLOBALS_INDEX);
    }

    // Re-fetch the namespace and write the field.
    PushString(L, tableName);
    GetTable(L, GLOBALS_INDEX);          // [tbl]
    PushString(L, methodName);           // [tbl, methodName]
    PushCClosure(L, func, 0);            // [tbl, methodName, closure]
    SetTable(L, -3);                     // pops methodName+closure: tbl[m]=c. [tbl]
    SetTop(L, -2);                       // pop tbl. []
}
} // namespace Lua

const FrameScript_Execute_t FrameScript_Execute =
    reinterpret_cast<FrameScript_Execute_t>(Offsets::FUN_FRAME_SCRIPT_EXECUTE);

namespace {
ModuleAutoRegister *g_moduleHead = nullptr;
} // namespace

ModuleAutoRegister::ModuleAutoRegister(Fn f) : fn(f), next(g_moduleHead) {
    g_moduleHead = this;
}

void RunModuleRegistrations() {
    for (auto *node = g_moduleHead; node != nullptr; node = node->next)
        node->fn();
}

} // namespace Game
