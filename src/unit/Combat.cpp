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

namespace Unit::Combat {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

// `InCombatLockdown()` — returns whether the local player is currently
// in combat. Modern WoW gates secure-frame UI manipulation on this; 1.12
// has no secure-frame system, so the function reduces to "is the player
// in combat" — `UnitAffectingCombat("player")` would compute the same
// answer, just via the slower string→token→unit→fields path. We read
// the flag directly off the player CGUnit's m_objectFields.
//
// The bit (`UNIT_FLAG_IN_COMBAT = 0x00080000`) is the same one
// `Script_UnitAffectingCombat` at `0x00517E4A`-`0x517E5C` tests.
static int __fastcall Script_InCombatLockdown(void *L) {
    auto resolve = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<const uint8_t *>(resolve("player"));
    if (player == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    auto *fields = *reinterpret_cast<const uint8_t *const *>(
        player + Offsets::OFF_UNIT_DESCRIPTOR);
    if (fields == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        fields + Offsets::OFF_UNIT_FIELD_FLAGS);
    Game::Lua::PushBoolean(L, (flags & Offsets::UNIT_FLAG_IN_COMBAT) != 0 ? 1 : 0);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("InCombatLockdown", &Script_InCombatLockdown);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Unit::Combat
