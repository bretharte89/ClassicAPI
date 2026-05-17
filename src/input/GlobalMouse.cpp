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

// `GLOBAL_MOUSE_DOWN` / `GLOBAL_MOUSE_UP` — modern WoW fires these
// on every raw mouse-button press/release on the game window, with a
// `button` string payload (`"LeftButton"` / `"RightButton"` /
// `"MiddleButton"` / `"Button4"` / `"Button5"`). Distinct from
// per-frame `OnMouseDown` scripts — these fire even when the click
// misses any UI frame.
//
// Same `WH_GETMESSAGE` thread-message hook pattern as
// [Input::Modifier](Modifier.cpp): we intercept the WM_*BUTTON*
// messages on the engine's main thread *before* dispatch. The hook
// scopes naturally to WoW's message queue, so clicks while
// alt-tabbed elsewhere don't deliver — no foreground check needed.
// XBUTTON side-key distinction comes from `GET_XBUTTON_WPARAM`.

#include "Game.h"
#include "event/Custom.h"

#include <windows.h>

namespace Input::GlobalMouse {

namespace {

constexpr const char *kDownEvent = "GLOBAL_MOUSE_DOWN";
constexpr const char *kUpEvent   = "GLOBAL_MOUSE_UP";

HHOOK g_msgHook = nullptr;

// Maps WM_*BUTTON{DOWN,UP} → ("LeftButton"|..., isDown). Returns
// nullptr for non-mouse messages so the caller can short-circuit.
const char *DecodeButton(UINT msg, WPARAM wParam, bool *outDown) {
    switch (msg) {
        case WM_LBUTTONDOWN: *outDown = true;  return "LeftButton";
        case WM_LBUTTONUP:   *outDown = false; return "LeftButton";
        case WM_RBUTTONDOWN: *outDown = true;  return "RightButton";
        case WM_RBUTTONUP:   *outDown = false; return "RightButton";
        case WM_MBUTTONDOWN: *outDown = true;  return "MiddleButton";
        case WM_MBUTTONUP:   *outDown = false; return "MiddleButton";
        case WM_XBUTTONDOWN:
            *outDown = true;
            return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? "Button4" : "Button5";
        case WM_XBUTTONUP:
            *outDown = false;
            return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? "Button4" : "Button5";
        default:
            return nullptr;
    }
}

void ProcessMouseMessage(UINT msg, WPARAM wParam) {
    bool down = false;
    const char *name = DecodeButton(msg, wParam, &down);
    if (name == nullptr)
        return;
    const int slot = Event::Custom::Lookup(down ? kDownEvent : kUpEvent);
    if (slot >= 0)
        Event::Custom::Fire(slot, "%s", name);
}

LRESULT CALLBACK GetMsgHook(int code, WPARAM wParam, LPARAM lParam) {
    // Only process when the message is being removed from the queue
    // (`PM_REMOVE` / `GetMessage`); `PM_NOREMOVE` peeks would deliver
    // the same click twice if we processed both.
    if (code == HC_ACTION && wParam == PM_REMOVE) {
        const MSG *m = reinterpret_cast<const MSG *>(lParam);
        ProcessMouseMessage(m->message, m->wParam);
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

void InstallHook() {
    if (g_msgHook != nullptr)
        return;
    g_msgHook = SetWindowsHookExA(WH_GETMESSAGE, &GetMsgHook, nullptr,
                                   GetCurrentThreadId());
}

void RegisterLuaFunctions() { InstallHook(); }

const Event::Custom::AutoReserve _reserveDown{kDownEvent};
const Event::Custom::AutoReserve _reserveUp{kUpEvent};
const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Input::GlobalMouse
