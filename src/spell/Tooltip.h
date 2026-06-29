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

namespace Spell::Tooltip {

// Resolves the `GameTooltip` self at Lua stack[1] (via the
// engine's `FrameScript_PushObject`/`FrameScript_GetObject` chain)
// and dispatches the engine's spell-tooltip builder for `spellID`.
// Silent no-op if `spellID <= 0` or stack[1] isn't a frame object.
//
// Used by `GameTooltip:SetSpellByID` directly, and by
// `GameTooltip:SetTalentByID` as a fallback for cross-class talents
// (where the engine's per-player TalentEntry tables don't apply,
// so we render the rank-1 spell tooltip instead).
void ShowByID(void *L, int spellID);

} // namespace Spell::Tooltip
