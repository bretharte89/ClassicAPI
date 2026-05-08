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

namespace Spell::Info {

// Spell.dbc record offsets (from BuildSpellTooltip / Script_GetSpellName / Script_GetSpellTexture).
static constexpr int OFF_ATTRIBUTES_EX = 0x1C;
static constexpr int OFF_CASTING_TIME_INDEX = 0x48;
static constexpr int OFF_POWER_TYPE = 0x7C;
static constexpr int OFF_MANA_COST = 0x80;
static constexpr int OFF_RANGE_INDEX = 0x90;
static constexpr int OFF_ICON_ID = 0x1D4;
static constexpr int OFF_NAME = 0x1E0;
static constexpr int OFF_RANK = 0x204;

// AttributesEx bit 6 — funnel flag per the public Spell.dbc schema.
static constexpr uint32_t SPELL_ATTR_EX_FUNNEL = 0x40;

template <typename T>
static T ReadGlobal(uintptr_t addr) {
    return *reinterpret_cast<T *>(addr);
}

// Sub-DBC lookup (SpellIcon, SpellCastTimes, SpellRange) — these are the
// "indirected fields" of a Spell.dbc record. Different base/count
// addresses than the main Spell.dbc, so they don't go through
// `Spell::Lookup::RecordForID`.
static const uint8_t *LookupSubRecord(uintptr_t baseAddr, uintptr_t countAddr, int id) {
    if (id <= 0)
        return nullptr;
    const int count = ReadGlobal<int>(countAddr);
    if (id > count)
        return nullptr;
    const uint8_t *const *records = ReadGlobal<const uint8_t *const *>(baseAddr);
    if (records == nullptr)
        return nullptr;
    return records[id];
}

struct SpellInfoData {
    int spellID;
    const char *name;       // localized; nullable if record's locale slot is unset
    const char *rank;       // localized; same nullability as name
    const char *iconPath;   // SpellIcon.dbc path; null if icon record missing
    int cost;               // base ManaCost (no per-level scaling)
    bool isFunnel;
    int powerType;          // 0=mana, 1=rage, 2=focus, 3=energy, 4=happiness
    int castTimeMs;         // base time from SpellCastTimes.dbc
    float minRange;
    float maxRange;
};

static bool ReadSpellInfo(int spellID, SpellInfoData &out) {
    const uint8_t *record = Spell::Lookup::RecordForID(spellID);
    if (record == nullptr)
        return false;

    const int locale = ReadGlobal<int>(Offsets::VAR_LOCALE_INDEX);

    out.spellID = spellID;
    out.name = *reinterpret_cast<const char *const *>(record + OFF_NAME + locale * 4);
    out.rank = *reinterpret_cast<const char *const *>(record + OFF_RANK + locale * 4);

    out.iconPath = nullptr;
    const int iconID = *reinterpret_cast<const int *>(record + OFF_ICON_ID);
    if (auto *iconRec = LookupSubRecord(Offsets::VAR_SPELL_ICON_RECORDS,
                                        Offsets::VAR_SPELL_ICON_COUNT, iconID)) {
        out.iconPath = *reinterpret_cast<const char *const *>(iconRec + 4);
    }

    out.cost = *reinterpret_cast<const int *>(record + OFF_MANA_COST);

    const uint32_t attrEx = *reinterpret_cast<const uint32_t *>(record + OFF_ATTRIBUTES_EX);
    out.isFunnel = (attrEx & SPELL_ATTR_EX_FUNNEL) != 0;

    out.powerType = *reinterpret_cast<const int *>(record + OFF_POWER_TYPE);

    out.castTimeMs = 0;
    const int castIndex = *reinterpret_cast<const int *>(record + OFF_CASTING_TIME_INDEX);
    if (auto *castRec = LookupSubRecord(Offsets::VAR_SPELL_CAST_TIMES_RECORDS,
                                        Offsets::VAR_SPELL_CAST_TIMES_COUNT, castIndex)) {
        out.castTimeMs = *reinterpret_cast<const int *>(castRec + 4);
    }

    out.minRange = 0.0f;
    out.maxRange = 0.0f;
    const int rangeIndex = *reinterpret_cast<const int *>(record + OFF_RANGE_INDEX);
    if (auto *rangeRec = LookupSubRecord(Offsets::VAR_SPELL_RANGE_RECORDS,
                                         Offsets::VAR_SPELL_RANGE_COUNT, rangeIndex)) {
        out.minRange = *reinterpret_cast<const float *>(rangeRec + 4);
        out.maxRange = *reinterpret_cast<const float *>(rangeRec + 8);
    }

    return true;
}

// Case-insensitive match against the literal string "pet". Anything else
// (including the explicit "spell") maps to the player spellbook —
// matches 1.12's `Script_GetSpellName` behavior, which `SStrCmpI`s the
// input against "pet" and treats every other value as the player book.
static bool BookTypeIsPet(const char *s) {
    if (s == nullptr)
        return false;
    auto lc = [](char c) -> char {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
    };
    return lc(s[0]) == 'p' && lc(s[1]) == 'e' && lc(s[2]) == 't' && s[3] == '\0';
}

// Resolves the Lua args to a spellID, supporting:
//   GetSpellInfo(spellID)              -- arg2 absent or non-string
//   GetSpellInfo(slot, "spell"|"pet")  -- arg2 string → spellbook lookup
// Returns 0 for invalid/empty inputs (Lua side surfaces 0 as nil since
// spellID 0 is never valid). Calls `lua_error` if arg1 isn't a number,
// matching the engine's own `lua_error`-on-misuse style.
static int ResolveLuaArgsToSpellID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetSpellInfo(spellID) or GetSpellInfo(slot, bookType)");
        return 0;
    }
    const int arg1 = static_cast<int>(Game::Lua::ToNumber(L, 1));

    if (Game::Lua::Type(L, 2) == Game::Lua::TYPE_STRING) {
        const char *book = Game::Lua::ToString(L, 2);
        const int bookType = BookTypeIsPet(book) ? 1 : 0;
        return Spell::Lookup::SpellbookSlotToID(arg1, bookType);
    }
    return arg1;
}

