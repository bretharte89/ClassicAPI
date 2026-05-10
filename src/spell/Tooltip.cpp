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
#include "spell/Tooltip.h"

#include <cstdint>

namespace Spell::Tooltip {

using BuildSpellTooltip_t = void(__thiscall *)(void *thisObj, int spellID, int arg2, int arg3,
                                               int isPet, int arg5, int arg6, int arg7);

constexpr uintptr_t addrFrameScriptPushObject = Offsets::FUN_FRAMESCRIPT_PUSH_OBJECT;
constexpr uintptr_t addrFrameScriptGetObject = Offsets::FUN_FRAMESCRIPT_GET_OBJECT;
constexpr uintptr_t addrLuaSetTop = Offsets::LUA_SET_TOP;

// Mirrors the prologue of Script_GameTooltip_SetSpell at 0x00532D4E-0x00532D74:
// push 0; mov edx, 1; mov ecx, L; call FrameScript_PushObject  -- pushes self's CObject ref
// or  edx, -1; mov ecx, L;        call FrameScript_GetObject   -- reads the pointer back
// mov edx, -2; mov ecx, L;        call lua_settop              -- balances the stack
static void *ResolveTooltipObject(void *L) {
    void *result = nullptr;
    __asm {
        push 0
        mov  edx, 1
        mov  ecx, L
        mov  eax, addrFrameScriptPushObject
        call eax

        or   edx, -1
        mov  ecx, L
        mov  eax, addrFrameScriptGetObject
        call eax
        mov  result, eax

        mov  edx, -2
        mov  ecx, L
        mov  eax, addrLuaSetTop
        call eax
    }
    return result;
}

void ShowByID(void *L, int spellID) {
    if (spellID <= 0)
        return;
    void *tooltipObj = ResolveTooltipObject(L);
    if (tooltipObj == nullptr)
        return;
    auto BuildSpellTooltip =
        reinterpret_cast<BuildSpellTooltip_t>(Offsets::FUN_GAMETOOLTIP_BUILD_SPELL_TOOLTIP);
    BuildSpellTooltip(tooltipObj, spellID, 0, 0, 0, 0, 0, 0);
}

static int __fastcall Script_GameTooltipSetSpellByID(void *L) {
    if (Game::Lua::Type(L, 1) != 5) {
        Game::Lua::Error(L, "Usage: GameTooltipSetSpellByID(self, spellID)");
        return 0;
    }
    if (!Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: GameTooltipSetSpellByID(self, spellID)");
        return 0;
    }
    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 2));
    ShowByID(L, spellID);
    return 0;
}

static const Game::Lua::FrameMethodEntry g_methods[] = {
    {"SetSpellByID", &Script_GameTooltipSetSpellByID},
};

static void RegisterLuaFunctions() {
    Game::Lua::RegisterFrameMethods(
        reinterpret_cast<void *>(Offsets::VAR_GAMETOOLTIP_METHOD_REGISTRY),
        g_methods,
        static_cast<int>(sizeof(g_methods) / sizeof(g_methods[0])));
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::Tooltip
