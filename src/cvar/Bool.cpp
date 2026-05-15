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

namespace CVar::Bool {

namespace {

using ScriptFn_t = int(__fastcall *)(void *L);

// Case-insensitive "true" check for the string-form match. Bounded
// 4-char comparison + null terminator — same shape `BookTypeIsPet`
// uses in spell/Info.cpp.
bool IsTrueString(const char *s) {
    if (s == nullptr)
        return false;
    auto lc = [](char c) -> char {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
    };
    return lc(s[0]) == 't' && lc(s[1]) == 'r' && lc(s[2]) == 'u' &&
           lc(s[3]) == 'e' && s[4] == '\0';
}

// Coerces a cvar's string value to bool per modern semantics:
//   - empty / nil  → false
//   - "0"           → false
//   - "1" or any numeric non-zero → true
//   - "true" (case-insensitive) → true
//   - anything else (non-numeric, non-"true") → false
bool StringToBool(const char *s) {
    if (s == nullptr || s[0] == '\0')
        return false;
    if (IsTrueString(s))
        return true;
    // Parse leading integer; non-zero means true. atoi-style.
    bool negative = false;
    int idx = 0;
    if (s[idx] == '-') { negative = true; ++idx; }
    else if (s[idx] == '+') { ++idx; }
    if (s[idx] < '0' || s[idx] > '9')
        return false;
    int value = 0;
    while (s[idx] >= '0' && s[idx] <= '9') {
        value = value * 10 + (s[idx] - '0');
        ++idx;
    }
    (void)negative; // sign doesn't matter — non-zero is non-zero
    return value != 0;
}

// `C_CVar.GetCVarBool(cvar)` — dispatches the engine's own
// `Script_GetCVar` to read the string, then coerces. Returns 1
// (one value pushed) on any input — modern behavior is "false" for
// missing/unknown cvars rather than nil/error.
int __fastcall Script_C_CVar_GetCVarBool(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: C_CVar.GetCVarBool(\"cvar\")");
        return 0;
    }
    // Reuse the existing stack[1] (the cvar name). Script_GetCVar
    // reads its arg from stack[1] and pushes the string (or nil) on
    // top. Don't blow the stack first — the name is already there.
    auto getCVar = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_CVAR);
    getCVar(L);

    const char *value = nullptr;
    if (Game::Lua::Type(L, -1) == Game::Lua::TYPE_STRING)
        value = Game::Lua::ToString(L, -1);

    const bool result = StringToBool(value);
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushBoolean(L, result ? 1 : 0);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_CVar", "GetCVarBool",
                                      &Script_C_CVar_GetCVarBool);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace CVar::Bool
