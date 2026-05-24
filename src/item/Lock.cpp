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

namespace Item::Lock {

// `C_Item.IsLocked(itemLocation)` — true iff the item is in the
// "transaction-in-progress" lock state (mid-trade, mid-mail-attach,
// picked up onto the cursor, etc.). Reads the CGItem instance flag at
// `+OFF_ITEM_CLIENT_LOCK` bit 0 — the same bit `Script_PickupContainerItem`
// and friends OR'd in via `FUN_004953E0` after building the cursor
// state, and the bit the SMSG_UPDATE_OBJECT post-processor clears via
// `FUN_00495420` once the server confirms the transaction.
//
// Earlier versions of this file read `descriptor[+0x3C] & 0x4` instead
// (claiming bit 2 of ITEM_FIELD_FLAGS was the lock). That was borrowed
// from modern WoW's encoding; vanilla 1.12's actual lock lives at the
// instance offset, not in m_objectFields. Vanilla's `ITEM_LOCK_CHANGED`
// event (engine ID 0xBC) fires on every change of this bit — addons
// that want push notifications can listen for it directly without
// polling.
static int __fastcall Script_C_Item_IsLocked(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    const uint8_t *item = Item::Location::Resolve(L, 1);
    if (item == nullptr) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        item + Offsets::OFF_ITEM_CLIENT_LOCK);
    Game::Lua::PushBool(L, (flags & Offsets::ITEM_CLIENT_LOCK_BIT) != 0);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "IsLocked", &Script_C_Item_IsLocked);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Lock
