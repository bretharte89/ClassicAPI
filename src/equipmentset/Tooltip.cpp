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

// `GameTooltip:SetEquipmentSet(name)` — modern method that fills the
// tooltip with summary info for a named equipment set. Mirrors the
// 4.3.4 implementation at `0x0046E690`, which dispatches to an inner
// builder (`FUN_0046CEB0`) producing this shape:
//
//   [Set Name]                          (header)
//   N items                              (ITEMS_VARIABLE_QUANTITY)
//   N equipped                           (ITEMS_EQUIPPED, green)
//   N in inventory                       (ITEMS_IN_INVENTORY)
//   N slots ignored                      (ITEM_SLOTS_IGNORED, gray)
//   N missing                            (ITEM_MISSING, red — counted,
//                                         not listed individually)
//
// We back the implementation with the same equipment-set storage
// `C_EquipmentSet.*` already uses (`EquipmentSet::Data`) — no new data
// model needed. Item classification reuses `Locations::FindGUID`,
// which walks the player's flat inventory + bag invMgrs the same way
// `GetItemCount` does, so bank items count as "in inventory" without
// needing the bank window open.
//
// Stock 4.3.4 enumerates each missing item by name (looks up the
// itemID via the engine's full item-cache name resolver). Our storage
// records only GUIDs, so we count missing items but don't reproduce
// each line — listing names would require persisting itemIDs
// alongside GUIDs in the WTF file, a storage-format change we punt
// on. Modern addons that need the missing-item names can iterate
// `C_EquipmentSet.GetItemIDs` + `C_EquipmentSet.GetItemLocations`
// themselves.
//
// Tooltip filling is done by calling back through Lua (`self:AddLine`
// etc.) rather than hitting the engine's tooltip-line internals
// directly. The Lua dispatch goes through the engine's own frame-
// method registry, so any addon-installed hook on AddLine (DBM,
// pfUI's tooltip skin, etc.) still runs.

#include "Game.h"
#include "Offsets.h"
#include "equipmentset/Data.h"
#include "equipmentset/Locations.h"
#include "equipmentset/Set.h"

#include <cstdint>

namespace EquipmentSet::Tooltip {

namespace {

// Calls `self:Method()` with no Lua args. Stack at entry must have
// the tooltip at index 1 (the standard `frame:method()` self slot).
// Leaves the stack unchanged.
void CallSelfNoArgs(void *L, const char *methodName) {
    Game::Lua::PushValue(L, 1);
    Game::Lua::PushString(L, methodName);
    Game::Lua::GetTable(L, -2); // method = self[methodName]
    Game::Lua::Insert(L, -2);   // [method, self]
    Game::Lua::Call(L, 1, 0);
}

// Calls `self:AddLine(text, r, g, b)`. Same self-on-stack precondition.
void AddLine(void *L, const char *text, double r, double g, double b) {
    Game::Lua::PushValue(L, 1);
    Game::Lua::PushString(L, "AddLine");
    Game::Lua::GetTable(L, -2);
    Game::Lua::Insert(L, -2);
    Game::Lua::PushString(L, text);
    Game::Lua::PushNumber(L, r);
    Game::Lua::PushNumber(L, g);
    Game::Lua::PushNumber(L, b);
    Game::Lua::Call(L, 5, 0);
}

// Calls `self:SetText(text, r, g, b)`. Same precondition. Used for
// the header line — `SetText` is the standard vanilla entry for
// "first line of a fresh tooltip" and handles internal redraw state
// `AddLine` skips when it's the first content after a clear.
void SetText(void *L, const char *text, double r, double g, double b) {
    Game::Lua::PushValue(L, 1);
    Game::Lua::PushString(L, "SetText");
    Game::Lua::GetTable(L, -2);
    Game::Lua::Insert(L, -2);
    Game::Lua::PushString(L, text);
    Game::Lua::PushNumber(L, r);
    Game::Lua::PushNumber(L, g);
    Game::Lua::PushNumber(L, b);
    Game::Lua::Call(L, 5, 0); // self + text + r + g + b
}

// Format-and-add one localized count line. Calls
// `Game::Lua::PushLocalizedFormatInt` to resolve the format string
// (Blizzard's FrameXML globals like `ITEMS_EQUIPPED` first, with the
// C `fallback` for stripped-down servers), then dispatches `AddLine`.
void AddLocalizedLineInt(void *L, const char *globalName, const char *fallback, int n,
                         double r, double g, double b) {
    const int savedTop = Game::Lua::GetTop(L);
    Game::Lua::PushLocalizedFormatInt(L, globalName, fallback, n);
    const char *text = Game::Lua::ToString(L, -1);
    if (text != nullptr)
        AddLine(L, text, r, g, b);
    Game::Lua::SetTop(L, savedTop);
}

struct Tally {
    int equipped;
    int inInventory;
    int ignored;
    int missing;
    // ItemIDs of slots classified as missing. Indexed 0..missing-1.
    // Capacity matches SLOT_COUNT (worst case: every slot is missing).
    // Stays 0 for entries loaded from pre-itemID-format files.
    uint32_t missingItemIDs[SLOT_COUNT];
};

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                      const uint64_t *guid, void *callback,
                                                      void *userData, int unused);

const uint8_t *PeekItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, 0);
}

