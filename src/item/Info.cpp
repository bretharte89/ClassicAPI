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
#include "item/ID.h"
#include "item/Location.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Item::Info {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                       const uint64_t *guid, void *callback,
                                                       void *userData, bool requestIfMissing);

static const char *PushedOrEmpty(const char *s) { return (s != nullptr && s[0] != 0) ? s : ""; }

static int CurrentLocaleIndex() { return *reinterpret_cast<int *>(Offsets::VAR_LOCALE_INDEX); }

static const uint8_t *FetchItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, false);
}

static const char *LookupItemClassName(uint32_t classID) {
    const int count = *reinterpret_cast<int *>(Offsets::VAR_ITEMCLASS_COUNT);
    if (static_cast<int>(classID) < 0 || static_cast<int>(classID) > count)
        return "";
    const uint8_t *const *records =
        *reinterpret_cast<const uint8_t *const *const *>(Offsets::VAR_ITEMCLASS_RECORDS);
    if (records == nullptr)
        return "";
    const uint8_t *record = records[classID];
    if (record == nullptr)
        return "";
    const char *name = *reinterpret_cast<const char *const *>(
        record + Offsets::OFF_ITEMCLASS_NAMES + CurrentLocaleIndex() * 4);
    return PushedOrEmpty(name);
}

static const char *LookupItemSubClassName(uint32_t classID, uint32_t subClassID) {
    const int count = *reinterpret_cast<int *>(Offsets::VAR_ITEMSUBCLASS_COUNT);
    if (count <= 0)
        return "";
    auto *records = *reinterpret_cast<const uint8_t *const *>(Offsets::VAR_ITEMSUBCLASS_RECORDS);
    if (records == nullptr)
        return "";
    const int locale = CurrentLocaleIndex();
    for (int i = 0; i < count; i++) {
        const uint8_t *record = records + i * Offsets::OFF_ITEMSUBCLASS_RECORD_STRIDE;
        if (*reinterpret_cast<const uint32_t *>(record + 0x00) != classID)
            continue;
        if (*reinterpret_cast<const uint32_t *>(record + 0x04) != subClassID)
            continue;
        // Mirror Script_GetItemInfo's fallback chain at 0x48E311..0x48E32C:
        // try the verbose name first, fall back to the short name.
        const char *verbose = *reinterpret_cast<const char *const *>(
            record + Offsets::OFF_ITEMSUBCLASS_DISPLAY_NAME + locale * 4);
        if (verbose != nullptr && verbose[0] != 0)
            return verbose;
        const char *shortName = *reinterpret_cast<const char *const *>(
            record + Offsets::OFF_ITEMSUBCLASS_NAME + locale * 4);
        return PushedOrEmpty(shortName);
    }
    return "";
}

static const char *LookupInvType(uint32_t invType) {
    if (invType > Offsets::INVTYPE_TABLE_MAX_INDEX)
        return "";
    auto **table = reinterpret_cast<const char **>(Offsets::VAR_INVTYPE_STRING_TABLE);
    return PushedOrEmpty(table[invType]);
}

static bool BuildIconPath(uint32_t displayInfoID, char *out, size_t outSize) {
    if (out == nullptr || outSize == 0)
        return false;
    out[0] = 0;
    const int count = *reinterpret_cast<int *>(Offsets::VAR_ITEMDISPLAYINFO_COUNT);
    if (static_cast<int>(displayInfoID) <= 0 || static_cast<int>(displayInfoID) > count)
        return false;
    const uint8_t *const *records =
        *reinterpret_cast<const uint8_t *const *const *>(Offsets::VAR_ITEMDISPLAYINFO_RECORDS);
    if (records == nullptr)
        return false;
    const uint8_t *record = records[displayInfoID];
    if (record == nullptr)
        return false;
    const char *iconName =
        *reinterpret_cast<const char *const *>(record + Offsets::OFF_ITEMDISPLAYINFO_ICON);
    if (iconName == nullptr || iconName[0] == 0)
        return false;
    snprintf(out, outSize, "Interface\\Icons\\%s", iconName);
    return true;
}

