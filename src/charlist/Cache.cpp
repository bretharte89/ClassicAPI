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

// `C_CharacterList.*` — exposes the realm's character list (name,
// class, race, level, etc.) from anywhere in the game, not just the
// character-select glue screen.
//
// Vanilla 1.12's `GetCharacterInfo` / `GetNumCharacters` are
// **glue-only** — registered on the glue FrameScript engine, the
// underlying data structure at `VAR_CHARLIST_ARRAY` is freed by
// `FUN_CHARLIST_FREE` (`0x00472090`) during the world-transition
// path. By the time a player is in-world, the data is gone.
//
// We hook `FUN_CHARLIST_FREE` to snapshot the array contents into
// our own cache before the engine frees them. Subsequent
// world-enters re-fetch SMSG_CHAR_ENUM and re-call the teardown, so
// our cache gets refreshed each time. Deleting a character refreshes
// the cache on the next char-select visit (server sends a new
// SMSG_CHAR_ENUM without the deleted character).
//
// The cache is **memory-only** — every game start re-fetches char-
// select before world-enter, so the cache is naturally rebuilt on
// every launch. No filesystem persistence, no staleness across game
// restarts, no need to deal with deleted-character entries.
//
// Hook target: `FUN_CHARLIST_FREE` has exactly one caller
// (FUN_0046AC90, the glue-shutdown wrapper). Not a target other DLLs
// touch.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace Charlist::Cache {

namespace {

struct Snapshot {
    std::string name;
    uint8_t raceID;
    uint8_t classID;
    uint8_t level;
    uint8_t sex;
    uint32_t areaID;
    uint64_t guid;
};

// Replaced wholesale on every teardown — we always reflect the most
// recently-observed char-select state. Read by the Lua bindings.
std::vector<Snapshot> g_chars;

int CurrentLocale() {
    return *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
}

// Looks up a localized name from a DBC's records-pointer + count
// globals, with a per-record offset to the locale-array. Returns
// nullptr on out-of-range index or missing record.
const char *LookupDBCName(uintptr_t recordsVar, uintptr_t countVar,
                          int id, int nameOffset) {
    if (id < 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(countVar);
    if (id > count)
        return nullptr;
    const uint8_t *const *records =
        *reinterpret_cast<const uint8_t *const *const *>(recordsVar);
    if (records == nullptr)
        return nullptr;
    const uint8_t *record = records[id];
    if (record == nullptr)
        return nullptr;
    const char *name = *reinterpret_cast<const char *const *>(
        record + nameOffset + CurrentLocale() * 4);
    return (name != nullptr && name[0] != '\0') ? name : nullptr;
}

// Reads the englishRace / class-file-token field from a DBC
// record (single-string, not locale-array).
const char *LookupDBCField(uintptr_t recordsVar, uintptr_t countVar,
                           int id, int fieldOffset) {
    if (id < 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(countVar);
    if (id > count)
        return nullptr;
    const uint8_t *const *records =
        *reinterpret_cast<const uint8_t *const *const *>(recordsVar);
    if (records == nullptr)
        return nullptr;
    const uint8_t *record = records[id];
    if (record == nullptr)
        return nullptr;
    return *reinterpret_cast<const char *const *>(record + fieldOffset);
}

void TakeSnapshot() {
    g_chars.clear();
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_CHARLIST_COUNT));
    if (count <= 0)
        return;
    auto *base = *reinterpret_cast<const uint8_t *const *>(
        static_cast<uintptr_t>(Offsets::VAR_CHARLIST_ARRAY));
    if (base == nullptr)
        return;
    g_chars.reserve(count);
    for (int i = 0; i < count; ++i) {
        const uint8_t *r = base + i * Offsets::CHARLIST_RECORD_STRIDE;
        Snapshot s;
        s.name = reinterpret_cast<const char *>(r + Offsets::OFF_CHARRECORD_NAME);
        s.raceID = *(r + Offsets::OFF_CHARRECORD_RACE);
        s.classID = *(r + Offsets::OFF_CHARRECORD_CLASS);
        s.sex = *(r + Offsets::OFF_CHARRECORD_SEX);
        s.level = *(r + Offsets::OFF_CHARRECORD_LEVEL);
        s.areaID = *reinterpret_cast<const uint32_t *>(
            r + Offsets::OFF_CHARRECORD_AREA_ID);
        s.guid = *reinterpret_cast<const uint64_t *>(
            r + Offsets::OFF_CHARRECORD_GUID);
        g_chars.push_back(std::move(s));
    }
}

// `C_CharacterList.GetNumCharacters()` — count of cached characters.
// Returns 0 if char-select never ran since DLL load (unlikely; every
// game launch hits char-select before world-enter).
int __fastcall Script_GetNumCharacters(void *L) {
    Game::Lua::PushNumber(L, static_cast<double>(g_chars.size()));
    return 1;
}

// `C_CharacterList.GetCharacterInfo(index)` — same 8-return tuple
// as the glue-only `GetCharacterInfo`, available from in-world:
//
//   name, localizedRace, localizedClass, level, areaName,
//   englishRace, sex, isGhost
//
// `englishRace` is the ChrRaces.dbc filename field (untranslated).
// `isGhost` is nil for live characters, 1 for ghost.
int __fastcall Script_GetCharacterInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: C_CharacterList.GetCharacterInfo(index)");
        return 0;
    }
    const int idx = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
    if (idx < 0 || idx >= static_cast<int>(g_chars.size()))
        return 0;
    const Snapshot &s = g_chars[idx];

    Game::Lua::PushString(L, s.name.c_str());

    const char *localRace = LookupDBCName(
        Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
        s.raceID, Offsets::OFF_CHRRACES_NAMES);
    if (localRace != nullptr) Game::Lua::PushString(L, localRace);
    else Game::Lua::PushString(L, "");

    const char *localClass = LookupDBCName(
        Offsets::VAR_CHRCLASSES_RECORDS, Offsets::VAR_CHRCLASSES_COUNT,
        s.classID, Offsets::OFF_CHRCLASSES_NAMES);
    if (localClass != nullptr) Game::Lua::PushString(L, localClass);
    else Game::Lua::PushString(L, "");

    Game::Lua::PushNumber(L, static_cast<double>(s.level));

    const char *area = LookupDBCName(
        Offsets::VAR_AREATABLE_RECORDS, Offsets::VAR_AREATABLE_COUNT,
        static_cast<int>(s.areaID), Offsets::OFF_AREATABLE_NAMES);
    if (area != nullptr) Game::Lua::PushString(L, area);
    else Game::Lua::PushNil(L);

    const char *englishRace = LookupDBCField(
        Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
        s.raceID, Offsets::OFF_CHRRACES_FILENAME);
    Game::Lua::PushString(L, englishRace != nullptr ? englishRace : "");

    Game::Lua::PushNumber(L, static_cast<double>(s.sex));

    // GUID is a 64-bit value; Lua doubles can't carry full precision
    // past 2^53. Format as `0x` + 16-hex-digit string (hi then lo,
    // matching `UnitGUID`'s convention) so addons can use it as a
    // stable string identifier.
    char buf[24];
    std::snprintf(buf, sizeof(buf), "0x%08X%08X",
                  static_cast<uint32_t>(s.guid >> 32),
                  static_cast<uint32_t>(s.guid));
    Game::Lua::PushString(L, buf);

    return 8;
}

