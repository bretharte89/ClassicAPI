// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

// `GLOBAL_MOUSE_DOWN` / `GLOBAL_MOUSE_UP` — modern WoW fires these
// on every raw mouse-button press/release, with a `button` string
// payload (`"LeftButton"` / `"RightButton"` / `"MiddleButton"` /
// `"Button4"` / `"Button5"`). Distinct from per-frame `OnMouseDown`
// scripts — these fire even when the click misses any UI frame.
//
// We poll Win32 `GetAsyncKeyState` per frame for the five mouse
// buttons and edge-detect transitions. Gated on WoW being the
// foreground window so alt-tabbed clicks elsewhere don't fire
// spurious events.

#include "event/Custom.h"
#include "tick/WorldTick.h"

#include <windows.h>

namespace Input::GlobalMouse {

namespace {

constexpr const char *kDownEvent = "GLOBAL_MOUSE_DOWN";
constexpr const char *kUpEvent   = "GLOBAL_MOUSE_UP";

struct Button {
    int vk;
    const char *name;
};

// Order matches modern WoW's button string identifiers. VK_XBUTTON1
// and VK_XBUTTON2 are the extra side buttons on a 5-button mouse.
constexpr Button kButtons[] = {
    {VK_LBUTTON,  "LeftButton"},
    {VK_RBUTTON,  "RightButton"},
    {VK_MBUTTON,  "MiddleButton"},
    {VK_XBUTTON1, "Button4"},
    {VK_XBUTTON2, "Button5"},
};

constexpr int kButtonCount = sizeof(kButtons) / sizeof(kButtons[0]);

bool g_prev[kButtonCount] = {};

bool WoWIsForeground() {
    HWND fg = GetForegroundWindow();
    if (fg == nullptr)
        return false;
    // Cache the WoW window by walking the foreground window's owner-
    // process chain on first sight. `GetForegroundWindow` returns the
    // window that has focus; for this process, that IS WoW's main
    // window (no child window has its own input focus). We compare by
    // process ID to avoid false positives if WoW is alt-tabbed.
    DWORD fgPid = 0;
    GetWindowThreadProcessId(fg, &fgPid);
    return fgPid == GetCurrentProcessId();
}

void OnWorldTick() {
    const bool focused = WoWIsForeground();
    for (int i = 0; i < kButtonCount; ++i) {
        // High-order bit of GetAsyncKeyState is "key is currently
        // down". Low bit is "key was pressed since last call" — we
        // don't use that since we maintain our own edge state.
        const bool down = (GetAsyncKeyState(kButtons[i].vk) & 0x8000) != 0;
        // Only honor down transitions when WoW has focus, so an
        // alt-tabbed click elsewhere doesn't fire. UP transitions
        // always fire — we want the matching release event even if
        // the user alt-tabbed mid-click, to avoid leaving addons in
        // a stuck "button is held" state.
        const bool effectiveDown = down && focused;
        if (effectiveDown == g_prev[i])
            continue;
        g_prev[i] = effectiveDown;
        const int slot = Event::Custom::Lookup(effectiveDown ? kDownEvent : kUpEvent);
        if (slot >= 0)
            Event::Custom::Fire(slot, "%s", kButtons[i].name);
    }
}

} // namespace

static const Event::Custom::AutoReserve _reserveDown{kDownEvent};
static const Event::Custom::AutoReserve _reserveUp{kUpEvent};
static const Tick::WorldTick::AutoSubscribe _tickSub{&OnWorldTick};

} // namespace Input::GlobalMouse
