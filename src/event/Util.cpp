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
#include <cstring>

namespace Event::Util {

// Walks the same `EventEntry` array that `Frame::RegisterEvent` strcmps
// against (base at `[VAR_EVENT_TABLE_BASE_PTR]`, count at
// `[VAR_EVENT_TABLE_COUNT]`, stride 0x10, name field at +0x00). This is
// the source of truth for "events the engine will accept registration
// for", so it picks up anything we've injected via `Event::Custom::Register`
// in addition to all built-in events.
static bool IsKnownEventName(const char *needle) {
    auto *base = *reinterpret_cast<uint8_t **>(Offsets::VAR_EVENT_TABLE_BASE_PTR);
    const int count = *reinterpret_cast<int *>(Offsets::VAR_EVENT_TABLE_COUNT);
    if (base == nullptr || count <= 0)
        return false;
    for (int i = 0; i < count; i++) {
        const uint8_t *entry = base + i * Offsets::EVENT_ENTRY_STRIDE;
        const char *name = *reinterpret_cast<const char *const *>(
            entry + Offsets::OFF_EVENT_ENTRY_NAME);
        if (name == nullptr)
            continue;
        if (std::strcmp(name, needle) == 0)
            return true;
    }
    return false;
}

static int __fastcall Script_IsEventValid(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_STRING) {
        Game::Lua::Error(L, "Usage: C_EventUtils.IsEventValid(eventName)");
        return 0;
    }
    const char *needle = Game::Lua::ToString(L, 1);
    if (needle == nullptr || needle[0] == 0) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    Game::Lua::PushBoolean(L, IsKnownEventName(needle) ? 1 : 0);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_EventUtils", "IsEventValid", &Script_IsEventValid);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Event::Util
