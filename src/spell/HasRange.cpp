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

// `C_Spell.SpellHasRange(spellIdentifier)` and the parallel
// `SpellHasRange(slot, bookType)` — true iff the spell's
// `SpellRange.dbc` row has a non-zero min or max range.
//
// Vanilla 1.12 doesn't ship `SpellHasRange` as a global at all; 3.3.5+
// added it but with the spellID-only signature. We provide the modern
// namespaced shape *and* a `(slot, bookType)` positional form mirroring
// the other dual-signature globals (`GetSpellInfo`, `GetSpellLink`,
// etc.) so callers can use whichever they already have.

#include "Arg.h"
#include "Lookup.h"

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"

#include <cstdint>
#include <cstring>

namespace Spell::HasRange {

namespace {

// Spell.dbc RangeIndex field — mirrors `Script_GetSpellInfo`'s
// local constant. Future refactor: promote to `Offsets.h`.
constexpr int OFF_SPELL_RANGE_INDEX = 0x90;

// SpellRange.dbc record layout (verified, documented in CLAUDE.md
// under "Sub-DBC record layouts"):
//   +0x00 id, +0x04 minRange (float), +0x08 maxRange (float), ...
constexpr int OFF_SPELLRANGE_MIN = 0x04;
constexpr int OFF_SPELLRANGE_MAX = 0x08;

bool SpellIDHasRange(int spellID) {
    if (spellID <= 0)
        return false;
    const uint8_t *spellRec = DBC::Record(Offsets::VAR_SPELL_RECORDS,
                                          Offsets::VAR_SPELL_RECORD_COUNT,
                                          static_cast<uint32_t>(spellID));
    if (spellRec == nullptr)
        return false;
    const uint32_t rangeIdx = *reinterpret_cast<const uint32_t *>(
        spellRec + OFF_SPELL_RANGE_INDEX);
    if (rangeIdx == 0)
        return false;
    const uint8_t *rangeRec = DBC::Record(Offsets::VAR_SPELL_RANGE_RECORDS,
                                          Offsets::VAR_SPELL_RANGE_COUNT,
                                          rangeIdx);
    if (rangeRec == nullptr)
        return false;
    const float minRange = *reinterpret_cast<const float *>(
        rangeRec + OFF_SPELLRANGE_MIN);
    const float maxRange = *reinterpret_cast<const float *>(
        rangeRec + OFF_SPELLRANGE_MAX);
    return minRange > 0.0f || maxRange > 0.0f;
}

} // namespace

static int __fastcall Script_C_Spell_SpellHasRange(void *L) {
    const int spellID = Spell::Arg::ResolveSpellID(L, 1);
    Game::Lua::PushBool(L, SpellIDHasRange(spellID));
    return 1;
}

// `SpellHasRange(slot, bookType)` — vanilla-style positional shape.
// `bookType` is the same string convention used by `GetSpellName`:
// `"spell"` (or omitted / anything-not-pet) → player book,
// `"pet"` → pet book.
static int __fastcall Script_SpellHasRange(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    const int slot = static_cast<int>(Game::Lua::ToNumber(L, 1));
    int bookType = 0;
    if (Game::Lua::IsString(L, 2)) {
        const char *s = Game::Lua::ToString(L, 2);
        if (s != nullptr && _stricmp(s, "pet") == 0)
            bookType = 1;
    }
    const int spellID = Spell::Lookup::SpellbookSlotToID(slot, bookType);
    Game::Lua::PushBool(L, SpellIDHasRange(spellID));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "SpellHasRange",
                                     &Script_C_Spell_SpellHasRange);
    Game::Lua::RegisterGlobalFunction("SpellHasRange",
                                      &Script_SpellHasRange);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::HasRange
