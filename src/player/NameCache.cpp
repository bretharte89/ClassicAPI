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

// Persistent name cache — see NameCache.h for the motivating use case.
//
// Two persistence files:
//
//   `WTF\Account\<acct>\ClassicAPI.txt`
//       Account-level settings. Single `PersistentNameCacheEnabled=1`
//       line for now; other ClassicAPI features may layer here later.
//       Loaded on first need (when the account-name global is
//       populated by the engine's login flow). Hand-edits survive.
//
//   `WTF\Account\<acct>\<realm>\ClassicAPI_NameCache.txt`
//       Per-realm cache contents. One entry per line:
//           `guidHi  guidLo  classID  name`
//       (fields tab-separated). Loaded once the realm name is
//       available post-login. Saved when (a) a recent write hasn't
//       been flushed AND (b) at least 30 seconds have elapsed since
//       the prior save, OR (c) DLL teardown / `Flush()` is called.
//
// Population sources:
//   1. The engine's NameCache writer (`FUN_PLAYER_INFO_CACHE_WRITE`)
//      — every SMSG_NAME_QUERY_RESPONSE the engine processes flows
//      through here. Covers chat, group/raid sync, guild roster
//      updates, visible-object resolution.
//   2. `C_PlayerInfo.RememberPlayer(guid, name, classToken)` from
//      Lua — lets addons feed in sources the engine cache doesn't
//      touch directly (e.g. /who results parsed via GetWhoInfo,
//      mouseover, target).
//
// Lookups are GUID-keyed, so the "deleted character recreated with
// same name, different class" failure mode of name-keyed caches
// (libunitscan, etc.) doesn't apply — different GUIDs are different
// entries, period.

#include "NameCache.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <windows.h>

namespace Player::NameCache {

namespace {

// Backstop flush interval — explicit `Flush()` calls from DllMain's
// `FrameScript_Initialize` hook (covers /reload and /logout) and
// `DLL_PROCESS_DETACH` (clean /quit) are the primary save path. The
// in-`Remember` `MaybeFlush` runs only as a defensive measure for
// long sessions that never reload AND end via hard process kill
// (task manager, OS crash) — in that case we bound data loss to
// roughly this interval. 5 minutes was chosen as a compromise:
// long enough that routine play has zero in-flight I/O, short
// enough that a rare bad-shutdown loses at most ~5 min of new
// entries.
constexpr DWORD kFlushIntervalMs = 5 * 60 * 1000;

// ChrClasses.dbc max ID. Anything above this is a corrupt input.
constexpr uint32_t kMaxClassID = 16;

std::unordered_map<uint64_t, Entry> g_entries;
bool g_enabled = false;
bool g_settingsLoaded = false;
bool g_cacheLoaded = false;
bool g_dirty = false;
DWORD g_lastSaveTick = 0;

// ----- engine globals helpers (mirrors EquipmentSet::Storage) -----

const char *ReadAccountName() {
    return *reinterpret_cast<const char *const *>(
        static_cast<uintptr_t>(Offsets::VAR_ACCOUNT_NAME_PTR));
}

const char *ReadRealmName() {
    auto *info = *reinterpret_cast<uint8_t **>(
        static_cast<uintptr_t>(Offsets::VAR_REALM_INFO_PTR));
    if (info == nullptr)
        return nullptr;
    return *reinterpret_cast<const char *const *>(
        info + Offsets::OFF_REALM_INFO_NAME);
}

// ----- path resolution -----

std::string SettingsPath() {
    const char *account = ReadAccountName();
    if (account == nullptr || account[0] == '\0')
        return {};
    std::string out = "WTF\\Account\\";
    out += account;
    out += "\\ClassicAPI.txt";
    return out;
}

std::string CachePath() {
    const char *account = ReadAccountName();
    if (account == nullptr || account[0] == '\0')
        return {};
    const char *realm = ReadRealmName();
    if (realm == nullptr || realm[0] == '\0')
        return {};
    std::string out = "WTF\\Account\\";
    out += account;
    out += '\\';
    out += realm;
    out += "\\ClassicAPI_NameCache.txt";
    return out;
}

// ----- settings load/save -----

void LoadSettingsIfNeeded() {
    if (g_settingsLoaded)
        return;
    const std::string path = SettingsPath();
    if (path.empty())
        return; // pre-login; retry on next call (g_settingsLoaded stays false)
    g_settingsLoaded = true; // path resolved — even an empty file is "loaded"

    std::ifstream file(path);
    if (!file.is_open())
        return; // no settings file = all defaults (disabled)
    std::string line;
    while (std::getline(file, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'
                                 || line.back() == ' ' || line.back() == '\t'))
            line.pop_back();
        if (line.empty() || line[0] == '#')
            continue;
        auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        if (key == "PersistentNameCacheEnabled")
            g_enabled = (value == "1" || value == "true");
    }
}

void SaveSettings() {
    const std::string path = SettingsPath();
    if (path.empty())
        return;
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out.is_open())
            return;
        out << "# ClassicAPI account-level settings\n";
        out << "PersistentNameCacheEnabled=" << (g_enabled ? 1 : 0) << "\n";
        if (!out)
            return;
    }
    std::remove(path.c_str());
    std::rename(tmp.c_str(), path.c_str());
}

