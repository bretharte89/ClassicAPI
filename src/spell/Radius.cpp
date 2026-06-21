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

// `C_Spell.GetSpellRadius(spellID)` and the parallel
// `GetSpellRadius(slot, bookType)` — the spell's AOE radius in yards, or
// nil for a non-AOE spell (no effect carries a radius).
//
// Two signatures mirroring the other dual-signature spell globals
// (`GetSpellInfo`, `SpellHasRange`, etc.): the namespaced form takes a
// modern spell *identifier* (numeric ID or name), the bare global takes
// the vanilla `(slot, bookType)` spellbook position. `bookType` is the
// `GetSpellName` convention: `"pet"` → pet book, anything else (or
// omitted) → player book.
//
// Reads `Spell.dbc`'s per-effect `EffectRadiusIndex[3]` and resolves each
// into `SpellRadius.dbc`; since a spell's effects can each have their own
// radius, we return the largest base radius found across the three
// effects.
//
// Base radius only — no caster-level scaling. The engine's own radius
// helper (FUN_006e6350) adds `radiusPerLevel * casterLevel`, but that
// needs a unit context and modern `GetSpellRadius` reports the base
// value; callers wanting the scaled radius can add it themselves.
//
// Both DBCs are resident from boot, so the lookup is synchronous with no
// caching.

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"
#include "spell/Arg.h"
#include "spell/Lookup.h"

#include <cstdint>
#include <cstring>

namespace Spell::Radius {

// Largest base radius across the spell's effects, or a negative sentinel
// (< 0) if the spell is invalid or no effect has a radius.
static float MaxRadius(int spellID) {
    const uint8_t *record = Spell::Lookup::RecordForID(spellID);
    if (record == nullptr)
        return -1.0f;

    float best = -1.0f;
    for (int i = 0; i < Offsets::SPELL_RECORD_EFFECT_COUNT; ++i) {
        const int radiusIndex = *reinterpret_cast<const int *>(
            record + Offsets::OFF_SPELL_RECORD_EFFECT_RADIUS_INDEX + i * 4);
        const uint8_t *rad = DBC::Record(Offsets::VAR_SPELL_RADIUS_RECORDS,
                                         Offsets::VAR_SPELL_RADIUS_COUNT,
                                         static_cast<uint32_t>(radiusIndex));
        if (rad == nullptr)
            continue;
        const float r = *reinterpret_cast<const float *>(rad + Offsets::OFF_SPELL_RADIUS_VALUE);
        if (r > best)
            best = r;
    }
    return best;
}

// Pushes the radius for `spellID`, or nil for an invalid / non-AOE spell.
static int PushRadiusForSpellID(void *L, int spellID) {
    const float radius = MaxRadius(spellID);
    if (radius < 0.0f)
        return 0; // nil
    Game::Lua::PushNumber(L, static_cast<double>(radius));
    return 1;
}

// `C_Spell.GetSpellRadius(spellID)` — modern spell-identifier form.
static int __fastcall Script_C_Spell_GetSpellRadius(void *L) {
    return PushRadiusForSpellID(L, Spell::Arg::ResolveSpellID(L, 1));
}

// `GetSpellRadius(slot, bookType)` — vanilla-style positional shape.
static int __fastcall Script_GetSpellRadius(void *L) {
    if (!Game::Lua::IsNumber(L, 1))
        return 0; // nil
    const int slot = static_cast<int>(Game::Lua::ToNumber(L, 1));
    int bookType = 0;
    if (Game::Lua::IsString(L, 2)) {
        const char *s = Game::Lua::ToString(L, 2);
        if (s != nullptr && _stricmp(s, "pet") == 0)
            bookType = 1;
    }
    return PushRadiusForSpellID(L, Spell::Lookup::SpellbookSlotToID(slot, bookType));
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "GetSpellRadius",
                                     &Script_C_Spell_GetSpellRadius);
    Game::Lua::RegisterGlobalFunction("GetSpellRadius", &Script_GetSpellRadius);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::Radius
