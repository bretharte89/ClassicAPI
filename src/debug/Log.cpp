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

#include "Log.h"

#include "Game.h"

#include <cstdio>

namespace Debug::Log {

namespace {

// Hardcoded path. Living in the source repo means agents/tooling can
// read it via the standard Read tool without having to know where
// WoW.exe is installed. The file is gitignored. If you move this
// project, update this constant — there's no fallback path search,
// keeping it dumb means a missing/wrong path fails loudly rather
// than silently writing somewhere unexpected.
constexpr const char *PATH = "C:\\Git\\ClassicAPI\\debug.log";

} // namespace

void Clear() {
    if (FILE *f = std::fopen(PATH, "w")) {
        std::fclose(f);
    }
}

void Append(const char *label, const char *content) {
    FILE *f = std::fopen(PATH, "a");
    if (f == nullptr)
        return;
    std::fprintf(f, "=== %s ===\n", label != nullptr ? label : "(no label)");
    std::fprintf(f, "%s\n\n", content != nullptr ? content : "");
    std::fclose(f);
}

namespace {

int __fastcall Script_LogClear(void *) {
    Clear();
    return 0;
}

int __fastcall Script_LogAppend(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: _classicapi_LogAppend(label, content)");
        return 0;
    }
    const char *label = Game::Lua::ToString(L, 1);
    const char *content = Game::Lua::IsString(L, 2)
                              ? Game::Lua::ToString(L, 2)
                              : "";
    Append(label, content);
    return 0;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("_classicapi_LogClear", &Script_LogClear);
    Game::Lua::RegisterGlobalFunction("_classicapi_LogAppend", &Script_LogAppend);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Debug::Log
