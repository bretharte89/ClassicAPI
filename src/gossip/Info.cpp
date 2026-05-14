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

// `C_GossipInfo.*` — retail-shaped wrappers around vanilla 1.12's
// flat gossip surface (`GetGossipText`, `GetGossipOptions`,
// `GetGossipAvailableQuests`, `GetGossipActiveQuests`, plus the
// `SelectGossip*` selectors).
//
// All of the underlying data lives in the engine's two gossip-state
// arrays at `VAR_GOSSIP_OPTIONS` (16 entries, stride `0x80C`) and
// `VAR_GOSSIP_QUESTS` (32 entries, stride `0x20C`), populated by the
// SMSG_GOSSIP_MESSAGE handler at `0x004E26E0` and cleared each open.
// We read the same shape the engine's own `Script_GetGossip*` reads.
//
// Missing modern fields — vanilla 1.12's server simply never sends
// them, so there's nothing to surface:
//   - `rewards`, `spellID` (post-vanilla; merged in with the
//     quest/spell system rework)
//   - `status` (Available/Unavailable/Locked/AlreadyComplete — vanilla
//     server doesn't per-option compute this)
//   - `overrideIconID`, `selectOptionWhenOnlyOption` — modern UX hints
//
// Selectors translate the modern (`gossipOptionID` / `questID`) arg
// shape back to vanilla's 1-based slot index, then tail-call the
// engine's existing `Script_SelectGossip*` so we share the engine's
// CMSG-send path and error semantics.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Gossip::Info {

namespace {

using EngineScriptFn = int(__fastcall *)(void *L);

const uint8_t *OptionEntry(int slot) {
    return reinterpret_cast<const uint8_t *>(
        static_cast<uintptr_t>(Offsets::VAR_GOSSIP_OPTIONS) +
        static_cast<uintptr_t>(slot) * Offsets::GOSSIP_OPTIONS_STRIDE);
}

const uint8_t *QuestEntry(int slot) {
    return reinterpret_cast<const uint8_t *>(
        static_cast<uintptr_t>(Offsets::VAR_GOSSIP_QUESTS) +
        static_cast<uintptr_t>(slot) * Offsets::GOSSIP_QUESTS_STRIDE);
}

int32_t OptionIndex(const uint8_t *entry) {
    return *reinterpret_cast<const int32_t *>(
        entry + Offsets::OFF_GOSSIP_OPTION_INDEX);
}

uint32_t QuestID(const uint8_t *entry) {
    return *reinterpret_cast<const uint32_t *>(
        entry + Offsets::OFF_GOSSIP_QUEST_ID);
}

uint32_t QuestStatus(const uint8_t *entry) {
    return *reinterpret_cast<const uint32_t *>(
        entry + Offsets::OFF_GOSSIP_QUEST_STATUS);
}

// Vanilla's two active-quest sentinels — anything else marks an
// available (deliverable) quest. Mirrors `FUN_004E2430`'s filter.
bool IsActiveQuest(uint32_t status) {
    return status == 3 || status == 4;
}

void SetTableNumberField(void *L, const char *key, double value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushNumber(L, value);
    Game::Lua::SetTable(L, -3);
}

void SetTableStringField(void *L, const char *key, const char *value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushString(L, value != nullptr ? value : "");
    Game::Lua::SetTable(L, -3);
}

void SetTableBoolField(void *L, const char *key, bool value) {
    Game::Lua::PushString(L, key);
    Game::Lua::PushBoolean(L, value ? 1 : 0);
    Game::Lua::SetTable(L, -3);
}

// `C_GossipInfo.GetText()` — pushes the greeting string the engine
// resolved for the gossip-giver's NPC_TEXT.dbc entry. Mirrors what
// `Script_GetGossipText` does (one inline `lua_pushstring` of the
// fixed buffer at `VAR_GOSSIP_GREETING_TEXT`).
int __fastcall Script_C_GossipInfo_GetText(void *L) {
    const char *text = reinterpret_cast<const char *>(
        static_cast<uintptr_t>(Offsets::VAR_GOSSIP_GREETING_TEXT));
    Game::Lua::PushString(L, text);
    return 1;
}

// `C_GossipInfo.GetOptions()` — returns an array of `GossipOptionUIInfo`
// tables in display order. Only fields the 1.12 server actually
// transmits are populated:
//
//   gossipOptionID  — vanilla `optionIndex` (the same value `SelectOption`
//                     looks up).
//   name            — option text.
//   icon            — vanilla 0..10 icon-type byte (gossip / vendor /
//                     taxi / trainer / healer / binder / banker /
//                     petition / tabard / battlemaster / auctioneer).
//                     NOT a retail-style fileID; addons that want a
//                     texture path do the lookup themselves.
//   flags           — bit 0 = boxCoded (password-protected option).
//   orderIndex      — 1-based position in the emitted list (matches
//                     `SelectGossipOption`'s expected 1-based arg).
int __fastcall Script_C_GossipInfo_GetOptions(void *L) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);

    int outIdx = 0;
    for (int slot = 0; slot < Offsets::GOSSIP_OPTIONS_MAX; ++slot) {
        const uint8_t *entry = OptionEntry(slot);
        const int32_t optIdx = OptionIndex(entry);
        if (optIdx < 0)
            continue;
        outIdx += 1;

        Game::Lua::PushNumber(L, static_cast<double>(outIdx));
        Game::Lua::NewTable(L);

        SetTableNumberField(L, "gossipOptionID",
                            static_cast<double>(optIdx));
        SetTableStringField(L, "name", reinterpret_cast<const char *>(
            entry + Offsets::OFF_GOSSIP_OPTION_TEXT));
        SetTableNumberField(L, "icon",
            static_cast<double>(entry[Offsets::OFF_GOSSIP_OPTION_ICON]));
        const uint8_t boxCoded = entry[Offsets::OFF_GOSSIP_OPTION_BOX_CODED];
        SetTableNumberField(L, "flags",
            static_cast<double>(boxCoded ? 1u : 0u));
        SetTableNumberField(L, "orderIndex", static_cast<double>(outIdx));

        Game::Lua::SetTable(L, -3);
    }

    return 1;
}

