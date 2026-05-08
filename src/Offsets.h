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

enum Offsets {
    FUN_FRAME_SCRIPT_INITIALIZE = 0x7039E0,
    FUN_FRAME_SCRIPT_EXECUTE = 0x704CD0,
    FUN_INVALID_FUNCTION_PTR_CHECK = 0x42A320,
    FUN_LOAD_SCRIPT_FUNCTIONS = 0x490250,

    // GameTooltip script-method prologue helpers (used to resolve self → CFrameScriptObject*).
    FUN_FRAMESCRIPT_PUSH_OBJECT = 0x6F3BC0,
    FUN_FRAMESCRIPT_GET_OBJECT = 0x6F3740,

    // Inner spell-tooltip builder, called from Script_GameTooltip_SetSpell at 0x00532E92.
    // __thiscall(spellID, 0, 0, isPet, 0, 0, 0); we always pass isPet=0.
    FUN_GAMETOOLTIP_BUILD_SPELL_TOOLTIP = 0x0052E610,

    // Iterator that registers an array of frame-method bindings on a per-frame-type
    // method registry (e.g. VAR_GAMETOOLTIP_METHOD_REGISTRY for GameTooltip).
    // __fastcall(ecx = MethodEntry table, edx = count, [stack] = context).
    FUN_REGISTER_FRAME_METHODS = 0x00701D80,
    VAR_GAMETOOLTIP_METHOD_REGISTRY = 0x00C0CF20,

    // Registers a single global Lua function. __fastcall(name, func).
    FUN_FRAMESCRIPT_REGISTER_FUNCTION = 0x00704120,

    // Game::ResolveUnitToken — __fastcall(ecx = const char *token) → CGUnit_C *.
    // Returns the unit pointer for "player", "target", "party1", etc. Use this
    // rather than the global at 0x00B41414 — that global holds something
    // related (its +0xC0 has the player GUID) but is NOT the same CGPlayer_C
    // pointer the inventory routines expect.
    FUN_RESOLVE_UNIT_TOKEN = 0x00515940,
    // Per-player inventory manager lives at this offset on the player object.
    OFF_PLAYER_INVENTORY_MANAGER = 0x1D38,
    // ItemMgr::GetItemBySlot — __thiscall(this, slot) → CGItem* (NULL if empty).
    // Slot is the engine's linearized slot index, not bagID/slot tuple.
    FUN_ITEMMGR_GET_ITEM_BY_SLOT = 0x006228A0,
    // PackBagSlot — __fastcall(L, void **outInvMgr, int *outLinearSlot, int *outUnused) → bool.
    // Reads bagID at Lua stack[1] and slot at stack[2], validates them, and
    // returns the inventory manager + linear slot ready to feed into GetItemBySlot.
    FUN_PACK_BAG_SLOT = 0x004F9820,
    // Per-item descriptor block (object/item-field array) lives at this offset
    // on the CGItem instance.
    OFF_ITEM_DESCRIPTOR = 0x114,
    // Within the descriptor, ITEM_FIELD_FLAGS is at +0x3C (a single dword).
    // Bit 0 = soulbound, bit 3 = broken (see GetInventoryItemBroken at 0x4C8626
    // which tests `[descriptor+0x3C] & 0x08`). Confirmed empirically by dumping
    // descriptor bytes for a worn-and-bound item.
    OFF_DESCRIPTOR_FLAGS = 0x3C,
    ITEM_FLAG_SOULBOUND = 0x01,

    // CGItem has TWO descriptor-like pointers and they hold different things:
    //   +0x114 = m_objectFields (the UpdateField array we read FLAGS from at
    //            +0x3C). For item instances, OBJECT_FIELD_ENTRY in this array
    //            is empirically 0 — don't try to read itemID from here.
    //   +0x08  = a separate identification block. Contains the item's GUID at
    //            +0x00 (qword) and the itemID at +0x0C (dword).
    // The canonical "look up the cache record for this item" sequence is:
    //   item = GetItemBySlot(invMgr, slot)
    //   id   = *(uint32_t *)(*(void **)(item + 0x08) + 0x0C)
    //   _GetRecord(cache, id, ...)
    // Verified at 0x004C8B1F-2D (inventory→cache lookup right after a
    // GetItemBySlot call). The same shape appears in many other inventory
    // sites that call _GetRecord.
    OFF_ITEM_INSTANCE_BLOCK = 0x08,
    OFF_INSTANCE_BLOCK_ITEM_ID = 0x0C,

