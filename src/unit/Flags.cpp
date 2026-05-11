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
#include <cstdio>
#include <cstring>

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

// Reads `[unit + 0xE68 + 0x0C]` (the CGPlayer sub-struct's guild-key
// field) with all the safety gates needed for arbitrary unit tokens.
// Returns 0 if the unit isn't player-controlled, isn't a player, or
// hasn't had its guild data synced.
uint32_t ReadGuildKey(const uint8_t *unit) {
    if (unit == nullptr)
        return 0;
    auto *fields = *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_UNIT_DESCRIPTOR);
    if (fields == nullptr)
        return 0;
    const uint32_t unitFlags = *reinterpret_cast<const uint32_t *>(
        fields + Offsets::OFF_UNIT_FIELD_FLAGS);
    if ((unitFlags & Offsets::UNIT_FLAG_PLAYER_CONTROLLED) == 0)
        return 0;
    auto *info = *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_CGPLAYER_INFO);
    if (info == nullptr)
        return 0;
    return *reinterpret_cast<const uint32_t *>(
        info + Offsets::OFF_PLAYER_INFO_GUILD_KEY);
}

// Resolves an input string (unit token or literal name) to a
// canonical character name by calling Lua's `UnitName(input)` via
// `lua_pcall`. Falls back to the literal input on error (invalid
// token / unknown name) or empty result.
//
// We use `Script_UnitName` indirectly rather than reimplementing
// its logic because it correctly handles every case:
//   - `"player"` reads directly from the local player object
//     (skips NameCache since the cache doesn't pre-populate the
//     local player's entry).
//   - `"target"`, `"party*"`, `"raid*"` etc. resolve via the
//     engine's GUID-to-object lookup, then `CGUnit::GetName` which
//     hits NameCache with the engine's own callback wiring (the
//     gate `_GetRecord` uses to allow returning loaded entries).
//   - Invalid tokens raise a Lua error, caught here by `pcall`.
const char *ResolveInputToName(void *L, const char *input, char (&nameBuf)[64]) {
    const int topBefore = Game::Lua::GetTop(L);

    Game::Lua::PushString(L, "UnitName");
    Game::Lua::GetTable(L, Game::Lua::GLOBALS_INDEX);
    if (Game::Lua::Type(L, -1) != Game::Lua::TYPE_FUNCTION) {
        Game::Lua::SetTop(L, topBefore);
        return input;
    }
    Game::Lua::PushString(L, input);
    const int status = Game::Lua::PCall(L, 1, 1, 0);
    if (status != 0) {
        // Errored (e.g. "Unknown unit name"). Pop the error message.
        Game::Lua::SetTop(L, topBefore);
        return input;
    }
    if (Game::Lua::Type(L, -1) != Game::Lua::TYPE_STRING) {
        Game::Lua::SetTop(L, topBefore);
        return input;
    }
    const char *resolved = Game::Lua::ToString(L, -1);
    if (resolved == nullptr || *resolved == '\0') {
        Game::Lua::SetTop(L, topBefore);
        return input;
    }
    // Copy out — Lua's `tostring` pointer is only stable while the
    // value remains on the stack.
    std::snprintf(nameBuf, sizeof(nameBuf), "%s", resolved);
    Game::Lua::SetTop(L, topBefore);
    return nameBuf;
}