// Shared body for `GetAvailableQuests` / `GetActiveQuests`. Both walk
// the same backing array and split by the `status` field — `wantActive`
// inverts the filter.
int PushQuestList(void *L, bool wantActive) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::NewTable(L);

    int outIdx = 0;
    for (int slot = 0; slot < Offsets::GOSSIP_QUESTS_MAX; ++slot) {
        const uint8_t *entry = QuestEntry(slot);
        const uint32_t questID = QuestID(entry);
        if (questID == 0)
            break;
        const uint32_t status = QuestStatus(entry);
        if (IsActiveQuest(status) != wantActive)
            continue;
        outIdx += 1;

        Game::Lua::PushNumber(L, static_cast<double>(outIdx));
        Game::Lua::NewTable(L);

        SetTableNumberField(L, "questID", static_cast<double>(questID));
        SetTableStringField(L, "title", reinterpret_cast<const char *>(
            entry + Offsets::OFF_GOSSIP_QUEST_TITLE));
        const int32_t level = *reinterpret_cast<const int32_t *>(
            entry + Offsets::OFF_GOSSIP_QUEST_LEVEL);
        SetTableNumberField(L, "questLevel", static_cast<double>(level));
        // `isComplete` is only meaningful for active quests; vanilla's
        // status==4 row corresponds to "ready to turn in" (the engine
        // also branches on this value in the gossip-icon-color path).
        if (wantActive)
            SetTableBoolField(L, "isComplete", status == 4);

        Game::Lua::SetTable(L, -3);
    }

    return 1;
}

int __fastcall Script_C_GossipInfo_GetAvailableQuests(void *L) {
    return PushQuestList(L, /*wantActive=*/false);
}

int __fastcall Script_C_GossipInfo_GetActiveQuests(void *L) {
    return PushQuestList(L, /*wantActive=*/true);
}

int __fastcall Script_C_GossipInfo_GetNumOptions(void *L) {
    int count = 0;
    for (int slot = 0; slot < Offsets::GOSSIP_OPTIONS_MAX; ++slot) {
        if (OptionIndex(OptionEntry(slot)) >= 0)
            count += 1;
    }
    Game::Lua::PushNumber(L, static_cast<double>(count));
    return 1;
}

int CountQuests(bool wantActive) {
    int count = 0;
    for (int slot = 0; slot < Offsets::GOSSIP_QUESTS_MAX; ++slot) {
        const uint8_t *entry = QuestEntry(slot);
        const uint32_t questID = QuestID(entry);
        if (questID == 0)
            break;
        if (IsActiveQuest(QuestStatus(entry)) == wantActive)
            count += 1;
    }
    return count;
}

int __fastcall Script_C_GossipInfo_GetNumAvailableQuests(void *L) {
    Game::Lua::PushNumber(L, static_cast<double>(CountQuests(false)));
    return 1;
}

int __fastcall Script_C_GossipInfo_GetNumActiveQuests(void *L) {
    Game::Lua::PushNumber(L, static_cast<double>(CountQuests(true)));
    return 1;
}

// Selector-half helpers — translate a retail-shape arg into vanilla's
// 1-based slot index, then re-enter the engine's existing
// `Script_SelectGossip*` by rewriting the Lua stack and tail-calling
// it. That keeps the engine's CMSG-send path and any error-printing
// behavior identical to a native `SelectGossipOption` call.

