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

// `C_Spell.IsAutoAttackSpell` / `C_Spell.IsRangedAutoAttackSpell` and
// the spellbook-slot variants. Vanilla has exactly three auto-attack
// spells, and their IDs have been stable across every WoW version:
//
//   6603 — "Auto Attack"        (melee, all classes)
//     75 — "Auto Shot"          (Hunter ranged)
//   5019 — "Shoot"              (caster wand)
//
// Modern WoW's equivalents fold attribute / class-script checks on top
// of these IDs to handle proc-spawned variants and other corner cases
// that vanilla doesn't have. A hardcoded ID test is the same answer
// you'd get from a Spell.dbc walk here, with no surface area for
// false positives.

#include "Lookup.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstring>

namespace Spell::AutoAttack {

namespace {

constexpr int SPELL_AUTO_ATTACK = 6603;
constexpr int SPELL_AUTO_SHOT = 75;
constexpr int SPELL_SHOOT_WAND = 5019;

bool IsMelee(int spellID) {
    return spellID == SPELL_AUTO_ATTACK;
}

bool IsRanged(int spellID) {
    return spellID == SPELL_AUTO_SHOT || spellID == SPELL_SHOOT_WAND;
}

int ReadSpellID(void *L) {
    if (!Game::Lua::IsNumber(L, 1))
        return 0;
    return static_cast<int>(Game::Lua::ToNumber(L, 1));
}

int ReadSpellBookSlotID(void *L) {
    if (!Game::Lua::IsNumber(L, 1))
        return 0;
    const int slot = static_cast<int>(Game::Lua::ToNumber(L, 1));
    int bookType = 0;
    if (Game::Lua::IsString(L, 2)) {
        const char *s = Game::Lua::ToString(L, 2);
        if (s != nullptr && _stricmp(s, "pet") == 0)
            bookType = 1;
    } else if (Game::Lua::IsNumber(L, 2)) {
        bookType = static_cast<int>(Game::Lua::ToNumber(L, 2));
    }
    return Spell::Lookup::SpellbookSlotToID(slot, bookType);
}

} // namespace

static int __fastcall Script_IsAutoAttackSpell(void *L) {
    Game::Lua::PushBool(L, IsMelee(ReadSpellID(L)));
    return 1;
}

static int __fastcall Script_IsRangedAutoAttackSpell(void *L) {
    Game::Lua::PushBool(L, IsRanged(ReadSpellID(L)));
    return 1;
}

static int __fastcall Script_IsAutoAttackSpellBookItem(void *L) {
    Game::Lua::PushBool(L, IsMelee(ReadSpellBookSlotID(L)));
    return 1;
}

static int __fastcall Script_IsRangedAutoAttackSpellBookItem(void *L) {
    Game::Lua::PushBool(L, IsRanged(ReadSpellBookSlotID(L)));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "IsAutoAttackSpell",
                                     &Script_IsAutoAttackSpell);
    Game::Lua::RegisterTableFunction("C_Spell", "IsRangedAutoAttackSpell",
                                     &Script_IsRangedAutoAttackSpell);
    Game::Lua::RegisterTableFunction("C_SpellBook", "IsAutoAttackSpellBookItem",
                                     &Script_IsAutoAttackSpellBookItem);
    Game::Lua::RegisterTableFunction("C_SpellBook", "IsRangedAutoAttackSpellBookItem",
                                     &Script_IsRangedAutoAttackSpellBookItem);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::AutoAttack
