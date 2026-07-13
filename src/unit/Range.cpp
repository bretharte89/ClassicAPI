// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.

// Player↔unit proximity, modern's shape. Vanilla 1.12 ships neither
// function but has all the underlying machinery — the object's
// `GetPosition` virtual (vtable slot 5) `CheckInteractDistance` uses:
//
//   `UnitInRange(unit) → (inRange, checkedRange)` — 40-yard healing-
//     range check (fixed threshold on the squared distance).
//   `UnitDistanceSquared(unit) → (distanceSquared, checkedPosition)` —
//     the raw squared world distance player→unit. Squared because
//     nearly every consumer compares against a threshold (`distSq <=
//     range*range`) or ranks by nearest, neither of which needs the
//     sqrt; that's why retail exposes only the squared form (added
//     5.0.4). Center-to-center — NOT reach-aware (that's UnitXP_SP3's
//     niche); equivalent to computing it from SuperWoW `UnitPosition`,
//     but self-contained (reads the position in C, no sibling-DLL
//     dependency, no clash with SuperWoW's own `UnitPosition`).

#include "Game.h"
#include "unit/Position.h"

#include <cstring>

namespace Unit::Range {

namespace {

constexpr float kRangeYards = 40.0f;
constexpr float kRangeYardsSq = kRangeYards * kRangeYards;

// Squared world distance from the player to `token`'s unit. Returns true
// and writes *outSq on success; false when either position is unavailable
// (unresolved / absent token, or an object with no known position yet —
// e.g. a party member outside the client's sync range). This false is
// exactly the `checkedPosition = false` case retail flags.
bool PlayerToUnitDistSq(const char *token, float *outSq) {
    float unitPos[3] = {};
    float playerPos[3] = {};
    if (!Unit::Position::ReadToken(token, unitPos) ||
        !Unit::Position::ReadToken("player", playerPos))
        return false;
    const float dx = unitPos[0] - playerPos[0];
    const float dy = unitPos[1] - playerPos[1];
    const float dz = unitPos[2] - playerPos[2];
    *outSq = dx * dx + dy * dy + dz * dz;
    return true;
}

int __fastcall Script_UnitInRange(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitInRange(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);

    // Documented Blizzard quirk: `UnitInRange("player")` returns
    // false. The function is meant for healing-frame "is *another*
    // unit in range" checks; querying yourself is meaningless and
    // the API returns nil/false to flag that. We match the modern
    // behavior — no position read, no range check.
    if (token != nullptr && std::strcmp(token, "player") == 0) {
        Game::Lua::PushBool(L, false);
        Game::Lua::PushBool(L, false);
        return 2;
    }

    float distSq = 0.0f;
    if (!PlayerToUnitDistSq(token, &distSq)) {
        Game::Lua::PushBool(L, false);
        Game::Lua::PushBool(L, false);
        return 2;
    }

    Game::Lua::PushBool(L, distSq <= kRangeYardsSq);
    Game::Lua::PushBool(L, true);
    return 2;
}

// `UnitDistanceSquared("unit") → (distanceSquared, checkedPosition)`.
// Unlike UnitInRange there's no self-quirk: `UnitDistanceSquared("player")`
// is a legitimate `(0, true)`. On a position miss returns `(0, false)` —
// matching retail's "always a number" shape; consumers must branch on
// `checkedPosition`, since a real `0` (self, or exactly co-located units)
// is indistinguishable from the miss placeholder by value alone.
int __fastcall Script_UnitDistanceSquared(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitDistanceSquared(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);

    float distSq = 0.0f;
    if (!PlayerToUnitDistSq(token, &distSq)) {
        Game::Lua::PushNumber(L, 0.0);
        Game::Lua::PushBool(L, false);
        return 2;
    }

    Game::Lua::PushNumber(L, static_cast<double>(distSq));
    Game::Lua::PushBool(L, true);
    return 2;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("UnitInRange", &Script_UnitInRange);
    Game::Lua::RegisterGlobalFunction("UnitDistanceSquared",
                                      &Script_UnitDistanceSquared);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Unit::Range
