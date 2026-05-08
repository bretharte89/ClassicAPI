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

namespace Event::Custom {

// Claims an unused (`name == NULL`) slot in the engine's event-registration
// table at `[VAR_EVENT_TABLE_BASE_PTR]`, sets its name to `eventName`, and
// returns the slot index. After a successful registration, addons can
// `frame:RegisterEvent(eventName)` and the engine will treat it like any
// built-in event; a matching `Fire*` call dispatches to those frames.
//
// `eventName` must outlive the engine — pass a static string literal or
// DLL-owned storage. Returns -1 if the table isn't initialized yet, or if
// no NULL slot was found in the live entry range. A reasonable place to
// call this is from a module's `RegisterLuaFunctions`, which fires after
// engine boot.
int Register(const char *eventName);

// Re-attempts `TryClaim` for any cached registration that hasn't succeeded
// yet. The engine populates the event table during boot AFTER our
// `LoadScriptFunctions` hook fires, so the boot-time `Register` calls
// usually return -1; calling `RetryAll` from a later hook point (e.g.
// `Frame::RegisterEvent`) catches them up just in time.
void RetryAll();

// Permits `TryClaim` to actually write to the event table. Until this is
// called, `Register` and `RetryAll` cache the name but DO NOT mutate the
// engine's table. Boot-time `RegisterEvent` calls (including those fired
// by SuperWoWhook and other DLLs) can race with the engine's own table
// init and trigger `SMemFree` on slots they expected to still be NULL —
// writing during that window crashed the engine. `DllMain.cpp` flips this
// after `LoadScriptFunctions_h` returns, so all our writes happen after
// the engine has finished its own setup.
void EnableWrites();

// Call this BEFORE the engine's `FrameScript_Initialize_o` runs (i.e.
// before each `/reload`). Walks our claimed slots and NULLs out the name
// pointers we wrote in. The engine's reload path iterates every entry
// and `SMemFree`s its name; our names are static literals in our DLL,
// not Storm-allocated, so they'd trip the SMem safety check and crash.
// Also drops the write gate and resets cached slot indices so the new
// table gets fresh claims.
void PrepareForReload();

// Dispatches an event registered via `Register` with two int args
// (format `"%d%d"`). Use this for `(itemID, success)`-style payloads;
// pass booleans as 0/1 since the engine has no native bool format code.
//
// Standard C++ doesn't let us forward varargs through `...` to a varargs
// callee, so each call shape lives in its own typed wrapper here. Add new
// shapes (`Fire_S`, `Fire_SD`, etc.) as additional events come online.
void Fire_DD(int eventID, int arg1, int arg2);

} // namespace Event::Custom
