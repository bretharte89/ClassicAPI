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

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Unit::Flags {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

// Reads a PLAYER_FLAGS bit for any player-controlled unit. Path:
//   - Resolve unit; gate on `UNIT_FLAG_PLAYER_CONTROLLED` to avoid
//     reading the +0xE68 pointer on a creature/NPC (it's uninitialized
//     for non-player units).
//   - Read `[unit + 0xE68] + 0x08` — the unit's CGPlayer-side info
//     struct, where byte +0x08 is PLAYER_FLAGS. Same struct
//     `Script_IsResting` reads at +0x05 bit (RESTING = 0x20) for the
//     local player; same struct the nameplate AFK-rendering code at
//     `0x005EC9E0` reads at bit 1 (AFK = 0x02) for **any** unit being
//     rendered. PLAYER_FLAGS lives here for all players — local and
//     remote — even though it's not in the broadcast UpdateField data.
//
// The `[unit + 0xE68]` pointer is the same one the visible-items
// helper (`FUN_UNIT_GET_VISIBLE_ITEM`) walks at offset `+0x118 +
// slot*0x30`; it's a CGPlayer-specific sub-struct that's allocated
// for any player-controlled unit, with PLAYER_FLAGS at +0x08 and
// visible-items table at +0x118.
bool TestPlayerFlag(void *L, uint32_t flagMask) {
    if (!Game::Lua::IsString(L, 1))
        return false;
    const char *token = Game::Lua::ToString(L, 1);
    if (token == nullptr)
        return false;

    auto resolve = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *unit = static_cast<const uint8_t *>(resolve(token));
    if (unit == nullptr)
        return false;

    auto *fields = *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_UNIT_DESCRIPTOR);
    if (fields == nullptr)
        return false;

    // Gate on player-controlled — `[unit + 0xE68]` is uninitialized
    // for creatures/NPCs and reading it would either return garbage
    // or crash on the +8 deref.
    const uint32_t unitFlags = *reinterpret_cast<const uint32_t *>(
        fields + Offsets::OFF_UNIT_FIELD_FLAGS);
    if ((unitFlags & Offsets::UNIT_FLAG_PLAYER_CONTROLLED) == 0)
        return false;

    auto *playerInfo = *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_CGPLAYER_INFO);
    if (playerInfo == nullptr)
        return false;
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        playerInfo + Offsets::OFF_PLAYER_INFO_FLAGS);
    return (flags & flagMask) != 0;
}

// `UnitIsAFK(unit)` — returns true if the unit is currently AFK
// (toggled via `/afk` or auto-set after idle timeout). Works for any
// player-controlled unit (player, target, party*, raid*, etc.). NPCs
// always return false.
int __fastcall Script_UnitIsAFK(void *L) {
    Game::Lua::PushBoolean(L,
        TestPlayerFlag(L, Offsets::PLAYER_FLAG_AFK) ? 1 : 0);
    return 1;
}

// `UnitIsDND(unit)` — returns true if the unit is currently DND
// ("Do Not Disturb", toggled via `/dnd`). Same unit-token coverage as
// `UnitIsAFK`.
int __fastcall Script_UnitIsDND(void *L) {
    Game::Lua::PushBoolean(L,
        TestPlayerFlag(L, Offsets::PLAYER_FLAG_DND) ? 1 : 0);
    return 1;
}

// `UnitIsFeignDeath(unit)` — returns true if the unit is feigning
// death (Hunter's `Feign Death`). Reads `UNIT_FIELD_FLAGS` bit 29
// (`0x20000000`) directly off the unit's m_objectFields descriptor.
// Unlike AFK/DND, this works for any unit token because
// UNIT_FIELD_FLAGS is broadcast in object updates — anyone watching
// the hunter sees the flag set.
int __fastcall Script_UnitIsFeignDeath(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitIsFeignDeath(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);
    if (token == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    auto resolve = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *unit = static_cast<const uint8_t *>(resolve(token));
    if (unit == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    auto *fields = *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_UNIT_DESCRIPTOR);
    if (fields == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const uint32_t unitFlags = *reinterpret_cast<const uint32_t *>(
        fields + Offsets::OFF_UNIT_FIELD_FLAGS);
    Game::Lua::PushBoolean(L,
        (unitFlags & Offsets::UNIT_FLAG_FEIGN_DEATH) != 0 ? 1 : 0);
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("UnitIsAFK", &Script_UnitIsAFK);
    Game::Lua::RegisterGlobalFunction("UnitIsDND", &Script_UnitIsDND);
    Game::Lua::RegisterGlobalFunction("UnitIsFeignDeath", &Script_UnitIsFeignDeath);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Unit::Flags
