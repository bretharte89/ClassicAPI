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

namespace Spell::CraftID {

// `GetCraftSpellID(craftIndex)` — companion to the engine's
// `GetCraftItemLink`, which despite its name returns an `Hspell:`
// link, not `Hitem:`. The "craft" entries in vanilla (enchanting
// formulae, beast-training tomes) are spells with no produced-item
// — the spell IS the deliverable. So unlike the rest of the
// scrape-companion family (which return itemIDs), this one returns
// the recipe's spellID. Addons scraping `GetCraftItemLink` with an
// itemID-parser are already getting nil here in vanilla; this gives
// them the actual identifier the engine uses for the entry.
//
// nil when the craft UI is closed or `craftIndex` is OOR.
static int __fastcall Script_GetCraftSpellID(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetCraftSpellID(craftIndex)");
        return 0;
    }
    const int craftIndex = static_cast<int>(Game::Lua::ToNumber(L, 1)) - 1;
    if (craftIndex < 0)
        return 0;

    const int count = static_cast<int>(*reinterpret_cast<const uint32_t *>(
        Offsets::VAR_CRAFT_COUNT));
    if (craftIndex >= count)
        return 0;

    // `VAR_CRAFT_ENTRIES` is a pointer slot, not the array — the
    // engine reads it via `MOV ECX, [imm32]; MOV EAX, [ECX + idx*4]`.
    auto **entries = *reinterpret_cast<const uint8_t ***>(Offsets::VAR_CRAFT_ENTRIES);
    if (entries == nullptr)
        return 0;
    auto *entry = entries[craftIndex];
    if (entry == nullptr)
        return 0;

    const int spellID = static_cast<int>(*reinterpret_cast<const uint32_t *>(entry));
    if (spellID <= 0)
        return 0;

    Game::Lua::PushNumber(L, static_cast<double>(spellID));
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetCraftSpellID", &Script_GetCraftSpellID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::CraftID
