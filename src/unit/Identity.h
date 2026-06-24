// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

#pragma once

#include <cstddef>
#include <cstdint>

// Unit identity helpers shared across modules. The Lua surface
// (`UnitGUID` / `UnitTokenFromGUID`) lives in `Identity.cpp`; these are the
// reusable C++ cores other modules call directly (e.g. `Aura::Data`
// resolving an aura's caster GUID to a `sourceUnit` token).

namespace Unit::Identity {

// The local player's 64-bit GUID, read from the canonical local-player
// pointer (`VAR_LOCAL_PLAYER_PTR + OFF_LOCAL_PLAYER_GUID`). Independent of
// object-manager state, so it stays valid across zone transitions / brief
// CGPlayer rebuilds. Returns 0 before login (pointer null) or before the
// GUID is populated. This is the canonical "who am I" read — prefer it over
// re-dereferencing the globals inline.
uint64_t PlayerGuid();

// Resolves a unit token (`"player"`, `"target"`, `"party3"`, …) to its
// 64-bit GUID, or 0 if the token currently maps to nothing. `"player"` is
// read from the canonical local-player pointer so it works regardless of
// object-manager state; everything else goes through the engine's
// token→GUID resolver (so out-of-range party/raid members resolve too).
// NOTE: raises a Lua error for tokens the engine doesn't recognize — only
// pass known unit tokens.
uint64_t GuidForToken(const char *token);

// Best-effort reverse lookup: given a unit GUID, writes the first unit
// token currently mapped to it into `buf` and returns `buf`, or returns
// nullptr if no known token points at it (or `guid` is 0). The mapping is
// unstable across frames — callers should treat the result as a snapshot.
// `buf` should be at least 32 bytes.
const char *TokenFromGUID(uint64_t guid, char *buf, size_t bufSize);

} // namespace Unit::Identity
