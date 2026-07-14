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

// `C_Spell.IsSpellInRange(spellIdentifier, targetUnit)` — true if the
// spell is in range of the unit, false if out of range, nil when the
// range check doesn't apply (rangeless spell, or an unresolvable
// spell / unit). `spellIdentifier` is a spellID, spell link, or the
// name of a spell in the player's spellbook; `targetUnit` is a unit
// token ("target", "mouseover", "party1", …).
//
// Backed by the engine's own geometric range core `FUN_006E47B0` — the
// same test the action-button range coloring uses:
//   Script_IsActionInRange (0x004E7550) → FUN_004E56F0 → FUN_006E4440
//     → FUN_006E47B0
// It derives the spell's min/max range (via FUN_006E3480, which folds
// in the target's bounding radius), then compares center-to-center
// distance, so the boundary matches the client's own "in range"
// exactly — melee ranges and min-range (too-close) spells included.
//
// We call the pure core, not the higher-level FUN_006E4440: that
// wrapper also gates on target *type* (friend/foe/etc.) and collapses
// "range doesn't apply" into the same truthy return as "in range", so
// it can't yield a clean true/false/nil. The core answers range only,
// matching retail IsSpellInRange's range-only semantics (it ignores
// line of sight). One deliberate consequence: we don't reject
// wrong-faction targets — a friendly-only heal still reports a range
// answer against an enemy. Callers needing target validity should
// check it separately.

#include "Arg.h"

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"
#include "unit/Position.h"

#include <cstdint>

namespace Spell::InRange {

namespace {

// Spell.dbc RangeIndex + SpellRange.dbc min/max offsets — mirrors
// `Spell::HasRange` (kept local to avoid a cross-TU dependency; small
// and stable, same layout documented in CLAUDE.md).
constexpr int OFF_SPELL_RANGE_INDEX = 0x90;
constexpr int OFF_SPELLRANGE_MIN = 0x04;
constexpr int OFF_SPELLRANGE_MAX = 0x08;

// True iff the spell's SpellRange.dbc row has a non-zero min or max.
// Rangeless spells (self buffs, etc.) have no meaningful "in range"
// answer, so we return nil for them — matching retail.
bool SpellIDHasRange(int spellID) {
    if (spellID <= 0)
        return false;
    const uint8_t *rec = DBC::Record(Offsets::VAR_SPELL_RECORDS,
                                     Offsets::VAR_SPELL_RECORD_COUNT,
                                     static_cast<uint32_t>(spellID));
    if (rec == nullptr)
        return false;
    const uint32_t rangeIdx =
        *reinterpret_cast<const uint32_t *>(rec + OFF_SPELL_RANGE_INDEX);
    if (rangeIdx == 0)
        return false;
    const uint8_t *rangeRec = DBC::Record(Offsets::VAR_SPELL_RANGE_RECORDS,
                                          Offsets::VAR_SPELL_RANGE_COUNT, rangeIdx);
    if (rangeRec == nullptr)
        return false;
    const float minRange =
        *reinterpret_cast<const float *>(rangeRec + OFF_SPELLRANGE_MIN);
    const float maxRange =
        *reinterpret_cast<const float *>(rangeRec + OFF_SPELLRANGE_MAX);
    return minRange > 0.0f || maxRange > 0.0f;
}

using SpellRangeCheck_t = char(__fastcall *)(void *caster, void *target,
                                             int spellID, char *outFlag);

int __fastcall Script_C_Spell_IsSpellInRange(void *L) {
    const int spellID = Spell::Arg::ResolveSpellID(L, 1);
    const char *unit =
        Game::Lua::IsString(L, 2) ? Game::Lua::ToString(L, 2) : nullptr;

    // nil when the range check can't apply: unknown/rangeless spell, or
    // no unit token given.
    if (spellID <= 0 || unit == nullptr || !SpellIDHasRange(spellID)) {
        Game::Lua::PushNil(L);
        return 1;
    }

    void *caster = Unit::Position::ResolveToken("player");
    void *target = Unit::Position::ResolveToken(unit);

    // Guard positions before handing the objects to the engine core:
    // FUN_006E47B0 dereferences each GetPosition result without a null
    // check, and a resolved-but-position-less object (or an absent token
    // → null) would otherwise crash. A failed read → nil.
    float casterPos[3];
    float targetPos[3];
    if (!Unit::Position::Read(caster, casterPos) ||
        !Unit::Position::Read(target, targetPos)) {
        Game::Lua::PushNil(L);
        return 1;
    }

    char minMaxFlag = 0;
    auto check =
        reinterpret_cast<SpellRangeCheck_t>(Offsets::FUN_SPELL_RANGE_CHECK);
    const char inRange = check(caster, target, spellID, &minMaxFlag);
    Game::Lua::PushBool(L, inRange != 0);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "IsSpellInRange",
                                     &Script_C_Spell_IsSpellInRange);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Spell::InRange
