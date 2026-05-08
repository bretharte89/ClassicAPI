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

#include "Lookup.h"

#include "Offsets.h"

namespace Spell::Lookup {

const uint8_t *RecordForID(int spellID) {
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

int SpellbookSlotToID(int slot1Based, int bookType) {
    const int slot = slot1Based - 1;
    if (slot < 0 || slot >= Offsets::SPELLBOOK_MAX_SLOTS)
        return 0;
    const auto base = (bookType == 1) ? Offsets::VAR_PET_SPELLBOOK
                                      : Offsets::VAR_PLAYER_SPELLBOOK;
    auto *array = reinterpret_cast<const int *>(static_cast<uintptr_t>(base));
    return array[slot];
}

} // namespace Spell::Lookup
