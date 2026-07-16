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

// `C_MapExplorationInfo.GetExploredMapTextures(areaID)` and
// `GetUnexploredMapTextures(areaID)` — the exploration-filtered views of a
// zone's `WorldMapOverlay.dbc` entries.
//
// - `GetExploredMapTextures` is the retail-named backport: only the overlays
//   the local player has **discovered**. Retail keys on `uiMapID` and returns
//   numeric `fileDataIDs`; here it's an `AreaTable` areaID and the tiles are
//   texture paths (`fileDataIDs` holds those paths, see Overlays.cpp).
// - `GetUnexploredMapTextures` is the ClassicAPI-extension complement: the
//   overlays NOT yet discovered — what a map-reveal addon draws over the
//   fogged base map. No retail equivalent (retail only ships the explored
//   getter); its inverse, `C_Map.GetMapOverlays`, returns *all* overlays.
//
// All three share `Map::Overlays::PushZoneOverlays` — same per-overlay table
// shape (texture, resolved tiles, hit rect, areaIDs, fileDataIDs); these two
// just pass an exploration Filter. "Explored" is computed per-areaID from the
// player's explored-areas bitfield (see Overlays.cpp `IsOverlayExplored`), so
// the areaID argument works for any zone, not just the one on screen.
//
// With no argument, both default to the world map's currently viewed zone
// (like `GetMapOverlays()`).

#include "Game.h"
#include "map/Area.h"
#include "map/Overlays.h"

#include <cstdint>

namespace Map::Exploration {

namespace {

int ResolveRow(void *L) {
    return Game::Lua::IsNumber(L, 1)
               ? Map::Area::RowForAreaID(
                     static_cast<uint32_t>(Game::Lua::ToNumber(L, 1)))
               : Map::Area::CurrentViewRow();
}

int __fastcall Script_GetExploredMapTextures(void *L) {
    const int row = ResolveRow(L);
    Game::Lua::SetTop(L, 0);
    Map::Overlays::PushZoneOverlays(L, row, Map::Overlays::Filter::Explored);
    return 1;
}

int __fastcall Script_GetUnexploredMapTextures(void *L) {
    const int row = ResolveRow(L);
    Game::Lua::SetTop(L, 0);
    Map::Overlays::PushZoneOverlays(L, row, Map::Overlays::Filter::Unexplored);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_MapExplorationInfo",
                                     "GetExploredMapTextures",
                                     &Script_GetExploredMapTextures);
    Game::Lua::RegisterTableFunction("C_MapExplorationInfo",
                                     "GetUnexploredMapTextures",
                                     &Script_GetUnexploredMapTextures);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Map::Exploration