// Resolves Lua arg at index 1 to an itemID. Accepts numeric values and strings
// containing the substring "item:NNN" (matches both the bare "item:1234..."
// shorthand and the full "|cff...|Hitem:1234:...|h[Name]|h|r" chat link).
// Returns 0 (sentinel for "couldn't resolve") for everything else; we follow
// vanilla GetItemInfo and don't accept item names.
static int ResolveItemID(void *L) {
    if (Game::Lua::IsNumber(L, 1))
        return static_cast<int>(Game::Lua::ToNumber(L, 1));
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_STRING)
        return 0;
    const char *s = Game::Lua::ToString(L, 1);
    if (s == nullptr)
        return 0;
    if (const char *itemMarker = strstr(s, "item:"))
        return atoi(itemMarker + 5);
    return atoi(s);
}

static int __fastcall Script_GetItemInfoInstant(void *L) {
    const int itemID = ResolveItemID(L);
    if (itemID <= 0)
        return 0;
    const uint8_t *record = FetchItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr)
        return 0;

    const uint32_t classID =
        *reinterpret_cast<const uint32_t *>(record + Offsets::OFF_ITEMSTATS_CLASS);
    const uint32_t subClassID =
        *reinterpret_cast<const uint32_t *>(record + Offsets::OFF_ITEMSTATS_SUBCLASS);
    const uint32_t displayInfoID =
        *reinterpret_cast<const uint32_t *>(record + Offsets::OFF_ITEMSTATS_DISPLAY_INFO_ID);
    const uint32_t invType =
        *reinterpret_cast<const uint32_t *>(record + Offsets::OFF_ITEMSTATS_INVENTORY_TYPE);

    char iconPath[260];
    if (!BuildIconPath(displayInfoID, iconPath, sizeof(iconPath)))
        iconPath[0] = 0;

    Game::Lua::PushNumber(L, static_cast<double>(itemID));
    Game::Lua::PushString(L, LookupItemClassName(classID));
    Game::Lua::PushString(L, LookupItemSubClassName(classID, subClassID));
    Game::Lua::PushString(L, LookupInvType(invType));
    Game::Lua::PushString(L, iconPath);
    Game::Lua::PushNumber(L, static_cast<double>(classID));
    Game::Lua::PushNumber(L, static_cast<double>(subClassID));
    return 7;
}

// Pushes the icon path for `itemID` and returns 1, or returns 0 (= nil
// to Lua) if the item isn't cached, the displayInfoID is missing, or
// the icon record's path slot is empty. Shared by all three
// icon-accessor entry points.
static int PushIconForItemID(void *L, int itemID) {
    if (itemID <= 0)
        return 0;
    const uint8_t *record = FetchItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr)
        return 0;
    const uint32_t displayInfoID = *reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_DISPLAY_INFO_ID);
    char iconPath[260];
    if (!BuildIconPath(displayInfoID, iconPath, sizeof(iconPath)))
        return 0;
    Game::Lua::PushString(L, iconPath);
    return 1;
}

// `GetItemIcon(itemID)` — global, takes a numeric itemID only. Modern
// WoW returns a fileID:number; we surface the icon path string since
// 1.12 has no fileID system. Direct passthrough to
// `texture:SetTexture(...)`.
static int __fastcall Script_GetItemIcon(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetItemIcon(itemID)");
        return 0;
    }
    return PushIconForItemID(L, static_cast<int>(Game::Lua::ToNumber(L, 1)));
}

// `C_Item.GetItemIcon(itemLocation)` — table-shape ItemLocation form.
// Routes through `Item::Location::Resolve` → CGItem → itemID, then the
// same cache lookup as the other variants.
static int __fastcall Script_C_Item_GetItemIcon(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemIcon(itemLocation)");
        return 0;
    }
    const int itemID = Item::ID::FromCGItem(Item::Location::Resolve(L, 1));
    return PushIconForItemID(L, itemID);
}

// `C_Item.GetItemIconByID(itemInfo)` — accepts numeric itemID or
// `"item:NN"` string (full chat links work too via the same parser
// `GetItemInfoInstant` uses).
static int __fastcall Script_C_Item_GetItemIconByID(void *L) {
    return PushIconForItemID(L, ResolveItemID(L));
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemInfoInstant", &Script_GetItemInfoInstant);
    Game::Lua::RegisterGlobalFunction("GetItemIcon", &Script_GetItemIcon);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemIcon", &Script_C_Item_GetItemIcon);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemIconByID", &Script_C_Item_GetItemIconByID);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Info
