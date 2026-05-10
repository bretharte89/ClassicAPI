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
#include "item/ID.h"
#include "item/Location.h"

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

// `GameTooltip:SetInventoryItemByID(itemID)` — modern method that
// renders the tooltip for the **equipped instance** of `itemID`,
// including enchants, random suffix stats, and broken/locked state.
// Distinct from `SetItemByID`, which renders the base ItemSparse
// data (clean, no enchants).
//
// Verified empirically: with run-speed-enchanted boots equipped,
// `SetInventoryItemByID(<bootsID>)` shows the enchant line in
// addition to the base stats; `SetItemByID(<bootsID>)` shows the
// boots without enchants.
//
// Implementation: walk character-pane slots 1..19 looking for an
// equipped item matching `itemID`; on hit, dispatch to the
// engine's existing `Script_GameTooltip_SetInventoryItem`
// (registry slot 19, `0x00532EE0`) with `("player", slot)`. The
// engine's function reads the actual CGItem instance (with
// descriptor flags + applied enchants) for the tooltip.
//
// Silent no-op if the item isn't equipped — caller should fall
// back to `SetItemByID` for unworn items.
static int __fastcall Script_GameTooltipSetInventoryItemByID(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetInventoryItemByID(itemID)");
        return 0;
    }
    if (!Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetInventoryItemByID(itemID)");
        return 0;
    }
    const int itemID = static_cast<int>(Game::Lua::ToNumber(L, 2));
    if (itemID <= 0)
        return 0;

    // Walk in ascending slot order (1..19). When the player has
    // duplicates of the same itemID equipped (matched MH/OH weapons,
    // identical rings, identical trinkets), modern client behavior
    // is to render the lower-numbered slot — MAINHAND (16) before
    // OFFHAND (17), FINGER1 (11) before FINGER2 (12), TRINKET1 (13)
    // before TRINKET2 (14). Confirmed against modern by the user;
    // our ascending walk + first-match-break naturally matches.
    int foundSlot = 0;
    for (int slot = Offsets::EQUIPMENT_SLOT_FIRST;
         slot <= Offsets::EQUIPMENT_SLOT_LAST; ++slot) {
        const uint8_t *item = Item::Location::ResolveEquipmentSlot(slot);
        if (item == nullptr)
            continue;
        if (Item::ID::FromCGItem(item) == itemID) {
            foundSlot = slot;
            break;
        }
    }
    if (foundSlot == 0)
        return 0;

    // Replace the itemID arg with ("player", slot) so the engine's
    // SetInventoryItem dispatcher reads its expected (unit, slot).
    Game::Lua::SetTop(L, 1);  // keep self at stack[1]
    Game::Lua::PushString(L, "player");
    Game::Lua::PushNumber(L, static_cast<double>(foundSlot));

    using Script_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<Script_t>(Offsets::FUN_SCRIPT_GAMETOOLTIP_SET_INVENTORY_ITEM);
    return fn(L);
}

static const Game::Lua::FrameMethodEntry g_methods[] = {
    {"SetItemByID", &Script_GameTooltipSetItemByID},
    {"SetInventoryItemByID", &Script_GameTooltipSetInventoryItemByID},
};

static void RegisterLuaFunctions() {
    Game::Lua::RegisterFrameMethods(
        reinterpret_cast<void *>(Offsets::VAR_GAMETOOLTIP_METHOD_REGISTRY),
        g_methods,
        static_cast<int>(sizeof(g_methods) / sizeof(g_methods[0])));
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Tooltip
