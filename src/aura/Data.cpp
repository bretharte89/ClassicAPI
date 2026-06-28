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
#include "aura/Source.h"
#include "dbc/Lookup.h"
#include "guid/Guid.h"
#include "unit/Identity.h"

#include <cstdint>
#include <cstring>

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

// Keys into the `Aura::Source` caster/expiration cache. Thin alias for the
// shared `Unit::Identity::GuidForObject` so the lookup here and the
// OnAuraAdded hook in `Aura::Source` read the GUID the same way.
uint64_t UnitGuid(const uint8_t *unit) {
    return Unit::Identity::GuidForObject(unit);
}

const uint8_t *SpellRecord(uint32_t spellID) {
    return DBC::Record(Offsets::VAR_SPELL_RECORDS,
                       Offsets::VAR_SPELL_RECORD_COUNT, spellID);
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
    return DBC::StringField(Offsets::VAR_SPELL_ICON_RECORDS,
                            Offsets::VAR_SPELL_ICON_COUNT,
                            static_cast<uint32_t>(iconID),
                            OFF_SPELLICON_PATH);
}

using ResolveUnitToken_t = void *(__fastcall *)(const char *);

// Returns the local player's CGUnit pointer, or nullptr pre-login.
const uint8_t *LocalPlayer() {
    auto fn = reinterpret_cast<ResolveUnitToken_t>(
        static_cast<uintptr_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN));
    return static_cast<const uint8_t *>(fn("player"));
}

// Looks up the player-buff-table entry for a given spellID. The table
// at `VAR_PLAYER_BUFF_TABLE` mirrors the local player's auras with
// timing data (which UNIT_FIELD_AURA lacks for all units). Returns
// nullptr when the spell isn't in any populated entry.
//
// Only meaningful when the caller is building data for the local
// player — every entry here belongs to "player". Caller is expected
// to gate on that.
const uint8_t *FindPlayerBuffEntry(uint32_t spellID) {
    auto *base = reinterpret_cast<const uint8_t *>(
        static_cast<uintptr_t>(Offsets::VAR_PLAYER_BUFF_TABLE));
    for (int i = 0; i < Offsets::PLAYER_BUFF_TABLE_COUNT; ++i) {
        const uint8_t *entry = base + i * Offsets::PLAYER_BUFF_ENTRY_STRIDE;
        const int slotCode = *reinterpret_cast<const int *>(
            entry + Offsets::OFF_PLAYER_BUFF_SLOT_CODE);
        if (slotCode < 0)
            continue;
        const uint32_t entrySpell = *reinterpret_cast<const uint32_t *>(
            entry + Offsets::OFF_PLAYER_BUFF_SPELL_ID);
        if (entrySpell == spellID)
            return entry;
    }
    return nullptr;
}

// Reads the absolute expiration timestamp (in seconds, GetTime-epoch)
// for a player-buff entry, or 0 if the entry has expired / has no
// timing. The engine stores ms-since-tickcount-epoch which matches
// what Lua's `GetTime()` returns (engineMs * 0.001), so converting
// `expirationMs * 0.001` lands directly on the same timeline addons
// compare against on the Lua side. No epoch reconciliation needed.
//
// **Read as `uint32_t`**: GetTickCount overflows `INT_MAX` at ~24.86
// days of system uptime. A signed read flips negative past that
// point, which broke a `<= 0` early-bail that was supposed to catch
// only the "no timing" (0) sentinel. The engine's own arithmetic
// works on the unsigned tick because both sides of `expiration -
// currentMs` wrap together.
double PlayerBuffExpirationSeconds(const uint8_t *entry) {
    if (entry == nullptr)
        return 0.0;
    const int slotCode = *reinterpret_cast<const int *>(
        entry + Offsets::OFF_PLAYER_BUFF_SLOT_CODE);
    if (slotCode < 0)
        return 0.0;
    auto *expirationTable = reinterpret_cast<const uint32_t *>(
        static_cast<uintptr_t>(Offsets::VAR_PLAYER_BUFF_EXPIRATION_TABLE));
    const uint32_t expirationMs = expirationTable[slotCode];
    if (expirationMs == 0)
        return 0.0;
    return static_cast<double>(expirationMs) * 0.001;
}

