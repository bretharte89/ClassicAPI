// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

// Aura-table primitives. See `Data.h` for the contract; see `Api.cpp`
// for the Lua-binding surface that consumes these.

#include "Data.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Aura::Data {

namespace {

// Spell.dbc record field offsets used locally. `spell/Info.cpp` has
// the same set of locals for `Script_GetSpellInfo`; promoting them
// to `Offsets.h` is a future refactor — for now we mirror its
// convention so both files stay in sync.
constexpr int OFF_SPELL_ICON_ID = 0x1D4;
constexpr int OFF_SPELLICON_PATH = 0x04;

const uint8_t *Descriptor(const uint8_t *unit) {
    if (unit == nullptr)
        return nullptr;
    return *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_CGUNIT_OBJECT_FIELDS);
}

const uint8_t *SpellRecord(uint32_t spellID) {
    if (spellID == 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_RECORD_COUNT));
    if (static_cast<int>(spellID) > count)
        return nullptr;
    auto *records = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_RECORDS));
    if (records == nullptr)
        return nullptr;
    return records[spellID];
}

int LocaleIndex() {
    return *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_LOCALE_INDEX));
}

const char *LocalizedSpellName(const uint8_t *spellRecord) {
    if (spellRecord == nullptr)
        return nullptr;
    const auto *names = reinterpret_cast<const char *const *>(
        spellRecord + Offsets::OFF_SPELL_NAMES);
    const char *s = names[LocaleIndex()];
    return (s == nullptr || *s == '\0') ? nullptr : s;
}

const char *SpellIconPath(const uint8_t *spellRecord) {
    if (spellRecord == nullptr)
        return nullptr;
    const int iconID = *reinterpret_cast<const int *>(
        spellRecord + OFF_SPELL_ICON_ID);
    if (iconID <= 0)
        return nullptr;
    const int iconCount = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_ICON_COUNT));
    if (iconID > iconCount)
        return nullptr;
    auto *iconRecords = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELL_ICON_RECORDS));
    if (iconRecords == nullptr)
        return nullptr;
    const uint8_t *iconRecord = iconRecords[iconID];
    if (iconRecord == nullptr)
        return nullptr;
    return *reinterpret_cast<const char *const *>(
        iconRecord + OFF_SPELLICON_PATH);
}

void SetString(void *L, const char *key, const char *value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushString(L, value);
    Game::Lua::SetTable(L, -3);
}

void SetNumber(void *L, const char *key, double value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushNumber(L, value);
    Game::Lua::SetTable(L, -3);
}

void SetBool(void *L, const char *key, bool value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushBoolean(L, value ? 1 : 0);
    Game::Lua::SetTable(L, -3);
}

} // namespace

uint32_t ReadSpellID(const uint8_t *unit, int slot) {
    if (slot < 0 || slot >= Offsets::UNIT_AURA_TOTAL)
        return 0;
    auto *desc = Descriptor(unit);
    if (desc == nullptr)
        return 0;
    return *reinterpret_cast<const uint32_t *>(
        desc + Offsets::OFF_UNIT_FIELD_AURA + slot * 4);
}

bool IsSlotPopulated(const uint8_t *unit, int slot) {
    if (slot < 0 || slot >= Offsets::UNIT_AURA_TOTAL)
        return false;
    auto *desc = Descriptor(unit);
    if (desc == nullptr)
        return false;
    const uint32_t spellID = *reinterpret_cast<const uint32_t *>(
        desc + Offsets::OFF_UNIT_FIELD_AURA + slot * 4);
    if (spellID == 0)
        return false;
    const uint8_t byte = *(desc + Offsets::OFF_UNIT_FIELD_AURAFLAGS + slot / 2);
    const uint8_t nibble = (byte >> ((slot & 1) * 4)) & 0xF;
    if ((nibble & Offsets::UNIT_AURA_VISIBLE_MASK) == 0)
        return false;
    return IsVisible(SpellRecord(spellID));
}

int FindNthSlot(const uint8_t *unit, int oneBasedIndex, Filter filter) {
    if (unit == nullptr || oneBasedIndex < 1)
        return -1;
    const int start = (filter == Filter::Harmful)
                          ? Offsets::UNIT_AURA_BUFF_COUNT
                          : 0;
    const int end = (filter == Filter::Harmful)
                        ? Offsets::UNIT_AURA_TOTAL
                        : Offsets::UNIT_AURA_BUFF_COUNT;
    int matches = 0;
    for (int slot = start; slot < end; ++slot) {
        if (!IsSlotPopulated(unit, slot))
            continue;
        if (++matches == oneBasedIndex)
            return slot;
    }
    return -1;
}

