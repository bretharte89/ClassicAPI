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

namespace Game {

namespace Lua {
const lua_isnumber_t IsNumber = reinterpret_cast<lua_isnumber_t>(Offsets::LUA_IS_NUMBER);
const lua_tonumber_t ToNumber = reinterpret_cast<lua_tonumber_t>(Offsets::LUA_TO_NUMBER);
const lua_type_t Type = reinterpret_cast<lua_type_t>(Offsets::LUA_TYPE);
const lua_error_t Error = reinterpret_cast<lua_error_t>(Offsets::LUA_ERROR);
} // namespace Lua

const FrameScript_Execute_t FrameScript_Execute =
    reinterpret_cast<FrameScript_Execute_t>(Offsets::FUN_FRAME_SCRIPT_EXECUTE);

} // namespace Game