// Computes the spell's base duration in seconds via Spell.dbc →
// SpellDuration.dbc lookup with level scaling. Returns 0 for spells
// without a real duration (the sentinel `base < 0 && perLevel == 0`
// path the engine uses for infinite auras like passives, paladin
// auras, racials).
//
// Doesn't include talent / glyph / aura-extension modifiers — those
// are caster-side and the engine applies them when computing the
// expiration timestamp. So `expirationTime - GetTime()` reflects the
// *true* remaining time; this `duration` is the base value modern
// addons use for "X / Y" timer rendering.
double SpellBaseDurationSeconds(uint32_t spellID, int unitLevel) {
    const uint8_t *spell = SpellRecord(spellID);
    if (spell == nullptr)
        return 0.0;
    const int durIdx = *reinterpret_cast<const int *>(
        spell + Offsets::OFF_SPELL_DURATION_INDEX);
    if (durIdx <= 0)
        return 0.0;
    const uint8_t *durRec = DBC::Record(Offsets::VAR_SPELLDURATION_RECORDS,
                                        Offsets::VAR_SPELLDURATION_COUNT,
                                        static_cast<uint32_t>(durIdx));
    if (durRec == nullptr)
        return 0.0;
    const int base = *reinterpret_cast<const int *>(
        durRec + Offsets::OFF_SPELLDURATION_BASE_MS);
    const int perLevel = *reinterpret_cast<const int *>(
        durRec + Offsets::OFF_SPELLDURATION_PER_LEVEL_MS);
    const int maxMs = *reinterpret_cast<const int *>(
        durRec + Offsets::OFF_SPELLDURATION_MAX_MS);
    // Engine's "infinite duration" sentinel — matches the check in
    // `FUN_004E44B0`'s "no real duration" branch at `0x004e455c`.
    if (base < 0 && perLevel < 1)
        return 0.0;
    const int spellBaseLevel = *reinterpret_cast<const int *>(
        spell + Offsets::OFF_SPELL_BASE_LEVEL);
    int effLevel = unitLevel;
    if (effLevel < spellBaseLevel)
        effLevel = spellBaseLevel;
    int durationMs = (effLevel - spellBaseLevel) * perLevel + base;
    if (maxMs > 0 && durationMs > maxMs)
        durationMs = maxMs;
    if (durationMs <= 0)
        return 0.0;
    return static_cast<double>(durationMs) * 0.001;
}

// Reads the player's level from UNIT_FIELD_LEVEL. Returns 0 when
// unresolved (pre-login, no descriptor) — caller should treat that
// as "skip level scaling".
int PlayerLevel(const uint8_t *player) {
    auto *desc = Descriptor(player);
    if (desc == nullptr)
        return 0;
    return *reinterpret_cast<const int *>(
        desc + Offsets::OFF_UNIT_FIELD_LEVEL);
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

bool IsPlayerCast(const uint8_t *unit, int slot) {
    const uint32_t spellID = ReadSpellID(unit, slot);
    if (spellID == 0)
        return false;
    uint64_t casterGuid = 0;
    uint32_t expMs = 0;
    uint32_t durMs = 0;
    if (!Aura::Source::Get(UnitGuid(unit), spellID, &casterGuid, &expMs, &durMs))
        return false;
    return casterGuid != 0 && casterGuid == Unit::Identity::PlayerGuid();
}

int FindNthSlot(const uint8_t *unit, int oneBasedIndex, Filter filter,
                bool playerOnly) {
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
        if (playerOnly && !IsPlayerCast(unit, slot))
            continue;
        if (++matches == oneBasedIndex)
            return slot;
    }
    return -1;
}

