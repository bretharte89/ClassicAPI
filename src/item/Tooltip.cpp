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
#include "item/Data.h"
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>
#include <cstdio>

namespace Item::Tooltip {

// `GameTooltip:SetItemByID(itemID)` — modern method that renders an
// item tooltip from just an itemID. The 1.12 workaround was to
// construct an item hyperlink and call `SetHyperlink` —
// `tooltip:SetHyperlink("item:" .. id .. ":0:0:0:0:0:0:0")` — which
// works but forces every caller to know the hyperlink format.
//
// Implementation: format the hyperlink string in C and dispatch to the
// existing `Script_GameTooltip_SetHyperlink` (registry slot 12 at
// `0x00531FD0`). Same registration pattern as `SetSpellByID` in
// [src/spell/Tooltip.cpp](src/spell/Tooltip.cpp).
static int __fastcall Script_GameTooltipSetItemByID(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetItemByID(itemID)");
        return 0;
    }
    if (!Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetItemByID(itemID)");
        return 0;
    }
    const int itemID = static_cast<int>(Game::Lua::ToNumber(L, 2));
    if (itemID <= 0)
        return 0;

    // Warm the cache if uncached. The cache-load callback fires
    // `GET_ITEM_INFO_RECEIVED` when data arrives. Addons that want
    // the tooltip to auto-refresh in place should listen for that
    // event and re-call SetItemByID — modern WoW (5.x+) does the
    // same internally; we don't replicate that engine-level hook
    // because the only viable C-side path here is invoking Lua
    // from a network callback, which has its own pitfalls.
    Item::Data::WarmCache(static_cast<uint32_t>(itemID));

    char hyperlink[64];
    std::snprintf(hyperlink, sizeof(hyperlink), "item:%d:0:0:0:0:0:0:0", itemID);

    // Replace stack[2] (the itemID) with the formatted hyperlink, so
    // the existing SetHyperlink sees `(self, "item:...")`.
    Game::Lua::SetTop(L, 1);            // keep self at stack[1]
    Game::Lua::PushString(L, hyperlink); // stack[2] = hyperlink

    using Script_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<Script_t>(Offsets::FUN_SCRIPT_GAMETOOLTIP_SET_HYPERLINK);
    return fn(L);
}

// `GameTooltip:SetInventoryItemByID(itemID)` — modern method that
// renders the tooltip for the **equipped instance** of `itemID`,
// including enchants, random suffix stats, and broken/locked state.
// Distinct from `SetItemByID`, which renders the base ItemSparse
// data (clean, no enchants).
//
// Verified empirically: with run-speed-enchanted boots equipped,
// `SetInventoryItemByID(<bootsID>)` shows the enchant line in
// addition to the base stats; `SetItemByID(<bootsID>)` shows the
// boots without enchants.
//
// Implementation: walk character-pane slots 1..19 looking for an
// equipped item matching `itemID`; on hit, dispatch to the
// engine's existing `Script_GameTooltip_SetInventoryItem`
// (registry slot 19, `0x00532EE0`) with `("player", slot)`. The
// engine's function reads the actual CGItem instance (with
// descriptor flags + applied enchants) for the tooltip.
//
// Silent no-op if the item isn't equipped — caller should fall
// back to `SetItemByID` for unworn items.
static int __fastcall Script_GameTooltipSetInventoryItemByID(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetInventoryItemByID(itemID)");
        return 0;
    }
    if (!Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetInventoryItemByID(itemID)");
        return 0;
    }
    const int itemID = static_cast<int>(Game::Lua::ToNumber(L, 2));
    if (itemID <= 0)
        return 0;

    // Walk in ascending slot order (1..19). When the player has
    // duplicates of the same itemID equipped (matched MH/OH weapons,
    // identical rings, identical trinkets), modern client behavior
    // is to render the lower-numbered slot — MAINHAND (16) before
    // OFFHAND (17), FINGER1 (11) before FINGER2 (12), TRINKET1 (13)
    // before TRINKET2 (14). Verified empirically against the modern
    // client; our ascending walk + first-match-break naturally
    // matches.
    int foundSlot = 0;
    for (int slot = Offsets::EQUIPMENT_SLOT_FIRST;
         slot <= Offsets::EQUIPMENT_SLOT_LAST; ++slot) {
        const uint8_t *item = Item::Location::ResolveEquipmentSlot(slot);
        if (item == nullptr)
            continue;
        if (Item::ID::FromCGItem(item) == itemID) {
            foundSlot = slot;
            break;
        }
    }
    if (foundSlot == 0)
        return 0;

    // Replace the itemID arg with ("player", slot) so the engine's
    // SetInventoryItem dispatcher reads its expected (unit, slot).
    Game::Lua::SetTop(L, 1);  // keep self at stack[1]
    Game::Lua::PushString(L, "player");
    Game::Lua::PushNumber(L, static_cast<double>(foundSlot));

    using Script_t = int(__fastcall *)(void *L);
    auto fn = reinterpret_cast<Script_t>(Offsets::FUN_SCRIPT_GAMETOOLTIP_SET_INVENTORY_ITEM);
    return fn(L);
}

