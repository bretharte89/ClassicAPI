// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.

// `HEARTHSTONE_BOUND` event — polyfills modern WoW's event of the
// same name. Fires every time the player binds their hearthstone at
// an innkeeper, even when they re-bind at the SAME inn — matches
// modern semantics where the bind ACTION fires the event regardless
// of whether the location string changes.
//
// Hooks the BINDPOINTUPDATE packet handler at `FUN_005ED3C0` and
// gates on `VAR_BIND_POINT_VALID` to suppress only the initial
// post-login sync. The engine zeroes that flag during per-character
// entry init (`FUN_005E2510`); the handler sets it to 1 after
// parsing. If the flag is 0 before the original runs, this packet
// is the initial sync — don't fire. Any subsequent invocation is
// a real innkeeper rebind. Character switch is handled automatically
// because the entry init re-zeroes the flag, so each character's
// first update is also treated as sync.
//
// Event has no payload — addons call `GetBindLocation()` to read
// the new location, matching modern semantics.

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"

#include <cstdint>

namespace Player::HearthstoneBound {

namespace {

constexpr const char *kEventName = "HEARTHSTONE_BOUND";

const Event::Custom::AutoReserve _reserve{kEventName};

using BindPointUpdateHandler_t = void(__fastcall *)(void *packetBuffer);
BindPointUpdateHandler_t BindPointUpdateHandler_o = nullptr;

void __fastcall BindPointUpdateHandler_h(void *packetBuffer) {
    const uint32_t wasValid = *reinterpret_cast<const uint32_t *>(
        static_cast<uintptr_t>(Offsets::VAR_BIND_POINT_VALID));

    BindPointUpdateHandler_o(packetBuffer);

    if (wasValid == 0)
        return; // initial post-login sync — not a user rebind

    const int slot = Event::Custom::Lookup(kEventName);
    Event::Custom::Fire(slot, "");
}

} // namespace

static const Game::HookAutoRegister _hook{
    Offsets::FUN_BINDPOINT_UPDATE_HANDLER,
    reinterpret_cast<void *>(&BindPointUpdateHandler_h),
    reinterpret_cast<void **>(&BindPointUpdateHandler_o)};

} // namespace Player::HearthstoneBound
