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

namespace Quest::Log {

static int __fastcall Script_GetQuestIDFromLogIndex(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetQuestIDFromLogIndex(index)");
        return 0;
    }

    const int idx = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
    const int total = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_QUEST_LOG_ENTRY_COUNT));
    if (idx < 0 || idx >= total)
        return 0; // nil for out-of-range

    auto *entry = reinterpret_cast<uint8_t *>(
        static_cast<uintptr_t>(Offsets::VAR_QUEST_LOG_ENTRIES)) + idx * 16;

    // Field +8 is the header indicator: non-NULL = header, NULL = real quest.
    // Verified by Script_GetQuestLogTitle's isHeader push at 0x004DF9A9 (it pushes
    // 1.0 when [+8] != 0) and by the helper at 0x004DF150 used by IsUnitOnQuest
    // (returns the +0 questID only when [+8] == 0, NULL otherwise).
    if (*reinterpret_cast<void *const *>(entry + 8) != nullptr) {
        Game::Lua::PushNumber(L, 0.0);
        return 1;
    }

    const int questID = *reinterpret_cast<const int *>(entry + 0);
    Game::Lua::PushNumber(L, static_cast<double>(questID));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_QuestLog", "GetQuestIDFromLogIndex",
                                     &Script_GetQuestIDFromLogIndex);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Quest::Log
