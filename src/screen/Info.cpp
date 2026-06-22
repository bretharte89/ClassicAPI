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

// `GetPhysicalScreenSize()` → (widthPixels, heightPixels) — the
// display's physical resolution in pixels. Modern WoW added this as a
// native in 7.0 (Legion); FrameXML's `PixelUtil` hangs its entire
// pixel↔UI-unit conversion off it (`768.0 / physicalHeight`), which is
// what lets addons snap frames to whole pixels.
//
// Vanilla 1.12 has no such native and no pre-parsed pixel-dimension
// global — the engine only ever holds the current mode as the
// `gxResolution` CVar string ("WIDTHxHEIGHT"). `Script_GetScreenWidth`
// / `GetScreenHeight` return UI-space units (derived from the UIParent
// aspect ratio at `[UIParent + 0x7c]`), not pixels, so they can't
// serve. `SetScreenResolution` (`FUN_0048bfd0`) and the display-init
// path both sscanf the `gxResolution` value on demand rather than
// caching width/height anywhere — so the CVar string is the
// authoritative current-resolution source, and we parse it the same
// way the engine does.
//
// Returns 1024x768 (→ factor 1.0, i.e. 1 pixel == 1 UI unit, making
// PixelUtil a no-op pass-through) when the CVar is missing or
// unparseable. Caveat: in windowed mode `gxResolution` reflects the
// configured render resolution, which may differ from the OS window's
// client size — vanilla exposes no separate window-pixel query.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Screen::Info {

namespace {

using FindCVar_t = const uint8_t *(__fastcall *)(const char *name);

// Parses "WIDTHxHEIGHT" (e.g. "1920x1080"). Tolerant of the separator
// (any run of non-digits between the two numbers) so an uppercase 'X'
// or stray spaces don't break it. Returns false unless both numbers
// are present and positive.
bool ParseResolution(const char *s, int *outW, int *outH) {
    if (s == nullptr)
        return false;

    const char *p = s;
    int w = 0;
    bool gotW = false;
    while (*p >= '0' && *p <= '9') {
        w = w * 10 + (*p - '0');
        ++p;
        gotW = true;
    }
    if (!gotW)
        return false;

    while (*p != '\0' && (*p < '0' || *p > '9')) // skip the 'x' separator
        ++p;

    int h = 0;
    bool gotH = false;
    while (*p >= '0' && *p <= '9') {
        h = h * 10 + (*p - '0');
        ++p;
        gotH = true;
    }
    if (!gotH || w <= 0 || h <= 0)
        return false;

    *outW = w;
    *outH = h;
    return true;
}

int __fastcall Script_GetPhysicalScreenSize(void *L) {
    int width = 1024;
    int height = 768; // fallback → 768/768 = factor 1.0 (no-op snapping)

    auto findCVar = reinterpret_cast<FindCVar_t>(Offsets::FUN_FIND_CVAR);
    const uint8_t *cvar = findCVar("gxResolution");
    if (cvar != nullptr) {
        const char *value = *reinterpret_cast<const char *const *>(
            cvar + Offsets::OFF_CVAR_VALUE_STR);
        ParseResolution(value, &width, &height);
    }

    Game::Lua::PushNumber(L, width);
    Game::Lua::PushNumber(L, height);
    return 2;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetPhysicalScreenSize",
                                      &Script_GetPhysicalScreenSize);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Screen::Info
