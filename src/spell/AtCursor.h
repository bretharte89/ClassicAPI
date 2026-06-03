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

namespace Spell::AtCursor {

// Resolves the engine's in-flight ground-target placement at the
// player's current cursor world position. Intended to be called
// immediately after initiating a ground-target cast (spell or item)
// to skip the manual click. Decision tree:
//
//   1. No placement active                  → return false (nothing to do)
//   2. Placement is unit-target, not ground → cancel placement, return false
//   3. Cursor not pointed at terrain        → cancel placement, return false
//   4. Cursor on terrain                    → commit at those world coords, return true
//
// Shared by `C_Spell.CastAtCursor` and `C_Item.UseAtCursor`. Safe to
// call from any in-world context — internally gates on a non-NULL
// click-info struct pointer.
bool Resolve();

} // namespace Spell::AtCursor
