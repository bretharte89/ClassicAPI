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

#pragma once

namespace Chat::CurrentGUID {

// Hook for FUN_CHAT_DISPATCH (0x0049A870). Saves the senderGUID into
// a file-scope global for the duration of the original call, then
// clears it on return. Inside that window the engine fires the
// appropriate CHAT_MSG_* event, so `GetCurrentChatGUID()` called from
// the addon's OnEvent sees the GUID for the message that triggered it.
//
// 12-arg __fastcall signature. The function has a stack-probe
// prologue (`mov eax, 0x1F0C; call _chkstk`) — its frame is ~8KB.
// Verified by disassembling 0x0049A870 in 1.12.1: stack-arg access
// reaches `[ebp+0x2C]` (= param 12) and the function exits with
// `ret 0x28` (cleans 40 bytes = 10 stack args + 2 register args).
//
//   ECX  = arg1 (sender name string)
//   EDX  = arg2 (chat type byte)
//   [esp+ 4] = arg3
//   [esp+ 8] = arg4
//   [esp+ c] = arg5
//   [esp+10] = arg6
//   [esp+14] = arg7
//   [esp+18] = arg8 — target GUID lo (for whisper/channel target)
//   [esp+1c] = arg9 — target GUID hi
//   [esp+20] = arg10
//   [esp+24] = arg11 — sender GUID lo     ← captured
//   [esp+28] = arg12 — sender GUID hi     ← captured
//
// Identified args 11/12 as senderGUID by tracing the AFK auto-reply
// path at 0x0049AB39: reads params 11/12, OR's them to check for
// non-zero, pushes them onto the packet stream via FUN_00418370
// (the "write u64" helper). That's the GUID the AFK whisper-back is
// addressed to — i.e. the original message sender.
//
// An earlier 10-arg signature derived from Ghidra (on the wrong
// binary) had `ret 0x20` cleanup. The 8-byte cleanup mismatch with
// the engine's `ret 0x28`-shaped call left 8 bytes of stack
// uncleaned per chat-dispatch invocation, corrupting the engine's
// stack frame and crashing on a later indirect call. See the
// commit message for the full diagnosis.
using ChatDispatch_t = void(__fastcall *)(
    const char *sender, int chatType,
    int arg3, int arg4, int arg5, int arg6, int arg7,
    int targetGuidLo, int targetGuidHi, int arg10,
    int senderGuidLo, int senderGuidHi);
extern ChatDispatch_t ChatDispatch_o;
void __fastcall ChatDispatch_h(
    const char *sender, int chatType,
    int arg3, int arg4, int arg5, int arg6, int arg7,
    int targetGuidLo, int targetGuidHi, int arg10,
    int senderGuidLo, int senderGuidHi);

} // namespace Chat::CurrentGUID
