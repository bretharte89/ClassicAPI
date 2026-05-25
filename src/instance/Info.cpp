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

namespace Instance::Info {

namespace {

// Map.dbc instance-type enum, indexable from 0..4. Vanilla never uses
// slot 4 (arena), but the modern API string set includes it.
const char *const kTypeNames[] = {
    "none",   // 0 — open world
    "party",  // 1 — 5-man dungeon
    "raid",   // 2 — raid
    "pvp",    // 3 — battleground
    "arena",  // 4 — unused in vanilla
};

// Per-type player cap. Vanilla has no per-map cap stored client-side
// (MapDifficulty.dbc was a TBC addition), so we return the type's
// canonical max. ZG / AQ20 / WSG / AB return 40 instead of their true
// caps; same answer for any custom raid on private servers. Addons
// that need exact caps must supply their own table.
int CapForType(uint32_t type) {
    switch (type) {
        case 1: return 5;
        case 2: return 40;
        case 3: return 40;
        case 4: return 5;
        default: return 0;
    }
}

const uint8_t *MapRecord(uint32_t mapID) {
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_MAP_COUNT));
    if (static_cast<int>(mapID) < 0 || static_cast<int>(mapID) > count)
        return nullptr;
    const uint8_t *const *records =
        *reinterpret_cast<const uint8_t *const *const *>(
            static_cast<uintptr_t>(Offsets::VAR_MAP_RECORDS));
    if (records == nullptr)
        return nullptr;
    return records[mapID];
}

} // namespace

// `GetInstanceInfo()` — returns the modern 7-tuple shape, with vanilla
// degeneracies for fields the 1.12 client doesn't track:
//   name, type, difficulty, difficultyName, maxPlayers, playerDiff, isDynamic
//
// - `name` / `type` come from Map.dbc.
// - `difficulty` / `difficultyName` are always `1` / `"Normal"` (no
//   heroic mode pre-TBC; MapDifficulty.dbc doesn't exist).
// - `maxPlayers` is the type's canonical cap (see `CapForType`).
// - `playerDiff` mirrors `difficulty`.
// - `isDynamic` is always `false`.
static int __fastcall Script_GetInstanceInfo(void *L) {
    const uint32_t mapID = *reinterpret_cast<const uint32_t *>(
        static_cast<uintptr_t>(Offsets::VAR_CURRENT_MAP_ID));

    const uint8_t *rec = MapRecord(mapID);
    if (rec == nullptr) {
        Game::Lua::PushString(L, "");
        Game::Lua::PushString(L, "none");
        Game::Lua::PushNumber(L, 1.0);
        Game::Lua::PushString(L, "Normal");
        Game::Lua::PushNumber(L, 0.0);
        Game::Lua::PushNumber(L, 1.0);
        Game::Lua::PushBool(L, false);
        return 7;
    }

    const uint32_t type = *reinterpret_cast<const uint32_t *>(
        rec + Offsets::OFF_MAP_INSTANCE_TYPE);
    const uint32_t locale = *reinterpret_cast<const uint32_t *>(
        static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
    const char *name = *reinterpret_cast<const char *const *>(
        rec + Offsets::OFF_MAP_NAME + locale * 4);

    const char *typeName =
        (type < sizeof(kTypeNames) / sizeof(kTypeNames[0])) ? kTypeNames[type]
                                                            : "none";

    Game::Lua::PushString(L, name != nullptr ? name : "");
    Game::Lua::PushString(L, typeName);
    Game::Lua::PushNumber(L, 1.0);
    Game::Lua::PushString(L, "Normal");
    Game::Lua::PushNumber(L, static_cast<double>(CapForType(type)));
    Game::Lua::PushNumber(L, 1.0);
    Game::Lua::PushBool(L, false);
    return 7;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetInstanceInfo",
                                      &Script_GetInstanceInfo);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Instance::Info
