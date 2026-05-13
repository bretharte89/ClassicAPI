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

namespace Item::Link {

using ScriptFn_t = int(__fastcall *)(void *L);

// Read an int field off the location table at stack `locIdx`. Returns
// true and writes `*out` on a numeric field; leaves the Lua stack as
// it found it.
static bool TryReadIntField(void *L, int locIdx, const char *fieldName, int *out) {
    Game::Lua::PushString(L, fieldName);
    Game::Lua::GetTable(L, locIdx);
    const bool ok = Game::Lua::IsNumber(L, -1);
    if (ok)
        *out = static_cast<int>(Game::Lua::ToNumber(L, -1));
    Game::Lua::SetTop(L, -2);
    return ok;
}

// `C_Item.GetItemLink(itemLocation)` — returns the fully-decorated
// per-instance hyperlink for the item at the location, matching what
// `GetContainerItemLink(bag, slot)` or `GetInventoryItemLink("player",
// slot)` would return for the same slot. Enchant ID, random-suffix,
// and any other per-instance data the server attached to the CGItem
// are preserved.
//
// Implementation: read the location table's (bagID, slotIndex) or
// (equipmentSlotIndex) fields, blow the Lua stack, push the right
// args, and tail-call the engine's link script function
// (`Script_GetContainerItemLink` or `Script_GetInventoryItemLink`).
// The engine reads its args from stack[1]/[2] and pushes the link
// string as its return value — we forward its return count to the
// Lua caller, so a nil push for an empty slot stays nil.
static int __fastcall Script_C_Item_GetItemLink(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemLink(itemLocation)");
        return 0;
    }

    int eqSlot = 0;
    if (TryReadIntField(L, 1, "equipmentSlotIndex", &eqSlot)) {
        Game::Lua::SetTop(L, 0);
        Game::Lua::PushString(L, "player");
        Game::Lua::PushNumber(L, static_cast<double>(eqSlot));
        auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_INVENTORY_ITEM_LINK);
        return fn(L);
    }

    int bagID = 0, slotIndex = 0;
    if (TryReadIntField(L, 1, "bagID", &bagID) &&
        TryReadIntField(L, 1, "slotIndex", &slotIndex)) {
        Game::Lua::SetTop(L, 0);
        Game::Lua::PushNumber(L, static_cast<double>(bagID));
        Game::Lua::PushNumber(L, static_cast<double>(slotIndex));
        auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_CONTAINER_ITEM_LINK);
        return fn(L);
    }

    return 0;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemLink", &Script_C_Item_GetItemLink);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Link
