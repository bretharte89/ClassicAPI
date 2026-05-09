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
#include "item/Data.h"

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
    // the start of FrameScript_Initialize), invalidate our cache. Our
    // injected names are Storm allocations, so the engine's
    // `SMemFree(entry.name)` teardown loop handles cleanup correctly on
    // its own — but the table is rebuilt at a fresh address afterwards,
    // and our cached slot indices point into the old layout. Drop the
    // writes gate and reset slots so post-rebuild `RetryAll` re-claims.
    Event::Custom::PrepareForReload();

    FrameScript_Initialize_o();
    // Set the `CLASSIC_API_VERSION` global directly via the Lua C API
    // rather than building a `"foo=N"` source string and routing it
    // through `FrameScript_Execute`. Faster (no parse), type-true (the
    // value is pushed as a number, not parsed as a numeric literal),
    // and avoids the need for any Lua source in our DLL.
    if (void *L = Game::Lua::State()) {
        Game::Lua::PushString(L, "CLASSIC_API_VERSION");
        Game::Lua::PushNumber(L, static_cast<double>(CLASSICAPI_VERSION_VALUE));
        Game::Lua::SetTable(L, Game::Lua::GLOBALS_INDEX);
    }
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
        // Auto-warm the item cache on `GetItemInfo(uncached_id)` calls, so
        // subsequent calls return valid data and GET_ITEM_INFO_RECEIVED
        // fires when the response arrives — matches modern WoW (5.x+)
        // behavior. Without this, vanilla 1.12's `GetItemInfo` returns
        // nil for misses and never fires a query, forcing addons to roll
        // their own warmup hacks.
        HOOK_FUNCTION(Offsets::FUN_SCRIPT_GET_ITEM_INFO, Item::Data::Script_GetItemInfo_h,
                      Item::Data::Script_GetItemInfo_o);
    } else if (reason == DLL_PROCESS_DETACH) {
        MH_Uninitialize();
    }
    return TRUE;
}
