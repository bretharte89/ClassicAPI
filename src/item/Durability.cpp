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

#include <cstdint>

namespace Item::Durability {

namespace {

using GetItemBySlot_t = void *(__thiscall *)(void *invMgr, int slot);
using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

// `GetInventoryItemDurability(invSlot)` — returns `current, maximum` for the
// player's equipped item at `invSlot` (1-based), or nothing if the slot is
// empty or the item has no durability (consumables, materials, etc.).
//
// Same field-access path `Script_GetInventoryItemBroken` uses (`0x004C8590`):
// resolve player → invMgr at `+0x1D38` → `GetItemBySlot(slot - 1)` → CGItem's
// m_objectFields descriptor at `+0x114` → read DURABILITY at `+0xA0` and
// MAX_DURABILITY at `+0xA4` as plain dwords.
//
// Player-only by design (matches modern API). Modern semantics: returns
// nothing — not (0, 0) — when `max == 0`. The 3.3.5 implementation at
// `0x005EA170` does the same: tests max via the GetMaxDurability getter and
// returns 0 values if zero.
static int __fastcall Script_GetInventoryItemDurability(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetInventoryItemDurability(invSlot)");
        return 0;
    }
    const int slot1Based = static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (slot1Based < 1)
        return 0;

    auto ResolveUnitToken =
        reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<uint8_t *>(ResolveUnitToken("player"));
    if (player == nullptr)
        return 0;

    auto *invMgr = player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
    auto GetItemBySlot = reinterpret_cast<GetItemBySlot_t>(
        Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    auto *item = static_cast<const uint8_t *>(GetItemBySlot(invMgr, slot1Based - 1));
    if (item == nullptr)
        return 0;

    auto *descriptor = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_DESCRIPTOR);
    if (descriptor == nullptr)
        return 0;

    const uint32_t cur = *reinterpret_cast<const uint32_t *>(
        descriptor + Offsets::OFF_DESCRIPTOR_DURABILITY);
    const uint32_t max = *reinterpret_cast<const uint32_t *>(
        descriptor + Offsets::OFF_DESCRIPTOR_MAX_DURABILITY);
    if (max == 0)
        return 0;

    Game::Lua::PushNumber(L, static_cast<double>(cur));
    Game::Lua::PushNumber(L, static_cast<double>(max));
    return 2;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetInventoryItemDurability",
                                      &Script_GetInventoryItemDurability);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Durability
