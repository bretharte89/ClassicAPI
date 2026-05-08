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

#pragma once

#include <cstdint>

namespace Spell::Lookup {

// Returns the Spell.dbc record pointer for `spellID`, or nullptr if the
// ID is out of range or the slot is empty. Records are 1-based; ID 0 is
// always invalid.
const uint8_t *RecordForID(int spellID);

// Resolves a 1-based Lua-facing spellbook slot to the spellID stored
// there. `bookType` is `0` for the player spellbook, `1` for the pet
// spellbook (matches the engine's encoding from `Script_GetSpellName`).
// Returns 0 if the slot is out of range or empty (Lua side surfaces 0
// as nil, since spellID 0 is never valid).
//
// `bookType` strings — the canonical mapping used by 1.12's
// `GetSpellName(slot, bookType)`:
//   - "spell" or anything-not-pet  → 0 (player)
//   - "pet"                          → 1 (pet)
int SpellbookSlotToID(int slot1Based, int bookType);

// Inverse of `SpellbookSlotToID` — finds the 1-based slot where a
// given `spellID` lives in the spellbook arrays. Searches the player
// book first (`bookType=0`), then the pet book (`bookType=1`). On a
// match, returns the 1-based slot and writes the matching `bookType`
// to `*outBookType` if it's non-null. Returns 0 if the spellID isn't
// in either book.
int FindSpellbookSlot(int spellID, int *outBookType);

} // namespace Spell::Lookup
