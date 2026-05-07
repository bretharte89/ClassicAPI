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
using FrameScript_Execute_t = bool(__fastcall *)(const char *script, const char *scriptName);
using LoadScriptFunctions_t = void(__fastcall *)();

namespace Lua {
using lua_isnumber_t = bool(__fastcall *)(void *L, int index);
using lua_tonumber_t = double(__fastcall *)(void *L, int index);
using lua_pushnumber_t = void(__fastcall *)(void *L, double n);
using lua_pushnil_t = void(__fastcall *)(void *L);
using lua_pushboolean_t = void(__fastcall *)(void *L, int b);
using lua_pushstring_t = void(__fastcall *)(void *L, const char *s);
using lua_type_t = int(__fastcall *)(void *L, int index);
using lua_error_t = void(__cdecl *)(void *L, const char *);

extern const lua_isnumber_t IsNumber;
extern const lua_tonumber_t ToNumber;
extern const lua_pushnumber_t PushNumber;
extern const lua_pushnil_t PushNil;
extern const lua_pushboolean_t PushBoolean;
extern const lua_pushstring_t PushString;
extern const lua_type_t Type;
extern const lua_error_t Error;
} // namespace Lua

extern const FrameScript_Execute_t FrameScript_Execute;

} // namespace Game
