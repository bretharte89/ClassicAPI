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

// Aura caster / duration cache, fed by a co-hook on `SMSG_SPELL_GO`
// (`FUN_SPELL_GO`). The vanilla unit aura array (`UNIT_FIELD_AURA`) stores
// only spell IDs — it has no record of *who* cast an aura, and no
// cast/expiration timing for any unit but the local player. The one place
// the client ever sees the caster + a server-authoritative duration is the
// `SMSG_SPELL_GO` packet at cast time. We parse it (the same packet
// nampower parses for its `AURA_CAST_ON_*` events) and remember, per
// `(targetGuid, spellId)`, who cast it and when it should expire.
//
// `Aura::Data::Push` consults this to fill `sourceUnit` (caster, resolved
// to a unit token) and `expirationTime` for non-player units. The cache is
// inherently best-effort: it only knows about casts observed since login,
// and an entry survives until it expires (combat-duration auras) or is
// evicted under pressure. Misses leave the modern-truthful defaults
// (`sourceUnit` nil, `expirationTime` 0) in place — same outcome as before
// this module existed.

namespace Aura::Source {

// Looks up the cached caster + timing for the aura `spellId` currently on
// the unit identified by `unitGuid`. Returns true and fills the out params
// on a hit. `*outExpirationMs` is an absolute `GetTickCount`-epoch timestamp
// (0 = unknown / infinite-duration aura); `*outDurationMs` is the applied
// duration including the caster's modifiers (talents etc.; 0 = none) — use
// it for the `duration` field so it stays consistent with `expirationTime`;
// `*outCaster` is the caster's 64-bit GUID (never 0 on a hit). Returns false
// on a miss or for zero inputs.
bool Get(uint64_t unitGuid, uint32_t spellId, uint64_t *outCaster,
         uint32_t *outExpirationMs, uint32_t *outDurationMs);

// One cached aura, as returned by `Enumerate`.
struct CachedAura {
    uint32_t spellId;
    uint64_t casterGuid;   // 0 = caster unknown (application hook, no SpellGo)
    uint32_t expirationMs; // 0 = infinite / unknown
    uint32_t durationMs;   // applied duration (incl. caster mods); 0 = none
};

// Fills `out` with up to `maxOut` cached, non-expired auras on `unitGuid`
// whose helpful/harmful classification matches `harmful`. Returns the count
// written. The classification is recorded from the aura's descriptor slot at
// application time (`OnAuraAdded`/`OnAuraStacksChanged`); entries we only ever
// saw via `SMSG_SPELL_GO` have no slot and so an unknown classification —
// those are excluded (we can't tell which range they belong to).
//
// This is the fallback source for `Aura::Data` when the unit descriptor has
// dropped an aura's slot (rogue stealth, party range fluctuation) even though
// the aura is still active server-side: the cache survives descriptor clears.
int Enumerate(uint64_t unitGuid, bool harmful, CachedAura *out, int maxOut);

} // namespace Aura::Source
