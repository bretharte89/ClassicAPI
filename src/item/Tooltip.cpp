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
#include "Offsets.h"
#include "item/Data.h"

#include <cstdint>
#include <cstdio>

namespace Item::Tooltip {

// `GameTooltip:SetItemByID(itemID)` — modern method that renders an
// item tooltip from just an itemID. The 1.12 workaround was to
// construct an item hyperlink and call `SetHyperlink` —
// `tooltip:SetHyperlink("item:" .. id .. ":0:0:0:0:0:0:0")` — which
// works but forces every caller to know the hyperlink format.
//
// Implementation: format the hyperlink string in C and dispatch to the
// existing `Script_GameTooltip_SetHyperlink` (registry slot 12 at
// `0x00531FD0`). Same registration pattern as `SetSpellByID` in
// [src/spell/Tooltip.cpp](src/spell/Tooltip.cpp).
static int __fastcall Script_GameTooltipSetItemByID(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetItemByID(itemID)");
        return 0;
    }
    if (!Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetItemByID(itemID)");
        return 0;
    }
    const int itemID = static_cast<int>(Game::Lua::ToNumber(L, 2));
    if (itemID <= 0)
        return 0;

    // Warm the cache if uncached. The cache-load callback fires
    // `GET_ITEM_INFO_RECEIVED` when data arrives. Addons that want
    // the tooltip to auto-refresh in place should listen for that
    // event and re-call SetItemByID — modern WoW (5.x+) does the
    // same internally; we don't replicate that engine-level hook
    // because the only viable C-side path here is invoking Lua
    // from a network callback, which has its own pitfalls.
    Item::Data::WarmCache(static_cast<uint32_t>(itemID));

    char hyperlink[64];
    std::snprintf(hyperlink, sizeof(hyperlink), "item:%d:0:0:0:0:0:0:0", itemID);

    // Replace stack[2] (the itemID) with the formatted hyperlink, so
    // the existing SetHyperlink sees `(self, "item:...")`.
    Game::Lua::SetTop(L, 1);            // keep self at stack[1]
    Game::Lua::PushString(L, hyperlink); // stack[2] = hyperlink

    using Script_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<Script_t>(Offsets::FUN_SCRIPT_GAMETOOLTIP_SET_HYPERLINK);
    return fn(L);
}

static const Game::Lua::FrameMethodEntry g_methods[] = {
    {"SetItemByID", &Script_GameTooltipSetItemByID},
};

static void RegisterLuaFunctions() {
    Game::Lua::RegisterFrameMethods(
        reinterpret_cast<void *>(Offsets::VAR_GAMETOOLTIP_METHOD_REGISTRY),
        g_methods,
        static_cast<int>(sizeof(g_methods) / sizeof(g_methods[0])));
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Tooltip