// ----- cache load/save -----

// Lightweight name sanitizer — vanilla player names are 2..12 ASCII
// letters, but for paranoia drop anything that would break a line
// (tab, newline) and cap length. Anything else passes through.
std::string SanitizeName(const char *name) {
    if (name == nullptr)
        return {};
    std::string out;
    for (const char *p = name; *p && out.size() < 16; ++p) {
        const char c = *p;
        if (c == '\t' || c == '\r' || c == '\n')
            continue;
        out.push_back(c);
    }
    return out;
}

void LoadCacheIfNeeded() {
    if (g_cacheLoaded)
        return;
    const std::string path = CachePath();
    if (path.empty())
        return; // pre-realm; retry on next call (g_cacheLoaded stays false)
    g_cacheLoaded = true;

    std::ifstream file(path);
    if (!file.is_open())
        return; // first run for this realm

    std::string line;
    while (std::getline(file, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.pop_back();
        if (line.empty() || line[0] == '#')
            continue;

        // tab-separated: hi  lo  classID  name
        std::istringstream is(line);
        std::string hiStr, loStr, classStr, name;
        if (!(is >> hiStr >> loStr >> classStr))
            continue;
        // Consume the single tab/space, then read remaining name
        // as-is (no spaces in vanilla names but tolerate them).
        while (is.peek() == ' ' || is.peek() == '\t')
            is.get();
        std::getline(is, name);
        if (name.empty())
            continue;

        const uint32_t hi = static_cast<uint32_t>(std::strtoul(hiStr.c_str(), nullptr, 16));
        const uint32_t lo = static_cast<uint32_t>(std::strtoul(loStr.c_str(), nullptr, 16));
        const uint32_t classID = static_cast<uint32_t>(std::strtoul(classStr.c_str(), nullptr, 10));
        if (lo == 0 && hi == 0)
            continue;
        if (classID > kMaxClassID)
            continue;
        const uint64_t guid = (static_cast<uint64_t>(hi) << 32) | lo;
        Entry e;
        e.guid = guid;
        e.name = SanitizeName(name.c_str());
        e.classID = classID;
        if (!e.name.empty())
            g_entries[guid] = std::move(e);
    }
}

void SaveCache() {
    const std::string path = CachePath();
    if (path.empty())
        return;
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out.is_open())
            return;
        out << "# ClassicAPI name cache v1 (per-realm)\n";
        out << "# guidHi\tguidLo\tclassID\tname\n";
        for (const auto &kv : g_entries) {
            const Entry &e = kv.second;
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%08X\t%08X\t%u\t",
                          static_cast<uint32_t>(e.guid >> 32),
                          static_cast<uint32_t>(e.guid),
                          e.classID);
            out << buf << e.name << "\n";
        }
        if (!out)
            return;
    }
    std::remove(path.c_str());
    std::rename(tmp.c_str(), path.c_str());
    g_dirty = false;
    g_lastSaveTick = GetTickCount();
}

void MaybeFlush() {
    if (!g_dirty)
        return;
    const DWORD now = GetTickCount();
    if ((now - g_lastSaveTick) < kFlushIntervalMs)
        return;
    SaveCache();
}

// ----- engine NameCache hook -----

// `__thiscall(this = cache, packet, guidLo, guidHi)`. We forward to
// the original first (so the engine settles its entry and dispatches
// callbacks), then read the same fields back out of the packet and
// mirror them into our store.
using CacheWrite_t = void(__thiscall *)(void *self, const uint8_t *packet,
                                        uint32_t guidLo, uint32_t guidHi);
