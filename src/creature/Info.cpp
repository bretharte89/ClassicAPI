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

// `C_CreatureInfo.GetCreatureInfoByID(creatureID)` -> table | nil
//
// Reads the client-side creature cache (creaturecache.wdb, fed by
// SMSG_CREATURE_QUERY_RESPONSE) for a creature/NPC by ID and returns:
//
//   {
//     creatureID = <id>,
//     name       = "Misthoof Stag",   -- primary name
//     subName    = "",                 -- title/subtitle ("" if none)
//     type       = 1,                  -- CreatureType (1=Beast, 7=Humanoid, …)
//     family     = 0,                  -- CreatureFamily
//     rank       = 0,                  -- 0=normal,1=elite,2=rareelite,3=worldboss,4=rare
//     displayID  = 10957,              -- model display ID
//   }
//
// `GetCreatureInfoByID` is a synchronous *peek*: returns the data only
// if the creature is already cached (seen this session, or loaded from
// creaturecache.wdb at login); nil otherwise. `RequestLoadCreatureByID`
// fetches an uncached creature — it issues the query and fires
// `CREATURE_DATA_LOAD_RESULT(creatureID, success)` when the response
// lands, via the shared `Cache::QueryLoad` dispatcher.
//
// Field offsets verified against the binary (rank read by the engine's
// classification helper as `[block+0x20]`) and real creaturecache.wdb
// rows. See Offsets.h `OFF_CREATURE_*`.
//
// This file also hosts the two static-DBC lookups on the same table:
//
//   C_CreatureInfo.GetRaceInfo(raceID)   -> { raceName, clientFileString, raceID }
//   C_CreatureInfo.GetClassInfo(classID) -> { className, classFile, classID }
//
// retail backports over `ChrRaces.dbc` / `ChrClasses.dbc`. Unlike the
// creature cache these DBCs are always loaded, localized, and synchronous —
// so a by-id lookup works for any race/class id at any time, with no WDB
// cold-cache problem. The file-string columns (`clientFileString` /
// `classFile`) are the same non-localized tokens `UnitRace`/`UnitClass`
// return as their 2nd value ("NightElf", "WARRIOR"), so they round-trip
// with the unit APIs. See Offsets.h `OFF_CHRRACES_*` / `OFF_CHRCLASSES_*`.

#include "Game.h"
#include "Offsets.h"
#include "cache/QueryLoad.h"
#include "dbc/Lookup.h"
#include "event/Custom.h"

#include <cstdint>

