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

#include "turtle/Detect.h"

#include "Game.h"

#include <cstring>

namespace Turtle {

namespace {

// Latched once we've read the global as a string — Turtle never un-sets
// it, so a true result is permanent. A false read is NOT cached: FrameXML
// may not have run yet, so we re-probe on each call until it lands.
bool g_detected = false;
char g_version[32] = {0};

void Probe() {
    if (g_detected)
        return;
    void *L = Game::Lua::State();
    if (L == nullptr)
        return;

    // Read `_G["TURTLE_WOW_VERSION"]` without disturbing the caller's
    // stack. RawGet avoids any (nonexistent, but free-to-skip) metatable
    // side effects on the globals table.
    const int top = Game::Lua::GetTop(L);
    Game::Lua::PushString(L, "TURTLE_WOW_VERSION");
    Game::Lua::RawGet(L, Game::Lua::GLOBALS_INDEX);
    if (Game::Lua::IsString(L, -1)) {
        const char *v = Game::Lua::ToString(L, -1);
        if (v != nullptr) {
            const size_t n = std::strlen(v);
            const size_t copy =
                (n < sizeof(g_version) - 1) ? n : sizeof(g_version) - 1;
            std::memcpy(g_version, v, copy);
            g_version[copy] = '\0';
        }
        g_detected = true;
    }
    Game::Lua::SetTop(L, top);
}

} // namespace

bool Detected() {
    Probe();
    return g_detected;
}

const char *Version() {
    Probe();
    return g_detected ? g_version : nullptr;
}

} // namespace Turtle