// `UnitIsInMyGuild(unitOrName)` — returns `1` if the unit/character
// shares the player's guild, `nil` otherwise. Accepts either a unit
// token (e.g. `"player"`, `"target"`, `"party1"`) or a literal
// character name (e.g. `"Bob"`), matching 3.3.5's
// `Script_UnitIsInMyGuild` (`0x0060C4B0`).
//
// Resolution strategy, in order:
//
// 1. Get the local player's guild key. If 0, the player isn't in any
//    guild — short-circuit to nil.
//
// 2. Fast path: if the input resolves to a `CGUnit *` (visible/loaded
//    unit) and that unit has a synced guild-key field, compare
//    directly. Works without a populated roster — `GetGuildInfo`
//    reads the same field. Covers the most common cases (player,
//    target, party/raid members in range, etc.).
//
// 3. Slow path: resolve the input to a canonical name (via
//    `FUN_TOKEN_TO_GUID` + NameCache, or use the literal input)
//    and walk the engine's guild roster array comparing names.
//    Handles OOR party/raid members and literal names. Requires
//    `GuildRoster()` to have been called.
//
// In 3.3.5 the engine resolves the input to a 64-bit GUID via
// `0x0060ABF0` (token-prefix dispatch with NameCache fallback) and
// then checks GUID membership via `0x00512A00`. 1.12's guild roster
// entries don't store GUIDs — they're name-keyed — so the slow path
// falls back to strcmp. Same end result in vanilla since character
// names are unique per-realm.
//
// Return convention matches 3.3.5: `1.0`/`nil`, not boolean —
// 3.3.5's `Script_UnitIsInMyGuild` pushes via `lua_pushnumber(1.0)`
// (`0x0060C530`) and `lua_pushnil` (`0x0060C515`), never
// `lua_pushboolean`.
//
// The slow path reads `VAR_GUILD_ROSTER_TOTAL_COUNT` (the same
// underlying global `GetNumGuildMembers(true)` returns), not the
// online-only count, so offline guildmates resolve too —
// the "show offline" UI toggle doesn't affect lookup.
int __fastcall Script_UnitIsInMyGuild(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitIsInMyGuild(\"unitOrName\")");
        return 0;
    }
    const char *input = Game::Lua::ToString(L, 1);
    if (input == nullptr || *input == '\0') {
        Game::Lua::PushNil(L);
        return 1;
    }

    auto resolve = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    const uint32_t playerKey = ReadGuildKey(
        static_cast<const uint8_t *>(resolve("player")));
    if (playerKey == 0) {
        // Player isn't in a guild — nobody is.
        Game::Lua::PushNil(L);
        return 1;
    }

    // Resolve input to a canonical name via `UnitName` (pcall'd to
    // catch the "Unknown unit name" error path for literal names).
    // This serves double duty: gives us a name for the roster walk
    // AND tells us whether the input is a valid token (success ⇒
    // token, error ⇒ literal name).
    char nameBuf[64];
    const char *nameToFind = ResolveInputToName(L, input, nameBuf);
    const bool inputIsToken = (nameToFind != input);

    if (inputIsToken) {
        // Input is a valid token — safe to call `resolve()`. Fast
        // path: direct guild-key comparison for visible/loaded units.
        auto *unit = static_cast<const uint8_t *>(resolve(input));
        if (unit != nullptr) {
            const uint32_t unitKey = ReadGuildKey(unit);
            if (unitKey == playerKey) {
                Game::Lua::PushNumber(L, 1.0);
                return 1;
            }
            if (unitKey != 0) {
                // Definitively a different guild.
                Game::Lua::PushNil(L);
                return 1;
            }
            // unitKey == 0: data not synced. Fall through to roster.
        }
    }

    // Roster walk by name (handles OOR party tokens via the resolved
    // name and literal names via the input itself).

    auto **roster = *reinterpret_cast<uint8_t ***>(
        Offsets::VAR_GUILD_ROSTER_PTR);
    const int total = *reinterpret_cast<const int *>(
        Offsets::VAR_GUILD_ROSTER_TOTAL_COUNT);
    if (roster == nullptr || total <= 0) {
        Game::Lua::PushNil(L);
        return 1;
    }
    for (int i = 0; i < total; ++i) {
        const uint8_t *entry = roster[i];
        if (entry == nullptr)
            continue;
        const char *name = reinterpret_cast<const char *>(
            entry + Offsets::OFF_GUILD_MEMBER_NAME);
        if (std::strcmp(name, nameToFind) == 0) {
            Game::Lua::PushNumber(L, 1.0);
            return 1;
        }
    }
    Game::Lua::PushNil(L);
    return 1;
}

// `UnitIsPossessed(unit)` — returns true if the unit is currently
// possessed (priest `Mind Control`, warlock `Subjugate Demon`). Reads
// `UNIT_FIELD_FLAGS` bit 24 (`UNIT_FLAG_POSSESSED = 0x01000000`)
// directly off the unit's m_objectFields descriptor. Same broadcast
// path as FEIGN_DEATH — works for any unit token.
int __fastcall Script_UnitIsPossessed(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitIsPossessed(\"unit\")");
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
        (unitFlags & Offsets::UNIT_FLAG_POSSESSED) != 0 ? 1 : 0);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("UnitIsAFK", &Script_UnitIsAFK);
    Game::Lua::RegisterGlobalFunction("UnitIsDND", &Script_UnitIsDND);
    Game::Lua::RegisterGlobalFunction("UnitIsFeignDeath", &Script_UnitIsFeignDeath);
    Game::Lua::RegisterGlobalFunction("UnitIsPossessed", &Script_UnitIsPossessed);
    Game::Lua::RegisterGlobalFunction("UnitIsInMyGuild", &Script_UnitIsInMyGuild);
}

} // namespace

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Unit::Flags