namespace Creature::Info {

namespace {

// Completion event for RequestLoadCreatureByID. Reserved at static-init
// so the engine allocates it in its event table; fired (creatureID,
// success) by the Cache::QueryLoad dispatcher.
constexpr const char *kCreatureDataLoadResult = "CREATURE_DATA_LOAD_RESULT";
const Event::Custom::AutoReserve _reserveLoadResult{kCreatureDataLoadResult};

// Generic cache `_GetRecord`. With callback=nullptr it only peeks the
// hash table (no network query) and returns the data block (entry+0x18)
// if the creature is loaded, else nullptr.
using GetCacheRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t id,
                                                       const uint64_t *guid, void *callback,
                                                       void *userData, char dedup);

const uint8_t *PeekCreature(uint32_t creatureID) {
    auto fn = reinterpret_cast<GetCacheRecord_t>(Offsets::FUN_CREATURE_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_CREATURE_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, creatureID, &zeroGuid, nullptr, nullptr, 0);
}

int __fastcall Script_GetCreatureInfoByID(void *L) {
    if (!Game::Lua::IsNumber(L, 1))
        return 0; // nil
    const int creatureID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (creatureID <= 0)
        return 0;

    const uint8_t *rec = PeekCreature(static_cast<uint32_t>(creatureID));
    if (rec == nullptr)
        return 0; // not cached -> nil

    const char *name = *reinterpret_cast<const char *const *>(rec + Offsets::OFF_CREATURE_NAME);
    const char *subName =
        *reinterpret_cast<const char *const *>(rec + Offsets::OFF_CREATURE_SUBNAME);
    const uint32_t type = *reinterpret_cast<const uint32_t *>(rec + Offsets::OFF_CREATURE_TYPE);
    const uint32_t family = *reinterpret_cast<const uint32_t *>(rec + Offsets::OFF_CREATURE_FAMILY);
    const uint32_t rank = *reinterpret_cast<const uint32_t *>(rec + Offsets::OFF_CREATURE_RANK);
    const uint32_t displayID =
        *reinterpret_cast<const uint32_t *>(rec + Offsets::OFF_CREATURE_DISPLAYID);

    Game::Lua::NewTable(L);
    Game::Lua::SetFieldNumber(L, "creatureID", creatureID);
    Game::Lua::SetFieldString(L, "name", name);       // NULL coerced to ""
    Game::Lua::SetFieldString(L, "subName", subName);
    Game::Lua::SetFieldNumber(L, "type", type);
    Game::Lua::SetFieldNumber(L, "family", family);
    Game::Lua::SetFieldNumber(L, "rank", rank);
    Game::Lua::SetFieldNumber(L, "displayID", displayID);
    return 1;
}

// `C_CreatureInfo.RequestLoadCreatureByID(creatureID)` -> bool. Async
// fetch for an uncached creature: issues SMSG_CREATURE_QUERY and fires
// `CREATURE_DATA_LOAD_RESULT(creatureID, success)` when the response
// lands (or on timeout). Fires success synchronously if already cached.
// Returns true if the request was accepted (false for bad input or a
// full pending set) — mirrors C_Item.RequestLoadItemData(ByID).
int __fastcall Script_RequestLoadCreatureByID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    const int creatureID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (creatureID <= 0) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_CREATURE_CACHE);
    const bool accepted =
        Cache::QueryLoad::RequestLoad(cache, static_cast<uint32_t>(creatureID));
    Game::Lua::PushBool(L, accepted);
    return 1;
}

// `C_CreatureInfo.GetRaceInfo(raceID)` — RaceInfo table, or nil for a
// non-numeric / non-positive id or one with no ChrRaces row.
int __fastcall Script_GetRaceInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1))
        return 0; // nil
    const int raceID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const char *file =
        (raceID > 0) ? DBC::StringField(Offsets::VAR_CHRRACES_RECORDS,
                                        Offsets::VAR_CHRRACES_COUNT,
                                        static_cast<uint32_t>(raceID),
                                        Offsets::OFF_CHRRACES_FILENAME)
                     : nullptr;
    if (file == nullptr)
        return 0; // unknown id -> nil
    const char *name = DBC::LocalizedField(Offsets::VAR_CHRRACES_RECORDS,
                                           Offsets::VAR_CHRRACES_COUNT,
                                           static_cast<uint32_t>(raceID),
                                           Offsets::OFF_CHRRACES_NAMES);

    Game::Lua::NewTable(L);
    Game::Lua::SetFieldString(L, "raceName", name); // NULL coerced to ""
    Game::Lua::SetFieldString(L, "clientFileString", file);
    Game::Lua::SetFieldNumber(L, "raceID", raceID);
    return 1;
}

// `C_CreatureInfo.GetClassInfo(classID)` — ClassInfo table, or nil for a
// non-numeric / non-positive id or one with no ChrClasses row.
int __fastcall Script_GetClassInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1))
        return 0; // nil
    const int classID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const char *file =
        (classID > 0) ? DBC::StringField(Offsets::VAR_CHRCLASSES_RECORDS,
                                         Offsets::VAR_CHRCLASSES_COUNT,
                                         static_cast<uint32_t>(classID),
                                         Offsets::OFF_CHRCLASSES_FILENAME)
                      : nullptr;
    if (file == nullptr)
        return 0; // unknown id -> nil
    const char *name = DBC::LocalizedField(Offsets::VAR_CHRCLASSES_RECORDS,
                                           Offsets::VAR_CHRCLASSES_COUNT,
                                           static_cast<uint32_t>(classID),
                                           Offsets::OFF_CHRCLASSES_NAMES);

    Game::Lua::NewTable(L);
    Game::Lua::SetFieldString(L, "className", name); // NULL coerced to ""
    Game::Lua::SetFieldString(L, "classFile", file);
    Game::Lua::SetFieldNumber(L, "classID", classID);
    return 1;
}

