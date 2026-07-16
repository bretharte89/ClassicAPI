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

#include <cstdint>

// `Map::Area` — shared `WorldMapArea.dbc` row resolution. A WorldMapArea
// row is the client's per-zone map descriptor (directory name + the
// world-coordinate rect used for world↔map transforms); every C_Map.*
// reader that keys off "which zone" resolves a row first. Both
// `Map::Overlays` (GetMapOverlays) and `Map::WorldSize` (GetMapWorldSize)
// go through here so the two ways of naming a zone — an AreaTable areaID
// argument, or the world map's current continent/zone view — live in one
// place.
namespace Map::Area {

// WorldMapArea row for an AreaTable `areaID` (the stable zone identity
// `C_Map.GetBestMapForUnit` returns). Linear walk of the ~175-row table.
// Returns -1 when no row carries that areaID.
int RowForAreaID(uint32_t areaID);

// WorldMapArea row for the world map's currently *viewed* zone — the same
// continent/zone selection the engine's map-name resolver (FUN_004a6cf0,
// behind GetMapInfo) performs: continent index → per-continent blob, zone
// index (-1 = whole continent) → that continent's zone-row array; no
// continent selected falls back to the default-row global. Returns -1 when
// the continent blob isn't resident.
int CurrentViewRow();

// Resolves world point (x, y) on continent `mapID` to the **tightest** zone
// whose WorldMapArea rect contains it, filling the zone's AreaTable areaID
// and a 0..100 zone-relative percent (`mapX` horizontal off world Y, `mapY`
// vertical off world X — the WoW/pfQuest convention). Returns false when no
// zone rect on that map encloses the point (open sea, degenerate rect, an
// instance with no zone row). Shared by GetAreaTriggerInfo and the taxi-node
// zone resolve.
bool ZonePercent(int mapID, float x, float y, int *outAreaID, double *outMapX,
                 double *outMapY);

// Projects world (x, y) on `mapID` to 0..1 CONTINENT-map coordinates using
// the continent's WorldMapArea row (the areaID == 0 row with a non-degenerate
// rect — some maps carry a stray zero-rect continent row that is skipped).
// `outPx` is horizontal (off world Y), `outPy` vertical (off world X).
// Returns false when the map has no usable continent row. This is the
// projection retail's TaxiNodeInfo.position uses.
bool ContinentPercent(int mapID, float x, float y, double *outPx, double *outPy);

} // namespace Map::Area
