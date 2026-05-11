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

// `BAG_UPDATE_DELAYED` event — fires once per frame in which any
// `BAG_UPDATE` fired. Polyfills modern WoW's event of the same name
// (added 5.4.x) with exact same-semantic coalescing: addons listen
// to DELAYED instead of BAG_UPDATE and rescan once per frame.
//
// See TODO #67 for the full investigation history.
//
// Architecture: three hooks total.
//
//   1. `FUN_004F91A0` — bag-slot diff loop. Hook just sets the
//      `g_pending` flag.
//   2. `FUN_004F9370` — item→bag resolver (most common path). Hook
//      just sets the `g_pending` flag.
//   3. `FUN_0066FD50` — engine's per-frame world-subsystem update.
//      Hook drains `g_pending` and fires DELAYED if set.
//
// All three targets are quiet (≤2 callers each, in code regions
// other DLLs don't touch). The previous attempt at this feature
// hooked `FUN_FIRE_EVENT` (100+ callers, in the chat path) and a
// per-frame UI dispatcher — both prime collision targets for
// SuperWoWhook / nampower / transmogfix. By contrast:
//
//   - `FUN_0066FD50` is the WORLD-subsystem update (not the UI one),
//     deep in the rendering pipeline. Only one caller, no DLL
//     touches it. It runs exactly once per frame and increments
//     the engine's own per-frame counter at `0x00C7B2C8`, so we
//     know it ticks 1:1 with frames.
//   - The bag-subsystem hooks (`FUN_004F91A0`, `FUN_004F9370`) live
//     in the `0x004F9xxx` region none of the popular DLLs care
//     about.
//
// Coverage limitation: the keyring `BAG_UPDATE(-2)` path goes
// through `FUN_004F8DB0` (a `__thiscall` callback registered with
// the engine's item-descriptor change system). Hooking it cleanly
// needs the right calling-convention plumbing, deferred. Real
// inventory changes (the 95% case) go through one of the two
// bag-subsystem hooks above.

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"

namespace Bag::UpdateDelayed {

namespace {

constexpr const char *kEventName = "BAG_UPDATE_DELAYED";

const Event::Custom::AutoReserve _reserve{kEventName};

// Set by the bag-update hooks, drained by the world-tick hook.
// Single bool, no atomics needed — the engine is single-threaded.
bool g_pending = false;

} // namespace

// `FUN_004F91A0` — `__cdecl void()`. Bag-slot diff loop. Walks the
// 10 bag slots, fires BAG_UPDATE for each that changed.
using BagSlotDiff_t = void(__cdecl *)();
BagSlotDiff_t BagSlotDiff_o = nullptr;
void __cdecl BagSlotDiff_h() {
    BagSlotDiff_o();
    g_pending = true;
}

// `FUN_004F9370` — `__stdcall void(int guidLo, int guidHi)`. Most
// common BAG_UPDATE path — fires from every per-item descriptor
// field change.
using BagItemToBag_t = void(__stdcall *)(int guidLo, int guidHi);
BagItemToBag_t BagItemToBag_o = nullptr;
void __stdcall BagItemToBag_h(int guidLo, int guidHi) {
    BagItemToBag_o(guidLo, guidHi);
    g_pending = true;
}

// `FUN_0066FD50` — `__fastcall(int, int, int)` per-frame world tick.
// We hook the TAIL: run the original first (so all of this frame's
// world-tick work completes), then drain the pending flag and fire
// DELAYED if any BAG_UPDATE happened this frame.
using WorldTick_t = void(__fastcall *)(int arg1, int arg2, int arg3);
WorldTick_t WorldTick_o = nullptr;
void __fastcall WorldTick_h(int arg1, int arg2, int arg3) {
    WorldTick_o(arg1, arg2, arg3);
    if (!g_pending)
        return;
    g_pending = false;
    const int slot = Event::Custom::Lookup(kEventName);
    if (slot >= 0)
        Event::Custom::Fire_None(slot);
}

static const Game::HookAutoRegister _hookSlotDiff{
    Offsets::FUN_BAG_SLOT_DIFF,
    reinterpret_cast<void *>(&BagSlotDiff_h),
    reinterpret_cast<void **>(&BagSlotDiff_o)};

static const Game::HookAutoRegister _hookItemToBag{
    Offsets::FUN_BAG_ITEM_TO_BAG,
    reinterpret_cast<void *>(&BagItemToBag_h),
    reinterpret_cast<void **>(&BagItemToBag_o)};

static const Game::HookAutoRegister _hookWorldTick{
    Offsets::FUN_WORLD_TICK,
    reinterpret_cast<void *>(&WorldTick_h),
    reinterpret_cast<void **>(&WorldTick_o)};

} // namespace Bag::UpdateDelayed
