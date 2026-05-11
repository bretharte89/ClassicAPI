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

namespace Event::Custom {

// Reserve an event name to be claimed in the engine's event table at
// the next safe opportunity. Place a static instance at file scope:
//
//   static const Event::Custom::AutoReserve _r{"MY_EVENT"};
//
// Static-init chains the name onto an internal list *before* `DllMain`
// runs. After the engine and any other DLLs have finished writing to
// the event table (signaled by the first `Frame::RegisterEvent` call
// from Lua, which our hook intercepts to fire `RetryClaims`), we walk
// the table from the END looking for NULL-name slots and claim them
// for our reserved names — engine-owned `SStrDup` storage, so the
// engine's reload teardown frees them correctly.
//
// We don't hook `RebuildEventTable` directly: chaining with other DLLs
// that hook the same function (SuperWoWhook, nampower, transmogfix,
// VanillaMinimapTracking) led to count→buffer-size mismatches and
// crashes in the engine's fill loop. The slot-claim approach lets each
// DLL operate on the table independently of the others.
//
// The name pointer must outlive the engine (a string literal does).
// Same name reserved twice is deduped; reserving more than 32 names
// total silently drops the overflow.
struct AutoReserve {
    explicit AutoReserve(const char *name);
};

// Returns the slot id currently assigned to `name`, or -1 if not yet
// claimed (e.g. before the first Lua-side `RegisterEvent` has
// triggered `RetryClaims`). Slot indices may change across `/reload`,
// so call this at fire time rather than caching the value.
int Lookup(const char *name);

// Dispatches an event with no payload (empty format string). Used by
// `EQUIPMENT_SETS_CHANGED` and other notifications whose only signal
// is "something in this domain changed — go re-read".
void Fire_None(int eventID);

// Dispatches an event with two int args (format `"%d%d"`). Pass
// booleans as 0/1 since the engine has no native bool format code.
void Fire_DD(int eventID, int arg1, int arg2);

// Three-int variant — `(eventID, %d%d%d, a, b, c)`. Used by
// `FACTION_STANDING_CHANGED(factionID, newStanding, repGained)`.
void Fire_DDD(int eventID, int arg1, int arg2, int arg3);

// Dispatches with `(string, int)` — used by `MODIFIER_STATE_CHANGED`
// for `(keyName, down)` payloads. `arg1` must outlive the call (the
// engine doesn't copy strings out of varargs); for compile-time
// literals this is automatic.
void Fire_SD(int eventID, const char *arg1, int arg2);

// Internal: try to claim a slot for every reservation that's still
// unclaimed (`slot < 0`). Called from the `Frame::RegisterEvent` hook
// in DllMain — every time Lua calls `frame:RegisterEvent(...)`, we
// catch any reservations that couldn't claim earlier.
void RetryClaims();

// Internal: permit `TryClaim` to write to the event table. Held
// closed until `LoadScriptFunctions_h` returns, so the boot phase
// (during which the engine and SuperWoWhook fire many internal
// `RegisterEvent` calls) can't trigger our writes. Writing during
// that window crashes the engine in `SMemFree` on slots it expected
// to still be NULL.
void EnableWrites();

// Internal: invalidate cached slot indices before `/reload`. The
// engine rebuilds the event table at a fresh allocation; our cached
// slots point into the old layout and need to re-claim.
void PrepareForReload();

} // namespace Event::Custom
