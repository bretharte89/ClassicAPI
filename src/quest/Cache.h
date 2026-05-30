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

#pragma once

#include <cstdint>

namespace Quest::Cache {

// Calls the engine's quest cache `_GetRecord` at `FUN_DBCACHE_QUEST_GET_RECORD`.
//
// With `fireRequest=false`, performs only the hash-table lookup. Returns
// the cached data block (`entry+0x18` per the engine's accessor) if the
// record is fully loaded, or nullptr if not cached / not yet loaded.
//
// With `fireRequest=true`, queues a CMSG_QUEST_QUERY when the data isn't
// already loaded and registers the supplied `callback` (called with
// `userData` and a 0/1 success byte once SMSG_QUEST_QUERY_RESPONSE
// lands). When the data is already loaded, returns immediately and does
// NOT invoke the callback — same as the item cache. Caller is responsible
// for any "synthesise success=1 ourselves on cache hit" logic.
//
// Internal layout of the returned data block (offsets within the block):
//   +0x9C  inline `char title[N]` — null-terminated, locale-applied
const uint8_t *Lookup(uint32_t questID, void *callback, void *userData);

// Convenience: pure cache-state probe with no request side effect.
inline const uint8_t *Peek(uint32_t questID) {
    return Lookup(questID, nullptr, nullptr);
}

// Field offsets within the data block returned by `Lookup`.
//
// Derived from the engine's own `Script_GetQuestLog*` accessors at
// `0x004DF930..0x004E12B0`. Each accessor calls `_GetRecord` then
// reads from one of these offsets; the existing comment block on each
// constant identifies the source function. See CLAUDE.md's
// "Quest static-info cache" section for the full trace.

// Quest level — uint32. Source: `FUN_004df340` (called by
// `Script_GetQuestLogTitle`) does `mov eax, [block + 0x08]` and
// pushes it as `level`.
constexpr int OFF_LEVEL = 0x08;
// Quest sort/category pointer — used by the quest log UI to group
// entries under headers. Source: `FUN_004dee60` reads `block[+0x0C]`
// and matches against `VAR_QUEST_LOG_CATEGORY_TABLE` to find the
// containing collapsible header index.
constexpr int OFF_SORT_PTR = 0x0C;
// QuestInfo.dbc row — uint32 row index into the small DBC at
// `0x00C0D9CC` (records) / `0x00C0D9D0` (count). Each record is
// `{id@+0, char *name[locale]@+4}`; the localized name is the
// quest-type tag string ("Dungeon", "Raid", "Group", "PvP", "Elite"
// — modern WoW's `frequency`/`questTag`). Source: `FUN_004df2a0`.
constexpr int OFF_QUEST_INFO_ROW = 0x10;
// Signed money delta — int32. Positive = reward copper, negative =
// `-required` copper. The engine has two accessors that share this
// field with complementary sign:
//   - `Script_GetQuestLogRewardMoney`   returns max(0, +money)
//   - `Script_GetQuestLogRequiredMoney` returns max(0, -money)
constexpr int OFF_MONEY_DELTA = 0x28;
// Level-cap reward money bonus — int32. Added to the reward when
// the player's level is ≥ 60. Source: `Script_GetQuestLogRewardMoney`
// does `cmp [player.unit + 0x70], 0x3C` (level >= 60) then
// `add edx, [block + 0x2C]`.
constexpr int OFF_LEVEL_CAP_BONUS_MONEY = 0x2C;
// Reward spellID — int32. 0 = no spell reward. Source:
// `Script_GetQuestLogRewardSpell` does `mov eax, [block + 0x30]` then
// resolves the SpellIcon path and locale-applied name via `Spell.dbc`.
constexpr int OFF_REWARD_SPELL_ID = 0x30;
// Quest flags — uint32. Bit definitions match modern WoW's
// `QUEST_FLAGS_*` enum, of which only bit 3 (`0x08` SHARABLE) is
// confirmed-tested by the engine (`Script_GetQuestLogPushable`'s
// `test byte ptr [block + 0x38], 0x08`). Other bits are presumed
// stored but unverified.
constexpr int OFF_QUEST_FLAGS = 0x38;
// Reward and choice item arrays — up to 4 reward and 6 choice
// itemIDs stored inline (4 bytes each). 0 in any slot means
// "no item in this slot". Used by `GetQuestLogItemLink` /
// `GetQuestLogItemID`.
//
// Each itemID array is paired with a parallel uint32 count array
// (the "how many of this item" reward). Layout in cache memory:
//   +0x3C  RewardItems[4]
//   +0x4C  RewardItemsCount[4]
//   +0x5C  ChoiceItems[6]
//   +0x74  ChoiceItemsCount[6]
// Verified from `Script_GetQuestLogRewardInfo` (`fild [block + slot*4 + 0x4C]`
// for numItems) and `Script_GetQuestLogChoiceInfo` (`fild [block + slot*4 + 0x74]`).
constexpr int OFF_REWARD_ITEMS = 0x3C;
constexpr int OFF_REWARD_ITEMS_COUNT = 0x4C;
constexpr int OFF_CHOICE_ITEMS = 0x5C;
constexpr int OFF_CHOICE_ITEMS_COUNT = 0x74;
constexpr int REWARD_COUNT = 4;
constexpr int CHOICE_COUNT = 6;
constexpr int OFF_TITLE = 0x9C;
// Inline narrative text — both locale-applied and printf-style
// format-expanded by the engine's variable substituter at
// `0x00506F70` (handles tokens like `$N` for player name, `$C` for
// class, etc.). Source: `Script_GetQuestLogQuestText` reads both,
// pushes `description` first then `objectives`.
//
// We deliberately expose the *raw* inline bytes, not the format-
// expanded version, because the substituter wants a 1024-byte
// scratch buffer and the substitutions (player name, class) belong
// in the addon layer anyway. Addons can run `string.gsub` if they
// need the runtime values, otherwise raw text is more useful for
// guides / databases.
constexpr int OFF_OBJECTIVES = 0x29C;
constexpr int OFF_DESCRIPTION = 0xA9C;
// Objective arrays — four uint32[4] parallel slabs carrying
// SMSG_QUEST_QUERY_RESPONSE's RequiredNPCOrGo / Count / Item / Count
// fields verbatim. NPC-or-GO is **signed**: positive = creature
// entry, negative = gameobject entry (engine strips the sign bit
// before the cache lookup). Empty slots hold 0 — `Script_GetQuestLogLeaderBoard`
// at 0x004E0110 skips zero entries when matching its 1-based objIdx
// arg, walking the NPC/GO array first then the item array (total max 8
// objectives). Verified by decoding the four `[esi + edi*4 + 0x14XC]`
// reads and the cache-instance routing in that function:
//   - 0xC0E354 (creature cache) for positive 0x149C → "monster"
//   - 0xC0E318 (gameobject cache) for negative 0x149C → "object"
//   - 0xC0E2A0 (item cache) for 0x14BC → "item"
constexpr int OFF_REQUIRED_NPC_OR_GO = 0x149C;        // int32[4]
constexpr int OFF_REQUIRED_NPC_OR_GO_COUNT = 0x14AC;  // uint32[4]
constexpr int OFF_REQUIRED_ITEM = 0x14BC;             // uint32[4]
constexpr int OFF_REQUIRED_ITEM_COUNT = 0x14CC;       // uint32[4]
constexpr int OBJECTIVE_COUNT = 4;

} // namespace Quest::Cache