int FindSlotBySpellID(const uint8_t *unit, uint32_t spellID,
                      const Filter *filter) {
    if (unit == nullptr || spellID == 0)
        return -1;
    const int start = (filter != nullptr && *filter == Filter::Harmful)
                          ? Offsets::UNIT_AURA_BUFF_COUNT
                          : 0;
    const int end = (filter != nullptr && *filter == Filter::Helpful)
                        ? Offsets::UNIT_AURA_BUFF_COUNT
                        : Offsets::UNIT_AURA_TOTAL;
    for (int slot = start; slot < end; ++slot) {
        if (!IsSlotPopulated(unit, slot))
            continue;
        if (ReadSpellID(unit, slot) == spellID)
            return slot;
    }
    return -1;
}

int ReadStacks(const uint8_t *unit, int slot) {
    if (slot < 0 || slot >= Offsets::UNIT_AURA_TOTAL)
        return 0;
    auto *desc = Descriptor(unit);
    if (desc == nullptr)
        return 0;
    const uint8_t stored = *reinterpret_cast<const uint8_t *>(
        desc + Offsets::OFF_UNIT_FIELD_AURAAPPLICATIONS + slot);
    return static_cast<int>(stored) + 1;
}

bool IsVisible(const uint8_t *spellRecord) {
    if (spellRecord == nullptr)
        return false;
    using Fn = char(__fastcall *)(const uint8_t *);
    auto fn = reinterpret_cast<Fn>(
        static_cast<uintptr_t>(Offsets::FUN_SPELL_IS_VISIBLE_AURA));
    return fn(spellRecord) != 0;
}

const char *DispelName(uint32_t dispelTypeID) {
    if (dispelTypeID == 0)
        return nullptr;
    const int count = *reinterpret_cast<const int *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELLDISPEL_COUNT));
    if (static_cast<int>(dispelTypeID) > count)
        return nullptr;
    auto *records = *reinterpret_cast<const uint8_t *const *const *>(
        static_cast<uintptr_t>(Offsets::VAR_SPELLDISPEL_RECORDS));
    if (records == nullptr)
        return nullptr;
    const uint8_t *record = records[dispelTypeID];
    if (record == nullptr)
        return nullptr;
    if (*reinterpret_cast<const int *>(
            record + Offsets::OFF_SPELLDISPEL_HAS_NAME) == 0)
        return nullptr;
    const char *name = *reinterpret_cast<const char *const *>(
        record + Offsets::OFF_SPELLDISPEL_NAME);
    return (name == nullptr || *name == '\0') ? nullptr : name;
}

void Push(void *L, const uint8_t *unit, int slot) {
    const uint32_t spellID = ReadSpellID(unit, slot);
    const uint8_t *spellRecord = SpellRecord(spellID);
    const bool isHelpful = slot < Offsets::UNIT_AURA_BUFF_COUNT;

    uint32_t dispelTypeID = 0;
    if (spellRecord != nullptr) {
        dispelTypeID = *reinterpret_cast<const uint32_t *>(
            spellRecord + Offsets::OFF_SPELL_DISPEL_TYPE);
    }

    const char *name = LocalizedSpellName(spellRecord);
    const char *icon = SpellIconPath(spellRecord);
    const char *dispel = DispelName(dispelTypeID);

    Game::Lua::NewTable(L);

    SetString(L, "name", name != nullptr ? name : "");
    SetString(L, "icon", icon != nullptr ? icon : "");
    SetNumber(L, "applications", static_cast<double>(ReadStacks(unit, slot)));
    SetNumber(L, "spellId", static_cast<double>(spellID));
    SetString(L, "dispelName", dispel != nullptr ? dispel : "");
    SetBool(L, "isHelpful", isHelpful);
    SetBool(L, "isHarmful", !isHelpful);

    // Numeric fields modern always sets — 0 for "not applicable" to
    // match modern semantics (`if data.duration > 0` etc. still
    // works the same on vanilla).
    SetNumber(L, "duration", 0);
    SetNumber(L, "expirationTime", 0);
    SetNumber(L, "charges", 0);
    SetNumber(L, "maxCharges", 0);
    SetNumber(L, "timeMod", 1);

    // Boolean fields whose modern semantics don't apply in vanilla.
    SetBool(L, "isStealable", false);
    SetBool(L, "isBossAura", false);
    SetBool(L, "isFromPlayerOrPlayerPet", false);
    SetBool(L, "isNameplateOnly", false);
    SetBool(L, "nameplateShowAll", false);
    SetBool(L, "nameplateShowPersonal", false);
    SetBool(L, "canApplyAura", false);
    SetBool(L, "shouldConsolidate", false);
    SetBool(L, "isRaid", false);

    // `sourceUnit`, `auraInstanceID`, `points` deliberately omitted
    // — modern returns nil for those when they don't apply, and
    // Lua reading a missing key yields nil, so the table doesn't
    // need an explicit entry.
}

} // namespace Aura::Data
