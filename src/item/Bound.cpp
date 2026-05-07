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

// Reads `loc.fieldName` and returns it as an int. Returns false via the
// boolean result if the field is missing or non-numeric. Always leaves the
// Lua stack as it found it.
static bool TryReadIntField(void *L, int locIdx, const char *fieldName, int *out) {
    Game::Lua::PushString(L, fieldName);
    Game::Lua::GetTable(L, locIdx); // pops key, pushes value
    const bool ok = Game::Lua::IsNumber(L, -1);
    if (ok)
        *out = static_cast<int>(Game::Lua::ToNumber(L, -1));
    Game::Lua::SetTop(L, -2); // pop value (or nil)
    return ok;
}

static bool LookupEquipment(int luaSlot, bool *outBound) {
    void *invMgr = ResolveActivePlayerInvMgr();
    if (invMgr == nullptr)
        return false;
    // GetItemBySlot expects 0-based linearized slot indices. The built-in
    // Lua functions all do `dec eax` on the slot argument before calling —
    // see helper at 0x004C8520.
    const int slot = luaSlot - 1;
    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    auto *item = static_cast<const uint8_t *>(GetItemBySlot(invMgr, slot));
    *outBound = (item != nullptr) && ItemIsSoulbound(item);
    return true;
}

static bool LookupBag(void *L, int bagID, int slotIndex, bool *outBound) {
    // PackBagSlot reads bagID/slotIndex from Lua stack[1] and stack[2] (it's
    // designed to be called from a Lua-callback context like
    // Script_GetContainerItemInfo). Replace the stack so positions 1 and 2
    // hold the raw values for PackBagSlot's internal lua_tonumber reads.
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bagID));
    Game::Lua::PushNumber(L, static_cast<double>(slotIndex));

    void *invMgr = nullptr;
    int linearSlot = 0;
    int unused = 0;
    auto PackBagSlot = reinterpret_cast<PackBagSlot_t>(Offsets::FUN_PACK_BAG_SLOT);
    if (!PackBagSlot(L, &invMgr, &linearSlot, &unused) || invMgr == nullptr)
        return false;

    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    auto *item = static_cast<const uint8_t *>(GetItemBySlot(invMgr, linearSlot));
    *outBound = (item != nullptr) && ItemIsSoulbound(item);
    return true;
}

static int __fastcall Script_IsBound(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: C_Item.IsBound(itemLocation)");
        return 0;
    }

    bool bound = false;
    int eqSlot = 0;
    if (TryReadIntField(L, 1, "equipmentSlotIndex", &eqSlot)) {
        LookupEquipment(eqSlot, &bound);
        Game::Lua::PushBoolean(L, bound ? 1 : 0);
        return 1;
    }

    int bagID = 0, slotIndex = 0;
    if (TryReadIntField(L, 1, "bagID", &bagID) &&
        TryReadIntField(L, 1, "slotIndex", &slotIndex)) {
        LookupBag(L, bagID, slotIndex, &bound);
        Game::Lua::PushBoolean(L, bound ? 1 : 0);
        return 1;
    }

    Game::Lua::PushBoolean(L, 0);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "IsBound", &Script_IsBound);
}

} // namespace Item::Bound
