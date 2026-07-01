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

// `C_Item.GetItemStats(itemLink)` and `C_Item.GetItemStatDelta(link1,
// link2)` — read an item's stats out of the cached `ItemStats_C` record
// (plus any random-suffix bonus decoded from the link) and return them
// as a table keyed by FrameXML global-string names, the same shape
// modern WoW's `GetItemStats` uses (so `_G[key]` yields the display
// label). `GetItemStatDelta` is the per-stat difference of two items.
//
// Reported stat set = vanilla's stored item stats: the base attributes
// (Str/Agi/Sta/Int/Spi, plus mana/health-on-equip), armor
// (`RESISTANCE0_NAME`), and the six resistance schools
// (`RESISTANCE1..6_NAME`). The rating stats later expansions added
// (crit / haste / mastery / …) don't exist in 1.12 data, so those keys
// never appear — this is why we map the *vanilla* ItemModType numbering
// directly rather than porting 3.3.5's renumbered enum + 73-entry table.
//
// Random suffix support ("of the Bear", "of Arcane Resistance", …): the
// link's third `item:id:enchant:SUFFIX:unique` field is a row into
// `ItemRandomProperties.dbc`, which lists `SpellItemEnchantment.dbc` IDs.
// In 1.12 the stat suffixes apply their bonus as an *equip spell*
// (enchant type 3 → a spellID whose `SPELL_AURA_MOD_STAT` /
// `SPELL_AURA_MOD_RESISTANCE` effect carries the value), while armor
// suffixes use a direct resistance enchant (type 4). We decode both and
// fold them into the same stat/armor/resistance keys as the base record.
// Bonuses a suffix grants through other spell auras (attack power, spell
// power, healing, mp5, weapon skill) are not reported — those aren't
// part of vanilla's stat set, and the base item stores them the same way
// (equip spells we likewise don't count), so the scope stays consistent.

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"
#include "item/Arg.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace Item::Stats {

namespace {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                      const uint64_t *guid, void *callback,
                                                      void *userData, bool requestIfMissing);

// Passive peek — same contract as GetItemData / GetItemInfoInstant. Does
// not warm the cache (callback = nullptr, requestIfMissing = false), so
// an uncached item reads back as nullptr rather than orphaning an entry
// (see the item-cache race note in CLAUDE.md).
const uint8_t *FetchItemRecord(uint32_t itemID) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    return fn(cache, itemID, &zeroGuid, nullptr, nullptr, false);
}

// Base-record m_statType uses the ItemModType enum: 0=mana, 1=health,
// 3=agility, 4=strength, 5=intellect, 6=spirit, 7=stamina (2 is
// deprecated). Returns nullptr for types with no vanilla key.
const char *StatKeyForItemModType(uint32_t statType) {
    switch (statType) {
    case 0: return "ITEM_MOD_MANA_SHORT";
    case 1: return "ITEM_MOD_HEALTH_SHORT";
    case 3: return "ITEM_MOD_AGILITY_SHORT";
    case 4: return "ITEM_MOD_STRENGTH_SHORT";
    case 5: return "ITEM_MOD_INTELLECT_SHORT";
    case 6: return "ITEM_MOD_SPIRIT_SHORT";
    case 7: return "ITEM_MOD_STAMINA_SHORT";
    default: return nullptr;
    }
}

// SPELL_AURA_MOD_STAT (29) misc uses the UnitStat enum instead: 0=Str,
// 1=Agi, 2=Sta, 3=Int, 4=Spi (-1 = all stats, not a single key).
// Verified against the "Increased <stat> 01" random-property spells.
const char *StatKeyForUnitStat(int stat) {
    switch (stat) {
    case 0: return "ITEM_MOD_STRENGTH_SHORT";
    case 1: return "ITEM_MOD_AGILITY_SHORT";
    case 2: return "ITEM_MOD_STAMINA_SHORT";
    case 3: return "ITEM_MOD_INTELLECT_SHORT";
    case 4: return "ITEM_MOD_SPIRIT_SHORT";
    default: return nullptr;
    }
}