// Pushes a key/string-value pair into the table at stack[-3] (the
// table we're populating in `Script_GetCharacters`).
void SetTableStringField(void *L, const char *key, const char *value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushString(L, value != nullptr ? value : "");
    Game::Lua::SetTable(L, -3);
}

void SetTableNumberField(void *L, const char *key, double value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushNumber(L, value);
    Game::Lua::SetTable(L, -3);
}

// `C_CharacterList.GetCharacters()` — returns a numeric-keyed table
// of CharacterInfo tables. Each entry is a struct-style table with
// the same fields `GetCharacterInfo` returns as a tuple, but named.
// Easier to consume than the multi-return form when iterating the
// whole list:
//
//   for _, c in ipairs(C_CharacterList.GetCharacters()) do
//       print(c.name, c.localizedClass, "Lv", c.level, c.guid)
//   end
int __fastcall Script_GetCharacters(void *L) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);

    for (size_t i = 0; i < g_chars.size(); ++i) {
        const Snapshot &s = g_chars[i];

        // Stack: [outer-table]
        Game::Lua::PushNumber(L, static_cast<double>(i + 1));
        Game::Lua::NewTable(L);
        // Stack: [outer-table, key, inner-table]

        SetTableStringField(L, "name", s.name.c_str());

        SetTableStringField(L, "localizedRace", LookupDBCName(
            Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
            s.raceID, Offsets::OFF_CHRRACES_NAMES));

        SetTableStringField(L, "englishRace", LookupDBCField(
            Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
            s.raceID, Offsets::OFF_CHRRACES_FILENAME));

        SetTableStringField(L, "localizedClass", LookupDBCName(
            Offsets::VAR_CHRCLASSES_RECORDS, Offsets::VAR_CHRCLASSES_COUNT,
            s.classID, Offsets::OFF_CHRCLASSES_NAMES));

        SetTableNumberField(L, "level", static_cast<double>(s.level));

        const char *area = LookupDBCName(
            Offsets::VAR_AREATABLE_RECORDS, Offsets::VAR_AREATABLE_COUNT,
            static_cast<int>(s.areaID), Offsets::OFF_AREATABLE_NAMES);
        if (area != nullptr)
            SetTableStringField(L, "areaName", area);

        SetTableNumberField(L, "sex", static_cast<double>(s.sex));

        char buf[24];
        std::snprintf(buf, sizeof(buf), "0x%08X%08X",
                      static_cast<uint32_t>(s.guid >> 32),
                      static_cast<uint32_t>(s.guid));
        SetTableStringField(L, "guid", buf);

        // Insert inner-table into outer-table at numeric key.
        Game::Lua::SetTable(L, -3);
    }

    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_CharacterList", "GetNumCharacters",
                                     &Script_GetNumCharacters);
    Game::Lua::RegisterTableFunction("C_CharacterList", "GetCharacterInfo",
                                     &Script_GetCharacterInfo);
    Game::Lua::RegisterTableFunction("C_CharacterList", "GetCharacters",
                                     &Script_GetCharacters);
}

} // namespace

// `FUN_CHARLIST_FREE` — `__cdecl void()`. We snapshot the records
// BEFORE running the original, so the cache is populated by the time
// the engine zeros the globals.
using CharlistFree_t = void(__cdecl *)();
CharlistFree_t CharlistFree_o = nullptr;
void __cdecl CharlistFree_h() {
    TakeSnapshot();
    CharlistFree_o();
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};
static const Game::HookAutoRegister _hookreg{
    Offsets::FUN_CHARLIST_FREE,
    reinterpret_cast<void *>(&CharlistFree_h),
    reinterpret_cast<void **>(&CharlistFree_o)};

} // namespace Charlist::Cache
