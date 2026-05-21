// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

// `C_Spell.GetSpellCooldown(spellIdentifier)` — modern table-shape
// cooldown query that accepts spellID, name, name(rank), or
// `|Hspell:N|h` hyperlink, and returns a `SpellCooldownInfo` table
// (or nil for unrecognized spells). Vanilla 1.12 only ships
// `GetSpellCooldown(slot, bookType)`, which forces callers to walk
// the spellbook first and rejects spellIDs not in it (talent
// passives, profession recipes, etc.).
//
// Same `FUN_SPELL_QUERY_COOLDOWN` helper `Spell::Usable` uses for
// the cooldown gate, just exposed through the modern table shape.

#include "Lookup.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace Spell::Cooldown {

namespace {

using QueryCooldown_t = void(__fastcall *)(int spellID, int bookType,
                                            int *outDuration,
                                            int *outStart,
                                            int *outEnable);

using NameToSpellID_t = int(__fastcall *)(const char *name, int *outIsPet);

// Pulls a spellID out of `|c...|Hspell:NNN[:rank]|h[Name]|h|r`
// hyperlink strings. Modern addons frequently route through
// `GetSpellLink(spellID)` and feed the result straight to
// `C_Spell.GetSpellCooldown`. Returns 0 if no `Hspell:` substring
// is found or the digits don't parse.
int SpellIDFromLink(const char *s) {
    const char *p = std::strstr(s, "Hspell:");
    if (p == nullptr)
        return 0;
    p += 7; // strlen("Hspell:")
    char *end = nullptr;
    const long id = std::strtol(p, &end, 10);
    if (end == p || id <= 0)
        return 0;
    return static_cast<int>(id);
}

// Resolves the spellIdentifier Lua arg into a Spell.dbc-valid
// spellID, or 0 on failure. Number → direct. String → try hyperlink
// parse, then fall back to the engine's name→spellID resolver
// (which respects `(Rank N)` suffixes via `FUN_004B3950`'s parser
// and bounds the search to the player's known spellbook).
int ResolveSpellID(void *L) {
    if (Game::Lua::IsNumber(L, 1)) {
        const int id = static_cast<int>(Game::Lua::ToNumber(L, 1));
        return id > 0 ? id : 0;
    }
    if (!Game::Lua::IsString(L, 1))
        return 0;
    const char *s = Game::Lua::ToString(L, 1);
    if (s == nullptr || *s == '\0')
        return 0;
    if (const int linkID = SpellIDFromLink(s); linkID > 0)
        return linkID;
    auto fn = reinterpret_cast<NameToSpellID_t>(
        static_cast<uintptr_t>(Offsets::FUN_RESOLVE_SPELL_NAME_TO_BOOK_ID));
    int isPet = 0;
    return fn(s, &isPet);
}

int __fastcall Script_C_Spell_GetSpellCooldown(void *L) {
    const int spellID = ResolveSpellID(L);
    if (spellID <= 0 || Spell::Lookup::RecordForID(spellID) == nullptr)
        return 0;

    auto fn = reinterpret_cast<QueryCooldown_t>(
        static_cast<uintptr_t>(Offsets::FUN_SPELL_QUERY_COOLDOWN));
    int durationMs = 0, startMs = 0, enable = 0;
    fn(spellID, 0 /* bookType=player */, &durationMs, &startMs, &enable);

    Game::Lua::NewTable(L);
    // Engine returns ms with the same epoch as `GetTime()` — multiply
    // by 0.001 to match modern's seconds-from-`GetTime` shape, same
    // conversion `Script_GetSpellCooldown` does at the Lua boundary.
    Game::Lua::SetFieldNumber(L, "startTime",
                              static_cast<double>(startMs) * 0.001);
    Game::Lua::SetFieldNumber(L, "duration",
                              static_cast<double>(durationMs) * 0.001);
    Game::Lua::SetFieldBool(L, "isEnabled", enable != 0);
    // Vanilla has no haste-on-cooldown mechanic. Hard-code 1.0 so
    // modern code that divides remaining-time by `modRate` works.
    Game::Lua::SetFieldNumber(L, "modRate", 1.0);
    // `activeCategory` / `timeUntilEndOfStartRecovery` / `isOnGCD`
    // are 11.1.5+ / 12.0+ fields with no vanilla source — leave
    // unset so they read as nil, matching modern's "field present
    // but inapplicable" semantics.
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "GetSpellCooldown",
                                     &Script_C_Spell_GetSpellCooldown);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::Cooldown
