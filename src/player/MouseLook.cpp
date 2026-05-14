// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

// `PLAYER_STARTED_LOOKING` / `PLAYER_STOPPED_LOOKING` —  LMB-drag
// free-look (camera-only rotation, character stays put).
// `PLAYER_STARTED_TURNING` / `PLAYER_STOPPED_TURNING` — RMB-drag /
// `MouselookStart` mouselook mode (character rotates with the
// camera). Mouse-only — modern WoW's TURNING does not fire for
// keyboard turn-in-place, despite what the docs imply; verified
// empirically.
//
// Both pairs are sibling to `PLAYER_STARTED_MOVING` /
// `PLAYER_STOPPED_MOVING`: vanilla 1.12 already maintains the
// underlying flags as part of the UI input controller's master
// state word; modern WoW exposes the transitions as events but
// 1.12 doesn't. We poll the state byte once per frame and edge-
// detect.

#include "Offsets.h"
#include "event/Custom.h"
#include "tick/WorldTick.h"

#include <cstdint>

namespace Player::MouseLook {

namespace {

constexpr const char *kStartedLookingEvent = "PLAYER_STARTED_LOOKING";
constexpr const char *kStoppedLookingEvent = "PLAYER_STOPPED_LOOKING";
constexpr const char *kStartedTurningEvent = "PLAYER_STARTED_TURNING";
constexpr const char *kStoppedTurningEvent = "PLAYER_STOPPED_TURNING";

bool g_wasLooking = false;
bool g_wasTurning = false;

const uint8_t *Controller() {
    return *reinterpret_cast<const uint8_t *const *>(Offsets::VAR_UI_INPUT_CONTROLLER);
}

void FireTransition(bool now, bool &prev, const char *startedName,
                    const char *stoppedName) {
    if (now == prev)
        return;
    prev = now;
    const int slot = Event::Custom::Lookup(now ? startedName : stoppedName);
    if (slot >= 0)
        Event::Custom::Fire_None(slot);
}

void OnWorldTick() {
    auto *ctrl = Controller();
    if (ctrl == nullptr) {
        // Pre-login / teardown — reset so first valid tick doesn't
        // fire spurious STOPPED transitions.
        g_wasLooking = false;
        g_wasTurning = false;
        return;
    }
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        ctrl + Offsets::OFF_UI_INPUT_FLAGS);
    const bool turning = (flags & Offsets::INPUT_FLAG_MOUSELOOK) != 0;
    const bool looking = (flags & Offsets::INPUT_FLAG_FREE_LOOK) != 0;
    FireTransition(turning, g_wasTurning, kStartedTurningEvent, kStoppedTurningEvent);
    FireTransition(looking, g_wasLooking, kStartedLookingEvent, kStoppedLookingEvent);
}

} // namespace

static const Event::Custom::AutoReserve _reserveStartedLooking{kStartedLookingEvent};
static const Event::Custom::AutoReserve _reserveStoppedLooking{kStoppedLookingEvent};
static const Event::Custom::AutoReserve _reserveStartedTurning{kStartedTurningEvent};
static const Event::Custom::AutoReserve _reserveStoppedTurning{kStoppedTurningEvent};
static const Tick::WorldTick::AutoSubscribe _tickSub{&OnWorldTick};

} // namespace Player::MouseLook
