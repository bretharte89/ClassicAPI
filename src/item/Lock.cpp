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

// `ITEM_FIELD_FLAGS` bit 2 — the "currently in a transaction" lock the
// server sets in response to pickup, trade-attach, mail-attach, etc.
// Same bit `EquipmentSet::Api::Script_EquipmentSetContainsLockedItems`
// reads off the descriptor.
static constexpr uint32_t ITEM_FLAG_LOCKED = 0x04;

// `C_Item.IsLocked(itemLocation)` — true iff the equipped/bagged item
// is in a server-side locked state (mid-trade, mid-mail-attach, picked
// up onto the cursor, etc.). Direct descriptor read: CGItem →
// `OFF_ITEM_DESCRIPTOR` → `OFF_DESCRIPTOR_FLAGS` → bit 2.
//
// 1.12 has no public Lua surface to *set* this flag — `C_Item.LockItem`
// and `C_Item.UnlockItem` aren't provided. The flag is exclusively
// server-driven via the `SMSG_ITEM_TIME_UPDATE` / `SMSG_ITEM_PUSH_RESULT`
// packet family. Callers that backport modern transaction-state code
// can still read this flag to gate UI actions; they just can't request
// the lock themselves.
static int __fastcall Script_C_Item_IsLocked(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const uint8_t *item = Item::Location::Resolve(L, 1);
    if (item == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    auto *descriptor = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_DESCRIPTOR);
    if (descriptor == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        descriptor + Offsets::OFF_DESCRIPTOR_FLAGS);
    Game::Lua::PushBoolean(L, (flags & ITEM_FLAG_LOCKED) != 0 ? 1 : 0);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "IsLocked", &Script_C_Item_IsLocked);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Lock