    // Item stats cache (the client-side cache of ItemSparse-style records that
    // gets populated by SMSG_ITEM_QUERY_SINGLE_RESPONSE). The cache instance
    // lives directly at this VA — `_GetRecord`'s `this` argument is the literal
    // address `0xC0E2A0`, not a pointer to it. See Script_GetItemInfo at
    // 0x0048E070 which calls `mov ecx, 0xC0E2A0` before the call.
    VAR_ITEMDB_CACHE = 0x00C0E2A0,
    // ItemStats_C *(__thiscall *)(void *cache, uint32_t itemID,
    //                             const uint64_t *guid /*may point to zero*/,
    //                             void *callback, void *userData,
    //                             bool requestIfMissing).
    // With `requestIfMissing=false`, returns NULL if not in cache (no server
    // round-trip) — exactly the "instant" semantics we want.
    FUN_DBCACHE_ITEMSTATS_GET_RECORD = 0x0055BA30,
    // ItemStats_C field offsets we read. Full struct layout in
    // VanillaHelpers's `Game.h` (`struct ItemStats_C`); we only need these.
    OFF_ITEMSTATS_CLASS = 0x00,
    OFF_ITEMSTATS_SUBCLASS = 0x04,
    OFF_ITEMSTATS_DISPLAY_INFO_ID = 0x18,
    OFF_ITEMSTATS_INVENTORY_TYPE = 0x2C,

    // ItemClass.dbc — standard 5-DWORD class shape. Records is an array of
    // record pointers indexed directly by classID. Each record has a 9-slot
    // localized name array at +0x0C (4 bytes per locale).
    VAR_ITEMCLASS_RECORDS = 0x00C0DC24,
    VAR_ITEMCLASS_COUNT = 0x00C0DC28,
    OFF_ITEMCLASS_NAMES = 0x0C,

    // ItemSubClass.dbc — class instance at 0x00C0DB90 has the standard 5-DWORD
    // shape (records at +0x08, count at +0x0C, per the reset at 0x53B700), but
    // the engine ALSO maintains a parallel (records, count) pair in the first
    // two unused slots (+0x00, +0x04) for fast compound-key iteration.
    // Script_GetItemInfo and three other callers all read from this parallel
    // pair, NOT the standard slots — verified by greping every reader of
    // 0xC0DB94. Records there is a flat array of 0x74-byte structs (not the
    // standard array-of-pointers); linear scan keyed by (classID, subClassID)
    // at record +0x00 / +0x04.
    //
    // Each record has TWO localized name string arrays:
    //   +0x28  short name (e.g. "Sword")          — 9 locale ptrs, stride 4
    //   +0x4C  verbose/display name (e.g. "One-Handed Swords") — same shape
    // Engine pattern: try verbose first; if that locale's slot is empty,
    // fall back to short. Many subclasses (e.g. Miscellaneous→Junk) only
    // populate the short array, so the fallback matters.
    VAR_ITEMSUBCLASS_RECORDS = 0x00C0DB90,
    VAR_ITEMSUBCLASS_COUNT = 0x00C0DB94,
    OFF_ITEMSUBCLASS_RECORD_STRIDE = 0x74,
    OFF_ITEMSUBCLASS_NAME = 0x28,
    OFF_ITEMSUBCLASS_DISPLAY_NAME = 0x4C,

    // ItemDisplayInfo.dbc — standard layout. Icon path char* at record +0x14
    // (per the helper at 0x005D88B0 that Script_GetItemInfo uses).
    VAR_ITEMDISPLAYINFO_RECORDS = 0x00C0DC10,
    VAR_ITEMDISPLAYINFO_COUNT = 0x00C0DC14,
    OFF_ITEMDISPLAYINFO_ICON = 0x14,

    // INVTYPE_* string table — array of char* indexed by m_inventoryType.
    // Index 0 is empty (0=INVTYPE_NON_EQUIP), indices 1..28 are valid
    // INVTYPE_HEAD..INVTYPE_RELIC, index 29 is empty (sentinel), and indices
    // 30+ are unrelated combat-log strings (MISS/WOUND/DODGE/...). Always
    // bound-check before indexing.
    VAR_INVTYPE_STRING_TABLE = 0x0083DDB0,
    INVTYPE_TABLE_MAX_INDEX = 28,

    // Quest log: 16-byte-stride entry array and active count.
    // Field +0 of each entry is the questID for real quests (a category index
    // for headers); field +8 is the header indicator: non-NULL = header,
    // NULL = real quest. Verified by Script_GetQuestLogTitle's isHeader push
    // at 0x004DF9A9 and by the helper at 0x004DF150 used by IsUnitOnQuest.
    VAR_QUEST_LOG_ENTRIES = 0x00BB71C0,
    VAR_QUEST_LOG_ENTRY_COUNT = 0x00BB7478,

    // Spell.dbc and friends. Pointer-to-records-array + record-count pairs.
    // Used by Script_GetSpellName/Texture and BuildSpellTooltip.
    VAR_SPELL_RECORDS = 0x00C0D788,            // SpellRecord *records[spellID]
    VAR_SPELL_RECORD_COUNT = 0x00C0D78C,       // max spellID
    VAR_SPELL_RANGE_RECORDS = 0x00C0D79C,      // SpellRange.dbc
    VAR_SPELL_RANGE_COUNT = 0x00C0D7A0,
    VAR_SPELL_ICON_RECORDS = 0x00C0D7EC,       // SpellIcon.dbc
    VAR_SPELL_ICON_COUNT = 0x00C0D7F0,
    VAR_SPELL_CAST_TIMES_RECORDS = 0x00C0D878, // SpellCastTimes.dbc
    VAR_SPELL_CAST_TIMES_COUNT = 0x00C0D87C,
    VAR_LOCALE_INDEX = 0x00C0E080,             // 0..8, picks one of the 9 localized strings

