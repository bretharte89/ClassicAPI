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

#include "Game.h"

namespace Item::Quality {

namespace {

// Item-quality enum values from `Enum.ItemQuality` (modern WoW). These
// are the integer codes returned in the 4th slot of `GetItemInfo` /
// `C_Item.GetItemInfoInstant`'s quality field, and used by addons to
// gate UI behavior on rarity (e.g., "highlight rare-or-better drops").
//
// Values 0..5 (POOR..LEGENDARY) exist as actual item qualities in
// vanilla 1.12. Higher values (ARTIFACT, HEIRLOOM, WOWTOKEN) were
// introduced in later expansions — we still expose them so that
// addons backporting modern code (`if quality >= LE_ITEM_QUALITY_RARE
// then ...`) compile cleanly. Vanilla items will never carry those
// higher quality values, so the comparisons resolve to the right
// branch automatically.
struct QualityConstant {
    const char *name;
    int value;
};

constexpr QualityConstant kQualityConstants[] = {
    {"LE_ITEM_QUALITY_POOR",      0}, // gray   — junk
    {"LE_ITEM_QUALITY_COMMON",    1}, // white
    {"LE_ITEM_QUALITY_UNCOMMON",  2}, // green
    {"LE_ITEM_QUALITY_RARE",      3}, // blue
    {"LE_ITEM_QUALITY_EPIC",      4}, // purple
    {"LE_ITEM_QUALITY_LEGENDARY", 5}, // orange
    {"LE_ITEM_QUALITY_ARTIFACT",  6}, // gold     — TBC+ only
    {"LE_ITEM_QUALITY_HEIRLOOM",  7}, // light blue — WotLK+ only
    {"LE_ITEM_QUALITY_WOWTOKEN",  8}, // orange   — WoD+ only
};

} // namespace

static void RegisterLuaFunctions() {
    void *L = Game::Lua::State();
    if (L == nullptr)
        return;
    for (const auto &c : kQualityConstants) {
        Game::Lua::PushString(L, c.name);
        Game::Lua::PushNumber(L, static_cast<double>(c.value));
        Game::Lua::SetTable(L, Game::Lua::GLOBALS_INDEX);
    }
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Quality
