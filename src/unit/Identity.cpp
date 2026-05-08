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

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

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
//   - the unit token is invalid / not currently resolvable
//   - the resolved unit's GUID slot is NULL or both halves are zero
//     (matches 3.3.5's all-zero → nil at `0x0060E68C`).
static int __fastcall Script_UnitGUID(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitGUID(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);
    if (token == nullptr)
        return 0;

    auto resolve = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *unit = static_cast<const uint8_t *>(resolve(token));
    if (unit == nullptr)
        return 0;

    auto *guidPtr = *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_UNIT_GUID_PTR);
    if (guidPtr == nullptr)
        return 0;

    const uint32_t lo = *reinterpret_cast<const uint32_t *>(guidPtr + 0);
    const uint32_t hi = *reinterpret_cast<const uint32_t *>(guidPtr + 4);
    if (lo == 0 && hi == 0)
        return 0;

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
