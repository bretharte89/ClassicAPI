// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.

#pragma once

#include <cstdint>

namespace Unit::Flags {

// Tests `UNIT_FIELD_FLAGS & UNIT_FLAG_PLAYER_CONTROLLED` on the unit's
// `m_objectFields` descriptor — same predicate
// `Script_UnitPlayerControlled` (`0x00516410`) implements. Required
// gate before dereferencing the CGPlayer-side sub-struct at
// `unit + OFF_CGPLAYER_INFO` (= +0xE68) because that slot is
// uninitialized for non-player CGUnit subclasses (CGCreature_C
// instances) and reading it AVs on the subsequent deref.
//
// Returns `false` for `nullptr` units and units with no
// descriptor pointer yet (transient pre-spawn / out-of-sync state).
bool IsPlayerControlled(const uint8_t *unit);

} // namespace Unit::Flags
