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
#include "Offsets.h"

#include <cstdint>
#include <cstring>

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

// Reads a localized-or-filename string field out of a DBC record.
// `recordsVar`, `countVar` are the engine globals holding the
// records pointer / count for the DBC. `offset` is one of:
//   - the filename field (single string ptr) — pass
//     `indexByLocale=false`
//   - the names array (9 locale string ptrs, indexed by
//     [VAR_LOCALE_INDEX]) — pass `indexByLocale=true`
// Returns nullptr if `id` is out of range, the slot is empty, or
// the field is null/empty.
const char *DBCStringField(uintptr_t recordsVar, uintptr_t countVar,
                           uint32_t id, int offset, bool indexByLocale) {
    if (id == 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(countVar);
    if (static_cast<int>(id) > count)
        return nullptr;
    const uint8_t *const *records =
        *reinterpret_cast<const uint8_t *const *const *>(recordsVar);
    if (records == nullptr)
        return nullptr;
    const uint8_t *record = records[id];
    if (record == nullptr)
        return nullptr;
    const char *result;
    if (indexByLocale) {
        const int locale = *reinterpret_cast<const int *>(
            static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
        const char *const *names = reinterpret_cast<const char *const *>(record + offset);
        result = names[locale];
    } else {
        result = *reinterpret_cast<const char *const *>(record + offset);
    }
    return (result == nullptr || *result == '\0') ? nullptr : result;
}

// Parses a `"0xHHHHHHHHLLLLLLLL"` GUID string into hi/lo dword
// pair. Matches the format `UnitGUID(unit)` returns. Returns true
// on success. Whitespace is not tolerated; 16 hex digits must
// follow `0x` / `0X`. Shorter `"0xLLLLLLLL"` (8 digits, hi=0) is
// also accepted because vanilla addons sometimes drop the high
// half when it's zero.
bool ParseGUID(const char *str, uint32_t &outHi, uint32_t &outLo) {
    if (str == nullptr)
        return false;
    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X'))
        return false;
    const char *hex = str + 2;
    const size_t len = std::strlen(hex);
    if (len != 8 && len != 16)
        return false;
    uint64_t value = 0;
    for (size_t i = 0; i < len; i++) {
        const char c = hex[i];
        uint64_t digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
        else if (c >= 'A' && c <= 'F') digit = 10 + (c - 'A');
        else return false;
        value = (value << 4) | digit;
    }
    if (len == 8) {
        outHi = 0;
        outLo = static_cast<uint32_t>(value);
    } else {
        outHi = static_cast<uint32_t>(value >> 32);
        outLo = static_cast<uint32_t>(value);
    }
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
    if (entry == nullptr)
        return 0; // cache miss / pending → nil

    const char *name = reinterpret_cast<const char *>(
        entry + Offsets::OFF_PLAYER_INFO_NAME);
    const char *realm = reinterpret_cast<const char *>(
        entry + Offsets::OFF_PLAYER_INFO_REALM);
    const uint32_t race = *reinterpret_cast<const uint32_t *>(
        entry + Offsets::OFF_PLAYER_INFO_RACE);
    const uint32_t sex = *reinterpret_cast<const uint32_t *>(
        entry + Offsets::OFF_PLAYER_INFO_SEX);
    const uint32_t classID = *reinterpret_cast<const uint32_t *>(
        entry + Offsets::OFF_PLAYER_INFO_CLASS);

    const char *englishClass = DBCStringField(
        Offsets::VAR_CHRCLASSES_RECORDS, Offsets::VAR_CHRCLASSES_COUNT,
        classID, Offsets::OFF_CHRCLASSES_FILENAME, /*indexByLocale=*/false);
    const char *localizedClass = DBCStringField(
        Offsets::VAR_CHRCLASSES_RECORDS, Offsets::VAR_CHRCLASSES_COUNT,
        classID, Offsets::OFF_CHRCLASSES_NAMES, /*indexByLocale=*/true);
    const char *englishRace = DBCStringField(
        Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
        race, Offsets::OFF_CHRRACES_FILENAME, /*indexByLocale=*/false);
    const char *localizedRace = DBCStringField(
        Offsets::VAR_CHRRACES_RECORDS, Offsets::VAR_CHRRACES_COUNT,
        race, Offsets::OFF_CHRRACES_NAMES, /*indexByLocale=*/true);

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
