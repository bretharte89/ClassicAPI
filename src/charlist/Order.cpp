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

// `GetSavedCharacterOrder(realm)` / `SetSavedCharacterOrder(realm, order)`
// — glue-only globals for persisting the character-select reorder UI
// across sessions. Storage:
//
//   WTF\Account\<account>\CharacterOrder.txt
//
// One `<realm>=<order>` line per realm. The `<order>` payload is opaque
// to this module — the Lua caller decides the encoding (the current
// GlueXML patch uses pipe-delimited character names).
//
// Account scoping is implicit via the file path (the engine writes
// `VAR_ACCOUNT_NAME_PTR` after auth response, so it's already populated
// by the time the user lands on character-select). Realm scoping is
// explicit via the key inside the file: multiple realms under one
// account each get an independent line. Vanilla 1.12 glue has no
// general-purpose persistence API — only the autologin-saturated
// `GetSavedAccountName`/`SetSavedAccountName` pair — so we roll the
// file format ourselves matching sibling persistence modules
// (`player::NameCache`, `equipmentset::Storage`).
//
// Registered on the **glue** Lua state only. In-world Lua sees a nil
// global; `/script GetSavedCharacterOrder("X")` from inside the world
// will error, by design — per TODO #94 the binding should not exist
// outside the screens that need it.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <map>
#include <string>

namespace Charlist::Order {

namespace {

// In-memory mirror of the on-disk file, pinned to whichever account
// it was loaded for. When the user switches autologin accounts on
// the glue screen, `VAR_ACCOUNT_NAME_PTR` updates and the next call
// to `EnsureLoaded` drops the stale cache and re-reads from the new
// account's path. Survives glue↔world transitions because the DLL
// stays loaded; the GlueModuleAutoRegister just re-attaches the two
// Lua C functions to the freshly created glue state each cycle.
std::map<std::string, std::string> g_orders;
std::string g_cachedAccount;
bool g_cacheValid = false;

const char *ReadAccountName() {
    return *reinterpret_cast<const char *const *>(
        static_cast<uintptr_t>(Offsets::VAR_ACCOUNT_NAME_PTR));
}

std::string ResolvePath(const std::string &account) {
    if (account.empty())
        return {};
    std::string out = "WTF\\Account\\";
    out += account;
    out += "\\CharacterOrder.txt";
    return out;
}

void RightTrim(std::string *s) {
    while (!s->empty()) {
        const char c = s->back();
        if (c != '\r' && c != '\n' && c != ' ' && c != '\t')
            break;
        s->pop_back();
    }
}

void EnsureLoaded() {
    const char *acctRaw = ReadAccountName();
    const std::string account = (acctRaw != nullptr) ? acctRaw : "";
    if (g_cacheValid && g_cachedAccount == account)
        return;
    g_cachedAccount = account;
    g_orders.clear();
    g_cacheValid = true; // mark valid even on empty/missing — re-loads when account changes
    const std::string path = ResolvePath(account);
    if (path.empty())
        return;
    std::ifstream file(path);
    if (!file.is_open())
        return; // first run on this account
    std::string line;
    while (std::getline(file, line)) {
        RightTrim(&line);
        if (line.empty() || line[0] == '#')
            continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if (key.empty())
            continue;
        g_orders[std::move(key)] = std::move(val);
    }
}

bool Save() {
    const std::string path = ResolvePath(g_cachedAccount);
    if (path.empty())
        return false;
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out.is_open())
            return false;
        out << "# ClassicAPI character-select order\n";
        out << "# <realm>=<opaque order payload> — one entry per realm\n";
        for (const auto &kv : g_orders) {
            if (kv.second.empty())
                continue; // skip cleared entries
            out << kv.first << '=' << kv.second << '\n';
        }
        if (!out)
            return false;
    }
    // Win32: `rename` fails if the destination exists. Removing first
    // leaves a brief window without the file, mitigated by the .tmp
    // surviving a crash here on the next save's recovery path.
    std::remove(path.c_str());
    if (std::rename(tmp.c_str(), path.c_str()) != 0)
        return false;
    return true;
}

// CR/LF inside either arg would corrupt the line-oriented file format
// on the next save (a key with an embedded newline would parse as two
// half-lines on reload). Reject up-front rather than sanitize — the
// Lua caller can do its own escaping if it ever has multi-line names.
bool HasFileBreakChars(const char *s) {
    for (const char *p = s; *p != '\0'; ++p)
        if (*p == '\r' || *p == '\n')
            return true;
    return false;
}

// `GetSavedCharacterOrder(realm) -> string`. Always returns a string
// (never nil): empty for unknown realm, missing/non-string arg,
// pre-login (no account name), or unreadable file. Per TODO #94 the
// Lua side relies on `""` as the "first-run / cleared" sentinel.
int __fastcall Script_GetSavedCharacterOrder(void *L) {
    EnsureLoaded();
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::PushString(L, "");
        return 1;
    }
    const char *realm = Game::Lua::ToString(L, 1);
    if (realm == nullptr || realm[0] == '\0') {
        Game::Lua::PushString(L, "");
        return 1;
    }
    const auto it = g_orders.find(realm);
    Game::Lua::PushString(L,
        it != g_orders.end() ? it->second.c_str() : "");
    return 1;
}

// `SetSavedCharacterOrder(realm, order)`. No return value. Empty
// `order` clears the realm's entry (removes the line on next save).
// Silent no-op when `realm` is missing/empty, when either arg
// contains a CR/LF, or when the account name isn't available yet.
int __fastcall Script_SetSavedCharacterOrder(void *L) {
    EnsureLoaded();
    if (!Game::Lua::IsString(L, 1) || !Game::Lua::IsString(L, 2))
        return 0;
    const char *realm = Game::Lua::ToString(L, 1);
    const char *order = Game::Lua::ToString(L, 2);
    if (realm == nullptr || realm[0] == '\0')
        return 0;
    if (order == nullptr)
        order = "";
    if (HasFileBreakChars(realm) || HasFileBreakChars(order))
        return 0;
    if (order[0] == '\0') {
        g_orders.erase(realm);
    } else {
        g_orders[realm] = order;
    }
    Save();
    return 0;
}

void RegisterGlue() {
    Game::Lua::RegisterGlueFunction("GetSavedCharacterOrder",
                                    &Script_GetSavedCharacterOrder);
    Game::Lua::RegisterGlueFunction("SetSavedCharacterOrder",
                                    &Script_SetSavedCharacterOrder);
}

static const Game::GlueModuleAutoRegister _autoreg{&RegisterGlue};

} // namespace

} // namespace Charlist::Order
