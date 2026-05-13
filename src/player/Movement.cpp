// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

// `PLAYER_STARTED_MOVING` / `PLAYER_STOPPED_MOVING` — modern events
// vanilla doesn't fire. Per-frame poll of the local player's
// movement-flags word (same word `IsFalling`/`IsSwimming` read) and
// edge-detect on the "translational motion" subset of bits. Fires
// STARTED on `false→true`, STOPPED on `true→false`.
//
// The mask intentionally excludes TURN_LEFT / TURN_RIGHT (turn-in-
// place doesn't count as moving on modern) and the protocol-level
// flags like WALK_MODE, ONTRANSPORT, LEVITATING that don't reflect
// active translation. Jumping and falling DO count — modern fires
// STARTED_MOVING when you jump in place, and the airborne portion of
// any jump/fall keeps the event "moving" until you touch down.
//
// We subscribe to the shared `Tick::WorldTick` registry rather than
// installing our own MinHook on `FUN_WORLD_TICK` — MinHook permits
// only one hook per target address, and `bag/UpdateDelayed.cpp`
// also wants per-frame ticks.

#include "Offsets.h"
#include "event/Custom.h"
#include "tick/WorldTick.h"

#include <cstdint>

namespace Player::Movement {

namespace {

constexpr const char *kStartedMovingEvent = "PLAYER_STARTED_MOVING";
constexpr const char *kStoppedMovingEvent = "PLAYER_STOPPED_MOVING";

constexpr uint32_t kMovingMask =
      Offsets::MOVEFLAG_FORWARD
    | Offsets::MOVEFLAG_BACKWARD
    | Offsets::MOVEFLAG_STRAFE_LEFT
    | Offsets::MOVEFLAG_STRAFE_RIGHT
    | Offsets::MOVEFLAG_JUMPING
    | Offsets::MOVEFLAG_FALLING
    | Offsets::MOVEFLAG_FALLING_FAR;

bool g_wasMoving = false;

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

const uint8_t *Player() {
    auto fn = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return static_cast<const uint8_t *>(fn("player"));
}

void OnWorldTick() {
    auto *player = Player();
    if (player == nullptr) {
        // Pre-world / character select / loading screen. Reset
        // state so the next valid tick doesn't fire a spurious
        // STOPPED when the player pointer first comes back.
        g_wasMoving = false;
        return;
    }
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        player + Offsets::OFF_PLAYER_MOVEMENT_FLAGS);
    const bool moving = (flags & kMovingMask) != 0;
    if (moving == g_wasMoving)
        return;
    g_wasMoving = moving;
    const int slot = Event::Custom::Lookup(
        moving ? kStartedMovingEvent : kStoppedMovingEvent);
    if (slot >= 0)
        Event::Custom::Fire_None(slot);
}

} // namespace

static const Event::Custom::AutoReserve _reserveStarted{kStartedMovingEvent};
static const Event::Custom::AutoReserve _reserveStopped{kStoppedMovingEvent};
static const Tick::WorldTick::AutoSubscribe _tickSub{&OnWorldTick};

} // namespace Player::Movement