// `C_CreatureInfo.GetCreatureFamilyInfo(creatureFamilyID)` — CreatureFamilyInfo
// table for a `CreatureFamily.dbc` row (1 = Wolf, 27 = Wind Serpent, …), or
// nil for a non-numeric / non-positive / unused id. `iconFile` is a texture
// path string here (retail exposes a numeric fileID; vanilla's DBC stores the
// path) — empty "" for families with no icon (e.g. warlock pets).
int __fastcall Script_GetCreatureFamilyInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1))
        return 0; // nil
    const int familyID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const char *name =
        (familyID > 0) ? DBC::LocalizedField(Offsets::VAR_CREATUREFAMILY_RECORDS,
                                             Offsets::VAR_CREATUREFAMILY_COUNT,
                                             static_cast<uint32_t>(familyID),
                                             Offsets::OFF_CREATUREFAMILY_NAMES)
                       : nullptr;
    if (name == nullptr)
        return 0; // unused / OOR id -> nil
    const char *icon = DBC::StringField(Offsets::VAR_CREATUREFAMILY_RECORDS,
                                        Offsets::VAR_CREATUREFAMILY_COUNT,
                                        static_cast<uint32_t>(familyID),
                                        Offsets::OFF_CREATUREFAMILY_ICON);

    Game::Lua::NewTable(L);
    Game::Lua::SetFieldNumber(L, "id", familyID);
    Game::Lua::SetFieldString(L, "name", name);
    Game::Lua::SetFieldString(L, "iconFile", icon); // NULL/empty coerced to ""
    return 1;
}

// `C_CreatureInfo.GetCreatureFamilyIDs()` — array of every populated
// `CreatureFamily.dbc` id (sparse: skips the unused rows). Round-trips with
// GetCreatureFamilyInfo for each element.
int __fastcall Script_GetCreatureFamilyIDs(void *L) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);

    const int count =
        *reinterpret_cast<const int *>(Offsets::VAR_CREATUREFAMILY_COUNT);
    int n = 0;
    for (int id = 1; id <= count; ++id) {
        const char *name = DBC::LocalizedField(Offsets::VAR_CREATUREFAMILY_RECORDS,
                                               Offsets::VAR_CREATUREFAMILY_COUNT,
                                               static_cast<uint32_t>(id),
                                               Offsets::OFF_CREATUREFAMILY_NAMES);
        if (name == nullptr)
            continue; // unused row
        ++n;
        Game::Lua::PushNumber(L, static_cast<double>(n));
        Game::Lua::PushNumber(L, static_cast<double>(id));
        Game::Lua::SetTable(L, -3);
    }
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "GetCreatureInfoByID",
                                     &Script_GetCreatureInfoByID);
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "RequestLoadCreatureByID",
                                     &Script_RequestLoadCreatureByID);
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "GetRaceInfo",
                                     &Script_GetRaceInfo);
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "GetClassInfo",
                                     &Script_GetClassInfo);
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "GetCreatureFamilyInfo",
                                     &Script_GetCreatureFamilyInfo);
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "GetCreatureFamilyIDs",
                                     &Script_GetCreatureFamilyIDs);
    Cache::QueryLoad::Register(reinterpret_cast<void *>(Offsets::VAR_CREATURE_CACHE),
                               kCreatureDataLoadResult, Offsets::FUN_CREATURE_GET_RECORD);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Creature::Info
