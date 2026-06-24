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

#include "Game.h"
#include "Offsets.h"
#include "cache/QueryLoad.h"
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
    auto fn = reinterpret_cast<GetCacheRecord_t>(Offsets::FUN_CACHE_GET_RECORD);
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

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "GetCreatureInfoByID",
                                     &Script_GetCreatureInfoByID);
    Game::Lua::RegisterTableFunction("C_CreatureInfo", "RequestLoadCreatureByID",
                                     &Script_RequestLoadCreatureByID);
    Cache::QueryLoad::Register(reinterpret_cast<void *>(Offsets::VAR_CREATURE_CACHE),
                               kCreatureDataLoadResult);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Creature::Info
