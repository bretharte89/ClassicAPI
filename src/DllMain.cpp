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

#include "Common.h"
#include "Game.h"
#include "MinHook.h"
#include "Offsets.h"
#include "event/Custom.h"

#include <string>

static Game::FrameScript_Initialize_t FrameScript_Initialize_o = nullptr;
static Game::LoadScriptFunctions_t LoadScriptFunctions_o = nullptr;

// `Frame::RegisterEvent` is `__thiscall(this, eventName)`. MSVC can't emit
// __thiscall on free functions, but a __fastcall function with a dummy EDX
// arg matches the same register layout (ECX=this, EDX unused, stack=name).
using FrameRegisterEvent_t = void(__fastcall *)(void *frame, void *edx,
                                                 const char *eventName);
static FrameRegisterEvent_t FrameRegisterEvent_o = nullptr;

static void __fastcall InvalidFunctionPtrCheck_h() {}

static bool __fastcall FrameScript_Initialize_h() {
    // BEFORE the engine tears down the old event table (which it does at
    // the start of FrameScript_Initialize), null out our injected event
    // names. The engine's teardown at 0x00701A54 calls SMemFree on every
    // entry's name; our names are static literals in our DLL and not
    // Storm-allocated, so leaving them in place trips the SMem safety
    // check on `/reload` and crashes. The check at 0x00701A45 skips the
    // free for any entry with NULL name, so this is the supported escape
    // hatch. On first boot the cache is empty and this is a no-op.
    Event::Custom::PrepareForReload();

    FrameScript_Initialize_o();
    const std::string luaScript =
        "CLASSIC_API_VERSION=" + std::to_string(CLASSICAPI_VERSION_VALUE);
    Game::FrameScript_Execute(luaScript.c_str(), "ClassicAPI.lua");
    return true;
}

static void __fastcall LoadScriptFunctions_h() {
    LoadScriptFunctions_o();
    Game::RunModuleRegistrations();
    // Permit table writes from this point on. Earlier writes (during the
    // engine's own boot-time `RegisterEvent` calls, including those fired
    // by SuperWoWhook) crash with `SMemFree` on slots the engine expected
    // to still be NULL — see commit history / CLAUDE.md.
    Event::Custom::EnableWrites();
}

// Fires for every `frame:RegisterEvent(...)` call from Lua. By this point
// the engine's event table is fully populated, so any custom events we
// couldn't claim during the LoadScriptFunctions hook can be claimed now —
// before the engine's strcmp scan against `eventName` actually runs.
static void __fastcall FrameRegisterEvent_h(void *frame, void *edx,
                                            const char *eventName) {
    Event::Custom::RetryAll();
    FrameRegisterEvent_o(frame, edx, eventName);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        if (MH_Initialize() != MH_OK)
            return FALSE;

        auto *target = reinterpret_cast<LPVOID>(Offsets::FUN_INVALID_FUNCTION_PTR_CHECK);
        if (MH_CreateHook(target, static_cast<LPVOID>(InvalidFunctionPtrCheck_h), nullptr) != MH_OK)
            return FALSE;
        if (MH_EnableHook(target) != MH_OK)
            return FALSE;

        HOOK_FUNCTION(Offsets::FUN_FRAME_SCRIPT_INITIALIZE, FrameScript_Initialize_h,
                      FrameScript_Initialize_o);
        HOOK_FUNCTION(Offsets::FUN_LOAD_SCRIPT_FUNCTIONS, LoadScriptFunctions_h,
                      LoadScriptFunctions_o);
        HOOK_FUNCTION(Offsets::FUN_FRAME_REGISTER_EVENT, FrameRegisterEvent_h,
                      FrameRegisterEvent_o);
    } else if (reason == DLL_PROCESS_DETACH) {
        MH_Uninitialize();
    }
    return TRUE;
}
