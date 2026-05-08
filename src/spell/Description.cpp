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

#include <cstdint>

namespace Spell::Description {

// Mirrors `Spell::Info::LookupRecord` but kept private here to avoid a
// header just for the one shared helper. The Spell.dbc class instance is
// at standard layout: records-array at `[VAR_SPELL_RECORDS]`, count at
// `[VAR_SPELL_RECORD_COUNT]`, indexed directly by spellID (1-based).
static const uint8_t *LookupSpellRecord(int spellID) {
    if (spellID <= 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_RECORD_COUNT));
    if (spellID > count)
        return nullptr;
    auto *const *records = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_RECORDS));
    if (records == nullptr)
        return nullptr;
    return records[spellID];
}

using FormatSpellDescription_t = void(__fastcall *)(const void *spellRecord, char *outBuf,
                                                    int bufLen, int contextFlag, int reserved3,
                                                    int useToolTipText, int reserved5,
                                                    int reserved6);

static int __fastcall Script_GetSpellDescription(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Spell.GetSpellDescription(spellID)");
        return 0;
    }
    const int spellID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const uint8_t *record = LookupSpellRecord(spellID);
    if (record == nullptr)
        return 0; // nil for invalid / out-of-range spell IDs

    // 1024 bytes is what every engine caller uses (`push 0x400`). Spell
    // descriptions in vanilla don't exceed this in any locale; the helper
    // null-terminates and stops writing on overflow, so the upper bound is
    // self-enforced.
    char buffer[0x400];
    buffer[0] = '\0';

    auto fn = reinterpret_cast<FormatSpellDescription_t>(
        Offsets::FUN_FORMAT_SPELL_DESCRIPTION);
    fn(record, buffer, sizeof(buffer),
       /*contextFlag*/ 0,
       /*reserved3*/ 0,
       /*useToolTipText*/ 0, // 0 = read description (+0x228), non-zero = ToolTip (+0x24C)
       /*reserved5*/ 1,
       /*reserved6*/ 0);

    if (buffer[0] == '\0')
        return 0; // empty / no description in current locale → nil
    Game::Lua::PushString(L, buffer);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Spell", "GetSpellDescription",
                                     &Script_GetSpellDescription);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::Description
