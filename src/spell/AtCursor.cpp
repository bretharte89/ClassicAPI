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

// `C_Spell.CastAtCursor(spellID)` — cast a ground-target spell at
// the player's current cursor world position, bypassing the manual
// click on the AoE reticle.
//
// Implementation chains the engine's existing primitives:
//
//   1. Initiate the cast the same way `Script_CastSpellByName` does:
//      resolve `spellID` (via our `Spell::CastByID` numeric-name hook)
//      to a spellbook slot through `FUN_RESOLVE_SPELL_NAME_TO_SLOT`,
//      then dispatch through `FUN_SPELL_CAST_DISPATCH`. For a ground-
//      target spell, the engine enters placement mode at this point
//      (sets `VAR_SPELL_PLACEMENT_STATE`); for a normal cast, it
//      fires immediately.
//
//   2. `Spell::AtCursor::Resolve()` then refreshes the cursor's world
//      raycast via `FUN_REFRESH_CURSOR_RAYCAST` and commits the
//      placement at the resulting world coords via
//      `FUN_COMMIT_PLACEMENT_COORDS`. Self-cancelling when the spell
//      isn't ground-target (unit-target placement → cancel) or when
//      the cursor isn't on terrain (over UI / off-screen → cancel).
//
// Returns `true` from `Resolve()` when we successfully placed at
// cursor; `false` otherwise (no placement was needed, or cursor was
// invalid). The cast itself proceeds either way — `Resolve()`'s
// return tells the caller whether the cursor-placement leg landed.

#include "AtCursor.h"

#include "Game.h"
#include "Offsets.h"
#include "spell/MacroPrimarySpell.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Spell::AtCursor {

namespace {

using RefreshCursorRaycast_t = void(__fastcall *)(void *clickInfo);
using CommitPlacementCoords_t = unsigned(__fastcall *)(const float *xyz);
using StopPlacement_t = void(__fastcall *)();

bool InWorld() {
    // Player resolves only when in-world. Same gate other modules
    // (e.g. `Item::Location::ResolveBag`) use to avoid touching
    // engine state during loading screens or glue.
    using ResolveUnitToken_t = void *(__fastcall *)(const char *);
    auto fn = reinterpret_cast<ResolveUnitToken_t>(
        Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return fn("player") != nullptr;
}

void CancelPlacement() {
    auto fn = reinterpret_cast<StopPlacement_t>(Offsets::FUN_STOP_PLACEMENT);
    fn();
}

void Commit(const float coords[3]) {
    auto commit = reinterpret_cast<CommitPlacementCoords_t>(
        Offsets::FUN_COMMIT_PLACEMENT_COORDS);
    commit(coords);
}

// Shared placement-readiness gate for both the cursor and explicit-
// coords commit paths. Decision tree (preserved from the original
// Resolve): no placement → false (no cancel); non-ground placement →
// cancel + false; not in world → false; otherwise ready → true.
bool PlacementReady() {
    const uint32_t state = *reinterpret_cast<const uint32_t *>(
        Offsets::VAR_SPELL_PLACEMENT_STATE);
    if (state == 0)
        return false; // no placement active — nothing to commit

    if ((state & Offsets::SPELL_PLACEMENT_GROUND_MASK) == 0) {
        // Spell wants a unit / item / corpse / etc. — not ground.
        // Cancel so the cursor recovers; caller learns via the false
        // return that no placement happened.
        CancelPlacement();
        return false;
    }

    return InWorld();
}

} // namespace

bool CommitAtCoords(const float coords[3]) {
    if (!PlacementReady())
        return false;
    Commit(coords);
    return true;
}

bool Resolve() {
    if (!PlacementReady())
        return false;

    void *clickInfo = *reinterpret_cast<void *const *>(
        Offsets::VAR_WORLDFRAME_CLICK_INFO);
    if (clickInfo == nullptr)
        return false;

    // Re-raycast at the current cursor screen position. Writes the
    // new click type to `[clickInfo + OFF_CLICK_INFO_TYPE]` and the
    // hit world coords to `[clickInfo + OFF_CLICK_INFO_COORDS]`.
    auto refresh = reinterpret_cast<RefreshCursorRaycast_t>(
        Offsets::FUN_REFRESH_CURSOR_RAYCAST);
    refresh(clickInfo);

    const uint32_t hitType = *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(clickInfo) + Offsets::OFF_CLICK_INFO_TYPE);
    if (hitType != 1) {
        // Cursor wasn't on terrain (over UI, off-screen, on a unit
        // we didn't ask for). Modern `[@cursor]` would error; we
        // chose silent cancel-and-return-false.
        CancelPlacement();
        return false;
    }

    const float *coords = reinterpret_cast<const float *>(
        static_cast<const uint8_t *>(clickInfo) + Offsets::OFF_CLICK_INFO_COORDS);

    Commit(coords);
    return true;
}

namespace {

using NameToSlot_t = int(__fastcall *)(const char *name, void *out);
using CastDispatch_t = void(__fastcall *)(unsigned slot, int bookType,
                                           unsigned targetGuidLo, float targetGuidHi);

} // namespace

bool DispatchSpellCast(int spellID) {
    if (spellID <= 0)
        return false;

    // Hand the spellID off as a numeric string — our `Spell::CastByID`
    // hook on `FUN_RESOLVE_SPELL_NAME_TO_SLOT` translates pure-numeric
    // input into the locale-resolved spell name internally, so the
    // engine treats it just like `CastSpellByName("Blizzard")`.
    char numBuf[16];
    std::snprintf(numBuf, sizeof(numBuf), "%d", spellID);

    auto nameToSlot = reinterpret_cast<NameToSlot_t>(
        Offsets::FUN_RESOLVE_SPELL_NAME_TO_SLOT);
    int bookType = 0;
    const int slot = nameToSlot(numBuf, &bookType);
    if (slot < 0)
        return false; // not in the player's spellbook

    auto dispatch = reinterpret_cast<CastDispatch_t>(
        Offsets::FUN_SPELL_CAST_DISPATCH);
    dispatch(static_cast<unsigned>(slot), bookType,
             0, 0.0f); // no implicit target unit; engine handles placement
    return true;
}

namespace {

int __fastcall Script_C_Spell_CastAtCursor(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 1));

    // Spell not in the player's spellbook → match `CastSpellByName`'s
    // silent failure.
    if (!DispatchSpellCast(spellID)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }

    const bool placed = Resolve();
    Game::Lua::PushBool(L, placed);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "CastAtCursor",
                                     &Script_C_Spell_CastAtCursor);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

// Macro-parser pattern: tag macros that call
// `C_Spell.CastAtCursor(<spellID>)` with the spellID, so action-bar
// UIs highlight the slot the same way they do for `/cast SpellName`.
// The Lua call only accepts a numeric spellID, so the extractor just
// reads a numeric literal — no name resolution needed.
int ExtractCastAtCursorArg(const char *p, const char *end) {
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
    "C_Spell.CastAtCursor(", &ExtractCastAtCursorArg};

} // namespace

} // namespace Spell::AtCursor