// Resistance school index → key. 0 = armor (physical), 1..6 = the magic
// schools in Holy/Fire/Nature/Frost/Shadow/Arcane order.
const char *ResistKey(int school) {
    static const char *const kKeys[7] = {
        "RESISTANCE0_NAME", "RESISTANCE1_NAME", "RESISTANCE2_NAME",
        "RESISTANCE3_NAME", "RESISTANCE4_NAME", "RESISTANCE5_NAME",
        "RESISTANCE6_NAME",
    };
    return (school >= 0 && school <= 6) ? kKeys[school] : nullptr;
}

// Accumulator over the full key set a vanilla item can touch: seven base
// stats plus armor (RESISTANCE0_NAME) and the six schools
// (RESISTANCE1..6_NAME). Fixed and small — linear scan by key is fine.
struct Accum {
    const char *key;
    long value;
};

constexpr int kAccumCount = 14;

void InitAccum(Accum *acc) {
    static const char *const kKeys[kAccumCount] = {
        "ITEM_MOD_MANA_SHORT",      "ITEM_MOD_HEALTH_SHORT",
        "ITEM_MOD_AGILITY_SHORT",   "ITEM_MOD_STRENGTH_SHORT",
        "ITEM_MOD_INTELLECT_SHORT", "ITEM_MOD_SPIRIT_SHORT",
        "ITEM_MOD_STAMINA_SHORT",   "RESISTANCE0_NAME",
        "RESISTANCE1_NAME",         "RESISTANCE2_NAME",
        "RESISTANCE3_NAME",         "RESISTANCE4_NAME",
        "RESISTANCE5_NAME",         "RESISTANCE6_NAME",
    };
    for (int i = 0; i < kAccumCount; ++i) {
        acc[i].key = kKeys[i];
        acc[i].value = 0;
    }
}

void AddByKey(Accum *acc, const char *key, long delta) {
    if (key == nullptr || delta == 0)
        return;
    for (int i = 0; i < kAccumCount; ++i)
        if (std::strcmp(acc[i].key, key) == 0) {
            acc[i].value += delta;
            return;
        }
}

long U32(const uint8_t *record, int offset) {
    return static_cast<long>(*reinterpret_cast<const uint32_t *>(record + offset));
}

// Folds the stat/resistance auras of an equip spell (SpellItemEnchantment
// type 3) into `acc`. Only SPELL_AURA_MOD_STAT (29) and MOD_RESISTANCE
// (22) map to a stat key — proc / spell-power / attack-power / weapon-
// skill auras are intentionally ignored (outside vanilla's stat set).
// The effect value is the fixed-die `EffectBasePoints + 1`.
void AddSpellStatAuras(Accum *acc, int spellID, long sign) {
    if (spellID <= 0)
        return;
    const uint8_t *sp = DBC::Record(Offsets::VAR_SPELL_RECORDS,
                                    Offsets::VAR_SPELL_RECORD_COUNT,
                                    static_cast<uint32_t>(spellID));
    if (sp == nullptr)
        return;
    auto aura = reinterpret_cast<const int32_t *>(
        sp + Offsets::OFF_SPELL_RECORD_EFFECT_APPLY_AURA_NAME);
    auto misc = reinterpret_cast<const int32_t *>(
        sp + Offsets::OFF_SPELL_RECORD_EFFECT_MISC_VALUE);
    auto base = reinterpret_cast<const int32_t *>(
        sp + Offsets::OFF_SPELL_RECORD_EFFECT_BASE_POINTS);
    for (int i = 0; i < Offsets::SPELL_RECORD_EFFECT_COUNT; ++i) {
        const long value = static_cast<long>(base[i]) + 1;
        switch (aura[i]) {
        case 29: // SPELL_AURA_MOD_STAT — misc = UnitStat index
            AddByKey(acc, StatKeyForUnitStat(misc[i]), sign * value);
            break;
        case 22: { // SPELL_AURA_MOD_RESISTANCE — misc = school bitmask
            for (int school = 0; school <= 6; ++school)
                if (misc[i] & (1 << school))
                    AddByKey(acc, ResistKey(school), sign * value);
            break;
        }
        default:
            break;
        }
    }
}

