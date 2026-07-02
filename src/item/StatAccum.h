// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>

// Shared item-stat accumulation engine. Reads an item's stats (base
// record + on-equip spell auras + random-suffix enchants) into a
// fixed key→value table keyed by FrameXML global-string names
// (`ITEM_MOD_*`, `RESISTANCE*_NAME`). Two consumers:
//   - `Item::Stats` — the `C_Item.GetItemStats` / `GetItemStatDelta`
//     Lua bindings, which turn an `Accum` into a Lua table.
//   - `Tooltip::Compare` — `GameTooltip:SetHyperlinkCompareItem`, which
//     diffs a hovered item against the equipped item and renders the
//     per-stat delta as colored tooltip lines.
//
// The full rationale for each stat source and the aura→key map lives in
// [src/item/StatAccum.cpp]; see also the "Item lookups" notes in
// CLAUDE.md for the `ItemStats_C` record shape.
namespace Item::StatAccum {

// One key→value pair. `key` is a FrameXML global-string name; `value`
// is the accumulated magnitude (native vanilla units — percents stay
// percents, not ratings).
struct Accum {
    const char *key;
    long value;
};

// Number of distinct keys an item can touch (base stats + armor + six
// resistances + the equip-spell "special" stats). Compile-time so
// callers can stack-allocate `Accum acc[kCount]`.
inline constexpr int kCount = 30;

// Fill `acc` (kCount entries) with the canonical key list and zero
// values. Call once before accumulating.
void Init(Accum *acc);

// Add `delta` to the entry matching `key` (no-op for an unknown key or
// a zero delta). Linear scan — the table is small and fixed.
void AddByKey(Accum *acc, const char *key, long delta);

// Passive cache peek for an item's `ItemStats_C` record. Does NOT warm
// the cache (returns nullptr for an uncached item) — see the item-cache
// race note in CLAUDE.md.
const uint8_t *FetchRecord(uint32_t itemID);

// Fold an item record's base stats + on-equip-spell stats into `acc`,
// scaled by `sign` (+1 to add, -1 to subtract for a delta). No-op for a
// null record.
void AccumulateRecord(Accum *acc, const uint8_t *record, long sign);

// Fold an item link's random-suffix bonus (ItemRandomProperties.dbc →
// SpellItemEnchantment.dbc) into `acc`, scaled by `sign`. No-op for
// suffixID <= 0.
void ApplyRandomSuffix(Accum *acc, int suffixID, long sign);

// Weapon DPS = average damage / swing time, or 0 for non-weapons.
double ComputeDPS(const uint8_t *record);

// Parse the random-property/suffix ID from the item link at 1-based Lua
// stack `idx` (`item:id:enchant:SUFFIX:unique`). 0 for a non-string
// arg, a bare `item:N`, or no `item:` payload.
int ParseRandomSuffixFromLink(void *L, int idx);

} // namespace Item::StatAccum
