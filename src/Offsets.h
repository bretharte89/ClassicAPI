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

enum Offsets {
    FUN_FRAME_SCRIPT_INITIALIZE = 0x7039E0,
    FUN_FRAME_SCRIPT_EXECUTE = 0x704CD0,
    FUN_INVALID_FUNCTION_PTR_CHECK = 0x42A320,
    FUN_LOAD_SCRIPT_FUNCTIONS = 0x490250,

    // GameTooltip script-method prologue helpers (used to resolve self → CFrameScriptObject*).
    FUN_FRAMESCRIPT_PUSH_OBJECT = 0x6F3BC0,
    FUN_FRAMESCRIPT_GET_OBJECT = 0x6F3740,

    // Inner spell-tooltip builder, called from Script_GameTooltip_SetSpell at 0x00532E92.
    // __thiscall(spellID, 0, 0, isPet, 0, 0, 0); we always pass isPet=0.
    FUN_GAMETOOLTIP_BUILD_SPELL_TOOLTIP = 0x0052E610,

    // Iterator that registers an array of GameTooltip method bindings.
    // __fastcall(ecx = MethodEntry table, edx = count, [stack] = context).
    FUN_REGISTER_GAMETOOLTIP_METHODS = 0x00701D80,
    VAR_GAMETOOLTIP_METHOD_REGISTRY = 0x00C0CF20,

    LUA_IS_NUMBER = 0x6F34D0,
    LUA_TO_NUMBER = 0x6F3620,
    LUA_TYPE = 0x6F3400,
    LUA_SET_TOP = 0x6F3080,
    LUA_ERROR = 0x6F4940,
};
