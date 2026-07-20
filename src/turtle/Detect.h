// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

#pragma once

// Turtle WoW client detection — the gate for every module under
// `src/turtle/`. Turtle's FrameXML sets a `TURTLE_WOW_VERSION` global (see
// `FrameXML/Globals.lua`, e.g. `"1.18.1"`); its presence is the
// authoritative "this is the Turtle client" signal — no realm-list
// sniffing (which conflates "connected to Turtle" with "running the
// Turtle client" and was rejected for the Rip combo-duration work), no
// binary fingerprinting.
namespace Turtle {

// True iff running on a Turtle WoW client. Reads the `TURTLE_WOW_VERSION`
// global and LATCHES true once seen. Safe to call any time: it returns
// false until the in-game Lua state exists AND FrameXML's Globals.lua has
// run (so it can read false very early in boot / on the glue screen), then
// flips true and stays true. Never caches a false result, so an early
// probe can't wrongly pin a Turtle client as non-Turtle.
bool Detected();

// The Turtle client version string (e.g. `"1.18.1"`), or nullptr when not
// Turtle / not yet detected. Points at an internally-cached copy.
const char *Version();

} // namespace Turtle
