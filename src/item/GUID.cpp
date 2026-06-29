// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

#include "Game.h"
#include "Offsets.h"
#include "guid/Guid.h"
#include "item/Location.h"

#include <cstdint>

namespace Item::GUID {

// `C_Item.GetItemGUID(itemLocation)` — returns the per-instance 64-bit
// GUID of the item at the location formatted as `"0xHHHHHHHHLLLLLLLL"`
// (16 hex digits, hi dword first). Same format `UnitGUID` returns —
// 1.12 GUIDs are plain qwords with no `"Item-Server-..."`-style prefix
// scheme (that's 6.x+). The format is stable per-character-session
// and survives bag moves, so addons can use it as a slot-independent
// identifier for a specific item instance.
//
// Reads CGItem instance block at `+0x08` → GUID qword at `+0x00`. No
// cache lookup; works for any inventory or equipment item.
//
// Returns nil if the location is invalid, empty, or the instance block
// is somehow missing (shouldn't happen for live items).
static int __fastcall Script_C_Item_GetItemGUID(void *L) {
    if (!Item::Location::IsLocationArg(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemGUID(itemLocation)");
        return 0;
    }
    const uint8_t *item = Item::Location::Resolve(L, 1);
    if (item == nullptr)
        return 0;
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    const uint64_t guid = *reinterpret_cast<const uint64_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_GUID);
    if (guid == 0)
        return 0;

    char buf[Guid::STRING_SIZE];
    Game::Lua::PushString(L, Guid::FormatAsString(guid, buf, sizeof buf));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemGUID", &Script_C_Item_GetItemGUID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::GUID
