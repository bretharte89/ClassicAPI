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
#include "Offsets.h"

#include <cstdint>
#include <cstring>

namespace Unit::Range {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
using GetPosition_t = float *(__thiscall *)(void *self, float outBuf[3]);

constexpr float kRangeYards = 40.0f;
constexpr float kRangeYardsSq = kRangeYards * kRangeYards;

void *ResolveUnit(const char *token) {
    if (token == nullptr)
        return nullptr;
    auto fn = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return fn(token);
}

// Calls `obj->vtable[5](outBuf)`. The returned float* may equal
// outBuf (the object filled it directly) or point at a cached
// position field on the object — either way, deref it for x/y/z.
// Returns nullptr if the object's vtable virtual returns null (no
// known position for this object yet).
float *ReadPosition(void *obj, float outBuf[3]) {
    if (obj == nullptr)
        return nullptr;
    auto **vtable = *reinterpret_cast<void ***>(obj);
    auto fn = reinterpret_cast<GetPosition_t>(
        vtable[Offsets::OFF_CGOBJECT_VTBL_GET_POSITION / 4]);
    return fn(obj, outBuf);
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

    void *unit = ResolveUnit(token);
    void *player = ResolveUnit("player");
    if (unit == nullptr || player == nullptr) {
        Game::Lua::PushBool(L, false);
        Game::Lua::PushBool(L, false);
        return 2;
    }

    float unitBuf[3] = {};
    float playerBuf[3] = {};
    const float *unitPos = ReadPosition(unit, unitBuf);
    const float *playerPos = ReadPosition(player, playerBuf);
    if (unitPos == nullptr || playerPos == nullptr) {
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
