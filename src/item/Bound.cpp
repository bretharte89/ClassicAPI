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

#include "Bound.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Item::Bound {

using GetItemBySlot_t = void *(__thiscall *)(void *thisInvMgr, int slot);
using PackBagSlot_t = int(__fastcall *)(void *L, void **outInvMgr,
                                         int *outLinearSlot, int *outUnused);
using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

static void *ResolveActivePlayerInvMgr() {
    auto ResolveUnitToken =
        reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<uint8_t *>(ResolveUnitToken("player"));
    if (player == nullptr)
        return nullptr;
    return player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
}

static bool ItemIsSoulbound(const uint8_t *item) {
    auto *descriptor = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_DESCRIPTOR);
    if (descriptor == nullptr)
        return false;
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        descriptor + Offsets::OFF_DESCRIPTOR_FLAGS);
    return (flags & Offsets::ITEM_FLAG_SOULBOUND) != 0;
}

// Equipment-slot path: Lua passes 1-based INVSLOT_* indices, but
// ItemMgr::GetItemBySlot expects 0-based (the engine's validators all do
// `dec eax` after lua_tonumber — see helper at 0x004C8520).
static int __fastcall Script_IsBoundEquipment(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.IsBound: equipmentSlotIndex must be a number");
        return 0;
    }

    void *invMgr = ResolveActivePlayerInvMgr();
    if (invMgr == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    const int slot = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    auto *item = static_cast<const uint8_t *>(GetItemBySlot(invMgr, slot));
    if (item == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    Game::Lua::PushBoolean(L, ItemIsSoulbound(item) ? 1 : 0);
    return 1;
}

// Bag path: PackBagSlot reads bagID at stack[1] and slotIndex at stack[2]
// (which is exactly the Lua call convention) and returns both the inventory
// manager and the linearized slot. The third output is unused for our purposes.
static int __fastcall Script_IsBoundBag(void *L) {
    void *invMgr = nullptr;
    int linearSlot = 0;
    int unused = 0;

    auto PackBagSlot = reinterpret_cast<PackBagSlot_t>(Offsets::FUN_PACK_BAG_SLOT);
    if (!PackBagSlot(L, &invMgr, &linearSlot, &unused) || invMgr == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    auto *item = static_cast<const uint8_t *>(GetItemBySlot(invMgr, linearSlot));
    if (item == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }

    Game::Lua::PushBoolean(L, ItemIsSoulbound(item) ? 1 : 0);
    return 1;
}

// Diagnostic — given an equipment slot, returns the item pointer plus the
// first 24 dwords (96 bytes) of the descriptor sub-object at *(item+0x114).
// Soulbound is a bit somewhere in there; comparing a bound vs. not-bound item
// will tell us exactly which offset+bit.
static int __fastcall Script_DebugItem(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: _classicapi_dbg_item(eqSlot)");
        return 0;
    }

    void *invMgr = ResolveActivePlayerInvMgr();
    if (invMgr == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }

    const int slot = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    auto *item = static_cast<const uint8_t *>(GetItemBySlot(invMgr, slot));
    if (item == nullptr) {
        Game::Lua::PushNumber(L, 0);
        return 1;
    }

    auto *descriptor = *reinterpret_cast<const uint8_t *const *>(item + 0x114);

    Game::Lua::PushNumber(L, static_cast<double>(reinterpret_cast<uintptr_t>(item)));
    Game::Lua::PushNumber(L, static_cast<double>(reinterpret_cast<uintptr_t>(descriptor)));

    if (descriptor == nullptr)
        return 2;

    for (int i = 0; i < 24; ++i) {
        Game::Lua::PushNumber(L, static_cast<double>(
            *reinterpret_cast<const uint32_t *>(descriptor + i * 4)));
    }
    return 2 + 24;
}

// We can't register directly into a Lua *table* via the engine's
// FrameScript_RegisterFunction (that only does globals). Workaround: register
// each C function under a temp global, then run a small Lua snippet that wires
// them under C_Item.IsBound as a dispatcher and clears the temps.
void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("_classicapi_C_Item_IsBound_eq",
                                      &Script_IsBoundEquipment);
    Game::Lua::RegisterGlobalFunction("_classicapi_C_Item_IsBound_bag",
                                      &Script_IsBoundBag);
    Game::Lua::RegisterGlobalFunction("_classicapi_dbg_item", &Script_DebugItem);

    Game::FrameScript_Execute(
        "if not C_Item then C_Item = {} end "
        "local eq, bag = _classicapi_C_Item_IsBound_eq, _classicapi_C_Item_IsBound_bag "
        "function C_Item.IsBound(loc) "
        "  if loc.equipmentSlotIndex then return eq(loc.equipmentSlotIndex) end "
        "  return bag(loc.bagID, loc.slotIndex) "
        "end "
        "_classicapi_C_Item_IsBound_eq = nil "
        "_classicapi_C_Item_IsBound_bag = nil",
        "ClassicAPI.lua");
}

} // namespace Item::Bound
