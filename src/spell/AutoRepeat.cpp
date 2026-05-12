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
#include "spell/Lookup.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Spell::AutoRepeat {

// Offset of the locale-resolved Name[9] table inside a Spell.dbc record
// (same value used by `Spell::Info`; duplicated here to keep this module
// header-free).
static constexpr int OFF_NAME = 0x1E0;

using CastSpellByName_t = int(__fastcall *)(void *L);
static const CastSpellByName_t Script_CastSpellByName_Engine =
    reinterpret_cast<CastSpellByName_t>(Offsets::FUN_SCRIPT_CAST_SPELL_BY_NAME);

static int ReadActiveSpellID() {
    return *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_ACTIVE_AUTO_REPEAT_SPELL));
}

static const char *LocaleName(int spellID) {
    const uint8_t *record = Spell::Lookup::RecordForID(spellID);
    if (record == nullptr)
        return nullptr;
    const int locale = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
    return *reinterpret_cast<const char *const *>(record + OFF_NAME + locale * 4);
}

// Case-insensitive name match for the safety gate. Walks `userInput` and
// `spellName` together; on the user side, treats `(` and trailing spaces
// before `(` as end-of-name so `"Shoot(Rank 1)"` matches the bare
// `"Shoot"` we read out of Spell.dbc — mirrors how the engine's own name
// resolver (`FUN_004B3950`) strips the parenthetical rank suffix before
// looking the spell up.
static bool NameMatches(const char *userInput, const char *spellName) {
    auto lc = [](unsigned char c) -> unsigned char {
        return (c >= 'A' && c <= 'Z') ? static_cast<unsigned char>(c + 32) : c;
    };
    if (userInput == nullptr || spellName == nullptr)
        return false;
    while (*userInput != '\0' && *userInput != '(' && *spellName != '\0') {
        if (lc(static_cast<unsigned char>(*userInput)) !=
            lc(static_cast<unsigned char>(*spellName))) {
            return false;
        }
        ++userInput;
        ++spellName;
    }
    while (*userInput == ' ')
        ++userInput;
    return (*userInput == '\0' || *userInput == '(') && *spellName == '\0';
}

