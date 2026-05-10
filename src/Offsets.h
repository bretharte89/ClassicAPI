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
    // `__fastcall(const char *script, const char *scriptName) → bool`.
    // Engine wrapper around `luaL_loadbuffer` + `lua_pcall` that runs a
    // Lua source string with error reporting. Currently unused — we set
    // globals via the Lua C API directly (see `FrameScript_Initialize_h`)
    // — but kept for reference: any future need to execute Lua source
    // from C++ should go through this rather than rolling its own
    // load/pcall sequence.
    FUN_FRAME_SCRIPT_EXECUTE = 0x704CD0,
    FUN_INVALID_FUNCTION_PTR_CHECK = 0x42A320,
    FUN_LOAD_SCRIPT_FUNCTIONS = 0x490250,

    // GameTooltip script-method prologue helpers (used to resolve self → CFrameScriptObject*).
    FUN_FRAMESCRIPT_PUSH_OBJECT = 0x6F3BC0,
    FUN_FRAMESCRIPT_GET_OBJECT = 0x6F3740,

    // Inner spell-tooltip builder, called from Script_GameTooltip_SetSpell at 0x00532E92.
    // __thiscall(spellID, 0, 0, isPet, 0, 0, 0); we always pass isPet=0.
    FUN_GAMETOOLTIP_BUILD_SPELL_TOOLTIP = 0x0052E610,

    // Existing GameTooltip method-table entries we dispatch to from
    // backported convenience methods. Each is `int __fastcall(void *L)`
    // expecting the standard self+args layout on the Lua stack.
    // (Slot numbers are the method-registry index per `docs/raw_methods.txt`.)
    FUN_SCRIPT_GAMETOOLTIP_SET_HYPERLINK = 0x00531FD0, // slot 12
    FUN_SCRIPT_GAMETOOLTIP_SET_UNIT_BUFF = 0x00534AC0, // slot 32
    FUN_SCRIPT_GAMETOOLTIP_SET_UNIT_DEBUFF = 0x00534E30, // slot 33

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
    // CGUnit visible-items helper used by `Script_GetInventoryItemLink` for
    // non-player units (target/party/inspect targets). __thiscall(this=unit,
    // int 0-based slot) → visible-item entry*, or NULL if slot is out of
    // [0, 18]. Reads `[unit + 0xE68]` (visible-items array base for the
    // unit) and indexes `base + 0x118 + slot * 0x30`. Each 0x30-byte entry
    // holds the itemID at `+0x08`; the engine reads it back exactly that
    // way before feeding it to `_GetRecord` for hyperlink construction
    // (verified in `Script_GetInventoryItemLink` at `0x004C8D05`-`0x4C8D34`).
    //
    // **Crash hazard**: `[unit + 0xE68]` is uninitialized for NPCs
    // (CGCreature_C objects). The helper has no NULL check — it computes
    // `garbage + 0x118 + slot*0x30` and returns that as a valid pointer.
    // The engine relies on callers to gate this with `UnitPlayerControlled`
    // first; we do the same in `Item::InventoryID`.
    FUN_UNIT_GET_VISIBLE_ITEM = 0x005F0D60,
    OFF_VISIBLE_ITEM_ITEM_ID = 0x08,

    // CGUnit m_objectFields pointer offset. Different from CGItem's
    // descriptor at +0x114 — these are sibling classes under CGObject
    // with class-specific descriptor offsets.
    OFF_UNIT_DESCRIPTOR = 0x110,
    // Pointer to the CGUnit's 8-byte GUID at `*(CGUnit + 0x08)`. Verified
    // in `Script_GetInventoryItemLink` at `0x004C8CB0`-`0x004C8CB5`:
    // `mov eax, [esi+8]; mov edi, [eax]; mov ecx, [eax+4]` reads the
    // GUID's lo+hi dwords through this indirection. CGItem uses the same
    // offset for its instance block (also containing a GUID + itemID),
    // so the layout is consistent across CGObject subclasses — but the
    // contents differ per class.
    OFF_UNIT_GUID_PTR = 0x08,
    // UNIT_FIELD_FLAGS within m_objectFields. Bit 3 (`0x08`) is
    // `UNIT_FLAG_PLAYER_CONTROLLED`, which `Script_UnitPlayerControlled`
    // (`0x00516410`) tests via `mov eax, [m_objectFields + 0xA0];
    // shr eax, 3; test al, 1`.
    // CGUnit m_objectFields current/max stat offsets, **verified
    // empirically** on Turtle WoW (1.12.1) by `_classicapi_DescDump`
    // searching for the live `UnitMana` value at descriptor offsets:
    //
    //   +0x40  HEALTH       (current)
    //   +0x44  POWER1       (current mana — verified at multiple values)
    //   +0x48  POWER2       (current rage)
    //   +0x4C  POWER3       (current focus)
    //   +0x50  POWER4       (current energy)
    //   +0x54  POWER5       (current happiness)
    //   +0x58  MAXHEALTH    (= 807 at full HP in test data)
    //   +0x5C  MAXPOWER1    (max mana — stays at 1435 even when current = 443)
    //   +0x60..+0x6C  MAXPOWER2..5
    //   +0x70  LEVEL        (= 26 in test data)
    //   +0xA0  FLAGS        (verified separately via `Script_UnitPlayerControlled`)
    //
    // **The 1.12.1 layout is offset 0x18 (= 6 fields) earlier than the
    // CMaNGOS-documented vanilla layout** (which puts HEALTH at field
    // 0x16 = +0x58). My initial implementation read MAXHEALTH/MAXMANA
    // when I wanted current values — caused `IsUsableSpell` to falsely
    // return usable even when mana was below cost. Trust the binary
    // (and `_classicapi_DescDump` if you ever need to re-verify), not
    // external emulator field tables.
    OFF_UNIT_FIELD_HEALTH = 0x40,
    OFF_UNIT_FIELD_POWER1 = 0x44,
    OFF_UNIT_FIELD_LEVEL = 0x70,
    OFF_UNIT_FIELD_FLAGS = 0xA0,
    UNIT_FLAG_PLAYER_CONTROLLED = 0x08,
    // Bit 19 of UNIT_FIELD_FLAGS — `Script_UnitAffectingCombat` at
    // `0x00517E4A`-`0x517E5C` tests it via `mov eax, [fields+0xA0];
    // shr eax, 19; test al, 1`. We use it for `InCombatLockdown`,
    // which in 1.12 collapses to "is the local player in combat" — no
    // secure-frame system here, so the modern lockdown semantics
    // reduce to a plain combat flag check.
    UNIT_FLAG_IN_COMBAT = 0x00080000,
    // Bit 29 of UNIT_FIELD_FLAGS — set by the engine when the player is
    // feigning death (Hunter's `Feign Death`). Standard vanilla
    // `UNIT_FLAG_FEIGN_DEATH = 0x20000000` per emulator source. Tested
    // by `UnitIsFeignDeath(unit)` — works on any unit since
    // UNIT_FIELD_FLAGS is broadcast in object updates.
    UNIT_FLAG_FEIGN_DEATH = 0x20000000,

    // CGPlayer-side sub-struct, allocated for any player-controlled
    // unit (local self, target, party, raid, inspect targets — all of
    // them). Holds player-specific data that's *not* in the broadcast
    // UpdateField descriptor:
    //
    //   +0x08             uint32  PLAYER_FLAGS  (AFK = bit 1, DND = bit 2,
    //                                            RESTING = bit 5)
    //   +0x118 + slot*0x30        visible-items table (slots 0..18,
    //                                            walked by FUN_UNIT_GET_VISIBLE_ITEM)
    //
    // Same offset works for any player; the visible-items helper and
    // `UnitIsAFK`-style flag readers share the same `[unit + 0xE68]`
    // pointer. NPCs / CGCreature_C objects have this slot
    // *uninitialized* — reading without a `UNIT_FLAG_PLAYER_CONTROLLED`
    // gate is a known crash path (helper does `garbage + 0x118 +
    // slot*0x30` then derefs).
    //
    // Flag bits verified by:
    //   - `Script_IsResting` (`0x00516EA0`): `[CGPlayer + 0xE68] +
    //     0x08`, `shr 5; test 1` → bit 5 = RESTING (0x20).
    //   - Nameplate AFK renderer (`0x005EC9E0`): `[unit + 0xE68] +
    //     0x08`, `test [+8], 2` → bit 1 = AFK (0x02). Works for ANY
    //     unit, not just local player — confirmed by user testing
    //     `<AFK>` rendering above other players' heads on stock 1.12.
    //   - DND bit by symmetry with chat-flag protocol; confirmed in-game.
    //
    // PLAYER_FLAGS being out-of-band (in this sub-struct rather than a
    // broadcast UpdateField) is a vanilla-only quirk — modern WoW
    // (3.0+) added PLAYER_FLAGS as a UpdateField at descriptor +0x228,
    // making it queryable through the standard UnitData broadcast
    // path. In 1.12 the +0x228 slot holds something else entirely
    // (multiple read sites in `.text`, but they test for non-zero
    // rather than specific bits — likely duel state).
    OFF_CGPLAYER_INFO = 0xE68,
    OFF_PLAYER_INFO_FLAGS = 0x08,
    PLAYER_FLAG_AFK = 0x02,
    PLAYER_FLAG_DND = 0x04,

    // PackBagSlot — __fastcall(L, void **outInvMgr, int *outLinearSlot, int *outUnused) → bool.
    // Reads bagID at Lua stack[1] and slot at stack[2], validates them, and
    // returns the inventory manager + linear slot ready to feed into GetItemBySlot.
    FUN_PACK_BAG_SLOT = 0x004F9820,
    // Player inventory manager layout — used for direct GUID-array reads
    // that bypass `GetItemBySlot`'s bank gate at `0x006228C1`.
    //   +0x00  uint32  max slot count
    //   +0x04  uint64* pointer to a flat GUID array, indexed by linear slot
    //                   (8 bytes per slot — low dword + high dword)
    //   +0x10  uint8   "bank-aware mode" flag (engine-internal; gates the
    //                   slot-range checks in `GetItemBySlot`'s validation
    //                   path)
    // Slot ranges, matching `PackBagSlot`'s linearization:
    //   0..22   equipment / paperdoll
    //   23..38  backpack (16 slots)
    //   39..62  main bank (24 slots)
    //   63..68  bank bag SLOTS (the bag items themselves; the bag CONTENTS
    //           live in each equipped bag's own invMgr, reachable via the
    //           bag's vtable +0x10)
    //   81+     keyring
    //
    // **Bank slots are populated from server data at login** (verified
    // empirically on Turtle WoW: fresh login + cleared WDB folder shows
    // bank GUIDs already populated in slots 39..62, with the gate at
    // `VAR_BANK_GATE_GUID` = 0). The gate doesn't hide data missing —
    // it hides data that's present from boot. Reading the GUID array
    // directly recovers it without ever opening the bank window.
    OFF_INVMGR_GUID_ARRAY = 0x04,
    INVMGR_BANK_MAIN_FIRST_SLOT = 39,
    INVMGR_BANK_MAIN_LAST_SLOT = 62,
    INVMGR_BANK_BAG_FIRST_SLOT = 63,
    INVMGR_BANK_BAG_LAST_SLOT = 68,
    // Engine's `ObjectMgr::Get`-style resolver — given a type and GUID,
    // returns the resolved CGObject pointer (or null). Same function the
    // engine itself uses inside `GetItemBySlot` (called at `0x00622904`)
    // and `PackBagSlot` (called at `0x004F98E2`). We invoke it directly
    // for bank slots so we sidestep the banker-GUID gate that
    // `GetItemBySlot` applies for slots 39..68 — the GUIDs in
    // `invMgr+0x04` are populated at login, only the gate gets toggled
    // by bank-window state.
    //
    //   __fastcall(int type, const char *debugName, u32 guidLo,
    //              u32 guidHi, int priority) → void *
    //
    // Type values: 2 = item (returns CGItem*), 4 = container/bag
    // (returns CGContainer*). Engine passes `"ItemMgr"` as debugName
    // and `0x172` as priority for both call sites we've decoded.
    FUN_OBJECT_RESOLVE_BY_GUID = 0x00468460,
    OBJ_TYPE_ITEM = 2,
    OBJ_TYPE_CONTAINER = 4,
    // Bank gate. The engine writes the active banker NPC's GUID here
    // when the bank window opens (8-byte qword) and zeroes it on close.
    // `GetItemBySlot` returns null for slots 39..68 if this GUID is zero,
    // even though the slot data in `invMgr+0x04` remains populated.
    // Bypassed in the direct-read bank path; informational only otherwise.
    VAR_BANK_GATE_GUID = 0x00BDD038,
    // `Script_GetContainerNumSlots` Lua C function — `__fastcall(void *L)`.
    // Reads bagID from Lua stack[1] and pushes the bag's slot count back.
    // We invoke this from C-side bag walks instead of hardcoding a max
    // (vanilla bags top out at 24, but custom servers can ship larger
    // bags). Returns 16 for the backpack, the actual bag size for slots
    // 1..4, or 0 if no bag is equipped at the slot — engine handles the
    // edge cases for us. Errors on out-of-range bagIDs.
    FUN_SCRIPT_GET_CONTAINER_NUM_SLOTS = 0x004F9560,
    // `Script_UseContainerItem` Lua C function — `__fastcall(void *L)`.
    // Reads bagID at Lua stack[1] and slot at stack[2], dispatches to the
    // engine's item-use machinery (same path the secure
    // `tooltip:Click()` /  `UseContainerItem(...)` flow takes from
    // Lua). We invoke this from `C_Container.UseHearthstone` after
    // locating the hearthstone in bags.
    FUN_SCRIPT_USE_CONTAINER_ITEM = 0x004FA0E0,
    // Vanilla 1.12 hearthstone is always itemID 6948. Modern WoW recognizes
    // many hearthstone-equivalent toys (Garrison, Dalaran, etc.) but those
    // items don't exist in 1.12, so a single-itemID match is exhaustive.
    HEARTHSTONE_ITEM_ID = 6948,
    // Per-item descriptor block (object/item-field array) lives at this offset
    // on the CGItem instance.
    OFF_ITEM_DESCRIPTOR = 0x114,
    // Within the descriptor, ITEM_FIELD_FLAGS is at +0x3C (a single dword).
    // Bit 0 = soulbound, bit 3 = broken (see GetInventoryItemBroken at 0x4C8626
    // which tests `[descriptor+0x3C] & 0x08`). Confirmed empirically by dumping
    // descriptor bytes for a worn-and-bound item.
    OFF_DESCRIPTOR_FLAGS = 0x3C,
    ITEM_FLAG_SOULBOUND = 0x01,
    // ITEM_FIELD_STACK_COUNT — single dword. Verified by decoding
    // `Script_GetContainerItemInfo` (`0x004F9670`): after resolving the
    // descriptor, `mov eax, [esi+0x114]; fild [eax+0x20]` is the count
    // return. Field index 0x8 in 1.12's item-field layout, which puts
    // STACK_COUNT *before* the contained/creator GUIDs (0x9..0xE) —
    // different from the more common documented layout that places
    // STACK_COUNT after them. Trust the binary, not external docs.
    OFF_DESCRIPTOR_STACK_COUNT = 0x20,
    // ITEM_FIELD_DURABILITY (current) and ITEM_FIELD_MAXDURABILITY (max) live
    // adjacent to each other in the descriptor as plain dwords. Verified in
    // `Script_GetInventoryItemBroken` (`0x004C8590`): after resolving the
    // descriptor, it reads `[ecx+0xA4]` for max and `[eax+0xA0]` for cur,
    // and treats the item as broken when `max > 0 && cur == 0`. Items with
    // no durability concept (consumables, materials) have both fields 0.
    OFF_DESCRIPTOR_DURABILITY = 0xA0,
    OFF_DESCRIPTOR_MAX_DURABILITY = 0xA4,

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

    // `Script_GetItemInfo` Lua C function. We hook this to auto-warm the
    // item cache on miss — vanilla 1.12's stock behavior is "return nil
    // for uncached items and don't fire any query"; modern WoW (5.x+)
    // auto-fires the load and emits `GET_ITEM_INFO_RECEIVED` when data
    // arrives. The hook makes `GetItemInfo(uncached_id)` return nil
    // first call but kick off the query, so subsequent calls work.
    FUN_SCRIPT_GET_ITEM_INFO = 0x0048E070,
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
    // Bag-only fields. `m_containerSlots` (slot count) and `m_bagFamily`
    // (the BagFamily bitfield — quiver=1, ammo pouch=2, soul bag=4 in
    // vanilla) live deep in the record. Offsets derived from
    // VanillaHelpers's `struct ItemStats_C` (full layout, sizeof =
    // ~0x1D4); cross-check by counting fields up through `m_stackable`
    // at +0x60 (which we already use) — `m_containerSlots` is the next
    // u32 at +0x64. `m_bagFamily` is the last u32, at +0x1D0.
    OFF_ITEMSTATS_CONTAINER_SLOTS = 0x64,
    // `m_bagFamily` is a **raw 1-based ID** in 1.12, not the modern
    // bitmask. Vanilla data: arrow=1, bullet=2, soul shard=3, herb=6,
    // etc. Wrath flipped the encoding to `1 << (ID-1)` (so soul shard
    // became 4, herb became 32, …) and addons from Wrath onward expect
    // the bitmask form. Reader functions must convert via
    // `bitmask = id ? (1 << (id - 1)) : 0` to maintain modern API
    // parity. Verified empirically on Turtle WoW: Soul Shard (6265)
    // returns raw 3 here, which is `1 << 2 = 0x4` in modern encoding.
    //
    // Field offset verified by decoding the engine's own
    // `GetItemBagFamily(itemID)` helper at `0x005DA050` — calls
    // `_GetRecord` (`0x0055BA30`) on the item cache (`0x00C0E2A0`),
    // tests the result for null, then `mov eax, [eax+0x1D0]; ret`.
    // (Found via xref scan for `mov reg, [reg+0x1D0]` in `.text`.)
    OFF_ITEMSTATS_BAG_FAMILY = 0x1D0,

    // First character-pane bag-slot index. INVSLOT_BAG1 = 20, BAG2 = 21,
    // BAG3 = 22, BAG4 = 23. Used to map a Lua bagID (1..4) onto the
    // equipment slot the bag occupies: `equipSlot = INVSLOT_BAG1 + bagID - 1`.
    INVSLOT_BAG1 = 20,
    BACKPACK_NUM_SLOTS = 16,

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

    // Faction "displayed list" — the engine maintains a sorted/visible list
    // of factions the player has rep with. `Script_GetNumFactions` (at
    // 0x004D64C0) returns `[VAR_FACTION_DISPLAY_COUNT]` (the primary list
    // count). `Script_GetFactionInfo` (at 0x004D64F0) maps its 1-based
    // index by calling the resolver below, which accepts a slightly wider
    // 0-based range bounded by `[VAR_FACTION_VISIBLE_MAX_INDEX]`. The two
    // ranges differ because the displayed list also exposes the "Inactive"
    // / collapsed-category rows past the primary count.
    //
    // The resolver `FUN_RESOLVE_FACTION_INDEX` is __fastcall(ecx = 0-based
    // index) → factionID (Faction.dbc record ID), or 0 for headers and
    // empty entries. The internal bounds-check is `cmp ecx, [maxIdx]; jbe`
    // — i.e., 0..maxIdx inclusive is valid. Out of range returns 0 too,
    // so we must bound-check ourselves to distinguish "header" (0 in
    // range) from "no such index" (out of range → nil).
    //
    // We re-use the resolver for `GetFactionIDByIndex` directly, and for
    // `GetFactionInfoByID` we walk it to find the displayed-index of a
    // given factionID, then replace Lua arg 1 and tail-call
    // `Script_GetFactionInfo` to produce all 11 returns.
    FUN_RESOLVE_FACTION_INDEX = 0x004D5FA0,
    FUN_SCRIPT_GET_FACTION_INFO = 0x004D64F0,
    VAR_FACTION_DISPLAY_COUNT = 0x00B73764,
    VAR_FACTION_VISIBLE_MAX_INDEX = 0x00B73760,

    // Faction.dbc — standard 5-DWORD class shape, records pointer at +0x08
    // and count at +0x0C of the class instance at 0x00C0DD48. Records is an
    // array of `FactionRec *` indexed directly by factionID (1-based;
    // records[0] is unused). We read it on the slow path of
    // `GetFactionInfoByID` for factions the player has never encountered
    // (i.e. not in the displayed reputation list), where the engine's
    // `Script_GetFactionInfo` can't help us.
    //
    // Record name/description offsets verified by disassembling
    // `Script_GetFactionInfo` at 0x004D6562 / 0x004D6573:
    //   mov edx, [edi+ecx*4+0x4C] ; Name[locale]
    //   mov edx, [edi+edx*4+0x70] ; Description[locale]
    // Each is a 9-entry array of locale string pointers, indexed by
    // `[VAR_LOCALE_INDEX]` (0..8).
    VAR_FACTION_DBC_RECORDS = 0x00C0DD50,
    VAR_FACTION_DBC_COUNT = 0x00C0DD54,
    OFF_FACTION_PARENT_ID = 0x48,
    OFF_FACTION_NAMES = 0x4C,
    OFF_FACTION_DESCRIPTIONS = 0x70,

    // Quest static-info cache (the client-side cache of QUEST_QUERY_RESPONSE
    // payloads — descriptions, objectives, reward text). Same five-arg
    // `_GetRecord` shape as the item cache, keyed by questID. Verified by
    // tracing `Script_GetQuestLogQuestText` (`0x004DFF20`):
    //   1. `[VAR_QUEST_LOG_SELECTED_QUEST_ID]` is the *questID* of the
    //      selected entry (not a pointer) — populated from
    //      `mov eax, [esi + 0x00BB71C0]` which reads field +0 of a
    //      `VAR_QUEST_LOG_ENTRIES` row, i.e. the questID itself.
    //   2. That value is passed as the `key` arg to `_GetRecord`, then
    //      hashed via `key & cache.mask` and matched with `cmp [bucket], key`.
    //
    // Cache-hit-already-loaded shortcut: if `[entry+0x18F8]` is set, the
    // function returns `entry+0x18` immediately and does NOT invoke the
    // queued callback. Modules that want a notification regardless of
    // cache state must fire it themselves (see `Item::Data` for the same
    // pattern).
    //
    // Callback shape verified at the dispatch sites near 0x00562EEB et al.:
    //   `mov ecx, [entry+0x18]; push 1; push ecx; call [entry+8]`
    //   → `__stdcall(void *userData, int success)` where `userData` is
    //   `arg4` we passed to `_GetRecord` (engine stores it at
    //   `entry+0x18` and replays it without dereferencing).
    VAR_QUEST_CACHE = 0x00C0E1B0,
    FUN_DBCACHE_QUEST_GET_RECORD = 0x00562A40,
    VAR_QUEST_LOG_SELECTED_QUEST_ID = 0x00BB7480,

    // Quest log: 16-byte-stride entry array and active count.
    // Field +0 of each entry is the questID for real quests (a category index
    // for headers); field +8 is the header indicator: non-NULL = header,
    // NULL = real quest. Verified by Script_GetQuestLogTitle's isHeader push
    // at 0x004DF9A9 and by the helper at 0x004DF150 used by IsUnitOnQuest.
    VAR_QUEST_LOG_ENTRIES = 0x00BB71C0,
    VAR_QUEST_LOG_ENTRY_COUNT = 0x00BB7478,

    // Player and pet spellbooks — flat int32 arrays indexed by 0-based slot.
    // Each entry is a spellID (0 for unused slots). Engine bounds-checks
    // each slot against an absolute max of `SPELLBOOK_MAX_SLOTS = 0x400`
    // before indexing (see `Script_GetSpellName` at `0x004B3FE0` and the
    // `cmp eax, 0x400; jb` guard at `0x004B4018`); the actual populated
    // count is much smaller, but slots past the populated range simply
    // hold 0, which we surface to Lua as nil.
    //
    // Used by `Spell::Lookup::SpellbookSlotToID` to support the
    // `GetSpellInfo(slot, bookType)` overload.
    VAR_PLAYER_SPELLBOOK = 0x00B700F0,
    VAR_PET_SPELLBOOK = 0x00B6F098,
    SPELLBOOK_MAX_SLOTS = 0x400,

    // Player-spell-knowledge bitmap — `[VAR_PLAYER_SPELL_BITMAP]` is a
    // pointer to a dword bitmap covering all spellIDs the player has
    // learned, including talent passives, racials, profession recipes,
    // and anything else granted via SMSG_LEARNED_SPELL / set in
    // SMSG_INITIAL_SPELLS. One bit per spellID:
    //
    //   bit set ⟺ player knows this spellID
    //   bitmap[spellID >> 5] & (1 << (spellID & 31))
    //
    // Verified by decoding the engine helper at `0x0060C740` (called
    // from `Script_GetTalentInfo`'s currentRank-derivation loop). Matches
    // the same bitmap pattern 5.4.8 uses for its `IsPlayerSpell`
    // (instance moved to `[0x011C25D8]` in that build).
    //
    // Bitmap covers spellIDs 0..VAR_SPELL_RECORD_COUNT inclusive — the
    // size matches Spell.dbc's row count. Pre-login the slot is NULL.
    VAR_PLAYER_SPELL_BITMAP = 0x00B710FC,

    // Per-spell cooldown query. `__fastcall(int spellID, int bookType,
    // int *outStart, int *outDuration, int *outEnable)`. Same path
    // `Script_GetSpellCooldown` (`0x004B40A0`) uses internally after
    // resolving the (slot, bookType) Lua args to a spellID — we
    // bypass the slot resolution and pass spellID directly with
    // `bookType=0` (player) since that's the standard
    // `IsUsableSpell(spellID)` use case. `outDuration > 0` means the
    // spell is currently on cooldown.
    FUN_SPELL_QUERY_COOLDOWN = 0x006E2EA0,

    // Spell.dbc reagent fields. Per CMaNGOS vanilla `SpellEntry` —
    // Reagent[8] at +0x110 (itemIDs), ReagentCount[8] at +0x130
    // (counts). Used by `IsUsableSpell` to check that the player has
    // each non-zero reagent in their bags. Unlike the unit-field
    // offsets, spell-record offsets DO match the CMaNGOS-documented
    // layout in 1.12.1 (verified previously: PowerType=+0x7C and
    // ManaCost=+0x80 both match).
    OFF_SPELL_REAGENT_ID = 0x110,
    OFF_SPELL_REAGENT_COUNT = 0x130,
    SPELL_MAX_REAGENTS = 8,

    // Per-spell *runtime* state cache, indexed by spellID via a hash
    // table (mask at `[VAR_SPELL_STATE_HASH_MASK]`, base at
    // `[VAR_SPELL_STATE_HASH_BASE]`). The engine maintains the cache as
    // player state changes — cooldown, silence, GCD, mana balance, etc.
    // each update flips the relevant byte. The action-bar usability
    // path at `0x004E5BA0` reads `+0x564` (usable) and `+0x568`
    // (noMana) directly off this cache; we do the same to back
    // `IsUsableSpell` / `C_Spell.IsSpellUsable`.
    //
    // `FUN_SPELL_LOOKUP_STATE` is `__fastcall(int spellID) → void *`
    // — the hash-walking helper. Returns null for spells the player
    // doesn't know (cache only holds known spellIDs) or pre-login
    // (when `[VAR_SPELL_STATE_HASH_MASK]` is `-1`).
    FUN_SPELL_LOOKUP_STATE = 0x004F0F40,
    OFF_SPELL_STATE_USABLE = 0x564,
    OFF_SPELL_STATE_NO_MANA = 0x568,

    // Spell description format helper. Reads the locale-resolved description
    // string from `record[+0x228 + locale*4]` and walks it character-by-
    // character substituting `$s1`/`$d`/`$o`/etc. placeholders with values
    // computed from the spell record. Engine has 8 callers (the spell
    // tooltip builder at 0x52F717, the talent UI, the trainer, ...). The
    // global cursor at `0x00BE0B80` is the helper's parser state; it sets
    // and walks it internally, so callers don't manage it.
    //
    // Calling convention (verified at 0x52F717 et al.):
    //   void __fastcall(
    //     void  *spellRecord,      // ecx — Spell.dbc record ptr
    //     char  *outputBuffer,     // edx — caller-provided, null-terminated on return
    //     int   bufLen,            // [ebp+0x08]
    //     int   contextFlag,       // [ebp+0x0C] — small int the talent/trainer paths
    //                              //   compute from a UI-state global; 0 is the safe
    //                              //   "no scaling context" default
    //     int   reserved3,         // [ebp+0x10] — always 0 across the 8 callers
    //     int   useToolTipText,    // [ebp+0x14] — 0=description (+0x228),
    //                              //   non-zero=ToolTip (+0x24C with fallback to +0x228)
    //     int   reserved5,         // [ebp+0x18] — always 1 across the 8 callers
    //     int   reserved6          // [ebp+0x1C] — always 0 across the 8 callers
    //   );
    FUN_FORMAT_SPELL_DESCRIPTION = 0x005075F0,

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
    LUA_TO_BOOLEAN = 0x6F3660,
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

    // Storm allocator — Blizzard's internal C utility library. `SMemAlloc`
    // wraps every block with bookkeeping (header magic + size + caller
    // file:line, footer magic, plus a global registered-block list); the
    // matching `SMemFree` validates against all of that and panics with
    // `ERROR #124 SMem3: Pointer does not refer to a valid allocated block
    // of memory` if the pointer didn't come from `SMemAlloc`.
    //
    // Both are `__stdcall`, 4 args, `ret 0x10`. Verified via disassembly
    // at the function bodies (push order matches arg order; `ret 0x10`
    // pops the 4 stack args). The Storm allocator instance both wrap is
    // the singleton at `0x00C51C58`.
    //
    //   void *__stdcall SMemAlloc(size_t size, const char *file, int line,
    //                             int flags); // flags: bit 3 (0x08) = zero-fill
    //   int   __stdcall SMemFree (void *ptr,    const char *file, int line,
    //                             int flags); // returns 1
    //
    // We use `SMemAlloc` for event-name storage in `Event::Custom` so the
    // engine's reload teardown loop (which `SMemFree`s every event entry's
    // name) doesn't trip its safety check on our injected pointers.
    FUN_STORM_SMEM_ALLOC = 0x006462E0,
    FUN_STORM_SMEM_FREE = 0x00646430,

    // Talent system — per-player talent state populated at login from
    // (class, race) + Talent.dbc / TalentTab.dbc. The engine maintains
    // one `TabInfo *` per talent tab in the array at
    // `[VAR_TALENT_TAB_INFO_ARRAY]`, with the count at
    // `[VAR_TALENT_TAB_COUNT]` (= 3 for all vanilla classes).
    // Verified by `Script_GetTalentTabInfo` (`0x004F3040`),
    // `Script_GetNumTalents` (`0x004F3160`), and `Script_GetTalentInfo`
    // (`0x004F3200`) — all three read the same pair of globals.
    //
    // TabInfo struct:
    //   +0x00  int           TalentTab.dbc rowID
    //   +0x04  int           pointsSpent
    //   +0x08  int           numTalents (in this tab)
    //   +0x0C  TalentEntry * talents (array, 0-indexed)
    //
    // TalentEntry, stride 0x54:
    //   +0x00         uint32 talentID (Talent.dbc primary key)
    //   +0x04..+0x0F  other fields (tier, column, ...)
    //   +0x10..+0x33  uint32 SpellRank[9] (rank-N spellID at index N-1)
    //   +0x34..+0x53  prereqs / flags / other
    //
    // Vanilla 1.12 talents have at most 5 ranks, so SpellRank[5..8] are
    // always zero; engine reserves 9 slots, we expose the same range.
    //
    // SpellRank offset verified by tracing the engine's currentRank-
    // derivation loop in `Script_GetTalentInfo`: it `lea edx, [ebx+0x30]`
    // (cur = highest slot) then walks **downward** with `sub ecx, 4`
    // (visible at 0x004F3305) for 9 iterations, ending at +0x10. So
    // SpellRank[0] (rank 1's spellID) is at +0x10, SpellRank[8] (rank 9
    // sentinel) at +0x30. Confirmed empirically: a debug build reading
    // `[entry+0x30]` returned 0 for Inner Focus (rank 1, 14751); the
    // value is at `[entry+0x10]`.
    VAR_TALENT_TAB_COUNT = 0x00BDCD24,
    VAR_TALENT_TAB_INFO_ARRAY = 0x00BDCD28,
    OFF_TABINFO_NUM_TALENTS = 0x08,
    OFF_TABINFO_TALENT_ARRAY = 0x0C,
    OFF_TALENT_SPELL_RANK = 0x10,
    TALENT_ENTRY_STRIDE = 0x54,
    TALENT_MAX_RANKS = 9,

    // Engine's `Script_GetTalentInfo` Lua C function. We call it from
    // `GetTalentSpellID` to derive the player's currentRank without
    // re-implementing the spell-knowledge checks at `0x0060C740` /
    // `0x0060C8D0`. Signature: `int __fastcall(void *L)` — reads tab and
    // talent index from Lua stack[1] and stack[2], pushes 6 returns
    // (name, icon, tier, column, currentRank, maxRank). Errors (calls
    // lua_error) on invalid input or pre-login state, so callers must
    // pre-validate.
    FUN_SCRIPT_GET_TALENT_INFO = 0x004F3200,

    // Game-time struct populated by SMSG_LOGIN_VERIFY_WORLD /
    // SMSG_LOGIN_SETTIMESPEED. The engine maintains it as the current
    // server clock, advanced by an internal tick handler. `Script_GetGameTime`
    // (`0x00515EE0`) reads `[+0x04]` and `[+0x00]` directly via `fild`,
    // and the engine's "to days-since-epoch" helper at `0x00642320`
    // builds a `struct tm` from `[+0x0C..+0x14]` and calls `_mkgmtime`,
    // confirming the field layout.
    //
    //   +0x00  int  minute     0..59
    //   +0x04  int  hour       0..23
    //   +0x08  ?               (uncertain — possibly seconds, treated as
    //                            0 by the engine's known consumers)
    //   +0x0C  int  day        0-based; engine `inc`s before storing as
    //                            tm_mday (which is 1-based)
    //   +0x10  int  month      0-based (0=Jan, matches tm_mon directly)
    //   +0x14  int  year       full year (e.g. 2026), engine normalizes
    //                            via `(year % 100) + 100` for tm_year
    //
    // Pre-login the struct is BSS-zero; year=0 is the "uninitialized"
    // sentinel we check for.
    VAR_GAMETIME_STRUCT = 0x00CE8538,
    OFF_GAMETIME_MINUTE = 0x00,
    OFF_GAMETIME_HOUR = 0x04,
    OFF_GAMETIME_DAY = 0x0C,
    OFF_GAMETIME_MONTH = 0x10,
    OFF_GAMETIME_YEAR = 0x14,

    // AddOn registry. The engine keeps a flat 4-byte-stride array of
    // `AddOnEntry *` at `[VAR_ADDON_ARRAY]`, with the in-use count at
    // `[VAR_ADDON_COUNT]`. Each entry's first 12 bytes are an inline
    // null-terminated name buffer, so an `AddOnEntry *` can be passed
    // directly to `lua_pushstring` to push the addon's name (which is
    // exactly what `Script_GetAddOnInfo` does for ret1).
    //
    // Per-field accessors (all `__fastcall(ecx=name) → char* or NULL`):
    // each one looks up the addon by name in the engine's hash table,
    // then reads a single hardcoded field from the addon's metadata
    // hash table and returns its string value (NULL if the addon
    // doesn't exist or the field is missing). The `name` arg can be
    // either a real C string OR an `AddOnEntry *` — the entry's
    // first 12 bytes are an inline null-terminated name, so it
    // doubles as a valid `const char *`.
    //
    // We use these directly in the `C_AddOns.*` wrappers to avoid
    // dispatching to `Script_GetAddOnInfo` (which pushes a 7-tuple
    // when we only want one field, and worse, errors via `lua_error`
    // for out-of-range numeric indices).
    FUN_ADDON_GET_BY_INDEX = 0x0051DF00,        // (idx0Based) → AddOnEntry* or NULL
    FUN_ADDON_GET_TITLE_BY_NAME = 0x0051DF20,   // (name) → title string or NULL
    FUN_ADDON_GET_NOTES_BY_NAME = 0x0051E050,   // (name) → notes string or NULL
    FUN_SCRIPT_GET_ADDON_INFO = 0x0048E390,
    VAR_ADDON_ARRAY = 0x00BE1B94,
    VAR_ADDON_COUNT = 0x00BE1B90,

};
