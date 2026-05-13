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
#include "item/Location.h"

#include <cstdint>

namespace Item::RepairCost {

namespace {

using RepairCostFn = int (__fastcall *)(void *item);
static const auto ComputeRepairCost = reinterpret_cast<RepairCostFn>(
    static_cast<uintptr_t>(Offsets::FUN_ITEM_REPAIR_COST));

// Pushes one int return (copper cost). 0 means "no repair needed" — the
// engine's helper returns 0 for null items, broken items, items without a
// durability concept, or fully-repaired items. We forward 0 in those cases
// rather than returning nothing, because the caller asked "what's the
// repair cost" and 0 is a meaningful answer ("nothing to repair").
static int PushRepairCostForItem(void *L, const uint8_t *item) {
    if (item == nullptr) {
        Game::Lua::PushNumber(L, 0.0);
        return 1;
    }
    const int cost = ComputeRepairCost(const_cast<uint8_t *>(item));
    Game::Lua::PushNumber(L, static_cast<double>(cost));
    return 1;
}

// `GetInventoryItemRepairCost(invSlot)` — returns the cost in copper to
// repair the player's equipped item at `invSlot` (1-based). Matches the
// third return value of `GameTooltip:SetInventoryItem`.
//
// The faction-rep + PvP-rank discount is only applied when a merchant
// window is currently open (the engine looks up the merchant GUID from
// DAT_00BDDFA0/A4 — populated by SMSG_LIST_INVENTORY, zeroed on
// merchant frame close). Outside a vendor interaction the engine
// short-circuits the discount path and we get the raw base cost.
//
// Player-only; mirrors the shape of our existing
// `GetInventoryItemDurability`. Returns 0 for empty slots or items
// that don't need repair.
static int __fastcall Script_GetInventoryItemRepairCost(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetInventoryItemRepairCost(invSlot)");
        return 0;
    }
    const int slot1Based = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (slot1Based < 1) {
        Game::Lua::PushNumber(L, 0.0);
        return 1;
    }
    return PushRepairCostForItem(L, Item::Location::ResolveEquipmentSlot(slot1Based));
}

// `C_Container.GetContainerItemRepairCost(containerIndex, slotIndex)` —
// bag/bank variant. Same engine helper, just a different slot-resolution
// chain. The engine's `RepairAllCost` (`FUN_004FBD60`) walks both
// equipment slots and bag contents through the same per-item function,
// so this is the same cost the merchant would charge for that item.
static int __fastcall Script_C_Container_GetContainerItemRepairCost(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L,
            "Usage: C_Container.GetContainerItemRepairCost(containerIndex, slotIndex)");
        return 0;
    }
    const int bagID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const int slotIndex = static_cast<int>(Game::Lua::ToNumber(L, 2));
    return PushRepairCostForItem(L, Item::Location::ResolveBag(L, bagID, slotIndex));
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetInventoryItemRepairCost",
                                      &Script_GetInventoryItemRepairCost);
    Game::Lua::RegisterTableFunction("C_Container", "GetContainerItemRepairCost",
                                     &Script_C_Container_GetContainerItemRepairCost);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::RepairCost
