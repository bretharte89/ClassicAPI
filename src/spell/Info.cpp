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

#include "Info.h"

#include "Game.h"
#include "Offsets.h"

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

// Public Spell.dbc schema documents AttributesEx bit 6 (0x40) as the "channeled 2"
// / funnel flag. Not directly verified in the binary, but matches the consensus
// vanilla schema; treat false negatives as acceptable since no addon depends on
// funnel detection being precise.
static constexpr uint32_t SPELL_ATTR_EX_FUNNEL = 0x40;

template <typename T>
static T ReadGlobal(uintptr_t addr) {
    return *reinterpret_cast<T *>(addr);
}

static const uint8_t *LookupRecord(uintptr_t baseAddr, uintptr_t countAddr, int id) {
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

static int __fastcall Script_GetSpellInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetSpellInfo(spellID)");
        return 0;
    }

    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const uint8_t *record = LookupRecord(Offsets::VAR_SPELL_RECORDS,
                                         Offsets::VAR_SPELL_RECORD_COUNT, spellID);
    if (record == nullptr)
        return 0; // nil for invalid / out-of-range spell IDs

    const int locale = ReadGlobal<int>(Offsets::VAR_LOCALE_INDEX);

    // 1. name
    Game::Lua::PushString(L, *reinterpret_cast<const char *const *>(
                                 record + OFF_NAME + locale * 4));

    // 2. rank
    Game::Lua::PushString(L, *reinterpret_cast<const char *const *>(
                                 record + OFF_RANK + locale * 4));

    // 3. icon — texture path string from SpellIcon.dbc
    const int iconID = *reinterpret_cast<const int *>(record + OFF_ICON_ID);
    if (auto *iconRec = LookupRecord(Offsets::VAR_SPELL_ICON_RECORDS,
                                     Offsets::VAR_SPELL_ICON_COUNT, iconID)) {
        // PushString tail-calls PushNil for null pointers, so this is safe even
        // if the path slot is unset.
        Game::Lua::PushString(L, *reinterpret_cast<const char *const *>(iconRec + 4));
    } else {
        Game::Lua::PushNil(L);
    }

    // 4. cost — base ManaCost (no per-level scaling, since for unlearned spells
    // we don't have a meaningful caster level to scale against).
    Game::Lua::PushNumber(L, static_cast<double>(
                                 *reinterpret_cast<const int *>(record + OFF_MANA_COST)));

    // 5. isFunnel — real boolean to match 3.3.5 behavior.
    const uint32_t attrEx = *reinterpret_cast<const uint32_t *>(record + OFF_ATTRIBUTES_EX);
    Game::Lua::PushBoolean(L, (attrEx & SPELL_ATTR_EX_FUNNEL) != 0);

    // 6. powerType (0=mana, 1=rage, 2=focus, 3=energy, 4=happiness)
    Game::Lua::PushNumber(L, static_cast<double>(
                                 *reinterpret_cast<const int *>(record + OFF_POWER_TYPE)));

    // 7. castTime — base time in milliseconds from SpellCastTimes.dbc
    int castTimeMs = 0;
    const int castIndex = *reinterpret_cast<const int *>(record + OFF_CASTING_TIME_INDEX);
    if (auto *castRec = LookupRecord(Offsets::VAR_SPELL_CAST_TIMES_RECORDS,
                                     Offsets::VAR_SPELL_CAST_TIMES_COUNT, castIndex)) {
        castTimeMs = *reinterpret_cast<const int *>(castRec + 4);
    }
    Game::Lua::PushNumber(L, static_cast<double>(castTimeMs));

    // 8, 9. minRange / maxRange (floats in SpellRange.dbc)
    float minRange = 0.0f, maxRange = 0.0f;
    const int rangeIndex = *reinterpret_cast<const int *>(record + OFF_RANGE_INDEX);
    if (auto *rangeRec = LookupRecord(Offsets::VAR_SPELL_RANGE_RECORDS,
                                      Offsets::VAR_SPELL_RANGE_COUNT, rangeIndex)) {
        minRange = *reinterpret_cast<const float *>(rangeRec + 4);
        maxRange = *reinterpret_cast<const float *>(rangeRec + 8);
    }
    Game::Lua::PushNumber(L, static_cast<double>(minRange));
    Game::Lua::PushNumber(L, static_cast<double>(maxRange));

    return 9;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetSpellInfo", &Script_GetSpellInfo);
}

} // namespace Spell::Info