static int __fastcall Script_GetSpellInfo(void *L) {
    const int spellID = ResolveLuaArgsToSpellID(L);
    if (spellID <= 0)
        return 0;

    SpellInfoData info;
    if (!ReadSpellInfo(spellID, info))
        return 0;

    // PushString tail-calls PushNil for null pointers, so unset locale
    // slots (rare, but possible) come through as nil rather than crash.
    Game::Lua::PushString(L, info.name);                         // 1. name
    Game::Lua::PushString(L, info.rank);                         // 2. rank
    Game::Lua::PushString(L, info.iconPath);                     // 3. icon (path string)
    Game::Lua::PushNumber(L, static_cast<double>(info.cost));    // 4. cost
    Game::Lua::PushBoolean(L, info.isFunnel ? 1 : 0);            // 5. isFunnel
    Game::Lua::PushNumber(L, static_cast<double>(info.powerType)); // 6. powerType
    Game::Lua::PushNumber(L, static_cast<double>(info.castTimeMs)); // 7. castTime (ms)
    Game::Lua::PushNumber(L, static_cast<double>(info.minRange)); // 8. minRange
    Game::Lua::PushNumber(L, static_cast<double>(info.maxRange)); // 9. maxRange
    Game::Lua::PushNumber(L, static_cast<double>(info.spellID));  // 10. spellID (NEW)
    return 10;
}

// Helpers for building the C_Spell.GetSpellInfo result table. Each
// expects the table at stack[-3] before the call (key + value get
// pushed onto top, then `RawSet(L, -3)` pops them and sets the field).
static void SetField(void *L, const char *key, const char *value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushString(L, value); // PushString handles NULL -> nil
    Game::Lua::RawSet(L, -3);
}
static void SetField(void *L, const char *key, double value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushNumber(L, value);
    Game::Lua::RawSet(L, -3);
}
static void SetFieldBool(void *L, const char *key, bool value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushBoolean(L, value ? 1 : 0);
    Game::Lua::RawSet(L, -3);
}

static int __fastcall Script_C_GetSpellInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Spell.GetSpellInfo(spellID)");
        return 0;
    }
    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 1));

    SpellInfoData info;
    if (!ReadSpellInfo(spellID, info))
        return 0; // nil for unknown spellID

    Game::Lua::NewTable(L);
    SetField(L, "name", info.name);
    // `iconID` deviates from modern's fileID:number — vanilla has no
    // fileID system, so we surface the icon path string instead.
    // Practical: feed it directly to texture:SetTexture(...).
    SetField(L, "iconID", info.iconPath);
    SetField(L, "castTime", static_cast<double>(info.castTimeMs));
    SetField(L, "minRange", static_cast<double>(info.minRange));
    SetField(L, "maxRange", static_cast<double>(info.maxRange));
    SetField(L, "spellID", static_cast<double>(info.spellID));
    // Vanilla extras beyond the modern signature — present in 1.12's
    // Spell.dbc, no harm including them for addons backporting from
    // 3.3.5 where the same data was exposed positionally.
    SetField(L, "rank", info.rank);
    SetField(L, "cost", static_cast<double>(info.cost));
    SetFieldBool(L, "isFunnel", info.isFunnel);
    SetField(L, "powerType", static_cast<double>(info.powerType));
    return 1;
}

static int __fastcall Script_C_GetSpellName(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Spell.GetSpellName(spellID)");
        return 0;
    }
    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const uint8_t *record = Spell::Lookup::RecordForID(spellID);
    if (record == nullptr)
        return 0; // nil for unknown spellID

    const int locale = ReadGlobal<int>(Offsets::VAR_LOCALE_INDEX);
    const char *name = *reinterpret_cast<const char *const *>(record + OFF_NAME + locale * 4);
    if (name == nullptr || *name == '\0')
        return 0; // empty / no name in current locale → nil
    Game::Lua::PushString(L, name);
    return 1;
}

static int __fastcall Script_C_GetSpellTexture(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Spell.GetSpellTexture(spellID)");
        return 0;
    }
    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const uint8_t *record = Spell::Lookup::RecordForID(spellID);
    if (record == nullptr)
        return 0;

    const int iconID = *reinterpret_cast<const int *>(record + OFF_ICON_ID);
    auto *iconRec = LookupSubRecord(Offsets::VAR_SPELL_ICON_RECORDS,
                                    Offsets::VAR_SPELL_ICON_COUNT, iconID);
    if (iconRec == nullptr)
        return 0;
    const char *path = *reinterpret_cast<const char *const *>(iconRec + 4);
    if (path == nullptr || *path == '\0')
        return 0;
    Game::Lua::PushString(L, path);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetSpellInfo", &Script_GetSpellInfo);
    Game::Lua::RegisterTableFunction("C_Spell", "GetSpellInfo", &Script_C_GetSpellInfo);
    Game::Lua::RegisterTableFunction("C_Spell", "GetSpellName", &Script_C_GetSpellName);
    Game::Lua::RegisterTableFunction("C_Spell", "GetSpellTexture", &Script_C_GetSpellTexture);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::Info