    LUA_IS_NUMBER = 0x6F34D0,
    LUA_IS_STRING = 0x6F3510,
    LUA_TO_NUMBER = 0x6F3620,
    LUA_TO_STRING = 0x6F3690,
    LUA_PUSH_NUMBER = 0x6F3810,
    LUA_PUSH_NIL = 0x6F37F0,
    LUA_PUSH_BOOLEAN = 0x6F39F0,
    LUA_PUSH_STRING = 0x6F3890,
    LUA_PUSH_VALUE = 0x6F30D0,    // (was 0x6F3350, which is lua_replace — see docs/LuaCAPI.md)
    LUA_PUSH_CCLOSURE = 0x6F3920,
    LUA_NEW_TABLE = 0x6F3C90,
    LUA_GET_TABLE = 0x6F3A40,     // (was 0x6F3EA0, which is lua_rawset)
    LUA_RAW_GET = 0x6F3B00,
    LUA_SET_TABLE = 0x6F3E20,
    LUA_RAW_SET = 0x6F3EA0,
    LUA_INSERT = 0x6F31A0,
    LUA_TYPE = 0x6F3400,
    LUA_SET_TOP = 0x6F3080,
    LUA_ERROR = 0x6F4940,

    // Global `lua_State *`. The engine keeps one main thread state here; we
    // read it on demand in helpers that run outside a Lua callback (e.g.
    // RegisterTableFunction during LoadScriptFunctions).
    VAR_LUA_STATE = 0x00CEEF74,

    // Static event-name "table" at runtime VA `0x00BE11D8` — this is what
    // `C_EventUtils.IsEventValid` walks. It is NOT the structure that
    // `frame:RegisterEvent` and the dispatcher actually use; that lives at
    // `[VAR_EVENT_TABLE_BASE_PTR]` (see below) with 16-byte entries. The
    // table at 0x00BE11D8 is sufficient for an existence check, but firing
    // or registering custom events requires the real structure.
    VAR_EVENT_NAME_TABLE = 0x00BE11D8,
    EVENT_NAME_TABLE_MAX_SLOTS = 1024,

    // The REAL event registration data structure used by the engine. An
    // `EventEntry` array, 16 bytes per entry:
    //   +0x00 char *name             event name string ptr
    //   +0x04 ?                      used by the chain allocator at 0x7052D0
    //   +0x08 ?
    //   +0x0C SubscriberNode *head   head of the subscribed-frames chain
    // Allocated/populated at engine boot. `[VAR_EVENT_TABLE_BASE_PTR]`
    // dereferences to the array base; `[VAR_EVENT_TABLE_COUNT]` is the
    // entry count. `Frame::RegisterEvent` at 0x00702140 case-insensitively
    // strcmps the input event name against each entry's `+0x00` and inserts
    // the frame into the matching entry's chain.
    VAR_EVENT_TABLE_BASE_PTR = 0x00CEEF68,
    VAR_EVENT_TABLE_COUNT = 0x00CEEF64,
    EVENT_ENTRY_STRIDE = 0x10,
    OFF_EVENT_ENTRY_NAME = 0x00,
    OFF_EVENT_ENTRY_HEAD = 0x0C,

    // `Frame::RegisterEvent` — the C++ helper called by the Lua
    // `frame:RegisterEvent(eventName)` method (`Script_RegisterEvent` at
    // `0x00774A40`). `__thiscall` with `(this=frame, eventName)`. Walks
    // the entry array at `[VAR_EVENT_TABLE_BASE_PTR]`, case-insensitively
    // strcmps against each entry's name, and appends `frame` to the
    // matching entry's chain. We hook this so any addon's RegisterEvent
    // call gives `Event::Custom::RetryAll` a chance to fix up custom
    // events that boot-time registration couldn't claim yet.
    FUN_FRAME_REGISTER_EVENT = 0x00702140,

    // `FireEvent` — the engine's event dispatcher. 149 callers in the
    // binary. `__cdecl void(int eventID, const char *format, ...)`.
    // Indexes into `[VAR_EVENT_TABLE_BASE_PTR] + eventID * 0x10` and walks
    // the subscriber chain at `+0x0C`. Format codes match printf:
    //   %s = const char *
    //   %d = int
    //   %u = unsigned int
    //   %f = float (promoted to double on the stack)
    // No `%b` for boolean — pass `%d` with 0/1; the engine has no native
    // bool concept here. No bounds check on eventID; pass valid indices.
    FUN_FIRE_EVENT = 0x00703F50,
    // Event name strings live in `.data` (mostly clustered around
    // 0x00851000..0x00855000); event-name string pointers also reach into
    // `.rdata` (starts at 0x007FF000) for some entries. Bound the dereference
    // range conservatively to "static-data section" — addresses outside this
    // window cannot be valid string pointers in this binary, so we treat
    // them as NULL during the walk.
    EVENT_TABLE_VALID_PTR_LO = 0x007FF000,
    EVENT_TABLE_VALID_PTR_HI = 0x00C00000,
};
