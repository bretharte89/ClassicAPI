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

#include "map/Area.h"

#include "Offsets.h"
#include "dbc/Lookup.h"

namespace Map::Area {

int RowForAreaID(uint32_t areaID) {
    const int count =
        *reinterpret_cast<const int *>(Offsets::VAR_WORLDMAP_AREA_COUNT);
    for (int id = 1; id <= count; ++id) {
        const uint8_t *rec = DBC::Record(Offsets::VAR_WORLDMAP_AREA_RECORDS,
                                         Offsets::VAR_WORLDMAP_AREA_COUNT,
                                         static_cast<uint32_t>(id));
        if (rec != nullptr &&
            *reinterpret_cast<const uint32_t *>(rec + Offsets::OFF_WMA_AREA_ID) ==
                areaID)
            return id;
    }
    return -1;
}

int CurrentViewRow() {
    const int continent =
        *reinterpret_cast<const int *>(Offsets::VAR_WORLDMAP_CONTINENT_INDEX);
    if (continent < 0)
        return *reinterpret_cast<const int *>(
            Offsets::VAR_WORLDMAP_DEFAULT_AREA_ROW);
    const uint8_t *blob = *reinterpret_cast<const uint8_t *const *>(
        Offsets::VAR_WORLDMAP_CONTINENT_DATA);
    if (blob == nullptr)
        return -1;
    const uint8_t *entry = blob + continent * Offsets::WORLDMAP_CONTINENT_STRIDE;
    const int zone =
        *reinterpret_cast<const int *>(Offsets::VAR_WORLDMAP_ZONE_INDEX);
    if (zone < 0)
        return *reinterpret_cast<const int *>(entry + 0x04);
    const int *zoneRows = *reinterpret_cast<const int *const *>(entry + 0x10);
    return (zoneRows != nullptr) ? zoneRows[zone] : -1;
}

namespace {
double FloatField(const uint8_t *rec, int off) {
    return *reinterpret_cast<const float *>(rec + off);
}
} // namespace

bool ZonePercent(int mapID, float x, float y, int *outAreaID, double *outMapX,
                 double *outMapY) {
    const int count =
        *reinterpret_cast<const int *>(Offsets::VAR_WORLDMAP_AREA_COUNT);
    bool found = false;
    double bestArea = 0.0;
    for (int id = 1; id <= count; ++id) {
        const uint8_t *rec = DBC::Record(Offsets::VAR_WORLDMAP_AREA_RECORDS,
                                         Offsets::VAR_WORLDMAP_AREA_COUNT,
                                         static_cast<uint32_t>(id));
        if (rec == nullptr)
            continue;
        if (*reinterpret_cast<const int *>(rec + Offsets::OFF_WMA_MAP_ID) != mapID)
            continue;
        const int areaID =
            *reinterpret_cast<const int *>(rec + Offsets::OFF_WMA_AREA_ID);
        if (areaID == 0)
            continue; // continent-spanning row, not a zone

        const double left = FloatField(rec, Offsets::OFF_WMA_LOC_LEFT);
        const double right = FloatField(rec, Offsets::OFF_WMA_LOC_RIGHT);
        const double top = FloatField(rec, Offsets::OFF_WMA_LOC_TOP);
        const double bottom = FloatField(rec, Offsets::OFF_WMA_LOC_BOTTOM);
        const double spanX = top - bottom; // > 0
        const double spanY = left - right; // > 0
        if (spanX <= 0.0 || spanY <= 0.0)
            continue; // degenerate / unmapped zone

        if (x < bottom || x > top || y < right || y > left)
            continue; // point outside this zone's rect

        const double area = spanX * spanY;
        if (found && area >= bestArea)
            continue; // a tighter zone already wins

        found = true;
        bestArea = area;
        *outAreaID = areaID;
        // mapX = horizontal (world Y), mapY = vertical (world X).
        *outMapX = (left - y) / spanY * 100.0;
        *outMapY = (top - x) / spanX * 100.0;
    }
    return found;
}

bool ContinentPercent(int mapID, float x, float y, double *outPx, double *outPy) {
    const int count =
        *reinterpret_cast<const int *>(Offsets::VAR_WORLDMAP_AREA_COUNT);
    for (int id = 1; id <= count; ++id) {
        const uint8_t *rec = DBC::Record(Offsets::VAR_WORLDMAP_AREA_RECORDS,
                                         Offsets::VAR_WORLDMAP_AREA_COUNT,
                                         static_cast<uint32_t>(id));
        if (rec == nullptr)
            continue;
        if (*reinterpret_cast<const int *>(rec + Offsets::OFF_WMA_MAP_ID) != mapID)
            continue;
        if (*reinterpret_cast<const int *>(rec + Offsets::OFF_WMA_AREA_ID) != 0)
            continue; // want the continent-spanning row

        const double left = FloatField(rec, Offsets::OFF_WMA_LOC_LEFT);
        const double right = FloatField(rec, Offsets::OFF_WMA_LOC_RIGHT);
        const double top = FloatField(rec, Offsets::OFF_WMA_LOC_TOP);
        const double bottom = FloatField(rec, Offsets::OFF_WMA_LOC_BOTTOM);
        const double spanX = top - bottom;
        const double spanY = left - right;
        if (spanX <= 0.0 || spanY <= 0.0)
            continue; // stray zero-rect continent row (e.g. "World") — skip

        *outPx = (left - y) / spanY; // horizontal (world Y)
        *outPy = (top - x) / spanX;  // vertical (world X)
        return true;
    }
    return false;
}

} // namespace Map::Area
