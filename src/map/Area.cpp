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

} // namespace Map::Area