// `CastAutoRepeatSpell(name | spellID)` — safe-to-spam variant of
// `CastSpellByName` for auto-repeat spells (Shoot/Wand/Auto-Shot).
//
// Reads the engine's "currently auto-repeating spellID" global at
// `[VAR_ACTIVE_AUTO_REPEAT_SPELL]` and branches:
//   - global == 0 → safe to cast; delegate to `Script_CastSpellByName`
//     to start the auto-repeat. Returns true iff the global went
//     non-zero (so non-auto-repeat spells like Fireball, which would
//     "succeed" via CastSpellByName but don't enter auto-repeat,
//     correctly report false).
//   - global == requested → already auto-repeating the right spell;
//     no-op, return true. This is the spam-safety: a held bind doesn't
//     toggle the auto-repeat back off.
//   - global == something else → a different auto-repeat is active;
//     no-op, return false. Don't disrupt the unrelated spell.
//
// Number arg compares by spellID directly. String arg compares by name
// against the active spell's locale-resolved Spell.dbc name (case-
// insensitive, rank-suffix tolerant via `NameMatches`).
static int __fastcall Script_CastAutoRepeatSpell(void *L) {
    char nameBuf[128];
    int requestedID = 0;

    const int argType = Game::Lua::Type(L, 1);
    if (argType == Game::Lua::TYPE_NUMBER) {
        requestedID = static_cast<int>(Game::Lua::ToNumber(L, 1));
        if (requestedID <= 0) {
            Game::Lua::PushBoolean(L, 0);
            return 1;
        }
        const char *name = LocaleName(requestedID);
        if (name == nullptr || *name == '\0') {
            Game::Lua::PushBoolean(L, 0);
            return 1;
        }
        std::snprintf(nameBuf, sizeof(nameBuf), "%s", name);
    } else if (argType == Game::Lua::TYPE_STRING) {
        const char *s = Game::Lua::ToString(L, 1);
        if (s == nullptr || *s == '\0') {
            Game::Lua::PushBoolean(L, 0);
            return 1;
        }
        // Copy off Lua's string heap — the SetTop+PushString below could
        // shift it and invalidate `s`.
        std::snprintf(nameBuf, sizeof(nameBuf), "%s", s);
    } else {
        Game::Lua::Error(L, "Usage: CastAutoRepeatSpell(\"name\" | spellID)");
        return 0;
    }

    const int activeID = ReadActiveSpellID();
    if (activeID != 0) {
        bool match;
        if (requestedID > 0) {
            match = (activeID == requestedID);
        } else {
            match = NameMatches(nameBuf, LocaleName(activeID));
        }
        Game::Lua::PushBoolean(L, match ? 1 : 0);
        return 1;
    }

    // Safe to start auto-repeat — delegate to the engine's cast path
    // with a fresh, one-arg stack so `lua_toboolean(L, 2)` (the
    // `onSelf` flag inside Script_CastSpellByName) sees nil → false.
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushString(L, nameBuf);
    Script_CastSpellByName_Engine(L);

    Game::Lua::PushBoolean(L, ReadActiveSpellID() != 0 ? 1 : 0);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("CastAutoRepeatSpell",
                                      &Script_CastAutoRepeatSpell);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

// ---------------------------------------------------------------------------
// Macro parser hook
//
// Without this hook, a macro whose only cast statement is
// `CastAutoRepeatSpell("Shoot")` doesn't get tagged in the spell-state
// cache with Shoot's spellID, because the engine's parser
// (`FUN_004EFE00`) only knows to look for `/cast` slash-aliases and
// `CastSpellByName("...")`. `IsAutoRepeatAction(macroSlot)` then has
// nothing to match against the active auto-repeat spell, so the slot
// fails to highlight in action-bar UIs (pfUI etc.).
//
// We hook `FUN_004EFE00`, let the original parser run, and if it
// didn't recognize any pattern (cache field stays 0), scan the body
// ourselves for `CastAutoRepeatSpell("<name>")` and use the engine's
// own name→spellID resolver to fill in the field.
// ---------------------------------------------------------------------------

using MacroParse_t = void(__fastcall *)(int macroEntry);
static MacroParse_t MacroParse_o = nullptr;

using NameToSpellID_t = int(__fastcall *)(const char *name, int *outIsPet);

static int ResolveNameViaEngine(const char *name) {
    auto fn = reinterpret_cast<NameToSpellID_t>(
        static_cast<uintptr_t>(Offsets::FUN_RESOLVE_SPELL_NAME_TO_BOOK_ID));
    int isPet = 0;
    return fn(name, &isPet);
}

// Scan a macro body for `CastAutoRepeatSpell("<name>")` and return the
// resolved spellID, or 0 if no recognized invocation is present or the
// name doesn't resolve to a known spellbook entry. Mirrors the engine
// parser's "first hit wins, line-at-a-time" walk order so behavior is
// consistent with how `/cast` and `CastSpellByName` are matched.
static int ScanMacroBodyForOurPattern(const char *body) {
    static constexpr char kPattern[] = "CastAutoRepeatSpell(";
    static constexpr size_t kPatternLen = sizeof(kPattern) - 1;

    const char *line = body;
    while (*line != '\0') {
        const char *eol = line;
        while (*eol != '\0' && *eol != '\n')
            ++eol;

        const char *p = line;
        const char *end = eol;
        while (p + kPatternLen <= end) {
            if (std::memcmp(p, kPattern, kPatternLen) == 0) {
                p += kPatternLen;
                while (p < end && (*p == ' ' || *p == '\t'))
                    ++p;
                if (p < end && *p == '"') {
                    ++p;
                    const char *q = p;
                    while (q < end && *q != '"')
                        ++q;
                    if (q < end) {
                        const size_t n = static_cast<size_t>(q - p);
                        if (n > 0 && n < 128) {
                            char name[128];
                            std::memcpy(name, p, n);
                            name[n] = '\0';
                            const int spellID = ResolveNameViaEngine(name);
                            if (spellID > 0)
                                return spellID;
                        }
                    }
                }
                break; // matched our prefix on this line; don't double-match
            }
            ++p;
        }

        line = (*eol == '\n') ? eol + 1 : eol;
    }
    return 0;
}

static void __fastcall MacroParse_h(int macroEntry) {
    MacroParse_o(macroEntry);

    int *cacheField = reinterpret_cast<int *>(
        macroEntry + Offsets::OFF_MACRO_PRIMARY_SPELL);
    // Only fill in if the engine didn't recognize anything. A zero
    // here means the parser walked every line and found no `/cast`
    // or `CastSpellByName(`; `0xFFFFFFFF` means it did find a
    // recognized line but the name was unresolvable — leave that
    // alone so the user's `/cast UnknownSpell` line still "claims"
    // the macro the way the engine intended.
    if (*cacheField != 0)
        return;

    const char *body = reinterpret_cast<const char *>(
        macroEntry + Offsets::OFF_MACRO_BODY);
    const int spellID = ScanMacroBodyForOurPattern(body);
    if (spellID > 0)
        *cacheField = spellID;
}

static const Game::HookAutoRegister _hookreg{
    Offsets::FUN_MACRO_PARSE_PRIMARY_SPELL,
    reinterpret_cast<void *>(&MacroParse_h),
    reinterpret_cast<void **>(&MacroParse_o)};

} // namespace Spell::AutoRepeat
