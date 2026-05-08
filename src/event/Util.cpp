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

#include "Util.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstring>

namespace Event::Util {

static bool IsKnownEventName(const char *needle) {
    const char *const *table =
        reinterpret_cast<const char *const *>(Offsets::VAR_EVENT_NAME_TABLE);
    for (int i = 0; i < Offsets::EVENT_NAME_TABLE_MAX_SLOTS; i++) {
        const char *slot = table[i];
        const auto addr = reinterpret_cast<uintptr_t>(slot);
        if (addr < Offsets::EVENT_TABLE_VALID_PTR_LO ||
            addr >= Offsets::EVENT_TABLE_VALID_PTR_HI)
            continue;
        if (std::strcmp(slot, needle) == 0)
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

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_EventUtils", "IsEventValid", &Script_IsEventValid);
}

} // namespace Event::Util
