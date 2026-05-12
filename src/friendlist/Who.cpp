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

// `C_FriendList.SendWhoQueryByName` — sends a /who name-filter query
// invisibly (the server's standard "Players found" chat message
// suppressed; results buffer into the engine's WhoList so the
// existing `GetWhoInfo` / `WHO_LIST_UPDATE` work normally).
//
// Vanilla 1.12 exposes only `SendWho(query)` + `SetWhoToUI(flag)` as
// separate primitives; addons that want to query a specific player
// without triggering the friends-panel popup have to do the dance
// pfUI's `chat.lua` does (~50 lines: pending state, cooldown timer,
// FriendsFrame_OnEvent wrap, flag toggle). This collapses the
// invocation half of that dance to one call.
//
// The friends-panel suppression itself still needs an addon-side
// `FriendsFrame_OnEvent` wrap — gated on `IsWhoQueryPending()` —
// because we don't hook the WHO_LIST_UPDATE engine dispatch yet
// (that requires interposing on `FrameScript_SignalEvent` or the
// SMSG_WHO handler, both higher-risk hook sites). With the wrap +
// gate, pfUI's ScanWhoName collapses to a one-liner.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstring>

#include <windows.h>

namespace FriendList::Who {

namespace {

// Server-enforced /who cooldown is ~5 seconds — a faster client gets
// silent-dropped. We mirror that on the client to avoid wasting
// queries that won't get a response, and to give addons a clean
// "did my query get sent?" signal via the bool return.
constexpr DWORD kCooldownMs = 5000;

// Window during which `IsWhoQueryPending()` reports true after a
// send. Generous upper bound on response RTT — the engine usually
// replies within a few hundred ms, but a stalled connection or
// dropped packet can stretch it. Larger than the cooldown so a
// time-based reset still pins down the worst case.
constexpr DWORD kPendingWindowMs = 5000;

DWORD g_lastSendTick = 0;
bool g_haveSent = false;

bool OnCooldown() {
    if (!g_haveSent)
        return false;
    const DWORD now = GetTickCount();
    return (now - g_lastSendTick) < kCooldownMs;
}

bool IsPending() {
    if (!g_haveSent)
        return false;
    const DWORD now = GetTickCount();
    return (now - g_lastSendTick) < kPendingWindowMs;
}

// `__thiscall(this = WhoSystem*, queryStr)` — the inner sender that
// `Script_SendWho` tail-calls. Parses the /who filter syntax and
// ships CMSG_WHO. Returns void; failure is silent (e.g. NULL system
// pointer pre-login).
using SendQuery_t = void(__thiscall *)(void *self, const char *queryStr);

bool SendQueryInternal(const char *fullQuery) {
    auto *system = *reinterpret_cast<void *const *>(
        static_cast<uintptr_t>(Offsets::VAR_WHO_SYSTEM));
    if (system == nullptr)
        return false;
    auto fn = reinterpret_cast<SendQuery_t>(Offsets::FUN_WHO_SYSTEM_SEND_QUERY);
    fn(system, fullQuery);
    return true;
}

void SetWhoToUIFlag(int value) {
    *reinterpret_cast<int *>(
        static_cast<uintptr_t>(Offsets::VAR_WHO_TO_UI_FLAG)) = value;
}

// `C_FriendList.SendWhoQueryByName(name)` — fires a /who query for
// exactly one character name, buffered into the WhoList so a normal
// `WHO_LIST_UPDATE` + `GetWhoInfo(i)` flow can read the result.
//
// Returns:
//   `true`  — query was sent (sets pending state; cooldown begins).
//   `false` — on cooldown, name was empty, or the WhoSystem isn't
//             yet initialized (pre-login).
//
// Does NOT suppress the friends-panel popup on response. Addons that
// don't want the popup wrap `FriendsFrame_OnEvent` themselves and
// gate on `C_FriendList.IsWhoQueryPending()`:
//
//   local original = FriendsFrame_OnEvent
//   _G.FriendsFrame_OnEvent = function()
//       if event == "WHO_LIST_UPDATE" and C_FriendList.IsWhoQueryPending() then
//           return
//       end
//       return original()
//   end
//
// (Hooking the engine's signal dispatch to suppress automatically is
// a follow-up — would need a hook on `FrameScript_SignalEvent` or
// the SMSG_WHO handler, both higher-risk than the surface we have
// today.)
int __fastcall Script_C_FriendList_SendWhoQueryByName(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L,
            "Usage: C_FriendList.SendWhoQueryByName(name)");
        return 0;
    }
    const char *name = Game::Lua::ToString(L, 1);
    if (name == nullptr || *name == '\0') {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    if (OnCooldown()) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    // Build the "n-<name>" filter the engine's parser expects. Bound
    // by the realistic max character name length (12) plus the
    // 2-char prefix plus a NUL — 16 bytes is comfortably enough; cap
    // at 48 for paranoia in case a server allows longer names.
    char query[64];
    const size_t prefixLen = 2;
    const size_t maxName = sizeof(query) - prefixLen - 1;
    size_t nameLen = std::strlen(name);
    if (nameLen > maxName)
        nameLen = maxName;
    std::memcpy(query, "n-", prefixLen);
    std::memcpy(query + prefixLen, name, nameLen);
    query[prefixLen + nameLen] = '\0';

    // Engine routes responses into the WhoList (and fires
    // WHO_LIST_UPDATE) when this flag is set; otherwise it prints
    // "Found N players matching..." to chat. We want list semantics.
    SetWhoToUIFlag(1);
    if (!SendQueryInternal(query)) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    g_lastSendTick = GetTickCount();
    g_haveSent = true;
    Game::Lua::PushBoolean(L, 1);
    return 1;
}

// `C_FriendList.IsWhoQueryPending()` — true within a short window
// (~5s) after the most recent `SendWhoQueryByName` returned true.
// Time-based — the engine doesn't expose a "response arrived" hook
// from C++ yet, so we conservatively report pending until the
// cooldown expires. In practice the WHO_LIST_UPDATE response lands
// well inside this window.
int __fastcall Script_C_FriendList_IsWhoQueryPending(void *L) {
    Game::Lua::PushBoolean(L, IsPending() ? 1 : 0);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_FriendList", "SendWhoQueryByName",
                                     &Script_C_FriendList_SendWhoQueryByName);
    Game::Lua::RegisterTableFunction("C_FriendList", "IsWhoQueryPending",
                                     &Script_C_FriendList_IsWhoQueryPending);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace FriendList::Who
