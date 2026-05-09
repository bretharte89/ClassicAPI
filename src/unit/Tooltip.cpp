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

#include <cstdint>
#include <cstring>

namespace Unit::Tooltip {

// `GameTooltip:SetUnitAura(unit, index, [filter])` — modern unified-aura
// method. 1.12 splits this into `SetUnitBuff` (slot 32) and
// `SetUnitDebuff` (slot 33); we dispatch to the right one based on the
// `filter` string ("HELPFUL" → SetUnitBuff, "HARMFUL" → SetUnitDebuff).
//
// `filter` defaults to "HELPFUL" when omitted, matching modern
// behavior. Anything other than a HARMFUL match goes to SetUnitBuff.
static int __fastcall Script_GameTooltipSetUnitAura(void *L) {
    // Args: self (table), unit (string), index (number), filter (optional string)
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetUnitAura(unit, index, [filter])");
        return 0;
    }
    if (!Game::Lua::IsString(L, 2) || !Game::Lua::IsNumber(L, 3)) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetUnitAura(unit, index, [filter])");
        return 0;
    }

    bool isHarmful = false;
    if (Game::Lua::Type(L, 4) == Game::Lua::TYPE_STRING) {
        const char *filter = Game::Lua::ToString(L, 4);
        // Case-insensitive prefix check for "HARM" — covers "HARMFUL"
        // (the canonical filter) and any reasonable variant. Anything
        // else, including missing/empty, falls through to the buff path.
        if (filter != nullptr) {
            const char a = filter[0], b = filter[1], c = filter[2], d = filter[3];
            isHarmful = (a == 'H' || a == 'h') && (b == 'A' || b == 'a') &&
                        (c == 'R' || c == 'r') && (d == 'M' || d == 'm');
        }
    }

    // Drop the filter arg before calling — the existing
    // SetUnitBuff/SetUnitDebuff take (self, unit, index) only.
    Game::Lua::SetTop(L, 3);

    using Script_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<Script_t>(
        isHarmful ? Offsets::FUN_SCRIPT_GAMETOOLTIP_SET_UNIT_DEBUFF
                  : Offsets::FUN_SCRIPT_GAMETOOLTIP_SET_UNIT_BUFF);
    return fn(L);
}

static const Game::Lua::FrameMethodEntry g_methods[] = {
    {"SetUnitAura", &Script_GameTooltipSetUnitAura},
};

static void RegisterLuaFunctions() {
    Game::Lua::RegisterFrameMethods(
        reinterpret_cast<void *>(Offsets::VAR_GAMETOOLTIP_METHOD_REGISTRY),
        g_methods,
        static_cast<int>(sizeof(g_methods) / sizeof(g_methods[0])));
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Unit::Tooltip