// Applies the item's random-suffix bonus (link field 2 → suffixID) to
// `acc`. Walks ItemRandomProperties.dbc → its SpellItemEnchantment IDs;
// type 3 enchants defer to the equip spell's auras, type 4 add a direct
// resistance, type 5 a direct stat. No-op for suffixID <= 0.
void ApplyRandomSuffix(Accum *acc, int suffixID, long sign) {
    if (suffixID <= 0)
        return;
    const uint8_t *rp = DBC::Record(Offsets::VAR_ITEMRANDOMPROP_RECORDS,
                                    Offsets::VAR_ITEMRANDOMPROP_COUNT,
                                    static_cast<uint32_t>(suffixID));
    if (rp == nullptr)
        return;
    auto enchants = reinterpret_cast<const uint32_t *>(
        rp + Offsets::OFF_ITEMRANDOMPROP_ENCHANT);
    for (int s = 0; s < Offsets::ITEMRANDOMPROP_ENCHANT_SLOT_COUNT; ++s) {
        const uint32_t enchantID = enchants[s];
        if (enchantID == 0)
            continue;
        const uint8_t *en = DBC::Record(Offsets::VAR_SPELLITEMENCHANT_RECORDS,
                                        Offsets::VAR_SPELLITEMENCHANT_COUNT, enchantID);
        if (en == nullptr)
            continue;
        auto type = reinterpret_cast<const int32_t *>(en + Offsets::OFF_SPELLITEMENCHANT_TYPE);
        auto amount = reinterpret_cast<const int32_t *>(en + Offsets::OFF_SPELLITEMENCHANT_AMOUNT);
        auto arg = reinterpret_cast<const int32_t *>(en + Offsets::OFF_SPELLITEMENCHANT_ARG);
        for (int i = 0; i < 3; ++i) { // SpellItemEnchantment has 3 effect slots
            switch (type[i]) {
            case 3: // equip spell — stat/resist value lives in the spell
                AddSpellStatAuras(acc, arg[i], sign);
                break;
            case 4: // resistance — arg = school index (0 = armor)
                AddByKey(acc, ResistKey(arg[i]), sign * static_cast<long>(amount[i]));
                break;
            case 5: // direct stat — arg = ItemModType, amount = value
                AddByKey(acc, StatKeyForItemModType(static_cast<uint32_t>(arg[i])),
                         sign * static_cast<long>(amount[i]));
                break;
            default: // 1 proc / 2 weapon-damage / 6 totem / 7 use spell
                break;
            }
        }
    }
}

// Adds item `record`'s base-record stats to `acc` scaled by `sign` (+1 to
// add, -1 to subtract — the delta path adds item2 then subtracts item1).
void Accumulate(Accum *acc, const uint8_t *record, long sign) {
    if (record == nullptr)
        return;
    auto *types = reinterpret_cast<const uint32_t *>(
        record + Offsets::OFF_ITEMSTATS_STAT_TYPE);
    auto *values = reinterpret_cast<const int32_t *>(
        record + Offsets::OFF_ITEMSTATS_STAT_VALUE);
    for (int i = 0; i < Offsets::ITEMSTATS_STAT_SLOT_COUNT; ++i)
        AddByKey(acc, StatKeyForItemModType(types[i]),
                 sign * static_cast<long>(values[i]));

    AddByKey(acc, "RESISTANCE0_NAME", sign * U32(record, Offsets::OFF_ITEMSTATS_ARMOR));
    AddByKey(acc, "RESISTANCE1_NAME", sign * U32(record, Offsets::OFF_ITEMSTATS_RES_HOLY));
    AddByKey(acc, "RESISTANCE2_NAME", sign * U32(record, Offsets::OFF_ITEMSTATS_RES_FIRE));
    AddByKey(acc, "RESISTANCE3_NAME", sign * U32(record, Offsets::OFF_ITEMSTATS_RES_NATURE));
    AddByKey(acc, "RESISTANCE4_NAME", sign * U32(record, Offsets::OFF_ITEMSTATS_RES_FROST));
    AddByKey(acc, "RESISTANCE5_NAME", sign * U32(record, Offsets::OFF_ITEMSTATS_RES_SHADOW));
    AddByKey(acc, "RESISTANCE6_NAME", sign * U32(record, Offsets::OFF_ITEMSTATS_RES_ARCANE));
}

