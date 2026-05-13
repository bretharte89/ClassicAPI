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

// `GetCurrentChatGUID()` — call from inside an addon's OnEvent for
// any CHAT_MSG_* to get the sender's GUID. Vanilla 1.12 omits the
// GUID from chat event args (added as arg12 of CHAT_MSG_* only in
// 3.0+), so addons currently strcmp the localized sender name string
// against an internal cache to identify the player. With this we
// just read the GUID directly.
//
// Implementation: the SMSG_MESSAGECHAT handler at FUN_0049D560 reads
// the senderGUID from the packet (offsets `[ebp-0x10]` / `[ebp-0xC]`
// in that function's stack frame) and forwards it as args 9-10 to
// the chat dispatcher at FUN_0049A870. The dispatcher in turn fires
// the appropriate CHAT_MSG_* event during its execution. We hook the
// dispatcher's entry, stash the GUID into a file-scope global, call
// the original (which fires the event synchronously), then clear the
// global on return. Inside the addon's OnEvent, `GetCurrentChatGUID`
// reads the global.
//
// Why not hook the SMSG handler directly: the GUID isn't in argument
// registers/stack at the handler's entry — it's read mid-function
// into locals. Hooking the dispatcher (which receives the GUID as
// args) is structurally simpler. The trade-off: the dispatcher is
// also called for synthetic chat (system notifications, arena team
// events, etc.) with GUID = 0; for those, `GetCurrentChatGUID()`
// returns nil — matches the modern API's behavior of nil GUID for
// non-player-sourced chat.

#include "CurrentGUID.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdio>

namespace Chat::CurrentGUID {

namespace {

uint32_t g_currentGuidLo = 0;
uint32_t g_currentGuidHi = 0;

int __fastcall Script_GetCurrentChatGUID(void *L) {
    if (g_currentGuidLo == 0 && g_currentGuidHi == 0)
        return 0;
    char buf[24];
    std::snprintf(buf, sizeof(buf), "0x%08X%08X",
                  g_currentGuidHi, g_currentGuidLo);
    Game::Lua::PushString(L, buf);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetCurrentChatGUID",
                                       &Script_GetCurrentChatGUID);
}

} // namespace

ChatDispatch_t ChatDispatch_o = nullptr;

void __fastcall ChatDispatch_h(
    const char *sender, int chatType,
    int arg3, int arg4, int arg5, int arg6, int arg7,
    int targetGuidLo, int targetGuidHi, int arg10,
    int senderGuidLo, int senderGuidHi)
{
    // Save before original — the engine's chat event fires DURING
    // origCall, and the addon's OnEvent runs synchronously inside
    // that window. The GUID must be readable at that point.
    const uint32_t prevLo = g_currentGuidLo;
    const uint32_t prevHi = g_currentGuidHi;
    g_currentGuidLo = static_cast<uint32_t>(senderGuidLo);
    g_currentGuidHi = static_cast<uint32_t>(senderGuidHi);

    ChatDispatch_o(sender, chatType, arg3, arg4, arg5, arg6, arg7,
                   targetGuidLo, targetGuidHi, arg10,
                   senderGuidLo, senderGuidHi);

    // Restore prior value rather than clearing to 0, so nested
    // dispatch (one chat event firing during another's OnEvent —
    // e.g. an addon calling SendChatMessage from its handler) leaves
    // the outer context's GUID intact when the inner returns.
    g_currentGuidLo = prevLo;
    g_currentGuidHi = prevHi;
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

static const Game::HookAutoRegister _hookreg{
    Offsets::FUN_CHAT_DISPATCH,
    reinterpret_cast<void *>(&ChatDispatch_h),
    reinterpret_cast<void **>(&ChatDispatch_o)};

} // namespace Chat::CurrentGUID