int CallEngineSelector(void *L, uintptr_t fn, int oneBasedIndex) {
    Game::Lua::SetTop(L, 0);
    Game::Lua::PushNumber(L, static_cast<double>(oneBasedIndex));
    return reinterpret_cast<EngineScriptFn>(fn)(L);
}

// `C_GossipInfo.SelectOption(gossipOptionID[, text])` — wraps vanilla's
// `SelectGossipOption(slotIndex)`. The retail signature also accepts an
// optional password string for boxCoded options; vanilla's
// `SelectGossipOption` doesn't, so we just drop the second arg.
int __fastcall Script_C_GossipInfo_SelectOption(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L,
            "Usage: C_GossipInfo.SelectOption(gossipOptionID)");
        return 0;
    }
    const int target = static_cast<int>(Game::Lua::ToNumber(L, 1));

    int oneBased = 0;
    for (int slot = 0; slot < Offsets::GOSSIP_OPTIONS_MAX; ++slot) {
        const uint8_t *entry = OptionEntry(slot);
        const int32_t optIdx = OptionIndex(entry);
        if (optIdx < 0)
            continue;
        oneBased += 1;
        if (optIdx == target)
            return CallEngineSelector(
                L, Offsets::FUN_SCRIPT_SELECT_GOSSIP_OPTION, oneBased);
    }
    return 0;
}

// `C_GossipInfo.SelectOptionByIndex(orderIndex)` — direct passthrough
// to vanilla's index-based selector. `orderIndex` here is already
// 1-based (matches what `GetOptions()` puts on each entry).
int __fastcall Script_C_GossipInfo_SelectOptionByIndex(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L,
            "Usage: C_GossipInfo.SelectOptionByIndex(orderIndex)");
        return 0;
    }
    const int oneBased = static_cast<int>(Game::Lua::ToNumber(L, 1));
    return CallEngineSelector(
        L, Offsets::FUN_SCRIPT_SELECT_GOSSIP_OPTION, oneBased);
}

int SelectQuestByID(void *L, uintptr_t fn, bool wantActive,
                    const char *errUsage) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, errUsage);
        return 0;
    }
    const uint32_t target = static_cast<uint32_t>(Game::Lua::ToNumber(L, 1));

    int oneBased = 0;
    for (int slot = 0; slot < Offsets::GOSSIP_QUESTS_MAX; ++slot) {
        const uint8_t *entry = QuestEntry(slot);
        const uint32_t questID = QuestID(entry);
        if (questID == 0)
            break;
        if (IsActiveQuest(QuestStatus(entry)) != wantActive)
            continue;
        oneBased += 1;
        if (questID == target)
            return CallEngineSelector(L, fn, oneBased);
    }
    return 0;
}

int __fastcall Script_C_GossipInfo_SelectAvailableQuest(void *L) {
    return SelectQuestByID(
        L, Offsets::FUN_SCRIPT_SELECT_GOSSIP_AVAILABLE_QUEST, false,
        "Usage: C_GossipInfo.SelectAvailableQuest(questID)");
}

int __fastcall Script_C_GossipInfo_SelectActiveQuest(void *L) {
    return SelectQuestByID(
        L, Offsets::FUN_SCRIPT_SELECT_GOSSIP_ACTIVE_QUEST, true,
        "Usage: C_GossipInfo.SelectActiveQuest(questID)");
}

int __fastcall Script_C_GossipInfo_CloseGossip(void *L) {
    return reinterpret_cast<EngineScriptFn>(
        Offsets::FUN_SCRIPT_CLOSE_GOSSIP)(L);
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_GossipInfo", "GetText",
                                     &Script_C_GossipInfo_GetText);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "GetOptions",
                                     &Script_C_GossipInfo_GetOptions);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "GetAvailableQuests",
                                     &Script_C_GossipInfo_GetAvailableQuests);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "GetActiveQuests",
                                     &Script_C_GossipInfo_GetActiveQuests);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "GetNumOptions",
                                     &Script_C_GossipInfo_GetNumOptions);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "GetNumAvailableQuests",
                                     &Script_C_GossipInfo_GetNumAvailableQuests);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "GetNumActiveQuests",
                                     &Script_C_GossipInfo_GetNumActiveQuests);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "SelectOption",
                                     &Script_C_GossipInfo_SelectOption);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "SelectOptionByIndex",
                                     &Script_C_GossipInfo_SelectOptionByIndex);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "SelectAvailableQuest",
                                     &Script_C_GossipInfo_SelectAvailableQuest);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "SelectActiveQuest",
                                     &Script_C_GossipInfo_SelectActiveQuest);
    Game::Lua::RegisterTableFunction("C_GossipInfo", "CloseGossip",
                                     &Script_C_GossipInfo_CloseGossip);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Gossip::Info