// Same `(L) → CFrameScriptObject *` resolution prologue that the
// engine's own SetX methods use — we don't have a public helper for
// this (Spell::Tooltip has a copy in its TU), so duplicate it here
// rather than expose Spell::Tooltip's private helper. The asm is the
// well-trodden trio: FrameScript_PushObject(L,1,0); GetObject();
// lua_settop(L,-2).
static constexpr uintptr_t addrFrameScriptPushObject = Offsets::FUN_FRAMESCRIPT_PUSH_OBJECT;
static constexpr uintptr_t addrFrameScriptGetObject = Offsets::FUN_FRAMESCRIPT_GET_OBJECT;
static constexpr uintptr_t addrLuaSetTop = Offsets::LUA_SET_TOP;
static void *ResolveTooltipObject(void *L) {
    void *result = nullptr;
    __asm {
        push 0
        mov  edx, 1
        mov  ecx, L
        mov  eax, addrFrameScriptPushObject
        call eax

        or   edx, -1
        mov  ecx, L
        mov  eax, addrFrameScriptGetObject
        call eax
        mov  result, eax

        mov  edx, -2
        mov  ecx, L
        mov  eax, addrLuaSetTop
        call eax
    }
    return result;
}

// Modern `Enum.ItemQuality` color codes for the SetItemByID fallback
// path (no CGItem → no engine link helper). For the CGItem-resolved
// path we delegate to the engine's own builder, which uses the same
// palette internally.
static const char *QualityHex(int quality) {
    static const char *const kHex[] = {
        "9d9d9d", // POOR
        "ffffff", // COMMON
        "1eff00", // UNCOMMON
        "0070dd", // RARE
        "a335ee", // EPIC
        "ff8000", // LEGENDARY
        "e6cc80", // ARTIFACT (TBC+)
        "00ccff", // HEIRLOOM (WotLK+)
    };
    if (quality < 0 || quality > 7)
        return kHex[1];
    return kHex[quality];
}

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                      const uint64_t *guid, void *callback,
                                                      void *userData, int unused);

static const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

using ResolveObjectByGuid_t = void *(__fastcall *)(int type, const char *debugName,
                                                    uint32_t guidLo, uint32_t guidHi,
                                                    int priority);
using BuildItemLink_t = const char *(__fastcall *)(void *cgItem);

// Calls the engine's CGItem → dressed-link builder at FUN_0052AE00.
// Reads itemID/quality off the instance block, permanent enchant /
// random-properties seed / random factor off the descriptor, and
// builds the suffix-decorated name (e.g. "Cowl of Nature's Breath")
// internally — all in one call. Returns a pointer to the engine's
// long-lived global string buffer; safe to PushString immediately
// (Lua copies). nullptr if the CGItem is null or its instance block
// can't be read.
static const char *BuildLinkForCGItem(void *cgItem) {
    if (cgItem == nullptr)
        return nullptr;
    auto fn = reinterpret_cast<BuildItemLink_t>(Offsets::FUN_GAMETOOLTIP_BUILD_ITEM_LINK);
    return fn(cgItem);
}

// Resolves a stored item GUID into a CGItem via the engine's own
// resolver. Same path Item::Count uses for direct bank reads — no
// gating, no inventory walk, works for any object the engine has
// loaded (player bags, equipment, merchant items, loot, trade,
// mailbox, etc.).
static void *ResolveItemByGuid(uint32_t guidLo, uint32_t guidHi) {
    if (guidLo == 0 && guidHi == 0)
        return nullptr;
    auto fn = reinterpret_cast<ResolveObjectByGuid_t>(Offsets::FUN_OBJECT_RESOLVE_BY_GUID);
    return fn(Offsets::OBJ_TYPE_ITEM, "GameTooltip:GetItem", guidLo, guidHi, 0x172);
}

