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

// Player state queries that modern WoW exposes as no-arg globals
// (`IsMounted`, `IsStealthed`, etc.) but vanilla 1.12 doesn't bind
// to Lua. The underlying data IS in the engine — broadcast in
// UpdateFields for some, tracked locally for others — we just
// read it directly off the player descriptor / object.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Unit::State {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

const uint8_t *Player() {
    auto fn = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    return static_cast<const uint8_t *>(fn("player"));
}

const uint8_t *PlayerDescriptor() {
    auto *player = Player();
    if (player == nullptr)
        return nullptr;
    return *reinterpret_cast<const uint8_t *const *>(
        player + Offsets::OFF_UNIT_DESCRIPTOR);
}

uint32_t PlayerMovementFlags() {
    auto *player = Player();
    if (player == nullptr)
        return 0;
    return *reinterpret_cast<const uint32_t *>(
        player + Offsets::OFF_PLAYER_MOVEMENT_FLAGS);
}

uint32_t MountDisplayID(const uint8_t *desc) {
    if (desc == nullptr)
        return 0;
    return *reinterpret_cast<const uint32_t *>(
        desc + Offsets::OFF_UNIT_FIELD_MOUNTDISPLAYID);
}

// `IsMounted()` — true iff the player is currently mounted. Reads
// `UNIT_FIELD_MOUNTDISPLAYID` from the descriptor; non-zero means
// the engine is rendering a mount under the player.
int __fastcall Script_IsMounted(void *L) {
    auto *desc = PlayerDescriptor();
    Game::Lua::PushBoolean(L, MountDisplayID(desc) != 0 ? 1 : 0);
    return 1;
}

// `IsStealthed()` — true iff the player is currently stealthed
// (Rogue Stealth / Druid Prowl). Checks `PLAYER_FIELD_VIS_BYTES`
// bit 0x02. Mount also sets that bit, so we AND-gate with
// `mountDisplayID == 0` to disambiguate. See the comment on
// `OFF_PLAYER_FIELD_VIS_BYTES` in Offsets.h for the full bit map
// and known limitations (untested for shapeshift forms).
int __fastcall Script_IsStealthed(void *L) {
    auto *desc = PlayerDescriptor();
    bool stealthed = false;
    if (desc != nullptr) {
        const uint32_t visBytes = *reinterpret_cast<const uint32_t *>(
            desc + Offsets::OFF_PLAYER_FIELD_VIS_BYTES);
        stealthed = (visBytes & Offsets::PLAYER_VIS_BIT_STEALTH) != 0
                    && MountDisplayID(desc) == 0;
    }
    Game::Lua::PushBoolean(L, stealthed ? 1 : 0);
    return 1;
}

// `IsFalling()` — true iff the player is currently mid-fall or
// jumping. Reads the local CGPlayer movement-flags word at
// `+0x9E8` and tests the FALLING / FALLING_FAR bits. Modern
// `IsFalling` returns true for the entire airtime of a jump — we
// match by accepting either bit (FALLING during the active jump,
// FALLING_FAR for sustained falls).
int __fastcall Script_IsFalling(void *L) {
    const uint32_t flags = PlayerMovementFlags();
    const uint32_t fallMask = Offsets::MOVEFLAG_FALLING |
                              Offsets::MOVEFLAG_FALLING_FAR;
    Game::Lua::PushBoolean(L, (flags & fallMask) != 0 ? 1 : 0);
    return 1;
}

// `IsSwimming()` — true iff the player is currently swimming
// (in liquid, with the swim animation/control set). Single-bit
// check on the SWIMMING flag in the local movement-flags word.
int __fastcall Script_IsSwimming(void *L) {
    const uint32_t flags = PlayerMovementFlags();
    Game::Lua::PushBoolean(L,
        (flags & Offsets::MOVEFLAG_SWIMMING) != 0 ? 1 : 0);
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("IsMounted", &Script_IsMounted);
    Game::Lua::RegisterGlobalFunction("IsStealthed", &Script_IsStealthed);
    Game::Lua::RegisterGlobalFunction("IsFalling", &Script_IsFalling);
    Game::Lua::RegisterGlobalFunction("IsSwimming", &Script_IsSwimming);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Unit::State
