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

// FACTION_STANDING_CHANGED — fires once per reputation change with
// `(factionID, newStanding, repGained)`. Polyfills modern WoW's event
// of the same name (modern fires `(factionID, newStanding)`); we add
// `repGained` as a third arg because the engine has the delta on hand
// in EDX and addons commonly need it for "+X reputation" displays
// without re-deriving it from the chat string. Vanilla 1.12 only
// exposes the localized `CHAT_MSG_COMBAT_FACTION_CHANGE` text, which
// addons currently scrape and reverse-resolve back to IDs.
//
// We hook the engine's per-rep-change notify dispatcher at
// `0x0062C5F0`. That function is the single chokepoint that fires the
// chat event after a successful standing update: it's only called when
// the value actually changed AND the engine's `notify` flag was set
// (i.e. from the per-update SMSG handler, not the bulk init at login).
// This matches modern WoW's semantics — FACTION_STANDING_CHANGED does
// not fire for initial faction sync.
//
// The hook target's calling convention is `__fastcall(ecx = factionID,
// edx = signedDelta)`. `delta` is forwarded as the `repGained` event
// arg (positive on gain, negative on loss). To produce the
// `newStanding` payload, we read back the live total via
// `FUN_REPUTATION_GET_STANDING(factionID)` — the setter has already
// written the new delta into the per-slot storage by the time our
// hook fires.

#include "StandingChanged.h"

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"

namespace Faction::StandingChanged {

namespace {

using GetStanding_t = int(__fastcall *)(int factionID);

constexpr const char *kEventName = "FACTION_STANDING_CHANGED";

const Event::Custom::AutoReserve _reserve{kEventName};

} // namespace

FireNotify_t FireNotify_o = nullptr;

void __fastcall FireNotify_h(int factionID, int delta) {
    // Forward to the engine first so the chat event fires before our
    // custom event observers can react — same order as if we weren't
    // here. Also ensures the standing value is fully settled (the
    // setter at 0x004D6330 wrote it before reaching the call site, but
    // following original-first keeps observer ordering predictable
    // even if a future engine change introduces post-write fixup).
    FireNotify_o(factionID, delta);

    const int slot = Event::Custom::Lookup(kEventName);
    if (slot < 0)
        return;

    // Live total = base + currentDelta from the per-slot rep storage.
    // The helper returns 0 for factionIDs not in the player's rep list
    // — but we shouldn't see those here, since the engine only calls
    // this notify after a successful per-slot update.
    auto getStanding = reinterpret_cast<GetStanding_t>(
        Offsets::FUN_REPUTATION_GET_STANDING);
    const int newStanding = getStanding(factionID);

    Event::Custom::Fire_DDD(slot, factionID, newStanding, delta);
}

static const Game::HookAutoRegister _hookreg{
    Offsets::FUN_REPUTATION_FIRE_NOTIFY,
    reinterpret_cast<void *>(&FireNotify_h),
    reinterpret_cast<void **>(&FireNotify_o)};

} // namespace Faction::StandingChanged