// Parses the random-property/suffix ID from an item link's third field
// (`item:id:enchant:SUFFIX:unique`). Returns 0 for a non-string arg, a
// link with no `item:` payload, or a short link that omits the field
// (bare `item:N`, `item:N:E`). The engine's own link format is the
// four-field `item:%d:%d:%d:%d`, so the suffix is always field index 2.
int ParseRandomSuffix(void *L, int idx) {
    if (Game::Lua::Type(L, idx) != Game::Lua::TYPE_STRING)
        return 0;
    const char *s = Game::Lua::ToString(L, idx);
    if (s == nullptr)
        return 0;
    const char *m = std::strstr(s, "item:");
    if (m == nullptr)
        return 0;
    m += 5; // start of field 0 (itemID)
    // Advance to the start of field 2 (past two ':' separators), stopping
    // at the end of the hyperlink payload if the link is too short.
    int colons = 0;
    while (*m != '\0' && *m != '|') {
        if (*m == ':') {
            if (++colons == 2) {
                ++m;
                break;
            }
        }
        ++m;
    }
    if (colons < 2)
        return 0;
    return std::atoi(m);
}

// Pushes a fresh Lua table with each non-zero accumulator entry as
// `t[key] = value`; leaves the table on the stack top for the caller to
// return. Zero entries are omitted (matches GetItemStats' sparse output
// and GetItemStatDelta showing only differences).
void PushAccumTable(void *L, const Accum *acc) {
    Game::Lua::NewTable(L);
    for (int i = 0; i < kAccumCount; ++i)
        if (acc[i].value != 0)
            Game::Lua::SetFieldNumber(L, acc[i].key,
                                      static_cast<double>(acc[i].value));
}

} // namespace

// `C_Item.GetItemStats(itemLink)` → statTable. Keys are FrameXML
// global-string names (`ITEM_MOD_STRENGTH_SHORT`, `RESISTANCE0_NAME` for
// armor, `RESISTANCE1..6_NAME` for the schools); values are the item's
// stat magnitudes including any random-suffix bonus. Only stats the item
// actually carries are present.
//
// Accepts a full chat link, an `item:N…` string, or a bare itemID (a
// superset of retail, which is link-only). A bare itemID or a link with
// no suffix field yields base stats only. Returns nil if the item isn't
// cached yet — warm it via GetItemInfo and retry.
int __fastcall Script_GetItemStats(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    if (itemID <= 0) {
        Game::Lua::Error(L, "Usage: C_Item.GetItemStats(itemLink)");
        return 0;
    }
    const int suffixID = ParseRandomSuffix(L, 1);
    const uint8_t *record = FetchItemRecord(static_cast<uint32_t>(itemID));
    if (record == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Accum acc[kAccumCount];
    InitAccum(acc);
    Accumulate(acc, record, +1);
    ApplyRandomSuffix(acc, suffixID, +1);
    PushAccumTable(L, acc);
    return 1;
}

// `C_Item.GetItemStatDelta(itemLink1, itemLink2)` → statTable of the
// per-stat difference `item2 - item1` (positive = item2 has more of that
// stat). Random-suffix bonuses on either link are included. Only stats
// whose delta is non-zero appear. Same key set, input forms, and caching
// caveat as `C_Item.GetItemStats`. Returns nil if either item is
// uncached.
int __fastcall Script_GetItemStatDelta(void *L) {
    const int id1 = Item::Arg::ResolveItemID(L, 1);
    const int id2 = Item::Arg::ResolveItemID(L, 2);
    if (id1 <= 0 || id2 <= 0) {
        Game::Lua::Error(
            L, "Usage: C_Item.GetItemStatDelta(itemLink1, itemLink2)");
        return 0;
    }
    const int suffix1 = ParseRandomSuffix(L, 1);
    const int suffix2 = ParseRandomSuffix(L, 2);
    const uint8_t *r1 = FetchItemRecord(static_cast<uint32_t>(id1));
    const uint8_t *r2 = FetchItemRecord(static_cast<uint32_t>(id2));
    if (r1 == nullptr || r2 == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    Accum acc[kAccumCount];
    InitAccum(acc);
    Accumulate(acc, r2, +1);
    ApplyRandomSuffix(acc, suffix2, +1);
    Accumulate(acc, r1, -1);
    ApplyRandomSuffix(acc, suffix1, -1);
    PushAccumTable(L, acc);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "GetItemStats", &Script_GetItemStats);
    Game::Lua::RegisterTableFunction("C_Item", "GetItemStatDelta",
                                     &Script_GetItemStatDelta);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Stats
