// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

// Modern WoW exposes input-state transitions as events; vanilla 1.12
// doesn't. One per-frame `Tick::WorldTick` callback reads the engine
// state once, edge-detects each pair, fires whichever transitioned.
//
//   PLAYER_STARTED_MOVING / STOPPED_MOVING
//     Source: UI-input controller flag bits (WASD / autorun).
//     Key-state based — STOPPED fires the instant the movement keys
//     release, even mid-air. Matches retail.
//
//   PLAYER_STARTED_TURNING / STOPPED_TURNING
//     Source: per-frame delta on the player's body yaw at
//     `CGPlayer + 0x9C4`. Fires when the character is *actually
//     rotating*, not merely on RMB press — matches retail's
//     "camera has moved" semantics. The mouselook bit is held
//     while RMB is down even if the user isn't dragging; gating
//     on it alone would over-fire on every click.
//
//   PLAYER_STARTED_LOOKING / STOPPED_LOOKING
//     Source: UI-input free-look bit (LMB-drag). Currently
//     bit-based (fires on click, not on actual camera move) —
//     a known divergence from retail. The camera-yaw global
//     lives outside CGPlayer; finding its address is a TODO.
//     Addons that want raw click detection should use
//     `GLOBAL_MOUSE_DOWN`/`UP` for now.

#include "Offsets.h"
#include "event/Custom.h"
#include "tick/WorldTick.h"

#include <cmath>
#include <cstdint>

namespace Player::InputEvents {

namespace {

constexpr const char *kStartedMovingEvent  = "PLAYER_STARTED_MOVING";
constexpr const char *kStoppedMovingEvent  = "PLAYER_STOPPED_MOVING";
constexpr const char *kStartedLookingEvent = "PLAYER_STARTED_LOOKING";
constexpr const char *kStoppedLookingEvent = "PLAYER_STOPPED_LOOKING";
constexpr const char *kStartedTurningEvent = "PLAYER_STARTED_TURNING";
constexpr const char *kStoppedTurningEvent = "PLAYER_STOPPED_TURNING";

// Below this per-frame delta, treat the yaw as stable. Picks up
// real rotation (smallest user-perceptible RMB-drag is ~0.5°/frame
// = 0.009 rad) while ignoring float drift / single-bit jitter on
// the broadcast-position copy.
constexpr float YAW_EPSILON = 0.001f;

bool g_wasMoving = false;
bool g_wasLooking = false;
bool g_wasTurning = false;

float g_prevBodyYaw = 0.0f;
bool g_havePrevYaw = false;

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

const uint8_t *Controller() {
    return *reinterpret_cast<const uint8_t *const *>(Offsets::VAR_UI_INPUT_CONTROLLER);
}

const uint8_t *Player() {
    auto fn = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return static_cast<const uint8_t *>(fn("player"));
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
    auto *player = Player();
    if (ctrl == nullptr || player == nullptr) {
        // Pre-login / teardown — reset so first valid tick doesn't
        // fire spurious STOPPED transitions.
        g_wasMoving = false;
        g_wasLooking = false;
        g_wasTurning = false;
        g_havePrevYaw = false;
        return;
    }

    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        ctrl + Offsets::OFF_UI_INPUT_FLAGS);
    const bool moving        = (flags & Offsets::INPUT_FLAGS_MOVING_ANY) != 0;
    const bool looking       = (flags & Offsets::INPUT_FLAG_FREE_LOOK) != 0;
    const bool mouselookHeld = (flags & Offsets::INPUT_FLAG_MOUSELOOK) != 0;

    // TURNING is a latched bit-AND-rotation signal:
    //   STARTED fires when the mouselook bit is held AND the body
    //     yaw changes (the first actual drag after pressing RMB).
    //   STOPPED fires when the mouselook bit clears (RMB release).
    // Once STARTED has fired during a hold, the latch stays on
    // until the bit clears — so a drag-stop-drag motion within
    // the same RMB hold doesn't flap STARTED/STOPPED.
    const float yaw = *reinterpret_cast<const float *>(
        player + Offsets::OFF_PLAYER_BODY_YAW);
    bool turning;
    if (!mouselookHeld) {
        turning = false;
    } else if (g_wasTurning) {
        turning = true; // latched on for the rest of this hold
    } else if (g_havePrevYaw &&
               std::fabs(yaw - g_prevBodyYaw) > YAW_EPSILON) {
        turning = true; // first rotation within this RMB hold
    } else {
        turning = false; // bit held but no drag yet — waiting
    }
    g_prevBodyYaw = yaw;
    g_havePrevYaw = true;

    FireTransition(moving,  g_wasMoving,  kStartedMovingEvent,  kStoppedMovingEvent);
    FireTransition(turning, g_wasTurning, kStartedTurningEvent, kStoppedTurningEvent);
    FireTransition(looking, g_wasLooking, kStartedLookingEvent, kStoppedLookingEvent);
}

} // namespace

static const Event::Custom::AutoReserve _reserveStartedMoving{kStartedMovingEvent};
static const Event::Custom::AutoReserve _reserveStoppedMoving{kStoppedMovingEvent};
static const Event::Custom::AutoReserve _reserveStartedLooking{kStartedLookingEvent};
static const Event::Custom::AutoReserve _reserveStoppedLooking{kStoppedLookingEvent};
static const Event::Custom::AutoReserve _reserveStartedTurning{kStartedTurningEvent};
static const Event::Custom::AutoReserve _reserveStoppedTurning{kStoppedTurningEvent};
static const Tick::WorldTick::AutoSubscribe _tickSub{&OnWorldTick};

} // namespace Player::InputEvents
