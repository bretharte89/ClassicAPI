// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

#pragma once

#include <cstdint>

// Shared aura-table primitives consumed by `C_UnitAuras.*` (and any
// future module that needs to inspect a unit's aura array). All
// reads go through the unit's `m_objectFields` descriptor at
// `unit + OFF_CGUNIT_OBJECT_FIELDS`; the layout is documented in
// `Offsets.h` under `OFF_UNIT_FIELD_AURA*`.
//
// Slot indexing convention used here matches the engine's:
//
//   slot 0..31  → buffs   (helpful)
//   slot 32..47 → debuffs (harmful)
//
// Callers that take a 1-based Lua index into "buffs" or "debuffs"
// translate to the absolute 0..47 slot before calling in.

namespace Aura::Data {

// Filter for slot iteration. The engine's `UnitBuff` / `UnitDebuff`
// pick one direction; modern `C_UnitAuras.GetAuraDataByIndex` defaults
// to helpful when no filter is specified.
enum class Filter { Helpful, Harmful };

// Reads the spell ID at the given absolute slot (0..47). Returns 0
// if the slot is empty or the unit pointer is null.
uint32_t ReadSpellID(const uint8_t *unit, int slot);

// True iff the slot is occupied AND the engine considers the aura
// applied (descriptor flag nibble & UNIT_AURA_VISIBLE_MASK != 0)
// AND the spell record passes the engine's visibility predicate.
// This is the same gate `Script_UnitBuff`/`Script_UnitDebuff` use
// to decide whether to surface an aura through Lua.
bool IsSlotPopulated(const uint8_t *unit, int slot);

// Finds the absolute slot of the `oneBasedIndex`-th populated aura
// matching `filter`. Returns -1 if no such aura. Walks
// 0..UNIT_AURA_BUFF_COUNT-1 for Helpful, UNIT_AURA_BUFF_COUNT..
// UNIT_AURA_TOTAL-1 for Harmful.
int FindNthSlot(const uint8_t *unit, int oneBasedIndex, Filter filter);

// Finds the absolute slot of a populated aura with the given spell
// ID, optionally restricted to one filter range. `filter` of
// nullptr searches both ranges (helpful first, then harmful).
// Returns -1 if not found.
int FindSlotBySpellID(const uint8_t *unit, uint32_t spellID,
                      const Filter *filter);

// Finds the absolute slot of a populated aura whose locale-resolved
// spell name exactly matches `spellName`. Case-sensitive. Otherwise
// same semantics as `FindSlotBySpellID`. Returns -1 if not found
// or if `spellName` is null/empty.
int FindSlotBySpellName(const uint8_t *unit, const char *spellName,
                        const Filter *filter);

// Display stack count for the given slot (engine stores stacks-1,
// we add 1). Returns 0 if the unit is null.
int ReadStacks(const uint8_t *unit, int slot);

// Returns true if the engine considers `spellRecord` a user-visible
// aura. Thin wrapper around `FUN_SPELL_IS_VISIBLE_AURA`. The record
// pointer must come from `Spell.dbc[spellID]`.
bool IsVisible(const uint8_t *spellRecord);

// Looks up `SpellDispelType.dbc[dispelTypeID]` and returns the
// locale-applied name (e.g. "Magic", "Curse", "Disease", "Poison"),
// or nullptr if the ID is 0, out of range, or has no name.
const char *DispelName(uint32_t dispelTypeID);

// Builds a modern-style `AuraData` table on top of the Lua stack
// from the aura currently in `unit`'s descriptor at `slot`. Caller
// is responsible for having validated that the slot's spell ID is
// non-zero and visible (or just wants the data anyway).
//
// Populates these fields with real data:
//   name, icon, applications, spellId, dispelName,
//   isHelpful, isHarmful, timeMod
//
// And these with vanilla-truthful defaults (matches modern's
// "field present but inapplicable" semantics):
//   duration=0, expirationTime=0, charges=0, maxCharges=0,
//   sourceUnit=nil, auraInstanceID=nil, points=nil,
//   isStealable=false, isBossAura=false, isFromPlayerOrPlayerPet=false,
//   isNameplateOnly=false, nameplateShowAll=false,
//   nameplateShowPersonal=false, canApplyAura=false,
//   shouldConsolidate=false, isRaid=false
//
// Net stack effect: +1 (the table).
void Push(void *L, const uint8_t *unit, int slot);

} // namespace Aura::Data