// Walks a set's slots, classifying each non-empty entry into the
// equipped / in-inventory / ignored / missing buckets via
// `Locations::FindGUID`. For missing slots, also records the saved
// itemID so the tooltip can resolve names. Stack-free.
Tally TallySet(const Set &s) {
    Tally t{};
    for (int i = 0; i < SLOT_COUNT; ++i) {
        const uint64_t g = s.items[i];
        if (g == GUID_IGNORED) {
            t.ignored++;
            continue;
        }
        if (g == GUID_EMPTY)
            continue;
        const int loc = Locations::FindGUID(g);
        if (loc == 0) {
            // Live GUID isn't in the player's object map. The saved
            // itemID (if any) is the only handle we have for naming.
            t.missingItemIDs[t.missing++] = s.itemIDs[i];
            continue;
        }
        // LOC_BAGS covers backpack / player bags / bank bags; LOC_BANK
        // alone (no LOC_BAGS) covers main bank slots. Both count as
        // "in inventory" for the tooltip summary — matches the modern
        // category where everything-but-equipped is one bucket.
        if ((loc & (LOC_BAGS | LOC_BANK)) != 0) {
            t.inInventory++;
        } else {
            t.equipped++;
        }
    }
    return t;
}

int __fastcall Script_GameTooltipSetEquipmentSet(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetEquipmentSet(\"setName\")");
        return 0;
    }
    if (Game::Lua::Type(L, 2) != Game::Lua::TYPE_STRING) {
        Game::Lua::Error(L, "Usage: GameTooltip:SetEquipmentSet(\"setName\")");
        return 0;
    }
    const char *setName = Game::Lua::ToString(L, 2);
    if (setName == nullptr)
        return 0;

    const Set *s = Data::FindByName(setName);
    if (s == nullptr)
        return 0;

    const Tally t = TallySet(*s);
    const int totalSlots = t.equipped + t.inInventory + t.missing;

    // Trim the Lua stack to just the tooltip self-arg so our internal
    // method-call sequences don't accumulate the original args past
    // their useful lifetime. `setName` was a `const char *` lifted out
    // by ToString — Lua's strings are pinned by reference count, so
    // dropping the string from the stack doesn't invalidate the
    // pointer for the lifetime of this call.
    Game::Lua::SetTop(L, 1);

    // Header — set name, white. Doubles as the "ClearLines" boundary;
    // SetText replaces line 1 and clears subsequent lines in vanilla.
    SetText(L, s->name.c_str(), 1.0, 1.0, 1.0);

    // Body lines — try Blizzard's localized FORMAT_* globals first.
    // Same key names retail FrameXML uses (`ITEMS_VARIABLE_QUANTITY`,
    // `ITEMS_EQUIPPED`, ...); addons that need their own wording can
    // override by reassigning `_G[name]` at runtime. The fallbacks
    // only fire on servers stripped of the standard GlobalStrings.
    AddLocalizedLineInt(L, "ITEMS_VARIABLE_QUANTITY", "%d items",
                        totalSlots, 1.0, 1.0, 1.0);

    if (t.equipped > 0) {
        AddLocalizedLineInt(L, "ITEMS_EQUIPPED", "%d equipped",
                            t.equipped, 0.0, 1.0, 0.0); // green
    }
    if (t.inInventory > 0) {
        AddLocalizedLineInt(L, "ITEMS_IN_INVENTORY", "%d in inventory",
                            t.inInventory, 1.0, 1.0, 1.0);
    }
    if (t.ignored > 0) {
        AddLocalizedLineInt(L, "ITEM_SLOTS_IGNORED", "%d slots ignored",
                            t.ignored, 0.5, 0.5, 0.5); // gray
    }
    // Missing slots: list each by name using the itemID we stored at
    // save time, then look the name up in the item cache. Matches the
    // 4.3.4 behavior of one `ITEM_MISSING` line per missing item.
    // Slots loaded from pre-itemID files have `itemID == 0`; for those
    // we can't render a name, so we fall back to summarizing them as
    // a `(N unnamed missing)` line below.
    int unnamedMissing = 0;
    for (int i = 0; i < t.missing; ++i) {
        const uint32_t id = t.missingItemIDs[i];
        if (id == 0) {
            unnamedMissing++;
            continue;
        }
        auto *record = PeekItemRecord(id);
        if (record == nullptr) {
            unnamedMissing++;
            continue;
        }
        const char *name = *reinterpret_cast<const char *const *>(
            record + Offsets::OFF_ITEMSTATS_NAME);
        if (name == nullptr || name[0] == '\0') {
            unnamedMissing++;
            continue;
        }
        // Build "Missing: <name>" using Blizzard's `ITEM_MISSING` format
        // global. Wrap inline since AddLocalizedLineInt only handles
        // %d args — we need %s here.
        const int savedTop = Game::Lua::GetTop(L);
        Game::Lua::PushString(L, "format");
        Game::Lua::GetTable(L, Game::Lua::GLOBALS_INDEX);
        Game::Lua::PushLocalizedString(L, "ITEM_MISSING", "Missing: %s");
        Game::Lua::PushString(L, name);
        Game::Lua::Call(L, 2, 1);
        const char *line = Game::Lua::ToString(L, -1);
        if (line != nullptr)
            AddLine(L, line, 1.0, 0.0, 0.0); // red
        Game::Lua::SetTop(L, savedTop);
    }
    if (unnamedMissing > 0) {
        // Pre-itemID-format files or items the cache has no record of.
        // No Blizzard format global maps cleanly here; addons can
        // override with `_G.CLASSICAPI_EQUIPMENTSET_MISSING`.
        AddLocalizedLineInt(L, "CLASSICAPI_EQUIPMENTSET_MISSING",
                            "%d missing", unnamedMissing, 1.0, 0.0, 0.0);
    }

    CallSelfNoArgs(L, "Show");
    return 0;
}

const Game::Lua::FrameMethodEntry g_methods[] = {
    {"SetEquipmentSet", &Script_GameTooltipSetEquipmentSet},
};

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterFrameMethods(
        reinterpret_cast<void *>(Offsets::VAR_GAMETOOLTIP_METHOD_REGISTRY),
        g_methods,
        static_cast<int>(sizeof(g_methods) / sizeof(g_methods[0])));
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace EquipmentSet::Tooltip
