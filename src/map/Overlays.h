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

#pragma once

namespace Map::Overlays {

// Which overlays of a zone to include, by the local player's exploration.
enum class Filter {
    All,        // every WorldMapOverlay.dbc row for the zone (C_Map.GetMapOverlays)
    Explored,   // only overlays the player has discovered
    Unexplored, // only overlays the player has NOT discovered
};

// Pushes an array of overlay tables for `wmaRow` (a WorldMapArea.dbc row),
// filtered by exploration, onto the Lua stack as a single value. Shared by
// `C_Map.GetMapOverlays` (Filter::All) and `C_MapExplorationInfo`'s
// explored / unexplored texture getters. The caller resolves the row (via
// `Map::Area`) and clears the stack first; this leaves exactly one table.
// The per-overlay table shape is documented in Overlays.cpp's header.
//
// Explored/Unexplored read the player's explored-areas bitfield; before the
// player is resident (character select) every overlay reads as unexplored.
void PushZoneOverlays(void *L, int wmaRow, Filter filter);

} // namespace Map::Overlays
