// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

// `IsLoggedIn()` — exposed on both Lua states (glue and in-game).
// Returns `true` once a character has been loaded *and identified*
// (GUID populated), `false` on glue, pre-load, and the brief window
// between struct allocation and character-data unpack.
//
// Check is two-step:
//   1. `[VAR_LOCAL_PLAYER_PTR]` non-NULL — the player aggregator
//      struct has been allocated
//   2. `*(uint64*)(player + OFF_LOCAL_PLAYER_GUID)` non-zero — the
//      character's GUID has been populated into the struct
//
// The pointer alone is too lax: the engine allocates the struct
// shortly before the character data lands, so there's a window where
// the pointer is non-NULL but the GUID is still zero. Most addons
// that check `IsLoggedIn` go straight to `UnitGUID("player")` next,
// and we don't want them to see GUID==nil right after IsLoggedIn
// reported true.
//
// 3.3.5 uses a single byte flag at `[0x00BD0793]` cleared on world
// entry — we couldn't pin down the analogous 1.12 flag (the engine
// path that signals event 253 wires through CVar-init machinery and
// doesn't map cleanly across the binary). The pointer+GUID check is
// the same effective signal: both flip together at the moment the
// player object is ready, and stay stable across zone transitions.

#include "Game.h"
#include "unit/Identity.h"

#include <cstdint>

namespace Player::LoginStatus {

namespace {

int __fastcall Script_IsLoggedIn(void *L) {
    Game::Lua::PushBool(L, Unit::Identity::PlayerGuid() != 0);
    return 1;
}

void RegisterInGame() {
    Game::Lua::RegisterGlobalFunction("IsLoggedIn", &Script_IsLoggedIn);
}

void RegisterGlue() {
    Game::Lua::RegisterGlueFunction("IsLoggedIn", &Script_IsLoggedIn);
}

} // namespace

static const Game::ModuleAutoRegister _autoreg{&RegisterInGame};
static const Game::GlueModuleAutoRegister _gluereg{&RegisterGlue};

} // namespace Player::LoginStatus
