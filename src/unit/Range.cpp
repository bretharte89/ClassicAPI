// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

// `UnitInRange(unit) → (inRange, checkedRange)` — 40-yard healing-
// range check, modern's `UnitInRange` shape. Vanilla 1.12 doesn't
// ship the function but has all the underlying machinery — the
// same `GetPosition` virtual (vtable slot 5) `CheckInteractDistance`
// uses, applied with a fixed 40-yard threshold.

#include "Game.h"
#include "unit/Position.h"

#include <cstring>

namespace Unit::Range {

namespace {

constexpr float kRangeYards = 40.0f;
constexpr float kRangeYardsSq = kRangeYards * kRangeYards;

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

    float unitPos[3] = {};
    float playerPos[3] = {};
    if (!Unit::Position::ReadToken(token, unitPos) ||
        !Unit::Position::ReadToken("player", playerPos)) {
        Game::Lua::PushBool(L, false);
        Game::Lua::PushBool(L, false);
        return 2;
    }

    const float dx = unitPos[0] - playerPos[0];
    const float dy = unitPos[1] - playerPos[1];
    const float dz = unitPos[2] - playerPos[2];
    const float distSq = dx * dx + dy * dy + dz * dz;

    Game::Lua::PushBool(L, distSq <= kRangeYardsSq);
    Game::Lua::PushBool(L, true);
    return 2;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("UnitInRange", &Script_UnitInRange);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Unit::Range