int FindSlotBySpellID(const uint8_t *unit, uint32_t spellID,
                      const Filter *filter, bool playerOnly) {
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
        if (ReadSpellID(unit, slot) != spellID)
            continue;
        if (playerOnly && !IsPlayerCast(unit, slot))
            continue;
        return slot;
    }
    return -1;
}

int FindSlotBySpellName(const uint8_t *unit, const char *spellName,
                        const Filter *filter, bool playerOnly) {
    if (unit == nullptr || spellName == nullptr || *spellName == '\0')
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
        const char *name = LocalizedSpellName(SpellRecord(ReadSpellID(unit, slot)));
        if (name == nullptr || std::strcmp(name, spellName) != 0)
            continue;
        if (playerOnly && !IsPlayerCast(unit, slot))
            continue;
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

// Writes the full modern AuraData table on top of the Lua stack from
// already-resolved values. Shared by the descriptor path (`Push`) and the
// cache-fallback path (`PushFromCache`). Net stack effect: +1 (the table).
static void BuildTable(void *L, uint32_t spellID, int applications,
                       bool isHelpful, double duration, double expirationTime,
                       uint64_t casterGuid) {
    const uint8_t *spellRecord = SpellRecord(spellID);

    uint32_t dispelTypeID = 0;
    if (spellRecord != nullptr) {
        dispelTypeID = *reinterpret_cast<const uint32_t *>(
            spellRecord + Offsets::OFF_SPELL_DISPEL_TYPE);
    }

    const char *name = LocalizedSpellName(spellRecord);
    const char *icon = SpellIconPath(spellRecord);
    const char *dispel = DispelName(dispelTypeID);

    Game::Lua::NewTable(L);

    Game::Lua::SetFieldString(L, "name", name);
    Game::Lua::SetFieldString(L, "icon", icon);
    Game::Lua::SetFieldNumber(L, "applications",
                              static_cast<double>(applications));
    Game::Lua::SetFieldNumber(L, "spellId", static_cast<double>(spellID));
    Game::Lua::SetFieldString(L, "dispelName", dispel);
    Game::Lua::SetFieldBool(L, "isHelpful", isHelpful);
    Game::Lua::SetFieldBool(L, "isHarmful", !isHelpful);
    Game::Lua::SetFieldNumber(L, "duration", duration);
    Game::Lua::SetFieldNumber(L, "expirationTime", expirationTime);

    if (casterGuid != 0) {
        // `sourceUnit` is a token (nil when the caster maps to no current
        // token); `sourceGUID` is the raw "0x..." GUID and is set whenever we
        // have a caster — a ClassicAPI extension (not in retail AuraData) that
        // survives the caster leaving token range and doubles as a unit token
        // under SuperWoW.
        char tokenBuf[32];
        char guidBuf[Guid::STRING_SIZE];
        const char *sourceUnit = Unit::Identity::TokenFromGUID(
            casterGuid, tokenBuf, sizeof tokenBuf);
        const char *sourceGUID =
            Guid::FormatAsString(casterGuid, guidBuf, sizeof guidBuf);
        if (sourceUnit != nullptr)
            Game::Lua::SetFieldString(L, "sourceUnit", sourceUnit);
        if (sourceGUID != nullptr)
            Game::Lua::SetFieldString(L, "sourceGUID", sourceGUID);
    }

    Game::Lua::SetFieldNumber(L, "charges", 0);
    Game::Lua::SetFieldNumber(L, "maxCharges", 0);
    Game::Lua::SetFieldNumber(L, "timeMod", 1);

    // Boolean fields whose modern semantics don't apply in vanilla.
    Game::Lua::SetFieldBool(L, "isStealable", false);
    Game::Lua::SetFieldBool(L, "isBossAura", false);
    Game::Lua::SetFieldBool(L, "isFromPlayerOrPlayerPet", false);
    Game::Lua::SetFieldBool(L, "isNameplateOnly", false);
    Game::Lua::SetFieldBool(L, "nameplateShowAll", false);
    Game::Lua::SetFieldBool(L, "nameplateShowPersonal", false);
    Game::Lua::SetFieldBool(L, "canApplyAura", false);
    Game::Lua::SetFieldBool(L, "shouldConsolidate", false);
    Game::Lua::SetFieldBool(L, "isRaid", false);

    // `auraInstanceID` and `points` are deliberately omitted — modern returns
    // nil for those when they don't apply, and Lua reading a missing key
    // yields nil, so the table doesn't need an explicit entry.
}

void Push(void *L, const uint8_t *unit, int slot) {
    const uint32_t spellID = ReadSpellID(unit, slot);
    const bool isHelpful = slot < Offsets::UNIT_AURA_BUFF_COUNT;

    // Timing fields. Vanilla doesn't broadcast cast time / duration for ANY
    // unit's auras except the local player's — for everyone else (target,
    // party, raid, mouseover) `expirationTime` stays 0 unless the
    // `Aura::Source` cache observed the cast. For the player we read the
    // engine's `VAR_PLAYER_BUFF_TABLE` (same data `GetPlayerBuffTimeLeft`
    // returns), which gives a real expiration timestamp.
    //
    // `duration` comes from Spell.dbc → SpellDuration.dbc with level scaling
    // (the *base* value); talent / glyph duration-extension modifiers are
    // already baked into the expiration timestamp on the caster's side, so
    // `expirationTime - GetTime()` reflects true remaining time even when the
    // base `duration` doesn't include the talent boost.
    double duration = 0.0;
    double expirationTime = 0.0;
    uint64_t casterGuid = 0;
    const uint8_t *player = LocalPlayer();
    if (spellID != 0) {
        const int unitLevel = (unit != nullptr) ? PlayerLevel(unit) : 0;
        duration = SpellBaseDurationSeconds(spellID, unitLevel);
    }
    if (unit != nullptr && unit == player) {
        const uint8_t *entry = FindPlayerBuffEntry(spellID);
        if (entry != nullptr)
            expirationTime = PlayerBuffExpirationSeconds(entry);
    }
    // Caster + non-player expiration from the SMSG_SPELL_GO cache, captured at
    // cast time (the descriptor has neither). The player-buff table above is
    // authoritative for the player's own expiration; the cache fills it for
    // everyone else. Prefer the applied (caster-modified) duration so it stays
    // consistent with `expirationTime` — otherwise a talented DoT (Improved
    // Shadow Word: Pain) would show remaining time exceeding the base.
    if (spellID != 0) {
        uint64_t c = 0;
        uint32_t expMs = 0;
        uint32_t durMs = 0;
        if (Aura::Source::Get(UnitGuid(unit), spellID, &c, &expMs, &durMs)) {
            if (expirationTime == 0.0 && expMs != 0)
                expirationTime = static_cast<double>(expMs) * 0.001;
            if (durMs != 0)
                duration = static_cast<double>(durMs) * 0.001;
            casterGuid = c;
        }
    }

    BuildTable(L, spellID, ReadStacks(unit, slot), isHelpful, duration,
               expirationTime, casterGuid);
}

namespace {

// Builds an AuraData table from an `Aura::Source` cache entry — the fallback
// when the descriptor has dropped the aura's slot. Stack count isn't broadcast
// in SMSG_SPELL_GO, so `applications` defaults to 1. `duration` uses the
// applied (caster-modified) ms when known, else the Spell.dbc base.
void PushFromCache(void *L, const uint8_t *unit,
                   const Aura::Source::CachedAura &c, bool isHelpful) {
    double duration = 0.0;
    if (c.durationMs != 0) {
        duration = static_cast<double>(c.durationMs) * 0.001;
    } else {
        const int unitLevel = (unit != nullptr) ? PlayerLevel(unit) : 0;
        duration = SpellBaseDurationSeconds(c.spellId, unitLevel);
    }
    const double expirationTime =
        (c.expirationMs != 0) ? static_cast<double>(c.expirationMs) * 0.001 : 0.0;
    BuildTable(L, c.spellId, 1, isHelpful, duration, expirationTime,
               c.casterGuid);
}

// Counts populated descriptor slots matching `filter` (and `playerOnly`).
int CountSlots(const uint8_t *unit, Filter filter, bool playerOnly) {
    if (unit == nullptr)
        return 0;
    const int start = (filter == Filter::Harmful)
                          ? Offsets::UNIT_AURA_BUFF_COUNT
                          : 0;
    const int end = (filter == Filter::Harmful) ? Offsets::UNIT_AURA_TOTAL
                                                 : Offsets::UNIT_AURA_BUFF_COUNT;
    int n = 0;
    for (int slot = start; slot < end; ++slot) {
        if (!IsSlotPopulated(unit, slot))
            continue;
        if (playerOnly && !IsPlayerCast(unit, slot))
            continue;
        ++n;
    }
    return n;
}

constexpr int kFallbackMax = Offsets::UNIT_AURA_TOTAL;

// Whether cache entry `c` should be surfaced as a fallback for `unit` under
// `filter`/`playerOnly`: it must not already be in a populated descriptor slot
// (no double-listing the same live aura) and, when player-filtered, must have
// been cast by us. Superseded / dispelled auras are kept out of the cache by
// `Aura::Source`'s removal eviction, not filtered here.
bool FallbackEligible(const uint8_t *unit, const Aura::Source::CachedAura &c,
                      Filter filter, bool playerOnly) {
    if (FindSlotBySpellID(unit, c.spellId, &filter, false) >= 0)
        return false; // still in the descriptor — returned via the slot path
    if (playerOnly && c.casterGuid != Unit::Identity::PlayerGuid())
        return false;
    return true;
}

} // namespace

bool PushNthCacheFallback(void *L, const uint8_t *unit, int oneBasedIndex,
                          Filter filter, bool playerOnly) {
    if (unit == nullptr)
        return false;
    // The descriptor held `descCount` matches; the caller already found fewer
    // than `oneBasedIndex` there, so the cache supplies index
    // `oneBasedIndex - descCount` (1-based) of the entries it didn't.
    const int target = oneBasedIndex - CountSlots(unit, filter, playerOnly);
    if (target < 1)
        return false;
    const uint64_t guid = UnitGuid(unit);
    if (guid == 0)
        return false;
    Aura::Source::CachedAura buf[kFallbackMax];
    const int n = Aura::Source::Enumerate(
        guid, filter == Filter::Harmful, buf, kFallbackMax);
    int seen = 0;
    for (int i = 0; i < n; ++i) {
        if (!FallbackEligible(unit, buf[i], filter, playerOnly))
            continue;
        if (++seen == target) {
            PushFromCache(L, unit, buf[i], filter == Filter::Helpful);
            return true;
        }
    }
    return false;
}

void AppendCacheFallbacks(void *L, const uint8_t *unit, Filter filter,
                          bool playerOnly, int outerIdx, int &nextKey) {
    if (unit == nullptr)
        return;
    const uint64_t guid = UnitGuid(unit);
    if (guid == 0)
        return;
    Aura::Source::CachedAura buf[kFallbackMax];
    const int n = Aura::Source::Enumerate(
        guid, filter == Filter::Harmful, buf, kFallbackMax);
    for (int i = 0; i < n; ++i) {
        if (!FallbackEligible(unit, buf[i], filter, playerOnly))
            continue;
        Game::Lua::PushNumber(L, static_cast<double>(nextKey++));
        PushFromCache(L, unit, buf[i], filter == Filter::Helpful);
        Game::Lua::SetTable(L, outerIdx);
    }
}

} // namespace Aura::Data