CacheWrite_t CacheWrite_o = nullptr;

void __fastcall CacheWrite_h(void *self, void *edx, const uint8_t *packet,
                             uint32_t guidLo, uint32_t guidHi) {
    // __fastcall stub forwards to __thiscall — MSVC's free-function
    // __fastcall lays out (ECX=self, EDX=unused, stack=packet, lo, hi),
    // matching the engine's __thiscall(self, packet, lo, hi).
    CacheWrite_o(self, packet, guidLo, guidHi);
    (void)edx;

    if (!g_enabled)
        return;
    if (packet == nullptr)
        return;
    if (guidLo == 0 && guidHi == 0)
        return;

    LoadCacheIfNeeded();

    const char *name = reinterpret_cast<const char *>(
        packet + Offsets::OFF_PLAYER_INFO_NAME);
    const uint32_t classID = *reinterpret_cast<const uint32_t *>(
        packet + Offsets::OFF_PLAYER_INFO_CLASS);
    if (name[0] == '\0')
        return;
    if (classID == 0 || classID > kMaxClassID)
        return;

    const uint64_t guid = (static_cast<uint64_t>(guidHi) << 32) | guidLo;
    Entry &e = g_entries[guid];
    const std::string sanitized = SanitizeName(name);
    bool changed = false;
    if (e.guid != guid) {
        e.guid = guid;
        changed = true;
    }
    if (e.name != sanitized) {
        e.name = sanitized;
        changed = true;
    }
    if (e.classID != classID) {
        e.classID = classID;
        changed = true;
    }
    if (changed) {
        g_dirty = true;
        MaybeFlush();
    }
}

static const Game::HookAutoRegister _hookreg{
    Offsets::FUN_PLAYER_INFO_CACHE_WRITE,
    reinterpret_cast<void *>(&CacheWrite_h),
    reinterpret_cast<void **>(&CacheWrite_o)};

// ----- Lua surface -----

// Parses `"0xHHHHHHHHLLLLLLLL"` (16-digit) or `"0xLLLLLLLL"` (8-digit
// with implicit hi=0). Mirrors `Player::Info::ParseGUID` exactly —
// kept inline here so we don't expand the public surface of that TU
// just for one helper.
bool ParseGUID(const char *str, uint64_t *outGUID) {
    if (str == nullptr || str[0] != '0' || (str[1] != 'x' && str[1] != 'X'))
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
    *outGUID = value;
    return true;
}

// Returns the ChrClasses.dbc record ID for a class token like
// `"WARRIOR"`, `"MAGE"`, etc. Case-insensitive. 0 if not matched.
// 1.12 has 9 player classes; we walk the DBC each call (cheap — 9
// records, each filename a few bytes) rather than building a static
// table that risks getting out-of-sync.
uint32_t ResolveClassToken(const char *token) {
    if (token == nullptr || *token == '\0')
        return 0;
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_CHRCLASSES_COUNT));
    const uint8_t *const *records = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_CHRCLASSES_RECORDS));
    if (records == nullptr)
        return 0;
    for (int i = 1; i <= count; ++i) {
        const uint8_t *rec = records[i];
        if (rec == nullptr)
            continue;
        const char *filename = *reinterpret_cast<const char *const *>(
            rec + Offsets::OFF_CHRCLASSES_FILENAME);
        if (filename == nullptr)
            continue;
        // Case-insensitive compare; vanilla tokens are uppercase
        // ("WARRIOR", "MAGE", ...) and modern Classic uses the same.
        const char *a = filename;
        const char *b = token;
        bool match = true;
        while (*a && *b) {
            char ca = *a, cb = *b;
            if (ca >= 'a' && ca <= 'z') ca -= 32;
            if (cb >= 'a' && cb <= 'z') cb -= 32;
            if (ca != cb) { match = false; break; }
            ++a; ++b;
        }
        if (match && *a == '\0' && *b == '\0')
            return static_cast<uint32_t>(i);
    }
    return 0;
}

