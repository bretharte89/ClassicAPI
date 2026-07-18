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

// Lua 5.1 math-library additions that 1.12's Lua 5.0 is missing:
//
//   - `math.fmod(x, y)` — floating-point remainder of `x / y`.
//
// This is a pure rename: Lua 5.0 ships the exact same C function under the
// name `math.mod`; 5.1 renamed the Lua-facing entry to `math.fmod`. We
// register `math.fmod` as a direct alias of the engine's `math.mod` C
// function (`FUN_LUA_MATH_FMOD`) — no wrapper. `math.mod` stays available too
// (we don't touch it), so code written for either name works.

#include "Game.h"
#include "Offsets.h"

namespace BaseLib::MathLib {

namespace {

// `math.fmod` is 5.0's `math.mod` verbatim — register the engine function
// pointer directly rather than wrapping it.
const auto Script_math_fmod =
    reinterpret_cast<Game::Lua::CFunction>(Offsets::FUN_LUA_MATH_FMOD);

void RegisterFns() {
    Game::Lua::RegisterTableFunction("math", "fmod", Script_math_fmod);
}

// Both states are Lua 5.0 and equally missing this; RegisterTableFunction
// targets whichever state is active in each registration hook.
const Game::ModuleAutoRegister _autoreg{&RegisterFns};
const Game::GlueModuleAutoRegister _autoregGlue{&RegisterFns};

} // namespace

} // namespace BaseLib::MathLib
