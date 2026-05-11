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

namespace Unit::Identity {

using TokenToGUID_t = uint64_t(__fastcall *)(const char *token);

// `UnitGUID(unit)` — returns the unit's 64-bit GUID formatted as a
// hex string `"0xHHHHHHHHLLLLLLLL"` (16 hex digits, hi dword first).
//
// 1.12 GUIDs are plain 64-bit integers — no type-prefix system like
// modern WoW's `"Player-1234-..."` / `"Creature-0-..."` shapes. Vanilla
// addons that tracked units across events used this `"0x..."` format,
// and 3.3.5's `Script_UnitGUID` (at `0x0060E630` in the Frostmourne
// client) emits the same shape after a much longer code path that
// special-cases sentinel GUID values introduced post-2.x.
//
// Returns `nil` if:
//   - the unit token resolves to a GUID of zero (e.g. `"target"`
//     when nothing's targeted; `"partyN"` when the slot is empty).
//
// Works for out-of-range party / raid members. The engine maintains
// a parallel GUID array populated by `SMSG_GROUP_LIST` that's
// independent of whether the unit's CGObject is currently active
// in the client. We call `FUN_TOKEN_TO_GUID` directly rather than
// going through `FUN_RESOLVE_UNIT_TOKEN` — the latter does an extra
// `FUN_00468460` step that returns NULL when the CGObject isn't
// live, which is why earlier versions of `UnitGUID` returned nil
// for `"party1"` when the member was on a different continent.
//
// Invalid tokens (arbitrary strings that aren't unit IDs) raise a
// Lua error via the engine's `"Unknown unit name: %s"` path — same
// as `Script_UnitName` and other engine unit-token consumers.
static int __fastcall Script_UnitGUID(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitGUID(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);
    if (token == nullptr)
        return 0;

    auto fn = reinterpret_cast<TokenToGUID_t>(Offsets::FUN_TOKEN_TO_GUID);
    const uint64_t guid = fn(token);
    if (guid == 0)
        return 0;

    const uint32_t lo = static_cast<uint32_t>(guid);
    const uint32_t hi = static_cast<uint32_t>(guid >> 32);

    char buf[24]; // "0x" + 16 hex digits + null = 19 bytes minimum
    std::snprintf(buf, sizeof(buf), "0x%08X%08X", hi, lo);
    Game::Lua::PushString(L, buf);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("UnitGUID", &Script_UnitGUID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Unit::Identity
