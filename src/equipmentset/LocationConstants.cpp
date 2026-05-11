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

// `ITEM_INVENTORY_LOCATION_*` constants — the bit layout used by
// `C_EquipmentSet.GetItemLocations` return codes. Matches Blizzard
// FrameXML's `Blizzard_FrameXMLBase/Classic/Constants.lua` so addon
// code copy-pasted from modern FrameXML / EquipmentManager.lua works
// unchanged on 1.12.
//
// Exposed as Lua globals (numbers) rather than under a table because
// that's how Blizzard ships them — no namespace, just constants. All
// fit comfortably in a Lua 5.0 double; max value here is `0x00400000`
// = 4,194,304, well under the 2^53 exact-integer range. No bit-
// library or 64-bit arithmetic needed to use them — addons combine
// these with `bit.band` / `bit.rshift` / `bit.lshift`, all of which
// the 1.12 engine's built-in `bit` library provides.

#include "Game.h"

namespace EquipmentSet::Constants {

namespace {

struct LocationConstant {
    const char *name;
    int value;
};

constexpr LocationConstant kLocationConstants[] = {
    {"ITEM_INVENTORY_LOCATION_PLAYER",  0x00100000},
    {"ITEM_INVENTORY_LOCATION_BAGS",    0x00200000},
    {"ITEM_INVENTORY_LOCATION_BANK",    0x00400000},
    {"ITEM_INVENTORY_BAG_BIT_OFFSET",   8},
    // Bank bag IDs (5..10) are packed as internal 1..6 in the bag
    // field; `EquipmentManager_UnpackLocation` adds this constant
    // back at unpack time to recover the user-facing 5..10 range.
    {"ITEM_INVENTORY_BANK_BAG_OFFSET",  4},
};

} // namespace

static void RegisterLuaFunctions() {
    void *L = Game::Lua::State();
    if (L == nullptr)
        return;
    for (const auto &c : kLocationConstants) {
        Game::Lua::PushString(L, c.name);
        Game::Lua::PushNumber(L, static_cast<double>(c.value));
        Game::Lua::SetTable(L, Game::Lua::GLOBALS_INDEX);
    }
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace EquipmentSet::Constants
