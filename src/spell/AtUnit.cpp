// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

// `C_Spell.CastAtUnit(spellID, unit)` — cast a ground-target spell at
// the given unit's feet, bypassing the manual click on the AoE
// reticle. `CastAtUnit(spellID, "player")` drops it at the player's
// position, `"target"` at the target's, etc.
//
// Same two-step shape as `C_Spell.CastAtCursor`, but the placement is
// committed at the unit's world position instead of the cursor
// raycast:
//
//   1. `Spell::AtCursor::DispatchSpellCast(spellID)` initiates the
//      cast; the engine enters placement mode for a ground-target
//      spell.
//   2. Read the unit's world position (vtable `GetPosition`) and
//      `Spell::AtCursor::CommitAtCoords()` commits the placement
//      there.
//
// Returns `true` when the placement was committed at the unit; `false`
// when the spell isn't ground-target (placement cancelled), the spell
// isn't in the spellbook, or the unit has no resolvable position. The
// cast itself still proceeds for non-ground spells, just with no
// implicit target — matching `CastAtCursor`'s semantics.

#include "Game.h"
#include "spell/AtCursor.h"
#include "spell/MacroPrimarySpell.h"
#include "unit/Position.h"

namespace Spell::AtUnit {

namespace {

int __fastcall Script_C_Spell_CastAtUnit(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsString(L, 2)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const char *token = Game::Lua::ToString(L, 2);

    // Resolve the unit's position before dispatching — fail fast so a
    // bad/absent unit doesn't leave the engine sitting in placement
    // mode. (A genuinely unrecognized token string raises the engine's
    // standard "Unknown unit" error inside the resolver, same as
    // `UnitHealth("garbage")`.)
    float pos[3];
    if (!Unit::Position::ReadToken(token, pos)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }

    if (!Spell::AtCursor::DispatchSpellCast(spellID)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }

    const bool placed = Spell::AtCursor::CommitAtCoords(pos);
    Game::Lua::PushBool(L, placed);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "CastAtUnit",
                                     &Script_C_Spell_CastAtUnit);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

// Macro-parser pattern: tag macros calling
// `C_Spell.CastAtUnit(<spellID>, ...)` with the spellID so action-bar
// UIs highlight the slot. The first arg is the numeric spellID, so the
// extractor just reads the leading numeric literal — same shape as
// `CastAtCursor`'s.
int ExtractCastAtUnitArg(const char *p, const char *end) {
    while (p < end && (*p == ' ' || *p == '\t'))
        ++p;
    int value = 0;
    const char *digitStart = p;
    while (p < end && *p >= '0' && *p <= '9') {
        if (value > 100000000) // sanity cap before overflow
            return 0;
        value = value * 10 + (*p - '0');
        ++p;
    }
    return (p > digitStart && value > 0) ? value : 0;
}

const Spell::MacroPrimarySpell::PatternAutoRegister _patreg{
    "C_Spell.CastAtUnit(", &ExtractCastAtUnitArg};

} // namespace

} // namespace Spell::AtUnit
