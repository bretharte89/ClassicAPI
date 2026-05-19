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

namespace Expansion {

// The DLL is built against vanilla 1.12 offsets, so the live engine is
// always `LE_EXPANSION_CLASSIC`. Published to Lua three ways:
//   - as `LE_EXPANSION_LEVEL_CURRENT` (see [[expansion/Constants.cpp]])
//   - as `GetClassicExpansionLevel()`'s return (see [[expansion/Level.cpp]])
//   - as the basis for `ClassicExpansionAt{Least,Most}` comparisons
// All three need to agree, so the constant lives here.
constexpr int kCurrentExpansionLevel = 0;

} // namespace Expansion
