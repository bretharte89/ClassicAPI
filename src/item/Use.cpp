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

// `C_Item.UseItemByName(itemInfo [, unit])` — finds the first item in
// the player's bags matching `itemInfo` (itemID, link, or name) and
// uses it via the engine's `CGItem::UseItem` primitive. Optional
// `unit` is a unit token (`"player"`, `"target"`, `"focus"`, …)
// passed to spell-cast items (scrolls, traps, on-use targeted
// effects) as the cast target.
//
// Same structure 3.3.5's `Script_UseItemByName` uses: locate the item
// directly, then hand it to the engine's by-item-pointer use call.
// We skip `Script_UseContainerItem` — that function is a state-machine
// dispatcher for cursor modes (repair vendor, spell-cast targeting,
// drop-on-bag, ...) that don't apply to an addon-issued call from a
// clean cursor state. The default-path primitive `FUN_ITEM_USE`
// (`0x005D8D00`) handles every item-type-specific branch internally:
// food / potion / hearthstone / scroll / quiver all route through
// the same entry.
//
// Target arg: `FUN_ITEM_USE`'s `param_1` is read as a `uint64_t *`
// pointing at the target's GUID. The engine writes it through to the
// "use item" event (`DAT_00c4c1d0`/`DAT_00c4c1d4` + event 0x122) for
// spell-cast items, and into the cast-targeting helper at
// `FUN_006e5a90`. For self-use items (hearthstone, food, potion) the
// engine overwrites with the item's own GUID before dispatch, so a
// stray target is harmless. We default to a zero GUID when the caller
// omits `unit` — matches existing call sites.
//
// Returns nothing. Silently no-ops on bad input, item not found in
// bags, item locked, on cooldown, etc. An unrecognized `unit` string
// is treated as "no target" rather than raising, matching arg-1's
// "silently no-ops on bad input" contract.

#include "Game.h"
#include "Offsets.h"
#include "item/Arg.h"
#include "item/Location.h"

#include <cstdint>
#include <cstring>

namespace Item::Use {

namespace {

using UseItem_t = unsigned(__thiscall *)(const void *item, const uint64_t *targetGuid,
                                          int flag);

using TokenToGUID_t = uint64_t(__fastcall *)(const char *token);

bool HasPrefix(const char *s, const char *prefix) {
    return std::strncmp(s, prefix, std::strlen(prefix)) == 0;
}

// Walk past any "target" suffixes (`targettarget`, `focustargettarget`,
// …); returns a pointer at the first non-"target" character.
const char *SkipTargetSuffixes(const char *s) {
    while (HasPrefix(s, "target"))
        s += 6;
    return s;
}

// Predicate mirror of the engine's token grammar inside
// `FUN_TOKEN_TO_GUID` (`0x00515970`) plus the families our own
// `Unit::TokenExtensions` hook adds (`focus`, `nameplateN`). The
// engine raises a Lua error on unknown input, so we gate before
// dispatching to keep our "silently no-ops on bad input" contract.
//
// Every base token may be followed by zero or more `target` suffixes
// (`playertarget`, `party1targettarget`, …) which the engine's
// resolver walks via `LAB_005159d3..0x00515A2C`.
bool MatchBase(const char *s, const char *base, bool requireDigits) {
    if (!HasPrefix(s, base))
        return false;
    const char *rest = s + std::strlen(base);
    if (requireDigits) {
        const char *p = rest;
        while (*p >= '0' && *p <= '9')
            ++p;
        if (p == rest)
            return false;
        rest = p;
    }
    return *SkipTargetSuffixes(rest) == '\0';
}

bool LooksLikeToken(const char *s) {
    if (s == nullptr || *s == '\0')
        return false;
    // Order matters for shared roots: longer prefix first
    // (`partypet`/`raidpet` before `party`/`raid`).
    if (MatchBase(s, "partypet", true))   return true;
    if (MatchBase(s, "raidpet", true))    return true;
    if (MatchBase(s, "party", true))      return true;
    if (MatchBase(s, "raid", true))       return true;
    if (MatchBase(s, "nameplate", true))  return true;
    if (MatchBase(s, "mouseover", false)) return true;
    if (MatchBase(s, "player", false))    return true;
    if (MatchBase(s, "target", false))    return true;
    if (MatchBase(s, "focus", false))     return true;
    if (MatchBase(s, "npc", false))       return true;
    if (MatchBase(s, "pet", false))       return true;
    return false;
}

uint64_t ResolveUnitGuid(void *L, int idx) {
    if (!Game::Lua::IsString(L, idx))
        return 0;
    const char *token = Game::Lua::ToString(L, idx);
    if (!LooksLikeToken(token))
        return 0;
    auto fn = reinterpret_cast<TokenToGUID_t>(Offsets::FUN_TOKEN_TO_GUID);
    return fn(token);
}

int __fastcall Script_C_Item_UseItemByName(void *L) {
    const auto arg = Item::Arg::Resolve(L, 1);
    if (arg.itemID <= 0 && arg.name == nullptr) {
        return 0;
    }

    Item::Location::ByGUIDResult found;
    if (!Item::Location::FindByArgInBags(L, arg, &found)) {
        return 0;
    }

    const uint64_t targetGuid = ResolveUnitGuid(L, 2);
    auto useItem = reinterpret_cast<UseItem_t>(Offsets::FUN_ITEM_USE);
    useItem(found.item, &targetGuid, 0);
    return 0;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "UseItemByName", &Script_C_Item_UseItemByName);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Use
