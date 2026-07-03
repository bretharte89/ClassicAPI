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

namespace Tooltip::SetItemEvent {

// RAII guard that suppresses OnTooltipSetItem firing for the duration of an
// engine item-build we invoke *internally* (Tooltip::Compare building the
// equipped item into a shopping tooltip). Without it, a handler would run in
// the middle of our line-shift / delta rendering and see a half-built tooltip.
// Nestable (ref-counted).
struct Suppressor {
    Suppressor();
    ~Suppressor();
    Suppressor(const Suppressor &) = delete;
    Suppressor &operator=(const Suppressor &) = delete;
};

} // namespace Tooltip::SetItemEvent