// `GameTooltip:GetItem()` → (name, link, itemID) for whichever item
// the tooltip is currently displaying. The third return is a
// non-modern addition — modern WoW only returns (name, link) and
// expects addons to parse the itemID out of the link. Returning it
// directly saves callers a gsub round.
//
// BuildItemTooltip stores two fields per Set* call:
//   - tooltip+0x398 → itemID         (always populated for any item path)
//   - tooltip+0x380/+0x384 → item GUID (populated only when there's a
//                                       real CGItem — SetBagItem,
//                                       SetInventoryItem, SetLootItem,
//                                       SetMerchantItem, etc. Zero for
//                                       SetItemByID / SetHyperlink
//                                       which have no instance.)
// Both are zeroed by the per-tooltip Clear at FUN_00530050.
//
// Two paths for the link:
//   - GUID non-zero → resolve to CGItem and dispatch to the engine's
//     own link builder at FUN_0052AE00. This produces the full dressed
//     link with enchant ID, random suffix factor, unique ID, and
//     random-suffix-decorated name. Same output as
//     GetInventoryItemLink / GetContainerItemLink for that item.
//   - GUID zero → SetItemByID-style tooltip; we have no per-instance
//     data, so build a basic colored link from the cached itemID and
//     quality (`|cff..|Hitem:N:0:0:0:0:0:0:0|h[Name]|h|r`).
//
// Returns nothing for: non-item tooltip (itemID == 0), uncached
// itemID on the no-GUID path (fires a background cache warmup), or
// empty name.
static int __fastcall Script_GameTooltipGetItem(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:GetItem()");
        return 0;
    }
    void *tooltipObj = ResolveTooltipObject(L);
    if (tooltipObj == nullptr)
        return 0;
    auto *base = static_cast<const uint8_t *>(tooltipObj);

    const int itemID = *reinterpret_cast<const int *>(base + Offsets::OFF_TOOLTIP_ITEM_ID);
    if (itemID <= 0)
        return 0;

    const uint32_t guidLo = *reinterpret_cast<const uint32_t *>(
        base + Offsets::OFF_TOOLTIP_ITEM_GUID_LO);
    const uint32_t guidHi = *reinterpret_cast<const uint32_t *>(
        base + Offsets::OFF_TOOLTIP_ITEM_GUID_HI);

    if (guidLo != 0 || guidHi != 0) {
        if (void *cgItem = ResolveItemByGuid(guidLo, guidHi)) {
            const char *link = BuildLinkForCGItem(cgItem);
            if (link != nullptr && *link != '\0') {
                // Engine's link builder also writes the dressed
                // (random-suffixed) name into the link's `[Name]`
                // slot; pull it out by reading the cached base name
                // for the return value. The engine doesn't expose
                // the dressed name as a separate string, so we
                // return the base name — matches modern semantics
                // where (name, link) name is the cached display name
                // and the link is the full hyperlink.
                const uint8_t *record = PeekItemRecord(static_cast<uint32_t>(itemID));
                if (record != nullptr) {
                    const char *name = *reinterpret_cast<const char *const *>(
                        record + Offsets::OFF_ITEMSTATS_NAME);
                    if (name != nullptr && *name != '\0') {
                        Game::Lua::PushString(L, name);
                        Game::Lua::PushString(L, link);
                        Game::Lua::PushNumber(L, static_cast<double>(itemID));
                        return 3;
                    }
                }
            }
        }
        // Fall through to the basic-link path if GUID resolve or link
        // build failed for any reason — better to return *something*
        // than nothing when we have an itemID.
    }

    // No-GUID path (SetItemByID, SetHyperlink for an item:N link with
    // no per-instance data): build the basic colored link from cached
    // itemID + quality + name.
    const uint8_t *record = PeekItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr) {
        Item::Data::WarmCache(static_cast<uint32_t>(itemID));
        return 0;
    }
    const char *name = *reinterpret_cast<const char *const *>(
        record + Offsets::OFF_ITEMSTATS_NAME);
    if (name == nullptr || *name == '\0')
        return 0;
    const uint32_t quality = *reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_QUALITY);

    char link[256];
    std::snprintf(link, sizeof(link),
                  "|cff%s|Hitem:%d:0:0:0:0:0:0:0|h[%s]|h|r",
                  QualityHex(static_cast<int>(quality)), itemID, name);

    Game::Lua::PushString(L, name);
    Game::Lua::PushString(L, link);
    Game::Lua::PushNumber(L, static_cast<double>(itemID));
    return 3;
}

// `GameTooltip:HasItem()` — boolean companion to `GetItem`. Returns
// true iff the tooltip is currently showing an item (any path:
// SetBagItem, SetHyperlink, SetItemByID, SetMerchantItem, etc.). Same
// `[tooltip + OFF_TOOLTIP_ITEM_ID]` check `GetItem` does — that field
// is non-zero only while an item tooltip is live.
static int __fastcall Script_GameTooltipHasItem(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:HasItem()");
        return 0;
    }
    void *tooltipObj = ResolveTooltipObject(L);
    if (tooltipObj == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const int itemID = *reinterpret_cast<const int *>(
        static_cast<const uint8_t *>(tooltipObj) + Offsets::OFF_TOOLTIP_ITEM_ID);
    Game::Lua::PushBoolean(L, itemID > 0 ? 1 : 0);
    return 1;
}

static const Game::Lua::FrameMethodEntry g_methods[] = {
    {"SetItemByID", &Script_GameTooltipSetItemByID},
    {"SetInventoryItemByID", &Script_GameTooltipSetInventoryItemByID},
    {"GetItem", &Script_GameTooltipGetItem},
    {"HasItem", &Script_GameTooltipHasItem},
};

static void RegisterLuaFunctions() {
    Game::Lua::RegisterFrameMethods(
        reinterpret_cast<void *>(Offsets::VAR_GAMETOOLTIP_METHOD_REGISTRY),
        g_methods,
        static_cast<int>(sizeof(g_methods) / sizeof(g_methods[0])));
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Tooltip
