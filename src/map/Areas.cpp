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

// `C_Map.GetAreaInfo(areaID)` and `C_Map.GetAreas()` — surface
// `AreaTable.dbc`'s localized name column to Lua. `AreaTable.dbc` is a
// client DBC (always loaded, synchronous, localized, ~1500 rows), so
// unlike creature/item names there's no WDB cold-cache problem — the name
// is always available for any id.
//
// - `GetAreaInfo(areaID)` is a retail backport: the localized name for one
//   area id (zone or subzone), matching retail's signature.
// - `GetAreas()` is a ClassicAPI extension (retail uses the uiMapID tree,
//   not raw AreaTable enumeration): the full id→name map, the load-bearing
//   piece for name databases that need name→id / the full id set (reverse
//   search, name resolution). Same "surface the hidden DBC" pattern as
//   `GetAreaTriggers` / `GetMapOverlays`.
//
// Names are the client's active locale (same source as `GetRealZoneText` /
// `Script_GetCharacterInfo`'s zone-name resolve), so they match what the
// game shows everywhere — no locale/rename drift vs a shipped table.

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"

#include <cstdint>

namespace Map::Areas {

namespace {

// `C_Map.GetAreaInfo(areaID)` — localized AreaTable name, or nil for
// non-numeric / non-positive input or an id with no row / empty name.
int __fastcall Script_GetAreaInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::PushNil(L);
        return 1;
    }
    const int areaID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const char *name =
        (areaID > 0) ? DBC::LocalizedField(Offsets::VAR_AREATABLE_RECORDS,
                                           Offsets::VAR_AREATABLE_COUNT,
                                           static_cast<uint32_t>(areaID),
                                           Offsets::OFF_AREATABLE_NAMES)
                     : nullptr;
    if (name == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::PushString(L, name);
    return 1;
}

// `C_Map.GetAreas()` — { [areaID] = name } for every row with a non-empty
// localized name. Empty-name rows (DBC::LocalizedField → nullptr) are
// omitted, so the map round-trips with GetAreaInfo for every key.
int __fastcall Script_GetAreas(void *L) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);

    const int count =
        *reinterpret_cast<const int *>(Offsets::VAR_AREATABLE_COUNT);
    for (int id = 1; id <= count; ++id) {
        const char *name = DBC::LocalizedField(Offsets::VAR_AREATABLE_RECORDS,
                                               Offsets::VAR_AREATABLE_COUNT,
                                               static_cast<uint32_t>(id),
                                               Offsets::OFF_AREATABLE_NAMES);
        if (name == nullptr)
            continue;
        Game::Lua::PushNumber(L, static_cast<double>(id));
        Game::Lua::PushString(L, name);
        Game::Lua::SetTable(L, -3);
    }
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Map", "GetAreaInfo", &Script_GetAreaInfo);
    Game::Lua::RegisterTableFunction("C_Map", "GetAreas", &Script_GetAreas);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Map::Areas
