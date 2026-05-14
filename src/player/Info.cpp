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

// `GetPlayerInfoByGUID(guid)` — modern signature returning
// `localizedClass, englishClass, localizedRace, englishRace, sex,
//  name, realm` for any player GUID the engine has cached. Vanilla
// 1.12 has the cache (populated by SMSG_NAME_QUERY_RESPONSE) but
// doesn't expose it to Lua.
//
// Cache lookup path: call `FUN_0055F080` on the cache instance at
// `VAR_PLAYER_NAME_CACHE` (0x00C0E228) with a NULL callback. The
// engine returns a pointer to the entry's data block if the GUID is
// loaded; NULL otherwise (miss, or pending request from prior
// engine-initiated query). We never call with a non-NULL callback
// from this read path — that would trigger CMSG_NAME_QUERY (opcode
// 0x50), which is the right behavior for an explicit
// `RequestLoadPlayerInfo` API but not for a passive getter that may
// be called many times per frame from tooltips/raid frames.
//
// Cache coverage: the engine populates entries for any GUID it
// receives a SMSG_NAME_QUERY_RESPONSE for, which is triggered by
// chat (whispers, says, etc.), raid/group events, guild updates,
// and visible-object resolution. So a friend you've never seen in
// chat won't be in the cache; once they whisper you (or you target
// them, or they appear in guild login spam), they will be.

#include "Game.h"
#include "NameCache.h"
#include "Offsets.h"
#include "dbc/Lookup.h"
#include "guid/Guid.h"

#include <cstdint>

namespace Player::Info {

namespace {

// `FUN_0055F080` — `__thiscall(this=cache, guidLo, guidHi, &cookie,
// callback, userData, retryFlag) → entryData* or NULL`.
using LookupOrFetch_t = const uint8_t *(__thiscall *)(
    void *cache,
    uint32_t guidLo,
    uint32_t guidHi,
    uint64_t *cookie,
    void *callback,
    void *userData,
    int retryFlag);

// Thin shim: parses a GUID string and splits to hi/lo dwords for the
// engine's `__thiscall` cache lookup, which takes them as separate
// args. Shared parser lives in `Guid::Parse`.
bool ParseGUID(const char *str, uint32_t &outHi, uint32_t &outLo) {
    uint64_t value = 0;
    if (!Guid::Parse(str, &value))
        return false;
    outHi = static_cast<uint32_t>(value >> 32);
    outLo = static_cast<uint32_t>(value);
    return true;
}

int __fastcall Script_GetPlayerInfoByGUID(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: GetPlayerInfoByGUID(\"0x...\")");
        return 0;
    }
    const char *guidStr = Game::Lua::ToString(L, 1);
    uint32_t hi, lo;
    if (!ParseGUID(guidStr, hi, lo))
        return 0;
    if (hi == 0 && lo == 0)
        return 0;

    auto fn = reinterpret_cast<LookupOrFetch_t>(Offsets::FUN_PLAYER_INFO_LOOKUP);
    auto *cache = reinterpret_cast<void *>(
        static_cast<uintptr_t>(Offsets::VAR_PLAYER_NAME_CACHE));

    uint64_t cookie = 0;
    const uint8_t *entry = fn(cache, lo, hi, &cookie,
                              nullptr /*callback*/, nullptr /*userData*/,
                              0 /*retryFlag*/);

    const char *name = nullptr;
    const char *realm = nullptr;
    uint32_t race = 0;
    uint32_t sex = 0;
    uint32_t classID = 0;

    if (entry != nullptr) {
        name = reinterpret_cast<const char *>(
            entry + Offsets::OFF_PLAYER_INFO_NAME);
        realm = reinterpret_cast<const char *>(
            entry + Offsets::OFF_PLAYER_INFO_REALM);
        race = *reinterpret_cast<const uint32_t *>(
            entry + Offsets::OFF_PLAYER_INFO_RACE);
        sex = *reinterpret_cast<const uint32_t *>(
            entry + Offsets::OFF_PLAYER_INFO_SEX);
        classID = *reinterpret_cast<const uint32_t *>(
            entry + Offsets::OFF_PLAYER_INFO_CLASS);
    } else {
        // Engine miss — fall back to the persistent name cache (if
        // enabled). Name + class + race + sex round-trip from disk;
        // realm comes back "" since vanilla is single-realm and we
        // never see the realm name as a per-player attribute.
        const NameCache::Entry *cached = NameCache::Lookup(
            (static_cast<uint64_t>(hi) << 32) | lo);
        if (cached == nullptr || cached->name.empty())
            return 0;
        name = cached->name.c_str();
        classID = cached->classID;
        race = cached->raceID;
        sex = cached->sex;
        realm = "";
    }

    const char *englishClass = DBC::StringField(
        Offsets::VAR_CHRCLASSES_RECORDS, Offsets::VAR_CHRCLASSES_COUNT,
        classID, Offsets::OFF_CHRCLASSES_FILENAME);
    const char *localizedClass = DBC::LocalizedField(
        Offsets::VAR_CHRCLASSES_RECORDS, Offsets::VAR_CHRCLASSES_COUNT,
        classID, Offsets::OFF_CHRCLASSES_NAMES);
    const char *englishRace = DBC::StringField(
        Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
        race, Offsets::OFF_CHRRACES_FILENAME);
    const char *localizedRace = DBC::LocalizedField(
        Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
        race, Offsets::OFF_CHRRACES_NAMES);

    Game::Lua::PushString(L, localizedClass ? localizedClass : "");
    Game::Lua::PushString(L, englishClass ? englishClass : "");
    Game::Lua::PushString(L, localizedRace ? localizedRace : "");
    Game::Lua::PushString(L, englishRace ? englishRace : "");
    // Match modern UnitSex convention: 2 = male, 3 = female. The
    // wire format and cache use 0/1 (per CMaNGOS GENDER_MALE/
    // GENDER_FEMALE); modern WoW remaps for the Lua surface.
    Game::Lua::PushNumber(L, static_cast<double>(sex + 2));
    Game::Lua::PushString(L, name);
    Game::Lua::PushString(L, realm);
    return 7;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetPlayerInfoByGUID",
                                       &Script_GetPlayerInfoByGUID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Player::Info
