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

    // Iterator that registers an array of frame-method bindings on a per-frame-type
    // method registry (e.g. VAR_GAMETOOLTIP_METHOD_REGISTRY for GameTooltip).
    // __fastcall(ecx = MethodEntry table, edx = count, [stack] = context).
    FUN_REGISTER_FRAME_METHODS = 0x00701D80,
    VAR_GAMETOOLTIP_METHOD_REGISTRY = 0x00C0CF20,

    // Registers a single global Lua function. __fastcall(name, func).
    FUN_FRAMESCRIPT_REGISTER_FUNCTION = 0x00704120,

    // Quest log: 16-byte-stride entry array and active count.
    // Field +0 of each entry is the questID for real quests (a category index
    // for headers); field +8 is the header indicator: non-NULL = header,
    // NULL = real quest. Verified by Script_GetQuestLogTitle's isHeader push
    // at 0x004DF9A9 and by the helper at 0x004DF150 used by IsUnitOnQuest.
    VAR_QUEST_LOG_ENTRIES = 0x00BB71C0,
    VAR_QUEST_LOG_ENTRY_COUNT = 0x00BB7478,

    // Spell.dbc and friends. Pointer-to-records-array + record-count pairs.
    // Used by Script_GetSpellName/Texture and BuildSpellTooltip.
    VAR_SPELL_RECORDS = 0x00C0D788,            // SpellRecord *records[spellID]
    VAR_SPELL_RECORD_COUNT = 0x00C0D78C,       // max spellID
    VAR_SPELL_RANGE_RECORDS = 0x00C0D79C,      // SpellRange.dbc
    VAR_SPELL_RANGE_COUNT = 0x00C0D7A0,
    VAR_SPELL_ICON_RECORDS = 0x00C0D7EC,       // SpellIcon.dbc
    VAR_SPELL_ICON_COUNT = 0x00C0D7F0,
    VAR_SPELL_CAST_TIMES_RECORDS = 0x00C0D878, // SpellCastTimes.dbc
    VAR_SPELL_CAST_TIMES_COUNT = 0x00C0D87C,
    VAR_LOCALE_INDEX = 0x00C0E080,             // 0..8, picks one of the 9 localized strings

    LUA_IS_NUMBER = 0x6F34D0,
    LUA_TO_NUMBER = 0x6F3620,
    LUA_PUSH_NUMBER = 0x6F3810,
    LUA_PUSH_NIL = 0x6F37F0,
    LUA_PUSH_BOOLEAN = 0x6F39F0,
    LUA_PUSH_STRING = 0x6F3890,
    LUA_TYPE = 0x6F3400,
    LUA_SET_TOP = 0x6F3080,
    LUA_ERROR = 0x6F4940,
};
