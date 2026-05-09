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

#include "Location.h"

#include "../Game.h"
#include "../Offsets.h"

namespace Item::Location {

namespace {

using GetItemBySlot_t = void *(__thiscall *)(void *thisInvMgr, int slot);
using PackBagSlot_t = int(__fastcall *)(void *L, void **outInvMgr, int *outLinearSlot,
                                         int *outUnused);
using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

void *ResolveActivePlayerInvMgr() {
    auto ResolveUnitToken =
        reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<uint8_t *>(ResolveUnitToken("player"));
    if (player == nullptr)
        return nullptr;
    return player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
}

// Reads `loc.fieldName` and returns it as an int. Returns false via the
// boolean result if the field is missing or non-numeric. Always leaves the
// Lua stack as it found it.
bool TryReadIntField(void *L, int locIdx, const char *fieldName, int *out) {
    Game::Lua::PushString(L, fieldName);
    Game::Lua::GetTable(L, locIdx); // pops key, pushes value
    const bool ok = Game::Lua::IsNumber(L, -1);
    if (ok)
        *out = static_cast<int>(Game::Lua::ToNumber(L, -1));
    Game::Lua::SetTop(L, -2); // pop value (or nil)
    return ok;
}

const uint8_t *ResolveBagSlot(void *L, int bagID, int slotIndex) {
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
        return nullptr;

    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    return static_cast<const uint8_t *>(GetItemBySlot(invMgr, linearSlot));
}

} // namespace

const uint8_t *ResolveEquipmentSlot(int slot1Based) {
    void *invMgr = ResolveActivePlayerInvMgr();
    if (invMgr == nullptr)
        return nullptr;
    // GetItemBySlot expects 0-based linearized slot indices. The built-in
    // Lua functions all do `dec eax` on the slot argument before calling —
    // see helper at 0x004C8520.
    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    return static_cast<const uint8_t *>(GetItemBySlot(invMgr, slot1Based - 1));
}

const uint8_t *ResolveBag(void *L, int bagID, int slotIndex) {
    return ResolveBagSlot(L, bagID, slotIndex);
}

const uint8_t *Resolve(void *L, int locIdx) {
    if (Game::Lua::Type(L, locIdx) != Game::Lua::TYPE_TABLE)
        return nullptr;

    int eqSlot = 0;
    if (TryReadIntField(L, locIdx, "equipmentSlotIndex", &eqSlot))
        return ResolveEquipmentSlot(eqSlot);

    int bagID = 0, slotIndex = 0;
    if (TryReadIntField(L, locIdx, "bagID", &bagID) &&
        TryReadIntField(L, locIdx, "slotIndex", &slotIndex))
        return ResolveBagSlot(L, bagID, slotIndex);

    return nullptr;
}

} // namespace Item::Location
