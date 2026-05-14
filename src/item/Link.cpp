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

namespace Item::Link {

namespace {

using ScriptFn_t = int(__fastcall *)(void *L);

bool TryReadIntField(void *L, int locIdx, const char *fieldName, int *out) {
    Game::Lua::PushString(L, fieldName);
    Game::Lua::GetTable(L, locIdx);
    const bool ok = Game::Lua::IsNumber(L, -1);
    if (ok)
        *out = static_cast<int>(Game::Lua::ToNumber(L, -1));
    Game::Lua::SetTop(L, -2);
    return ok;
}

// Stomps the stack and tail-calls `Script_GetInventoryItemLink`
// with `("player", eqSlot)`. The engine pushes the dressed-link
// string and returns 1; we forward that count.
int DispatchEquipmentLink(void *L, int eqSlot) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushString(L, "player");
    Game::Lua::PushNumber(L, static_cast<double>(eqSlot));
    auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_INVENTORY_ITEM_LINK);
    return fn(L);
}

// Same shape as the equipment dispatcher, but for bag/slot via
// `Script_GetContainerItemLink`.
int DispatchBagLink(void *L, int bagID, int slotIndex) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(bagID));
    Game::Lua::PushNumber(L, static_cast<double>(slotIndex));
    auto fn = reinterpret_cast<ScriptFn_t>(Offsets::FUN_SCRIPT_GET_CONTAINER_ITEM_LINK);
    return fn(L);
}

} // namespace

// `C_Item.GetItemLink(itemLocation)` — returns the fully-decorated
// per-instance hyperlink for the item at the location, matching what
// `GetContainerItemLink(bag, slot)` or `GetInventoryItemLink("player",
// slot)` would return for the same slot. Enchant ID, random-suffix,
// and any other per-instance data the server attached to the CGItem
// are preserved.
//
// Accepts table form (`{equipmentSlotIndex=N}` or `{bagID=B,
// slotIndex=S}`) and GUID string form (the value
// `C_Item.GetItemGUID` returns). For the GUID form we walk
// equipment + bags to find the matching CGItem, then dispatch to
// the same engine helper using the recovered slot tuple.
//
// Implementation: blow the Lua stack, push the right args, and
// tail-call the engine's link script function
// (`Script_GetContainerItemLink` or `Script_GetInventoryItemLink`).
// The engine reads its args from stack[1]/[2] and pushes the link
// string as its return value — we forward its return count to the
// Lua caller, so a nil push for an empty slot stays nil.
static int __fastcall Script_C_Item_GetItemLink(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemLink(itemLocation)");
        return 0;
    }

    // GUID-string form: walk to find the resident slot, then dispatch
    // by whichever kind of slot it lives in.
    if (Game::Lua::Type(L, 1) == Game::Lua::TYPE_STRING) {
        uint64_t guid = 0;
        if (!Item::Location::ParseGUIDString(Game::Lua::ToString(L, 1), &guid))
            return 0;
        Item::Location::ByGUIDResult found;
        if (!Item::Location::FindByGUID(L, guid, &found))
            return 0;
        if (found.equipmentSlotIndex != 0)
            return DispatchEquipmentLink(L, found.equipmentSlotIndex);
        return DispatchBagLink(L, found.bagID, found.slotIndex);
    }

    int eqSlot = 0;
    if (TryReadIntField(L, 1, "equipmentSlotIndex", &eqSlot))
        return DispatchEquipmentLink(L, eqSlot);

    int bagID = 0, slotIndex = 0;
    if (TryReadIntField(L, 1, "bagID", &bagID) &&
        TryReadIntField(L, 1, "slotIndex", &slotIndex))
        return DispatchBagLink(L, bagID, slotIndex);

    return 0;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemLink", &Script_C_Item_GetItemLink);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Link
