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

#include "Set.h"

#include <vector>

namespace EquipmentSet::Storage {

// Resolves the per-character file path. Returns an empty string until
// the engine has populated all three session globals (account, realm,
// character). Path format: `WTF\Account\<acct>\<realm>\<char>\
// ClassicAPI_EquipmentSets.txt`. Once the engine has the player in
// world the values are stable for the rest of the session, so callers
// can cache after the first non-empty return.
std::string ResolveFilePath();

// Reads the file at `path` and overwrites `outSets` with the parsed
// contents. Returns true on success (including "file doesn't exist
// yet, leave outSets empty"). False on malformed content.
bool Load(const std::string &path, std::vector<Set> *outSets);

// Atomic write: serializes `sets` to `<path>.tmp` then renames into
// place. Returns false if the write or rename failed.
bool Save(const std::string &path, const std::vector<Set> &sets);

} // namespace EquipmentSet::Storage