// `C_PlayerInfo.RememberPlayer(guid, name, classToken)` — stores the
// triple in the persistent cache. `classToken` is the uppercase
// engine token (`"WARRIOR"`, `"MAGE"`, ...). Returns `true` on
// successful insert/update, `false` if the cache is disabled or the
// args are malformed.
int __fastcall Script_C_PlayerInfo_RememberPlayer(void *L) {
    if (!IsEnabled()) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    if (!Game::Lua::IsString(L, 1) || !Game::Lua::IsString(L, 2)
            || !Game::Lua::IsString(L, 3)) {
        Game::Lua::Error(L,
            "Usage: C_PlayerInfo.RememberPlayer(guid, name, classToken)");
        return 0;
    }
    uint64_t guid;
    if (!ParseGUID(Game::Lua::ToString(L, 1), &guid) || guid == 0) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const char *name = Game::Lua::ToString(L, 2);
    if (name == nullptr || name[0] == '\0') {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const char *classToken = Game::Lua::ToString(L, 3);
    const uint32_t classID = ResolveClassToken(classToken);
    // classID == 0 is acceptable (caller passed an unknown token);
    // we keep the prior class if any. This mirrors what Remember()
    // does when called with classID==0 elsewhere.
    Remember(guid, name, classID);
    Game::Lua::PushBoolean(L, 1);
    return 1;
}

// `ClassicAPI.SetPersistentNameCacheEnabled(bool)` — opt-in toggle.
// Persists to the account-level settings file. Enabling triggers a
// lazy load of any prior cache contents for the current realm.
int __fastcall Script_ClassicAPI_SetPersistentNameCacheEnabled(void *L) {
    LoadSettingsIfNeeded();
    bool desired = false;
    if (Game::Lua::IsNumber(L, 1))
        desired = Game::Lua::ToNumber(L, 1) != 0;
    else
        desired = Game::Lua::ToBoolean(L, 1) != 0;
    if (g_enabled != desired) {
        g_enabled = desired;
        SaveSettings();
        if (g_enabled) {
            LoadCacheIfNeeded();
        } else {
            // Flush any pending writes before going quiet.
            if (g_dirty)
                SaveCache();
        }
    }
    return 0;
}

int __fastcall Script_ClassicAPI_GetPersistentNameCacheEnabled(void *L) {
    LoadSettingsIfNeeded();
    Game::Lua::PushBoolean(L, g_enabled ? 1 : 0);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_PlayerInfo", "RememberPlayer",
                                     &Script_C_PlayerInfo_RememberPlayer);
    Game::Lua::RegisterTableFunction("ClassicAPI", "SetPersistentNameCacheEnabled",
                                     &Script_ClassicAPI_SetPersistentNameCacheEnabled);
    Game::Lua::RegisterTableFunction("ClassicAPI", "GetPersistentNameCacheEnabled",
                                     &Script_ClassicAPI_GetPersistentNameCacheEnabled);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

const Entry *Lookup(uint64_t guid) {
    if (!g_enabled)
        return nullptr;
    LoadCacheIfNeeded();
    auto it = g_entries.find(guid);
    if (it == g_entries.end())
        return nullptr;
    return &it->second;
}

void Remember(uint64_t guid, const char *name, uint32_t classID) {
    if (!g_enabled || guid == 0 || name == nullptr || name[0] == '\0')
        return;
    LoadCacheIfNeeded();
    if (classID > kMaxClassID)
        classID = 0;
    Entry &e = g_entries[guid];
    const std::string sanitized = SanitizeName(name);
    bool changed = false;
    if (e.guid != guid) {
        e.guid = guid;
        changed = true;
    }
    if (e.name != sanitized) {
        e.name = sanitized;
        changed = true;
    }
    // classID==0 means "caller doesn't know"; don't overwrite a
    // real classID with unknown. Only update on real → real change.
    if (classID != 0 && e.classID != classID) {
        e.classID = classID;
        changed = true;
    }
    if (changed) {
        g_dirty = true;
        MaybeFlush();
    }
}

bool IsEnabled() {
    LoadSettingsIfNeeded();
    return g_enabled;
}

void SetEnabled(bool enabled) {
    LoadSettingsIfNeeded();
    if (g_enabled == enabled)
        return;
    g_enabled = enabled;
    SaveSettings();
    if (g_enabled)
        LoadCacheIfNeeded();
    else if (g_dirty)
        SaveCache();
}

void Flush() {
    if (!g_enabled || !g_dirty)
        return;
    // SaveCache resolves the path itself — returns silently if realm
    // name isn't available yet (pre-login).
    SaveCache();
}

} // namespace Player::NameCache
