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

// Persistent debug log shared by all probe modules. Writes to
// `C:\Git\ClassicAPI\debug.log` so it can be read directly from the
// repo by tooling/agents — the file is gitignored. Used during
// offset-finding sessions where we need to compare the same dump
// across multiple in-game states (e.g., baseline / mounted /
// stealthed) without having to paste each result back into chat.
//
// Each Append call writes a labeled section:
//   === <label> ===
//   <content>
//   <blank line>
//
// `Clear` truncates the file. Pair Clear() at the start of a probe
// session with one Append() per state.

namespace Debug::Log {

// Truncates the debug log file. Safe to call before any Append.
void Clear();

// Appends a labeled block. Both args are nul-terminated C strings;
// `label` may be nullptr (rendered as "(no label)") and `content`
// may be nullptr (rendered as empty). No formatting is applied to
// `content` — it's written verbatim, so the caller controls layout
// (newlines, hex prefixes, etc.).
void Append(const char *label, const char *content);

} // namespace Debug::Log
