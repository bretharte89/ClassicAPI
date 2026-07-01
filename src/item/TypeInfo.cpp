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

#include <cstdint>

namespace Item::TypeInfo {

namespace {

int LocaleIndex() {
    return *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
}

// Linear-scan ItemSubClass.dbc for the (classID, subClassID) compound-key
// row — same scan Item::GetData uses. Returns the record or nullptr.
const uint8_t *FindSubClass(uint32_t classID, uint32_t subClassID) {
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_ITEMSUBCLASS_COUNT));
    if (count <= 0)
        return nullptr;
    auto *records = *reinterpret_cast<const uint8_t *const *>(
        static_cast<uintptr_t>(Offsets::VAR_ITEMSUBCLASS_RECORDS));
    if (records == nullptr)
        return nullptr;
    for (int i = 0; i < count; ++i) {
        const uint8_t *rec =
            records + i * Offsets::OFF_ITEMSUBCLASS_RECORD_STRIDE;
        if (*reinterpret_cast<const uint32_t *>(
                rec + Offsets::OFF_ITEMSUBCLASS_CLASS_ID) == classID &&
            *reinterpret_cast<const uint32_t *>(
                rec + Offsets::OFF_ITEMSUBCLASS_SUBCLASS_ID) == subClassID)
            return rec;
    }
    return nullptr;
}

// Verbose name first, short name on miss (same fallback the engine and
// GetItemInfoInstant use); locale-applied.
const char *SubClassName(const uint8_t *rec) {
    const int locale = LocaleIndex();
    const char *verbose = *reinterpret_cast<const char *const *>(
        rec + Offsets::OFF_ITEMSUBCLASS_DISPLAY_NAME + locale * 4);
    if (verbose != nullptr && verbose[0] != '\0')
        return verbose;
    const char *shortName = *reinterpret_cast<const char *const *>(
        rec + Offsets::OFF_ITEMSUBCLASS_NAME + locale * 4);
    return (shortName != nullptr && shortName[0] != '\0') ? shortName : "";
}

// The engine's INVTYPE table holds the equipLoc KEY string ("INVTYPE_HEAD",
// …) indexed by inventory type. Returns "" for out-of-range / empty slots.
const char *InvTypeKey(int invType) {
    if (invType < 0 || invType > Offsets::INVTYPE_TABLE_MAX_INDEX)
        return "";
    auto **table = reinterpret_cast<const char **>(
        static_cast<uintptr_t>(Offsets::VAR_INVTYPE_STRING_TABLE));
    const char *s = table[invType];
    return (s != nullptr && s[0] != '\0') ? s : "";
}

// `C_Item.GetItemSubClassInfo(classID, subClassID)` →
// `subClassName, subClassUsesInvType`.
//
// `subClassName` is the localized ItemSubClass.dbc name (verbose form, e.g.
// "One-Handed Swords"; falls back to the short form for subclasses that
// only populate it, e.g. "Consumable"). `subClassUsesInvType` is true for
// subclasses whose items display by inventory slot rather than by subclass
// name — the armor material types (Misc / Cloth / Leather / Mail / Plate),
// per the ItemSubClass flags bit at +0x10. Returns `nil` if the
// (classID, subClassID) pair has no row.
int __fastcall Script_GetItemSubClassInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2)) {
        Game::Lua::Error(
            L, "Usage: C_Item.GetItemSubClassInfo(classID, subClassID)");
        return 0;
    }
    const auto classID = static_cast<uint32_t>(Game::Lua::ToNumber(L, 1));
    const auto subClassID = static_cast<uint32_t>(Game::Lua::ToNumber(L, 2));
    const uint8_t *rec = FindSubClass(classID, subClassID);
    if (rec == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    const uint32_t flags = *reinterpret_cast<const uint32_t *>(
        rec + Offsets::OFF_ITEMSUBCLASS_FLAGS);
    Game::Lua::PushString(L, SubClassName(rec));
    Game::Lua::PushBool(L,
                        (flags & Offsets::ITEMSUBCLASS_FLAG_USES_INVTYPE) != 0);
    return 2;
}

// `C_Item.GetItemInventorySlotKey(inventorySlot)` → the equipLoc key string
// for an `Enum.InventoryType` value (e.g. `1` → "INVTYPE_HEAD"). `nil` for
// non-equip (0) or out-of-range values.
int __fastcall Script_GetItemInventorySlotKey(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(
            L, "Usage: C_Item.GetItemInventorySlotKey(inventorySlot)");
        return 0;
    }
    const char *key = InvTypeKey(static_cast<int>(Game::Lua::ToNumber(L, 1)));
    if (key[0] == '\0') {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::PushString(L, key);
    return 1;
}

// `C_Item.GetItemInventorySlotInfo(inventorySlot)` → the localized display
// name for an `Enum.InventoryType` value (e.g. `1` → "Head"). The engine's
// INVTYPE table stores the KEY ("INVTYPE_HEAD"); the display name is the
// FrameXML global string of that name (`INVTYPE_HEAD = "Head"`), so we
// resolve it via that global with the key itself as the fallback. `nil` for
// non-equip (0) or out-of-range values.
int __fastcall Script_GetItemInventorySlotInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(
            L, "Usage: C_Item.GetItemInventorySlotInfo(inventorySlot)");
        return 0;
    }
    const char *key = InvTypeKey(static_cast<int>(Game::Lua::ToNumber(L, 1)));
    if (key[0] == '\0') {
        Game::Lua::PushNil(L);
        return 1;
    }
    Game::Lua::PushLocalizedString(L, key, key);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemSubClassInfo",
                                     &Script_GetItemSubClassInfo);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemInventorySlotKey",
                                     &Script_GetItemInventorySlotKey);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemInventorySlotInfo",
                                     &Script_GetItemInventorySlotInfo);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Item::TypeInfo
