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
    FUN_SCRIPT_GAMETOOLTIP_SET_INVENTORY_ITEM = 0x00532EE0, // slot 19
    FUN_SCRIPT_GAMETOOLTIP_SET_UNIT_BUFF = 0x00534AC0, // slot 32
    FUN_SCRIPT_GAMETOOLTIP_SET_UNIT_DEBUFF = 0x00534E30, // slot 33
    FUN_SCRIPT_GAMETOOLTIP_SET_TALENT = 0x00535170, // slot 34

    // Iterator that registers an array of frame-method bindings on a per-frame-type
    // method registry (e.g. VAR_GAMETOOLTIP_METHOD_REGISTRY for GameTooltip).
    // __fastcall(ecx = MethodEntry table, edx = count, [stack] = context).
    FUN_REGISTER_FRAME_METHODS = 0x00701D80,
    VAR_GAMETOOLTIP_METHOD_REGISTRY = 0x00C0CF20,

    // "Currently displayed thing" state fields on a GameTooltip frame
    // instance. Each Set* path writes one of these (and zero or two
    // others), and the per-tooltip Clear at FUN_00530050 zeroes all of
    // them on Hide/before-redraw. The Get* methods are simple reads —
    // whichever field is non-zero tells us what kind of tooltip is up.
    //
    // Verified by decoding the builder functions:
    //   - BuildItemTooltip  (0x0052B650) writes +0x380/+0x384 (item
    //                       GUID, only when there's a real CGItem) and
    //                       +0x398 (itemID) at 0x0052B6CE / 0x0052B6FE.
    //   - BuildSpellTooltip (0x0052E610) writes +0x39C (spellID) at
    //                       0x0052E6D5 (param_7==0 branch — skipped for
    //                       the next-rank tooltip side-build).
    //   - Clear             (0x00530050) zeroes all of them.
    OFF_TOOLTIP_ITEM_GUID_LO = 0x380, // 0 for SetItemByID (no CGItem)
    OFF_TOOLTIP_ITEM_GUID_HI = 0x384,
    OFF_TOOLTIP_ITEM_ID = 0x398,
    OFF_TOOLTIP_SPELL_ID = 0x39C,

    // CGItem → fully-dressed item link string. __fastcall(ecx = CGItem *)
    // → const char *. Reads the item's itemID, quality, permanent
    // enchant ID, random-properties seed/factor, and unique ID off the
    // CGItem's instance block + descriptor, builds the dressed name via
    // FUN_005D8BC0 (handles random-suffix decoration like "Foo of the
    // Bear"), then sprintf's into the global buffer at DAT_00C0CF68 and
    // returns its address. The returned pointer is to a long-lived
    // engine global, safe to read until the next call.
    //
    // Same helper Script_GetContainerItemLink (0x004F9930) and
    // Script_GetInventoryItemLink (0x004C8C10) call after resolving
    // their slot-form args. Bypassing them lets us build dressed links
    // for tooltips set via SetMerchantItem / SetLootItem / etc. where
    // the item isn't in the player's bag/equipment.
    FUN_GAMETOOLTIP_BUILD_ITEM_LINK = 0x0052AE00,

    // Engine's inventory swap-and-send. Same primitive
    // `Script_EquipCursorItem` (0x00489660) uses after the cursor's
    // source location has been resolved. Sends opcode 0x10D
    // (CMSG_SWAP_INV_ITEM) for same-container swaps or 0x10C
    // (CMSG_AUTOEQUIP_ITEM) for cross-container, then runs the
    // packet through the engine's own send pipeline at FUN_005AB630.
    //
    // Signature:
    //   void __thiscall(
    //     CGPlayer *this,
    //     u32 srcItemGuidLo, u32 srcItemGuidHi,
    //     u32 srcContainerGuidLo, u32 srcContainerGuidHi,
    //     u32 srcLinearSlot,
    //     u32 dstContainerGuidLo, u32 dstContainerGuidHi,
    //     u32 dstLinearSlot,
    //     int flag);   // 0 = normal path
    //
    // Linear-slot encoding for sources/dests in player invMgr:
    //   0..18  paperdoll (1-based slot - 1)
    //   19..22 equipped bag containers (bag IDs 1..4 themselves)
    //   23..38 backpack contents (1-based bag-0 slot S → 22 + S)
    // For sources in a CGContainer (equipped bag B = 1..4), the
    // container GUID is the bag's own GUID and srcLinearSlot is
    // 0-based within that bag (1-based Lua slot - 1).
    //
    // This call neither reads nor writes the cursor-state globals at
    // [0xBE0810] / [0xBE0814]; cursor visibility is purely a side
    // effect of the cursor-pickup path that normally precedes it.
    // Calling this directly produces a server-side swap with no
    // client-side cursor manipulation.
    FUN_INVENTORY_SWAP = 0x005E0C40,

    // Registers a single global Lua function. __fastcall(name, func).
    FUN_FRAMESCRIPT_REGISTER_FUNCTION = 0x00704120,

    // Direct cvar lookup — `__fastcall(const char *name) → CVar* | NULL`.
    // Hash-table by-name lookup over the CVar registry; same call
    // `Script_GetCVar` makes internally before the engine wraps the
    // result in lua_pushstring + a "CVar doesn't exist" error path.
    // Calling it directly lets us skip both the Lua roundtrip and the
    // unknown-cvar error — we coerce NULL to false instead, matching
    // modern `C_CVar.GetCVarBool` semantics.
    //
    // The returned struct holds the value string at `+0x20` (read by
    // `Script_GetCVar` at `0x00488BF9` as `mov edx, [eax+0x20]; call
    // lua_pushstring`). The value is the raw `char *` the engine
    // stores; reading it directly is safe for the lifetime of the
    // cvar (vanilla cvars don't get re-allocated outside `/console
    // set` flows, which we don't race here).
    FUN_FIND_CVAR = 0x0063DEC0,
    OFF_CVAR_VALUE_STR = 0x20,

    // Game::ResolveUnitToken — __fastcall(ecx = const char *token) → CGUnit_C *.
    // Returns the unit pointer for "player", "target", "party1", etc. Use this
    // rather than the global at 0x00B41414 — that global holds something
    // related (its +0xC0 has the player GUID) but is NOT the same CGPlayer_C
    // pointer the inventory routines expect.
    FUN_RESOLVE_UNIT_TOKEN = 0x00515940,

    // Local-player CGObject-like global. Not the same pointer as
    // ResolveUnitToken("player") returns — that one's the canonical
    // CGPlayer_C used by inventory etc. This pointer's +0xC0 field
    // holds the local player's 64-bit GUID, and the visible-object
    // iterator at FUN_CLNT_OBJ_MGR_ENUM_VISIBLE_OBJECTS walks its
    // +0xAC list. Useful for "is this GUID me?" checks without
    // round-tripping through Lua.
    VAR_LOCAL_PLAYER_PTR = 0x00B41414,
    OFF_LOCAL_PLAYER_GUID = 0xC0,
    // `__fastcall(ecx = const char *token) → uint64_t GUID` — the
    // inner token-to-GUID step that `FUN_RESOLVE_UNIT_TOKEN` calls
    // before doing the active-object lookup. Returns the unit's
    // GUID without depending on the unit being visible / loaded as
    // a CGObject, so `partyN` / `raidN` resolve correctly even when
    // the member is out of range or on a different continent
    // (`Script_UnitName` uses this path too; that's why `UnitName`
    // already works for OOR party members in vanilla).
    //
    // Internal dispatch:
    //   - "player"             → `[localPlayer + 0x08]` (GUID ptr)
    //   - "target"             → `DAT_00B4E2D8`/`DAT_00B4E2DC` globals
    //   - "mouseover"          → `DAT_00B4E2C8`/`DAT_00B4E2CC` globals
    //   - "partyN"             → `FUN_004E81A0(slot)` → `[VAR_PARTY_GUIDS + slot*8]`
    //   - "raidN"              → `FUN_00491940(slot)` → raid GUID array
    //   - "partypetN", "raidpetN" similar (pet-slot variants)
    //   - "<arbitrary name>"   → raises "Unknown unit name: %s" via
    //                            the engine's error helper (so this
    //                            still errors on bad tokens — same
    //                            semantics as the existing
    //                            `UnitGUID` documented behavior).
    //
    // Don't use this from code paths that need to handle literal
    // character names — see CLAUDE.md "Resolving input to a name"
    // for the `lua_pcall(UnitName)` workaround. For pure unit-token
    // input it's the right primitive.
    FUN_TOKEN_TO_GUID = 0x00515970,

    // Chat-event dispatcher — single choke point through which all
    // CHAT_MSG_* events fire after the SMSG_MESSAGECHAT packet handler
    // (opcode 0x96 → FUN_0049D560) parses the wire data. Called with
    // the sender GUID as stack args 9 and 10 (lo, hi).
    //
    // Calling convention: `__fastcall` with 10 args — ECX = sender
    // name string, EDX = chat type, then 8 stack args ending in the
    // GUID pair. Called from:
    //   - FUN_0049D560 directly for live (non-throttled) chat
    //   - The pending-chat queue processor (`__AUPENDINGCHAT`) for
    //     messages buffered via FUN_0049CAE0 when the engine flag at
    //     0x008435FC is set
    //   - Many synthetic chat synthesizers throughout the codebase
    //     (system notifications, arena team membership changes, etc.)
    //     which pass 0 / NULL for the GUID args
    //
    // Hooked by `Chat::CurrentGUID::ChatDispatch_h` to capture the
    // GUID into a global for `GetCurrentChatGUID()` to read during an
    // addon's CHAT_MSG_* OnEvent.
    FUN_CHAT_DISPATCH = 0x0049A870,
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
    // Bit 24 of UNIT_FIELD_FLAGS — set by the engine when a player is
    // possessed (priest's `Mind Control`, warlock's `Subjugate Demon`).
    // Standard vanilla `UNIT_FLAG_POSSESSED = 0x01000000` per emulator
    // source (CMaNGOS/TrinityCore). Read by `UnitIsPossessed(unit)`.
    UNIT_FLAG_POSSESSED = 0x01000000,

    // `UNIT_FIELD_CHANNEL_OBJECT` lo/hi (64-bit GUID of the object the
    // unit is currently channeling onto — bobber for fishing, lock for
    // lockpicking, etc.). CMaNGOS vanilla field 20 (= 0x50) minus the
    // 1.12.1 -0x18 unit-fields shift = 0x38. Verified empirically by
    // diffing the player descriptor during fishing: 0x38/0x3C went
    // `0 → bobber GUID` (high half `0xF110xxxx` = the vanilla
    // GameObject-GUID prefix), back to 0 on cast end.
    //
    // Note: not every channel populates this. The warlock's Ritual of
    // Summoning channel leaves it at 0 — that channel binds to the
    // participants via the spell, not via UNIT_CHANNEL_OBJECT.
    OFF_UNIT_FIELD_CHANNEL_OBJECT = 0x38,

    // `UNIT_FIELD_CHANNEL_SPELL` (u32 spell ID the unit is currently
    // channeling, or 0). CMaNGOS vanilla field 144 (= 0x240) minus the
    // -0x18 shift = 0x228. Verified empirically: the warlock's diff
    // shows `0x228 = 0 → 0x2BA` (698 = Ritual of Summoning) for the
    // duration of his channel; same value appears on every clicker
    // participating in the ritual; fishing populates it with 7620
    // (Fishing spell ID). Broadcast UpdateField, so it's readable for
    // any visible unit, not just the local player.
    OFF_UNIT_FIELD_CHANNEL_SPELL = 0x228,

    // Aura arrays in the unit's `m_objectFields` descriptor (at `unit
    // + OFF_CGUNIT_OBJECT_FIELDS`). 48 total auras packed as two
    // parallel sub-ranges (32 buffs, then 16 debuffs) sharing the
    // flags / levels / applications side-arrays indexed by absolute
    // slot (0..47). Derived by decompiling `Script_UnitBuff`
    // (`0x00519500`) and `Script_UnitDebuff` (`0x005198F0`):
    //
    //   Script_UnitBuff   iterates desc[+0xA4 .. +0x124)  step 4 (slots  0..31)
    //   Script_UnitDebuff iterates desc[+0x124 .. +0x164) step 4 (slots 32..47)
    //   Both read flag nibble at desc[+0x164 + slot/2] >> ((slot&1)*4) & 0xF
    //                and gate on `(nibble & 0x0E) != 0` (visibility)
    //   Both read stacks at desc[+0x1AC + slot] and display as `byte + 1`
    //
    // For our purposes we don't decode individual flag bits — we
    // distinguish helpful vs harmful by absolute slot range, and we
    // derive `dispelName` from `Spell.dbc[+0x10]` (see
    // `OFF_SPELL_DISPEL_TYPE` below), not from the flags nibble.
    OFF_UNIT_FIELD_AURA = 0xA4,                // u32 spell ID per slot, 48 slots total
    OFF_UNIT_FIELD_AURAFLAGS = 0x164,          // 4 bits per aura, 2 per byte, covers all 48
    OFF_UNIT_FIELD_AURALEVELS = 0x17C,         // u8 caster level per aura, 48 bytes
    OFF_UNIT_FIELD_AURAAPPLICATIONS = 0x1AC,   // u8 (stacks-1) per aura, display value = byte+1
    UNIT_AURA_BUFF_COUNT = 32,                 // slot range 0..31 (helpful)
    UNIT_AURA_DEBUFF_COUNT = 16,               // slot range 32..47 (harmful)
    UNIT_AURA_TOTAL = 48,
    UNIT_AURA_VISIBLE_MASK = 0x0E,             // nibble mask used by the engine's visibility gate

    // Reusable "is this spell record a user-visible aura" predicate.
    // `__fastcall(spellRecord*) -> bool`. Checks Spell.dbc Attributes
    // (high bit of byte at +0x18 must be clear), AttributesEx bit
    // 0x10000000 must be clear, has a non-trivial effect (effects at
    // +0x16C..+0x174 must contain at least one not in {0x2C, 0x2D,
    // 0x97}), and a mechanic-vs-player-immunity-bitmap gate via
    // `+0x2B0`. Both `Script_UnitBuff` and `Script_UnitDebuff` call
    // it to filter their aura iteration; we call it the same way.
    FUN_SPELL_IS_VISIBLE_AURA = 0x00519860,

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
    // (3.0+) added PLAYER_FLAGS as a UpdateField at descriptor +0x228.
    // In 1.12, descriptor +0x228 is repurposed as UNIT_CHANNEL_SPELL
    // (see OFF_UNIT_FIELD_CHANNEL_SPELL below); PLAYER_FLAGS only
    // exists on the +0xE68 sub-struct here.
    OFF_CGPLAYER_INFO = 0xE68,
    OFF_PLAYER_INFO_FLAGS = 0x08,
    PLAYER_FLAG_AFK = 0x02,
    PLAYER_FLAG_DND = 0x04,

    // Guild-key field on the CGPlayer sub-struct. Verified by
    // disassembling `Script_GetGuildInfo` at `0x004C9330`: after
    // resolving the unit and checking the GUID-is-player bit, the
    // function reads `mov ecx, [edi+0xE68]; mov ecx, [ecx+0x0C]` —
    // if the value is 0, returns nil; otherwise passes it as the
    // first arg to the guild cache lookup at `0x00560D30`. Two
    // players in the same guild end up with the same value here
    // (the cache key uniquely identifies a guild record), so
    // `UnitIsInMyGuild` can compare this field between `"player"`
    // and the input unit when the data is loaded — sidestepping the
    // roster fetch requirement entirely for visible units.
    //
    // Populated immediately for the local player on guild join, and
    // for any other player-controlled unit whose `+0xE68` sub-struct
    // the engine has synced (party members, raid members, the
    // current target, etc.). For unsynced units it reads 0.
    OFF_PLAYER_INFO_GUILD_KEY = 0x0C,

    // Guild roster — the per-character cache populated by
    // SMSG_GUILD_ROSTER. `[VAR_GUILD_ROSTER_PTR]` is `GuildMember **`
    // (a heap-allocated array of pointers), indexed `[0..total-1]` where
    // total = `[VAR_GUILD_ROSTER_TOTAL_COUNT]`. Includes offline
    // members — the "show offline" UI toggle controls what
    // `GetNumGuildMembers()` returns by default and how the roster
    // panel renders, but the underlying array always holds every
    // member the server sent. Entries can still be NULL for other
    // reasons (e.g. pending refresh), so null-check per entry.
    //
    // Verified by disassembling `Script_GetGuildRosterInfo` at
    // `0x004D1200`:
    //   mov ecx, [0x00B73118]         ; count
    //   cmp eax, ecx; jae bail
    //   mov edx, [0x00B72704]         ; roster base
    //   mov edi, [edx+eax*4]          ; entry = roster[idx]
    //   test edi, edi; je bail
    //   lea edx, [edi+8]; call lua_pushstring   ; name at +0x08 (inline char[])
    //
    // Other GuildMember fields the engine pushes from the same loop:
    //   +0x38  int level
    //   +0x3C  int classIndex
    //   +0x44  int rankIndex
    // (We don't read these, but they're verified in the same function.)
    //
    // The companion `[0x00B7311C]` global is the online-only count
    // (returned by `GetNumGuildMembers()` when called with no args).
    // The roster array spans the full total — both counts share the
    // same backing storage; online vs. total only affects the iteration
    // bound the Lua API uses, not the underlying array.
    VAR_GUILD_ROSTER_PTR = 0x00B72704,
    VAR_GUILD_ROSTER_TOTAL_COUNT = 0x00B73118,
    OFF_GUILD_MEMBER_NAME = 0x08,

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
    // Engine's Lua C functions for the per-instance item link of a bag
    // or character-pane slot. Reads (bagID, slotIndex) from stack[1]/[2]
    // for the container form, or (unit, slot) from stack[1]/[2] for the
    // inventory form. Pushes a fully-decorated `|cffXXXXXX|Hitem:…|h…`
    // hyperlink with the actual enchant / random-suffix data baked in
    // from the per-instance CGItem (not the base cache record). Used by
    // `C_Item.GetItemLink` to match modern semantics where the location
    // form returns the dressed link.
    FUN_SCRIPT_GET_CONTAINER_ITEM_LINK = 0x004F9930,
    FUN_SCRIPT_GET_INVENTORY_ITEM_LINK = 0x004C8C10,
    // `Script_UseContainerItem` Lua C function — `__fastcall(void *L)`.
    // Reads bagID at Lua stack[1] and slot at stack[2], dispatches to the
    // engine's item-use machinery (same path the secure
    // `tooltip:Click()` /  `UseContainerItem(...)` flow takes from
    // Lua). We invoke this from `C_Container.UseHearthstone` after
    // locating the hearthstone in bags.
    FUN_SCRIPT_USE_CONTAINER_ITEM = 0x004FA0E0,
    // Vanilla 1.12 hearthstone is always itemID 6948 and uses spell 8690
    // ("Hearthstone" - 10-second cast, teleport to bind). Modern WoW
    // recognizes many hearthstone-equivalent toys (Garrison, Dalaran,
    // etc.) but those items don't exist in 1.12. Custom servers
    // (Turtle WoW and similar) sometimes ship alternate hearthstone
    // items that reuse spell 8690 as their on-use, so the hearthstone
    // matcher OR's `itemID == HEARTHSTONE_ITEM_ID` against
    // `OnUseSpellIDForItemID(itemID) == HEARTHSTONE_SPELL_ID` to catch
    // both shapes.
    HEARTHSTONE_ITEM_ID = 6948,
    HEARTHSTONE_SPELL_ID = 8690,
    // `Script_PickupContainerItem` Lua C function — `__fastcall(void *L)`.
    // Reads bagID at Lua stack[1] and slot at stack[2]; if an item is
    // present there, it goes onto the cursor (or, if cursor already holds
    // an item, swaps with the bag slot). Used by `EquipItemByName` to
    // pick up the source item before dispatching to the equip helpers.
    FUN_SCRIPT_PICKUP_CONTAINER_ITEM = 0x004F9B30,
    // `Script_EquipCursorItem` Lua C function — `__fastcall(void *L)`.
    // Reads dstSlot at Lua stack[1] (1-based character-pane slot 1..19)
    // and equips the cursor item to that slot. No-op if cursor is empty
    // or the item type doesn't fit the slot.
    FUN_SCRIPT_EQUIP_CURSOR_ITEM = 0x00489660,
    // `Script_AutoEquipCursorItem` Lua C function — `__fastcall(void *L)`.
    // Takes no Lua args; equips the cursor item to its natural slot
    // (engine picks based on inventory type). No-op if cursor is empty.
    FUN_SCRIPT_AUTO_EQUIP_CURSOR_ITEM = 0x0048A040,
    // `Script_CursorHasItem` Lua C function — `__fastcall(void *L)`.
    // Pushes a boolean: 1 if the cursor currently holds an item, 0
    // otherwise. Used by `C_Item.EquipItemByName` to refuse the swap
    // when cursor is non-empty (the cursor-based dispatch path would
    // otherwise clobber the player's held item).
    //
    // Background: 1.12 has a clean CMSG_SWAP_INV_ITEM (0x10D) packet
    // builder at `0x005E0B50` that would let us skip the cursor entirely,
    // matching the 2.4.3 `ItemMgr::EquipByGUID` semantic — but it has
    // zero callers in the binary (verified by xref scan), so it's dead
    // code and doesn't actually send when invoked. The other 0x10D
    // build sites are deeply coupled to internal cursor-tracking
    // globals (`[0x00BE0810]` / `[0x00BE0814]`) and aren't safe to
    // call from outside that state machine. So instead of bypassing
    // the cursor we just refuse to clobber it.
    FUN_SCRIPT_CURSOR_HAS_ITEM = 0x004895D0,
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
    // ITEM_FIELD_SPELL_CHARGES[0] — first of five signed dwords. Field
    // indices 10..14 (fields 8..9 are STACK_COUNT and DURATION; field 15
    // is FLAGS at +0x3C, which gives a tight sandwich around the
    // SPELL_CHARGES range). Derived from the descriptor-field name table
    // builder at `FUN_0047f840`, which consumes a list of `{ name_ptr,
    // ?, count, ?, ? }` entries starting at `0x0083a328` (14 items) —
    // summing the `count` field in declaration order gives each name's
    // starting dword index. STACK_COUNT/FLAGS/DURABILITY/MAX_DURABILITY
    // landing on their known offsets cross-checks the derivation. The
    // tooltip "X Charges" text in `FUN_0052b650` reads from the
    // *enchantment*-charges range at +0x48 (slot N + 0x8) which is a
    // different field (ITEM_FIELD_ENCHANTMENT, fields 16..36).
    //
    // Value is signed: positive = rechargeable (wand recovers charges
    // on use? no — they decrement; positive means "doesn't destroy on
    // last use"). Negative = single-use, destroyed when count hits 0.
    // `GetItemCount(includeUses=true)` uses abs() for the multiplier.
    OFF_DESCRIPTOR_SPELL_CHARGES_0 = 0x28,
    // ITEM_FIELD_DURABILITY (current) and ITEM_FIELD_MAXDURABILITY (max) live
    // adjacent to each other in the descriptor as plain dwords. Verified in
    // `Script_GetInventoryItemBroken` (`0x004C8590`): after resolving the
    // descriptor, it reads `[ecx+0xA4]` for max and `[eax+0xA0]` for cur,
    // and treats the item as broken when `max > 0 && cur == 0`. Items with
    // no durability concept (consumables, materials) have both fields 0.
    OFF_DESCRIPTOR_DURABILITY = 0xA0,
    OFF_DESCRIPTOR_MAX_DURABILITY = 0xA4,

    // Per-item repair-cost helper used by Script_GameTooltip_SetInventoryItem
    // (3rd return value). __fastcall(ecx = CGItem *) -> int copperCost.
    // Returns 0 for null/broken/no-durability/fully-repaired items, otherwise
    // the cost in copper, with the merchant discount applied IFF a merchant
    // window is currently open.
    //
    // Internals: FUN_005DA330 is the raw calculator; FUN_004FAF30 is the
    // discount wrapper that calls it. The raw side:
    //   1. Look up the item's ItemStats record via DBCache_ItemStats_C_GetRecord
    //      (callback=NULL → returns NULL for uncached items).
    //   2. Index DurabilityCosts.dbc by `subClass` to get the per-class row.
    //   3. Branch on inventoryType: weapon (2) uses cols at row+4..row+0x54,
    //      armor (4) uses cols at row+0x58..row+0x78.
    //   4. Multiply by DurabilityQuality.dbc's quality multiplier (indexed
    //      by `quality * 2 + 1`).
    //   5. Round to nearest int; clamp 0 → 1 if there was any cost at all.
    //
    // The discount wrapper:
    //   - Resolves the local player object via FUN_00468550 (returns the
    //     player's GUID from [0x00B41414 + 0xC0]).
    //   - Resolves the current-merchant object via the GUID stored in
    //     DAT_00BDDFA0 / DAT_00BDDFA4. Those globals are set by
    //     FUN_004FACF0 on SMSG_LIST_INVENTORY (merchant frame opens) and
    //     zeroed by FUN_004FAC50 when the merchant frame closes.
    //   - If both lookups succeed, calls FUN_00612B80(merchant, player)
    //     to compute (factionDiscount + pvpRankDiscount). Faction
    //     contribution: 0% below Friendly, +base% (5%) at Friendly and
    //     above. PvP rank contribution: +5% for PvP rank > 5
    //     (Sergeant Major+), another +5% for rank > 7
    //     (Knight-Lieutenant+).
    //   - Otherwise (no merchant context) the multiplier stays at 1.0
    //     and the raw cost is returned undiscounted.
    //
    // So this helper produces the merchant's displayed cost ONLY when
    // the player has a vendor open; called from anywhere else (HUD
    // refresh, idle check) it returns the raw base cost.
    FUN_ITEM_REPAIR_COST = 0x004FAF30,

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
    // `char *m_name[4]` per VanillaHelpers's `struct ItemStats_C`. Only
    // m_name[0] is the primary localized item name (e.g., "Linen Cloth");
    // m_name[1..3] are random-property suffix slots that are typically
    // null for non-randomized items. Used by `C_Item.IsEquippedItem`'s
    // name-string match form.
    OFF_ITEMSTATS_NAME = 0x08,
    OFF_ITEMSTATS_DISPLAY_INFO_ID = 0x18,
    OFF_ITEMSTATS_QUALITY = 0x1C,    // u32 — 0=Poor … 5=Legendary
    OFF_ITEMSTATS_INVENTORY_TYPE = 0x2C,
    OFF_ITEMSTATS_ITEM_LEVEL = 0x38, // u32 — base ilvl from ItemSparse
    // `m_stackable` — max stack size for this item type (1 for non-
    // stackable; 5/20/200/etc. for stackable). The instance-level
    // current stack count is a different field (`OFF_DESCRIPTOR_STACK_COUNT`
    // at +0x20 on the CGItem's m_objectFields descriptor — see
    // `Item::Count.cpp`).
    OFF_ITEMSTATS_STACKABLE = 0x60,
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

    // Item-spell tables — five parallel arrays starting at +0x11C in
    // each `ItemStats_C` record. Each item has up to 5 spell slots
    // (vanilla server data uses ≤2 for most items: e.g. Hearthstone
    // 6948 uses slot 0 for spell 8690 with trigger 0 = `ON_USE`).
    //
    // Field offsets computed by summing struct members in
    // VanillaHelpers's `ItemStats_C` definition; verified empirically
    // against the Hearthstone case (spellID 8690, trigger 0).
    //
    // Trigger enum (CMaNGOS `ITEM_SPELLTRIGGER_*`):
    //   0 = ON_USE        (potions, trinkets, scrolls, hearthstone)
    //   1 = ON_EQUIP      (passive proc auras on equipped gear)
    //   2 = CHANCE_ON_HIT (weapon proc effects)
    //   4 = SOULSTONE     (soulstone-style on-death effect)
    //   5 = ON_USE_NO_DELAY
    //   6 = LEARN_SPELL   (recipe items)
    //
    // `GetItemSpell` matches modern WoW semantics — returns the
    // ON_USE (trigger=0) entry only, ignoring ON_EQUIP procs / weapon
    // procs / recipes. Walks all 5 slots since the ON_USE entry isn't
    // always at index 0 (some Turtle WoW custom items put it later).
    OFF_ITEMSTATS_SPELL_ID = 0x11C,        // u32[5] — spell ID per slot
    OFF_ITEMSTATS_SPELL_TRIGGER = 0x130,   // u32[5] — trigger code per slot
    ITEMSTATS_SPELL_SLOT_COUNT = 5,
    ITEM_SPELLTRIGGER_ON_USE = 0,

    // `Spell.dbc` `Name[9]` field at record +0x1E0 (9-locale string
    // array, indexed by `[VAR_LOCALE_INDEX]`). Used by `GetItemSpell`
    // to push the spell name half of its return.
    OFF_SPELL_NAMES = 0x1E0,

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
    // ChrClasses.dbc — class metadata. Standard 5-DWORD class shape.
    // Records is a flat array of record pointers indexed by classID
    // (records[0] unused, records[1..count]).
    FUN_RESOLVE_FACTION_INDEX = 0x004D5FA0,
    FUN_SCRIPT_GET_FACTION_INFO = 0x004D64F0,
    //
    // Per-record fields used by `FillLocalizedClassList`, derived from
    // `Script_GetSelectedClass` (`0x004716E0`):
    //   +0x14   char *Name[9]    — localized class names (locale-indexed)
    //   +0x38   char *Filename   — class token ("WARRIOR", "MAGE", etc.)
    //
    // Vanilla 1.12 has no separate female-name array — `Name[9]` is
    // exactly the 36 bytes between `+0x14` and `+0x38`, so the
    // `isFemale` arg of `FillLocalizedClassList` is a no-op for this
    // client (and matches the male names that English/most locales
    // use anyway).
    VAR_CHRCLASSES_RECORDS = 0x00C0DEF4,
    VAR_CHRCLASSES_COUNT = 0x00C0DEF8,
    OFF_CHRCLASSES_NAMES = 0x14,
    OFF_CHRCLASSES_FILENAME = 0x38,

    // ChrRaces.dbc — standard 5-DWORD class shape at 0x00C0DED8,
    // records-pointer at +0x08, count at +0x0C. 29 columns,
    // 0x74-byte record stride. Field offsets verified by decoding
    // `Script_UnitRace` (registration entry at 0x00850580, function at
    // 0x00518200): the function loads `[VAR_CHRRACES_RECORDS]` →
    // `records[raceID]` → reads `+0x44 + locale*4` for the localized
    // race name and `+0x3C` for the non-localized file string (e.g.
    // `"Human"`, `"NightElf"`, `"Scourge"`), pushes both as Lua
    // returns. The two offsets are what we mirror to populate
    // `GetPlayerInfoByGUID`'s `localizedRace` / `englishRace` slots.
    VAR_CHRRACES_RECORDS = 0x00C0DEE0,
    VAR_CHRRACES_COUNT = 0x00C0DEE4,
    OFF_CHRRACES_FILENAME = 0x3C,
    OFF_CHRRACES_NAMES = 0x44,

    // AreaTable.dbc — used by `Script_GetCharacterInfo` to resolve
    // each character's last-known-area to a localized zone name.
    // Same shape as the other DBCs (records pointer + count globals,
    // 9-slot locale-string array at the per-record offset).
    VAR_AREATABLE_RECORDS = 0x00C0E048,
    VAR_AREATABLE_COUNT = 0x00C0E04C,
    OFF_AREATABLE_NAMES = 0x2C,

    // Character-select character list. Populated by SMSG_CHAR_ENUM
    // at glue, freed by `FUN_CHARLIST_FREE` on world-enter. Records
    // are 0x120 bytes each in a flat array.
    //
    // We hook FUN_CHARLIST_FREE to snapshot the records into our own
    // cache before the engine frees them, so `C_CharacterList.*` can
    // read the data after the player is in-world (the underlying
    // engine globals are zeroed at that point).
    VAR_CHARLIST_COUNT = 0x00B42140,    // int
    VAR_CHARLIST_ARRAY = 0x00B42144,    // CharRecord *
    CHARLIST_RECORD_STRIDE = 0x120,
    // Per-record field offsets, derived from Script_GetCharacterInfo
    // disassembly (0x004732A0).
    OFF_CHARRECORD_GUID = 0x00,         // u64 character GUID
                                        // (verified via
                                        // Script_RenameCharacter at
                                        // 0x00473520 reading
                                        // `*puVar4` / `puVar4[1]`
                                        // and passing them to the
                                        // rename-by-GUID helper).
    OFF_CHARRECORD_NAME = 0x08,         // inline char[] (max 12 + NUL)
    OFF_CHARRECORD_AREA_ID = 0x3C,      // u32, indexes AreaTable.dbc
    OFF_CHARRECORD_RACE = 0x100,        // u8, indexes ChrRaces.dbc
    OFF_CHARRECORD_CLASS = 0x101,       // u8, indexes ChrClasses.dbc
    OFF_CHARRECORD_SEX = 0x102,         // u8 (0/1)
    OFF_CHARRECORD_LEVEL = 0x108,       // u8

    // Char-list teardown — `__cdecl void()`, walks the record array
    // calling a per-entry cleanup, `SMemFree`s the array, zeros the
    // globals. Only one caller in the entire binary (FUN_0046AC90,
    // the glue-shutdown wrapper that runs on world transition). Very
    // quiet hook target.
    FUN_CHARLIST_FREE = 0x00472090,

    // Engine player-info cache — populated by the
    // SMSG_NAME_QUERY_RESPONSE handler (opcode 0x51) at 0x005551A0,
    // which reads (GUID, name[48], realm[256], race, sex, class) from
    // the packet and writes them into an entry via the cache method
    // at 0x0055F310. Used by chat, raid frames, guild events — any
    // engine code that needs name/class/race for a GUID. NOT limited
    // to visible objects (an earlier note in TODO #35 was wrong — the
    // visible-only cache at 0x00CE870C is a different smaller one
    // used for chat name resolution).
    //
    // Lookup primitive at `FUN_PLAYER_INFO_LOOKUP`:
    //   `__thiscall uint8_t *(this=cache, guid_lo, guid_hi, &cookie,
    //                          callback, userData, retryFlag)`
    //
    // Returns:
    //   - `nullptr` if the GUID isn't loaded (cache miss, or pending
    //     response from a prior request).
    //   - Pointer to the entry's data block (= entry +0x20) if loaded.
    //
    // Side effect: if `callback != nullptr` AND the entry isn't yet
    // allocated, the engine builds a CDataStore, writes the GUID, and
    // calls `FUN_005AB630` to send CMSG_NAME_QUERY (opcode 0x50). The
    // callback fires with `(userData, success)` when the response
    // arrives (`__stdcall void(void *, int)`, same shape as the item
    // cache callback).
    //
    // For pure cache reads (our `GetPlayerInfoByGUID`), pass
    // `callback=nullptr, userData=nullptr, retryFlag=0` and a dummy
    // 8-byte cookie buffer. We don't fire the request from the read
    // path — addons that want to trigger one can layer a
    // `C_PlayerInfo.RequestLoadPlayerByID`-style helper on top later.
    VAR_PLAYER_NAME_CACHE = 0x00C0E228,
    FUN_PLAYER_INFO_LOOKUP = 0x0055F080,

    // NameCache write entry — `__thiscall(this = cache, packetData,
    // guidLo, guidHi)` at 0x0055F310. Called once per
    // SMSG_NAME_QUERY_RESPONSE after the engine's opcode handler
    // builds a `packetData` buffer with the layout:
    //   +0x000  name[48]
    //   +0x030  realm[256]
    //   +0x130  unknown[8]   (probably (charGuidLo, charGuidHi))
    //   +0x138  race  (u32)
    //   +0x13C  sex   (u32, 0/1 wire)
    //   +0x140  class (u32)
    // Writes that data into the cache entry and dispatches any
    // pending callbacks. Decoded by Ghidra; field offsets verified
    // against the `*(uint*)(param_2 + N)` reads in the function body.
    //
    // Hooking here gives us a single chokepoint covering every name-
    // query response the engine processes — chat, group/raid sync,
    // guild roster, visible-object resolution. The hook runs after
    // the engine has settled the entry, so reading from the cache
    // post-hook is safe.
    FUN_PLAYER_INFO_CACHE_WRITE = 0x0055F310,

    // Entry-data offsets. The lookup returns `entry + 0x20`, so these
    // are relative to that pointer (not to the entry's hash-table
    // header). Field layout verified by decoding the cache writer
    // FUN_0055F310 at 0x0055F32D-0x0055F365:
    //   - copy 48 bytes from packet[+0]   into entry+0x20   (name)
    //   - copy 256 bytes from packet[+0x30] into entry+0x50 (realm)
    //   - copy 4 bytes from packet[+0x138] into entry+0x158 (race)
    //   - copy 4 bytes from packet[+0x13C] into entry+0x15C (sex)
    //   - copy 4 bytes from packet[+0x140] into entry+0x160 (class)
    OFF_PLAYER_INFO_NAME = 0x000,   // 48-byte inline C string
    OFF_PLAYER_INFO_REALM = 0x030,  // 256-byte inline C string
    OFF_PLAYER_INFO_RACE = 0x138,   // u32 (race ID 1..8)
    OFF_PLAYER_INFO_SEX = 0x13C,    // u32 (0=male, 1=female on wire)
    OFF_PLAYER_INFO_CLASS = 0x140,  // u32 (class ID 1..11)

    // Visible-object iterator. Walks the engine's linked list of
    // currently-in-range CGObjects (the same list the minimap blip
    // renderer uses). __fastcall(ecx = callback, edx = context).
    // Callback signature: __fastcall(ecx = context, edx unused, [stack] = uint64_t guid).
    // Callback returns 0 to stop iteration, 1 to continue.
    // Verified by disassembling the iterator body at 0x00468380 —
    // ECX/EDX preserved into ESI/EDI, ECX restored to context before
    // each callback invocation, guidLo/guidHi pushed as 8 stack bytes.
    FUN_CLNT_OBJ_MGR_ENUM_VISIBLE_OBJECTS = 0x00468380,

    // GUID → CGObject resolver. __fastcall with this signature:
    //   CGObject *(__fastcall *)(uint32_t typeMask, void *unused,
    //                            uint64_t guid, int unused2);
    // typeMask filters to specific object types; returns NULL if the
    // GUID isn't loaded or its type doesn't match the mask.
    FUN_CLNT_OBJ_MGR_OBJECT_PTR = 0x00468460,

    // Type-mask flags accepted by FUN_CLNT_OBJ_MGR_OBJECT_PTR. Single-bit
    // flags can be OR'd together.
    TYPEMASK_OBJECT        = 0x01,
    TYPEMASK_ITEM          = 0x02,
    TYPEMASK_CONTAINER     = 0x04,
    TYPEMASK_UNIT          = 0x08,
    TYPEMASK_PLAYER        = 0x10,
    TYPEMASK_GAMEOBJECT    = 0x20,
    TYPEMASK_DYNAMICOBJECT = 0x40,
    TYPEMASK_CORPSE        = 0x80,

    // CGUnit_C.m_objectFields slot. Different from CGItem's +0x114 —
    // sibling classes use class-specific descriptor offsets. Inside the
    // descriptor at +0x79 lives the class byte (UNIT_FIELD_BYTES_0
    // field's second byte). Verified at 0x005183FE-0x00518404 in
    // Script_UnitClass's general-unit-token path:
    //   mov edx, [eax + 0x110]      ; descriptor
    //   movzx eax, byte [edx + 0x79] ; class
    OFF_CGUNIT_OBJECT_FIELDS = 0x110,
    // UNIT_FIELD_BYTES_0 packs race/class/sex/powerType into one dword
    // at descriptor +0x78. Individual bytes:
    OFF_UNIT_DESCRIPTOR_RACE_BYTE = 0x78,
    OFF_UNIT_DESCRIPTOR_CLASS_BYTE = 0x79,
    OFF_UNIT_DESCRIPTOR_SEX_BYTE = 0x7A,

    // SkillLineAbility.dbc — maps each (class, race) pair to its
    // learnable spell list with skill-rank gating. Indexed by record
    // ID; reads via the standard `records[id]` pattern. Used by
    // `GetCurrentLevelSpells` to filter the global spell set down to
    // entries our class/race can learn.
    //
    // Class instance VA `0x00C0D94C` per docs/DBCs.md. Standard 5-DWORD
    // class shape so the records-array pointer slot sits at +0x08 and
    // the count slot at +0x0C.
    VAR_SKILL_LINE_ABILITY_RECORDS = 0x00C0D954,
    VAR_SKILL_LINE_ABILITY_COUNT = 0x00C0D958,

    // Record-internal offsets, vanilla 1.12 layout (CMaNGOS-canonical):
    //   +0x00 id
    //   +0x04 skillId       (→ SkillLine.dbc; e.g. 44 = "Swords")
    //   +0x08 spellId       (→ Spell.dbc — the spell taught by this entry)
    //   +0x0C raceMask      (bit `1 << (race - 1)`; 0 = all races)
    //   +0x10 classMask     (bit `1 << (class - 1)`; 0 = all classes)
    //   +0x14 excludeRace   (bitmask of races this entry does NOT apply to)
    //   +0x18 excludeClass  (bitmask of classes this entry does NOT apply to)
    //   +0x1C minSkillRank
    //   ... (more fields)
    OFF_SLA_SKILL_ID = 0x04,
    OFF_SLA_SPELL_ID = 0x08,
    OFF_SLA_RACE_MASK = 0x0C,
    OFF_SLA_CLASS_MASK = 0x10,
    OFF_SLA_EXCLUDE_RACE = 0x14,
    OFF_SLA_EXCLUDE_CLASS = 0x18,

    // SkillLine.dbc — the skill/category each SLA row points into via
    // its `skillId` field. Each record carries a 9-locale localized
    // name array (the user-facing tab name in the spellbook, profession
    // header, weapon-skill name, etc.) plus a SpellIcon ID at +0x54.
    // Standard 5-DWORD class instance at `0x00C0D924`; records-array
    // pointer at `0x00C0D92C`, count at `0x00C0D930`.
    //
    // Record layout (verified against `Script_GetSpellTabInfo` at
    // `0x004B3CE0`):
    //   +0x00 id
    //   +0x0C `Name[9]` — locale string pointers, indexed via the
    //         global locale at `0x00C0E080` (read as
    //         `*(char **)(record + 0x0C + locale * 4)`)
    //   +0x54 SpellIcon.dbc ID (looked up via `SpellIcon` for the
    //         spellbook tab/profession icon)
    VAR_SKILL_LINE_RECORDS = 0x00C0D92C,
    VAR_SKILL_LINE_COUNT = 0x00C0D930,
    OFF_SKILL_LINE_NAME = 0x0C,

    // Spell.dbc `BaseLevel` — the level a spell becomes available
    // (trainer offers it, quest reward grants it, etc.). Distinct
    // from `SpellLevel` at +0x74 which is the effective level used
    // for damage scaling and resistance math. Per CMaNGOS-canonical
    // 1.12 Spell.dbc layout; cross-checked against known fields at
    // ±a few offsets (PowerType at +0x7C is verified).
    OFF_SPELL_RECORD_BASE_LEVEL = 0x70,

    // `UNIT_BYTES_1` (CMaNGOS field 132 in the 1.12.1 layout). 32-bit
    // composite field; the low byte is the standstate (standing /
    // sitting / sleeping / kneeling / etc.), the upper bytes hold
    // PetTalents / VisFlag / AnimTier per CMaNGOS docs (uncalibrated
    // for 1.12). Broadcast field, so the standstate works for any
    // synced unit (player, target, party/raid, inspect targets) —
    // not local-only.
    //
    // Verified via the `IsAssistingRitual` development session by
    // observing `0xEE00 → 0xEE05` on `/sit` over a chair (the medium-
    // chair sit value 5).
    OFF_UNIT_FIELD_BYTES_1 = 0x210,
    // Inner watched-faction setter — `__fastcall(ecx = factionID) → void`.
    // The engine's `Script_SetWatchedFactionIndex` (0x004D6B60) is a
    // thin Lua-side wrapper that takes a 1-based displayed-list index,
    // resolves it to a factionID via FUN_RESOLVE_FACTION_INDEX, and
    // forwards to this. Going through the wrapper requires a round-
    // trip through the resolver (and excludes unencountered factions),
    // so for `C_Reputation.SetWatchedFactionByID` we call this
    // directly with the user-supplied factionID. factionID == 0
    // clears the watched faction (the engine handles 0 as "no
    // watched"). Updates `[player+0xE68]+0x10C4` and persists the
    // value via the engine's CVar / event machinery.
    FUN_PLAYER_SET_WATCHED_FACTION = 0x004D6240,
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

    // Who-query (the /who system).
    //
    // `Script_SendWho` (0x005AD3B0) is a 32-byte wrapper:
    //   - validate arg1 is a string via lua_isstring
    //   - load WhoSystem singleton from `[VAR_WHO_SYSTEM]`
    //   - if non-NULL, lua_tostring(L, 1) for the query string
    //   - tail-call `FUN_WHO_SYSTEM_SEND_QUERY` with
    //     `__thiscall(this = WhoSystem, queryStr)`
    //
    // The query string follows the engine's standard /who filter syntax
    // (`"n-Bob"`, `"r-Alliance"`, `"c-Warlock"`, etc.); the inner
    // function at `FUN_WHO_SYSTEM_SEND_QUERY` (~1.5KB) does the
    // parse + CMSG_WHO build + network send.
    //
    // `Script_SetWhoToUI` (0x005AD870) is even smaller — 13 bytes that
    // store arg1 (as int, default 0) to `[VAR_WHO_TO_UI_FLAG]`. The
    // SMSG_WHO response handler at ~0x005ADF80 reads the flag at
    // `0x005ADF9C` to decide whether the results buffer into the
    // engine's WhoList (flag != 0 → list path, friends-frame fires
    // WHO_LIST_UPDATE; flag == 0 → results go to chat as
    // `"Found N players matching..."`).
    //
    // Server-side cooldown for CMSG_WHO is ~5 seconds — a faster
    // client gets silent-dropped, so any client-side gating just
    // matches that.
    VAR_WHO_SYSTEM = 0x00C28168,
    VAR_WHO_TO_UI_FLAG = 0x00C2A12C,
    FUN_WHO_SYSTEM_SEND_QUERY = 0x005AEBB0,

    // SMSG_WHO opcode handler — opcode 0x63 (99) per the registration
    // in `FUN_005adc50`: `FUN_005ab650(99, FUN_005adf60, 0)`. Reads
    // `VAR_WHO_TO_UI_FLAG` mid-function (at `0x005ADF9C`) to decide
    // between WhoList + WHO_LIST_UPDATE vs the chat-count printer.
    //
    // We hook post-original to restore the flag after each response
    // so the routing override only affects responses we initiated.
    // Otherwise a user-typed `/who` inheriting our flag=1 stays
    // silent (no chat-count message, no FriendsFrame popup) when the
    // user actually wanted both.
    FUN_SMSG_WHO_RESPONSE = 0x005ADF60,

    // Per-faction current standing — `__fastcall(ecx = factionID) → int`.
    // Returns `base + delta` where the two values are stored at
    // `+0x08` / `+0x0C` of the player's rep slot (slot stride 0x10,
    // 64 slots at `0x00B73290`). Returns 0 for factionIDs not in the
    // player's reputation list (i.e. unencountered). Used by the
    // FACTION_STANDING_CHANGED event firing to push the live total
    // after the engine's setter has written the new delta.
    FUN_REPUTATION_GET_STANDING = 0x004D6370,

    // Reaction-band classifier — `__fastcall(ecx = factionID) → int`.
    // Internally calls `FUN_REPUTATION_GET_STANDING` and maps the total
    // through the 7-band threshold ladder (Hated..Exalted). Returns
    // 0..7 (0=Hated, 7=Exalted); `Script_GetWatchedFactionInfo` pushes
    // this `+ 1` to produce the 1..8 standingID Lua addons see.
    FUN_REPUTATION_GET_REACTION_BAND = 0x004D63A0,

    // Reputation-list slots — the player's 64-entry per-faction rep
    // store. Stride `0x10`; layout per entry:
    //   +0x00  i32  factionID (0 if slot unused)
    //   +0x04  u8   flags (bit 1 = atWar, bit 4 = canToggleAtWar)
    //   +0x08  i32  base standing
    //   +0x0C  i32  delta standing
    // Slot index = RepListID = Faction.dbc record `+0x04`. Indexed by
    // `FUN_004D5600(factionID) → repListID`. Reverse direction is
    // `FUN_004D5620(repListID) → factionID`.
    VAR_PLAYER_REP_SLOTS = 0x00B73290,
    REP_SLOT_STRIDE = 0x10,
    OFF_REP_SLOT_FACTION_ID = 0x00,
    OFF_REP_SLOT_FLAGS = 0x04,
    OFF_REP_SLOT_BASE_STANDING = 0x08,
    OFF_REP_SLOT_DELTA_STANDING = 0x0C,
    REP_SLOT_FLAG_AT_WAR = 0x02,
    REP_SLOT_FLAG_CAN_TOGGLE_AT_WAR = 0x10,
    MAX_REP_SLOTS = 64,

    // Static reaction-band threshold tables, indexed by reaction band
    // 0..7 (returned by `FUN_REPUTATION_GET_REACTION_BAND`). The pair
    // gives the `[min, max)` standing range that defines each band —
    // currentReactionThreshold / nextReactionThreshold for modern
    // `C_Reputation.GetWatchedFactionData`-style table fields.
    VAR_REACTION_MIN_TABLE = 0x0080928C,
    VAR_REACTION_MAX_TABLE = 0x00809290,

    // `[player +0xE68] + 0x10C4` — the RepListID (rep-slot index) of
    // the faction currently being watched (the one shown above the XP
    // bar). Written by `FUN_PLAYER_SET_WATCHED_FACTION` after
    // translating the user-supplied factionID via
    // `FUN_004D5600 → RepListID`. Negative or out-of-range means "no
    // watched faction", which `Script_GetWatchedFactionInfo` checks
    // against `FUN_004D5620(slot)`'s return.
    OFF_CGPLAYER_INFO_WATCHED_REP_LIST_ID = 0x10C4,

    // `__fastcall(ecx = factionID, edx = signedDelta)`. This is the
    // engine's "reputation changed, fire the chat event" notify
    // helper — it formats the localized chat message and dispatches
    // `CHAT_MSG_COMBAT_FACTION_CHANGE`. Called from `FUN_004D6330`
    // (the per-slot setter) at `0x004D635C`, only when:
    //   1. The stored standing value actually changed for this slot.
    //   2. The setter's `notify` arg was non-zero (i.e. the call
    //      came from the per-update SMSG handler, not the bulk init
    //      handler that runs at login).
    // Hooking here matches modern WoW's `FACTION_STANDING_CHANGED`
    // semantics: fires once per real reputation change, skipping the
    // initial faction sync at login. ECX/EDX layout means the hook
    // can read factionID + delta in registers; for the modern
    // `(factionID, newStanding)` payload, call
    // `FUN_REPUTATION_GET_STANDING(factionID)` from inside the hook
    // (the setter has already written the new value by this point,
    // so it returns the updated total).
    FUN_REPUTATION_FIRE_NOTIFY = 0x0062C5F0,

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

    // The currently-auto-repeating spellID (0 when nothing is
    // auto-repeating). Same dword backs Shoot/Wand/Auto-Shot — no
    // per-class branching. Written by the cast-start path at
    // `FUN_006E54F0` (line `DAT_00ceac30 = *piVar5;` where `piVar5` is
    // the Spell.dbc record) and cleared on logout/world unload. Read by
    // `Script_IsAutoRepeatAction`'s inner helper at `0x004E55B0` and
    // exposed as a 1-line getter at `0x006E9FD0`.
    VAR_ACTIVE_AUTO_REPEAT_SPELL = 0x00CEAC30,

    // `Script_CastSpellByName` — engine's Lua wrapper for the
    // `CastSpellByName(name [, onSelf])` global. `int __fastcall(void *L)`
    // — standard Lua C function ABI. Reads `name` from stack[1] (string)
    // and `onSelf` from stack[2] (boolean), resolves the name (with
    // optional `(Rank N)` suffix) through `FUN_004B3950`, then dispatches
    // to the engine's cast path at `FUN_004B3300`. Returns 0 (no Lua
    // results). Callable directly from C++ — push args on stack, call.
    FUN_SCRIPT_CAST_SPELL_BY_NAME = 0x004B4AB0,

    // Macro "primary spell" parser — `__fastcall(MacroEntry *entry)`.
    // Walks the macro body (stored at `entry + OFF_MACRO_BODY`) line
    // by line, looking for `/cast <name>`, `/castsequence <name>`,
    // etc. (via the engine's `SLASH_CAST%d` registry) and the
    // `CastSpellByName("<name>")` Lua pattern. On first hit, resolves
    // the name to a spellID via `FUN_004B3BC0` and writes it to
    // `entry + OFF_MACRO_PRIMARY_SPELL`. Writes `0xFFFFFFFF` when a
    // pattern matched but the name didn't resolve (unknown spell);
    // writes `0` when no line matched any pattern. Called from macro
    // create/edit/refresh paths.
    //
    // Hooked by `Spell::AutoRepeat` to teach the engine to recognize
    // `CastAutoRepeatSpell("<name>")` as a primary-spell pattern, so
    // macros using it get tagged in the spell-state cache the same
    // way `CastSpellByName` macros do. Without this, `IsAutoRepeatAction`
    // returns false for macro slots whose only cast is via our function
    // (the engine's parser doesn't know to look for our identifier),
    // breaking action-bar highlighting in pfUI etc.
    FUN_MACRO_PARSE_PRIMARY_SPELL = 0x004EFE00,

    // Engine's "spell name → spellbook-resolved spellID" helper.
    // `__fastcall(const char *name, int *outIsPet)`. Internally calls
    // `FUN_004B3950` (the rank-stripping name-with-`(Rank N)` parser)
    // to get a spellbook slot, then returns
    // `[VAR_PLAYER_SPELLBOOK][slot]` (or `[VAR_PET_SPELLBOOK][slot]`
    // when `*outIsPet` is set on entry). Returns 0 if the name doesn't
    // resolve to a known spellbook entry. This is the resolver the
    // engine's macro parser uses — name → spellID lookup that respects
    // the player's known spell list.
    FUN_RESOLVE_SPELL_NAME_TO_BOOK_ID = 0x004B3BC0,

    // The single name → spellbook-slot resolver underneath everything
    // spell-cast related. `__fastcall(const char *name, void *out) ->
    // int slot` (or `-1`). Strips a trailing `(Rank N)` suffix, then
    // calls `FUN_004B3A10` for the actual lookup. Called by
    // `Script_CastSpellByName` (every Lua cast) AND by
    // `FUN_RESOLVE_SPELL_NAME_TO_BOOK_ID` (which the macro parser
    // uses). Hooking here makes one change to both the runtime cast
    // path AND the macro-tagging path — useful for accepting numeric
    // spellID input as if it were a name (the `Spell::CastByID`
    // module does this to enable `/cast 5019`-style macros).
    FUN_RESOLVE_SPELL_NAME_TO_SLOT = 0x004B3950,

    // MacroEntry struct offsets (verified by tracing FUN_004EFE00).
    // The macro body is an inline null-terminated string at `+0x164`;
    // line breaks are `\n`. The primary-spell cache (what
    // `IsAutoRepeatAction` ultimately reads via the spell-state hash
    // lookup at `[0x00C0E2A0]`) is at `+0x564`.
    OFF_MACRO_BODY = 0x164,
    OFF_MACRO_PRIMARY_SPELL = 0x564,

    // Spell.dbc and friends. Pointer-to-records-array + record-count pairs.
    // Used by Script_GetSpellName/Texture and BuildSpellTooltip.
    VAR_SPELL_RECORDS = 0x00C0D788,            // SpellRecord *records[spellID]
    VAR_SPELL_RECORD_COUNT = 0x00C0D78C,       // max spellID

    // Spell.dbc School field — 0-based integer at record +0x04.
    // Verified empirically against Fireball (133) → School=2 (Fire)
    // and Frostbolt (116) → School=4 (Frost) on Octo 1.12.1. Vanilla
    // doesn't use the multi-bit SchoolMask form (TBC+); a spell
    // belongs to exactly one school. Mapping:
    //   0 = Physical  1 = Holy  2 = Fire    3 = Nature
    //   4 = Frost     5 = Shadow 6 = Arcane
    OFF_SPELL_SCHOOL = 0x04,
    SPELL_SCHOOL_COUNT = 7,

    // Spell.dbc DispelType field — u32 at record +0x10, indexes into
    // `SpellDispelType.dbc` (`VAR_SPELLDISPEL_RECORDS` below). 0 =
    // not dispellable. Verified by decompiling `Script_UnitDebuff`,
    // which reads `*(int*)(spellRecord + 0x10)` and feeds it into
    // the DispelType DBC to produce the third return value (the
    // dispel-type name string).
    OFF_SPELL_DISPEL_TYPE = 0x10,

    VAR_SPELL_RANGE_RECORDS = 0x00C0D79C,      // SpellRange.dbc
    VAR_SPELL_RANGE_COUNT = 0x00C0D7A0,
    VAR_SPELL_ICON_RECORDS = 0x00C0D7EC,       // SpellIcon.dbc
    VAR_SPELL_ICON_COUNT = 0x00C0D7F0,
    VAR_SPELL_CAST_TIMES_RECORDS = 0x00C0D878, // SpellCastTimes.dbc
    VAR_SPELL_CAST_TIMES_COUNT = 0x00C0D87C,

    // SpellDispelType.dbc — maps dispel-type IDs (1..N) to localized
    // names like "Magic", "Curse", "Disease", "Poison". Used by
    // `Script_UnitDebuff` to fill its third return value. Standard
    // 5-DWORD class instance shape; records array at +0x08, count at
    // +0x0C of the instance. Records themselves have a "has-name"
    // gate at +0x28 (non-zero means the name slot is populated) and
    // the locale-applied string pointer at +0x2C. The engine's
    // accessor does the locale lookup before exposing the pointer
    // here, so we get a ready-to-use string with no further indexing.
    VAR_SPELLDISPEL_RECORDS = 0x00C0D83C,
    VAR_SPELLDISPEL_COUNT = 0x00C0D840,
    OFF_SPELLDISPEL_HAS_NAME = 0x28,
    OFF_SPELLDISPEL_NAME = 0x2C,

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
    // `lua_pushlstring(L, ptr, len)` — binary-safe push. Same TValue
    // tag as `pushstring` but takes an explicit length so the string
    // can contain NULs.
    LUA_PUSH_LSTRING = 0x6F3840,
    // `lua_strlen(L, idx)` — returns the byte length of the string
    // value at `idx`. Pairs with `lua_tostring` for binary-safe
    // string reads.
    LUA_STR_LEN = 0x6F36E0,
    // Verified empirically via `_classicapi_PushValueProbe` —
    // pushes a number then dups via PushValue, checks GetTop tracks.
    // **Doc note:** `docs/LuaCAPI.md` reports `0x6F30D0` as lua_pushvalue
    // and `0x6F32B0` as lua_remove; both are wrong. `0x6F30D0` is
    // actually `lua_remove` (has the shift-down loop and `decr_top`),
    // and `0x6F3350` is `lua_pushvalue` (index-resolve → TValue copy
    // to L->top → `add [ecx+8], 0x10` incr_top). `0x6F32B0` is
    // `lua_replace` (single TValue copy + decr_top).
    LUA_PUSH_VALUE = 0x6F3350,
    LUA_PUSH_CCLOSURE = 0x6F3920,
    LUA_NEW_TABLE = 0x6F3C90,
    LUA_GET_TABLE = 0x6F3A40,     // (was 0x6F3EA0, which is lua_rawset)
    LUA_RAW_GET = 0x6F3B00,
    LUA_SET_TABLE = 0x6F3E20,
    LUA_RAW_SET = 0x6F3EA0,
    LUA_INSERT = 0x6F31A0,
    LUA_REMOVE = 0x6F30D0,        // (was 0x6F32B0, which is lua_replace)
    LUA_REPLACE = 0x6F32B0,
    LUA_TYPE = 0x6F3400,
    LUA_GET_TOP = 0x6F3070,
    LUA_SET_TOP = 0x6F3080,
    LUA_CALL = 0x6F4180,
    LUA_PCALL = 0x6F41A0,
    // `lua_next(L, idx)` — pops a key, pushes (newKey, value) for the
    // next pair in the table at `idx`. Returns 0 when iteration is
    // done. Documented in `docs/LuaCAPI.md`; calls `luaH_next` at
    // `0x006FA2A0` internally.
    LUA_NEXT = 0x6F4450,
    LUA_ERROR = 0x6F4940,

    // Global `lua_State *`. The engine keeps one main thread state here; we
    // read it on demand in helpers that run outside a Lua callback (e.g.
    // RegisterTableFunction during LoadScriptFunctions).
    VAR_LUA_STATE = 0x00CEEF74,

    // zlib 1.2.2, statically linked into WoW.exe (version string at
    // `0x008745F0`, copyright banners at `0x00815528`/`0x00815608`).
    //
    // One-shot helpers (always Zlib-format with adler32 trailer):
    //   int compress2(dest, *destLen, source, sourceLen, level)
    //   int uncompress(dest, *destLen, source, sourceLen)
    //
    // Lower-level streaming API — needed for Gzip and raw Deflate
    // formats since the one-shots only do Zlib format. The `Init2_`
    // variants take an explicit `windowBits` which selects format:
    //   +15        = Zlib (default)
    //   +15 + 16   = Gzip
    //   -15        = raw Deflate (no header, no checksum)
    //   +15 + 32   = auto-detect (decode only; Zlib or Gzip)
    //
    // Call signatures (all `__fastcall`, callee cleans the stack):
    //   int deflateInit2_(strm[ECX], level[EDX],
    //                     stack: method, windowBits, memLevel,
    //                            strategy, version, stream_size)
    //   int deflate(strm[ECX], flush[EDX])
    //   int deflateEnd(strm[ECX])
    //   int inflateInit2_(strm[ECX], windowBits[EDX],
    //                     stack: version, stream_size)
    //   int inflate(strm[ECX], flush[EDX])
    //   int inflateEnd(strm[ECX])
    //
    // Return codes match zlib's: 0 = Z_OK, 1 = Z_STREAM_END,
    // -3 = Z_DATA_ERROR, -4 = Z_MEM_ERROR, -5 = Z_BUF_ERROR.
    FUN_ZLIB_COMPRESS2 = 0x00730BC0,
    FUN_ZLIB_UNCOMPRESS = 0x00734810,
    FUN_ZLIB_DEFLATE_INIT2 = 0x00732A70,
    FUN_ZLIB_DEFLATE = 0x00733040,
    FUN_ZLIB_DEFLATE_END = 0x00733650,
    FUN_ZLIB_INFLATE_INIT2 = 0x00730D60,
    FUN_ZLIB_INFLATE = 0x00730EA0,
    FUN_ZLIB_INFLATE_END = 0x00732320,
    // The version string deflateInit2_/inflateInit2_ checks against
    // (compile-time mismatch returns Z_VERSION_ERROR). Same string the
    // one-shot helpers pass through.
    VAR_ZLIB_VERSION_STRING = 0x008745F0,
    ZLIB_STREAM_SIZE = 0x38,  // sizeof(z_stream) in 1.2.2 build

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
    // matching entry's chain.
    FUN_FRAME_REGISTER_EVENT = 0x00702140,

    // `RebuildEventTable` — the engine's bulk event-table population
    // routine. `__fastcall(const char **names, int count) -> void`
    // (ECX = names array, EDX = count). Tears down any existing table
    // (calling the per-entry teardown helper at 0x00701A40 + freeing
    // the base pointer via SMemFree), allocates a fresh table of size
    // `count * 16`, then loops `names[0..count-1]` and for each
    // non-empty entry calls `SStrDup(name, file, line)` to allocate
    // an engine-owned copy at `entry+0x00`.
    //
    // Called twice during boot (with two different name arrays — 26
    // entries at 0x00B41E70, then 549 entries at 0x00BE1198) and once
    // per `/reload`. We **don't hook this** — third-party DLLs
    // (SuperWoWhook, transmogfix, nampower, etc.) all hook here too, and
    // chaining their modifications to the `(names, count)` args makes the
    // final table size unreliable (crashes seen with count → buffer-size
    // mismatch). Instead, `Event::Custom` claims NULL slots from the live
    // table after the rebuild settles. Kept for reference.
    FUN_REBUILD_EVENT_TABLE = 0x00703D90,

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

    // `SStrDup(const char *src, const char *file, int line)`. `__stdcall`.
    // Storm's string-copy wrapper around `SMemAlloc` — used by the engine
    // itself to populate event entry names. We use it from `Event::Custom`
    // so the engine's reload-teardown `SMemFree` sees blocks that came
    // from `SMemAlloc` (which it requires).
    FUN_STORM_SSTRDUP = 0x0064A620,

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

    // Talent.dbc — standard 5-DWORD class shape, records pointer at
    // `+0x08` and count at `+0x0C` of the class instance at
    // `0x00C0D6E0`. Records is an array of `TalentRec *` indexed
    // directly by talentID (sparse — many slots NULL since talent IDs
    // skip across classes). Used by `GameTooltip:SetTalentByID` to
    // resolve cross-class talents that aren't in the local player's
    // loaded TabInfo arrays.
    //
    // Record layout (verified standard vanilla Talent.dbc schema):
    //   +0x00  uint32  ID
    //   +0x04  uint32  TabID
    //   +0x08  uint32  TierID (row, 0-based)
    //   +0x0C  uint32  ColumnIndex (0-based)
    //   +0x10  uint32  SpellRank[0..8]   — rank-1 spellID at +0x10
    //
    // The per-player `TalentEntry` (in `TabInfo->talents[]`) shares
    // the same SpellRank offset, which is why we can reuse
    // `OFF_TALENT_SPELL_RANK` here without redefining it.
    VAR_TALENT_DBC_RECORDS = 0x00C0D6E8,
    VAR_TALENT_DBC_COUNT = 0x00C0D6EC,

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

    // Per-field accessors `Script_GetAddOnInfo` calls internally for
    // returns 5 (loadable), 6 (reason string), 7 (security category).
    // We dispatch to these directly so the wrappers don't need to
    // bounce through `Script_GetAddOnInfo` and unwind 7 returns.
    //
    //   IS_LOAD_ON_DEMAND `__fastcall(entry) → byte`
    //     Hash-table lookup in the addon registry. Non-zero ⇒
    //     `## LoadOnDemand: 1` in the .toc.
    //
    //   CAN_LOAD `__fastcall(entry, char fullCheck=1, int *outStatus,
    //                        int *outDepHandle, const char *accountName) → byte`
    //     Recursive load-check that resolves all forward `## Dependencies`.
    //     On `0` return, writes one of 8 status codes into `*outStatus`
    //     (see `VAR_ADDON_LOADABLE_REASON_TABLE` for the index→string
    //     mapping). `outDepHandle` carries a dep-pointer when status is 0
    //     ("DEP: %s" path), unused otherwise. We pass `&zeroDep` for
    //     `outDepHandle` — we don't render the dep-name into the reason
    //     since the modern API gives just the status enum.
    //
    //   GET_REASON_STRING `__thiscall(int statusCode, char *buf, uint cap)`
    //     Copies the status-code's static string into `buf`, or formats
    //     `"DEP: <name>"` when `this == 0`. We bypass this entirely and
    //     index the table directly (see `VAR_ADDON_LOADABLE_REASON_TABLE`).
    //
    //   GET_SECURITY_INDEX `__fastcall(entry) → uint`
    //     Hash-table lookup that returns the security category index
    //     (0=SECURE, 1=INSECURE, 2=BANNED). Defaults to `1` (INSECURE)
    //     when the entry isn't in the override table.
    FUN_ADDON_IS_LOAD_ON_DEMAND = 0x0051E6F0,
    FUN_ADDON_CAN_LOAD = 0x0051E780,
    FUN_ADDON_GET_SECURITY_INDEX = 0x0051E990,

    // Returns a pointer to the inline-stored login account name buffer
    // at `0x00C27D88`, or NULL if no character is logged in yet.
    // `Script_GetAddOnInfo` feeds this into the loadable + enabled
    // checks; same convention.
    FUN_GET_LOGIN_ACCOUNT_NAME = 0x005ABDC0,

    // Two parallel `const char *[]` tables in `.data` covering every
    // string `Script_GetAddOnInfo`'s returns 6 + 7 can produce.
    //
    //   LOADABLE_REASON_TABLE — 8 entries indexed by the status code
    //     `FUN_ADDON_CAN_LOAD` writes:
    //       0 LOADABLE          (only emitted for the DEP fallback)
    //       1 MISSING
    //       2 DISABLED
    //       3 BANNED
    //       4 CORRUPT
    //       5 INSECURE
    //       6 NOT_DEMAND_LOADED
    //       7 INTERFACE_VERSION
    //
    //   SECURITY_TABLE — 3 entries indexed by the value
    //     `FUN_ADDON_GET_SECURITY_INDEX` returns. Lives at
    //     `LOADABLE_REASON_TABLE + 8 * sizeof(char *)` — the two
    //     tables are physically contiguous in the binary.
    //       0 SECURE
    //       1 INSECURE
    //       2 BANNED
    VAR_ADDON_LOADABLE_REASON_TABLE = 0x0085365C,
    VAR_ADDON_SECURITY_TABLE = 0x0085367C,
    VAR_ADDON_ARRAY = 0x00BE1B94,
    VAR_ADDON_COUNT = 0x00BE1B90,

    // Player visibility/state byte field, broadcast in UpdateFields.
    // Found empirically via _classicapi_DescDump (see src/debug/).
    // Treated as a u32 but the meaningful bits all live in byte 0:
    //   0x02 — set when the player has any visibility/form-modifying
    //          state active (stealth, mount, possibly shapeshift —
    //          set by stealth and mount in our test data).
    //   0x08, 0x10 — set together when mounted (alongside 0x02).
    //
    // Ambiguity: bit 0x02 alone doesn't distinguish stealth from
    // other visibility-modifying states. Stealth detection AND-gates
    // with `mountDisplayID == 0` to exclude mount; if shapeshift
    // also sets bit 0x02 we'd false-positive on druids in form
    // (untested). The robust path is to walk the player's aura
    // list looking for the actual stealth/prowl spell — left as
    // future work if false-positives turn up.
    OFF_PLAYER_FIELD_VIS_BYTES = 0x17C,
    PLAYER_VIS_BIT_STEALTH = 0x02,

    // Mount display ID — the creature display ID the engine renders
    // under the mounted player. Zero when not mounted, non-zero
    // (typically a 4-digit creature display ID like 0x4306) when
    // mounted. Broadcast in UpdateFields, so works for any unit's
    // descriptor — though we only expose `IsMounted()` for the
    // player to match modern API semantics.
    OFF_UNIT_FIELD_MOUNTDISPLAYID = 0x1FC,

    // Local player movement-flags u32. Lives directly on the
    // CGPlayer object (not the broadcast descriptor) — this is
    // client-side state the engine maintains for outbound
    // MSG_MOVE_* packets, never visible for remote units. Found
    // empirically by diffing _classicapi_PlayerLog snapshots
    // (standing/swimming/falling).
    //
    // Bit values match vanilla MOVEFLAG_* protocol constants —
    // verified by matching observed values to the documented
    // 1.12 movement-flag set:
    //   0x0001 FORWARD     0x2000  FALLING (in-air, moving down)
    //   0x4000 FALLING_FAR (extended fall)
    //   0x8000 PENDING_STOP
    //   0x200000 SWIMMING
    //   0x1000000 FLYING (no flying mounts in vanilla, but the bit
    //                     exists for druid Flight Form etc.)
    OFF_PLAYER_MOVEMENT_FLAGS = 0x9E8,
    MOVEFLAG_FORWARD = 0x1,
    MOVEFLAG_BACKWARD = 0x2,
    MOVEFLAG_STRAFE_LEFT = 0x4,
    MOVEFLAG_STRAFE_RIGHT = 0x8,
    // 0x10 / 0x20 are TURN_LEFT / TURN_RIGHT — turn-in-place, not
    // translational movement. Excluded from "moving" semantics
    // because modern `PLAYER_STARTED_MOVING` doesn't fire for them.
    MOVEFLAG_JUMPING = 0x1000,
    MOVEFLAG_FALLING = 0x2000,
    MOVEFLAG_FALLING_FAR = 0x4000,
    MOVEFLAG_SWIMMING = 0x200000,

    // CGPlayer-local pointer to the GameObject the current spell cast
    // is targeting. Holds a heap pointer (high bits in the user-mode
    // range, e.g. `0x42908708` / `0x52FA1A08`) while the player is
    // casting/channeling onto a GameObject; cleared on cast end or
    // movement break. Local-only — does NOT broadcast.
    //
    // Empirically distinct from UNIT_FIELD_CHANNEL_OBJECT — the engine
    // uses one or the other depending on the cast shape:
    //
    //   - **Set** during: clicking a warlock summoning portal (channel-
    //     spell that doesn't populate UNIT_CHANNEL_OBJECT); mining
    //     (regular spell cast on an ore node).
    //   - **0** during: warlock channeling a spell (no GO target);
    //     fishing (uses UNIT_CHANNEL_OBJECT for the bobber GUID);
    //     herb gathering (no spell, pure GO-loot mechanism); mailbox /
    //     gossip / sit / NPC interaction (instant, no animation).
    //
    // Used together with UNIT_FIELD_CHANNEL_SPELL to detect ritual
    // participation — see `IsAssistingRitual()` in `unit/State.cpp`.
    // The combined check `channelSpell != 0 && castGameObject != 0`
    // uniquely identifies the portal-clicker state in 1.12 (mining
    // fails `channelSpell != 0`; warlock/fishing fail
    // `castGameObject != 0`).
    OFF_CGPLAYER_CAST_GAMEOBJECT_PTR = 0xB4,

    // No `MOVEFLAG_FLYING` constant intentionally. Vanilla 1.12
    // doesn't flip a flying bit during taxi flights (verified
    // empirically: `IsFlying()` checking 0x01000000 stayed false
    // throughout a flightpath on which `UnitOnTaxi("player")`
    // correctly returned 1). Server-driven splines drive taxi
    // animation directly without touching the local movement-
    // flags word. Use `UnitOnTaxi` for taxi detection.

    // Equipment-slot index for the off-hand. Slots 1..19 are the
    // character-pane equipment slots (1=head … 19=tabard); 17 is
    // the off-hand slot the engine uses when looking up shields,
    // off-hand weapons, and held items via `ItemMgr::GetItemBySlot`.
    INVSLOT_OFFHAND = 17,

    // INVTYPE values that count as "weapon" for `OffhandHasWeapon`.
    // INVTYPE_WEAPON (13) covers any one-handed weapon equippable in
    // either hand (sword, axe, mace, dagger, fist). INVTYPE_WEAPONOFFHAND
    // (22) covers weapons that can ONLY go in the off-hand. Shields
    // (14), Holdables (23 — tomes/orbs), and empty slots all return
    // false. INVTYPE_2HWEAPON (17) is excluded because two-handers
    // occupy the main-hand slot exclusively.
    INVTYPE_WEAPON = 13,
    INVTYPE_WEAPONOFFHAND = 22,

    // Character-pane equipment slot range: 1..19 (head=1, neck=2, …,
    // tabard=19). Used by `C_Item.IsEquippedItem` to walk every slot
    // looking for a matching item.
    EQUIPMENT_SLOT_FIRST = 1,
    EQUIPMENT_SLOT_LAST = 19,

    // CGItem instance block GUID — qword at +0x00 of the instance block
    // (which sits behind the pointer at CGItem +0x08). Sibling of
    // OFF_INSTANCE_BLOCK_ITEM_ID at +0x0C in the same block.
    OFF_INSTANCE_BLOCK_GUID = 0x00,

    // `Script_ClearCursor` / `Script_PickupInventoryItem` Lua C
    // functions — both `__fastcall(void *L)`. ClearCursor takes no
    // args, drops anything held on the cursor. PickupInventoryItem
    // reads a 1-based character-pane slot at Lua stack[1] and lifts
    // that equipped item onto the cursor (or swaps with the existing
    // cursor contents). Used by `C_EquipmentSet.UseEquipmentSet` to
    // assemble the pickup→equip chain for each set item.
    FUN_SCRIPT_CLEAR_CURSOR = 0x004895B0,
    FUN_SCRIPT_PICKUP_INVENTORY_ITEM = 0x004C8DA0,

    // Bag-update fire sites — see TODO #67 for the full Ghidra
    // investigation. Hook these two to detect BAG_UPDATE fires and
    // emit BAG_UPDATE_DELAYED once per frame in which any fired.
    //
    //   FUN_BAG_SLOT_DIFF (`0x004F91A0`) — __cdecl `void()` (zero
    //   args, ends with plain `RET`). Walks linear slots `0x13..0x16`
    //   (player bags 1..4) and `0x3B..0x44` (bank bags 5..10),
    //   comparing each against a cached GUID snapshot at
    //   `DAT_00BDD060`. Fires `BAG_UPDATE(bagID)` for each slot
    //   whose GUID changed. Called from per-session init
    //   (FUN_004F8CC0) and a packet handler (FUN_004F8EC0) — 2
    //   callers total, very quiet hook target.
    //
    //   FUN_BAG_ITEM_TO_BAG (`0x004F9370`) — __stdcall
    //   `void(int guidLo, int guidHi)` (ends with `RET 0x8`).
    //   Given an item GUID, searches the bag cache for which bag
    //   contains it. Fires `BAG_UPDATE(0)` for backpack,
    //   `BAG_UPDATE(N)` for bag N, returns silently if not in any
    //   bag. **Most common BAG_UPDATE path** — every per-item
    //   field-change goes through here. 3 callers, all within the
    //   bag subsystem at `0x004F9xxx`.
    //
    // A third site (FUN_004F8DB0 at `0x004F8DB0`) handles keyring
    // updates with a `BAG_UPDATE(-2)` direct fire — __thiscall
    // calling convention plus ECX-context arg make the hook
    // signature awkward; deferred for now. The two below cover
    // the common cases.
    FUN_BAG_SLOT_DIFF = 0x004F91A0,
    FUN_BAG_ITEM_TO_BAG = 0x004F9370,
    // World-subsystem per-frame update — runs exactly once per frame
    // as part of the main world tick (`FUN_00482EA0`). Only one caller
    // in the entire binary; deep in the rendering/world pipeline,
    // nowhere near the chat/cast/transmog code regions that other
    // DLLs (SuperWoWhook / nampower / transmogfix) hook. We hook the
    // tail of this function to drain `BAG_UPDATE_DELAYED`'s pending
    // flag — exact per-frame coalescing matching modern WoW.
    //
    // Calling convention: `__fastcall(int ecx, int edx, int stackArg)`,
    // `RET 0x4` (callee cleans the one stack arg).
    FUN_WORLD_TICK = 0x0066FD50,

    // Engine session globals used by `EquipmentSet::Storage` to build
    // the per-character WTF path. Same layout VanillaMinimapTracking
    // uses for its own persistence file. Account is the value WoW
    // writes the per-character WTF directory under — NOT the saved-
    // credentials CVAR. Character name is an inline buffer (NUL byte
    // at index 0 means "no active character yet"). Realm name lives
    // inside a struct one indirection in.
    VAR_ACCOUNT_NAME_PTR = 0x00BE1C0C,
    VAR_CHARACTER_NAME = 0x00C27D88,
    VAR_REALM_INFO_PTR = 0x00C28130,
    OFF_REALM_INFO_NAME = 0x20,

    // Player body yaw (orientation in radians, 0..2π) — the
    // character's facing direction in the world. Inline float on
    // CGPlayer; mirrored at `+0x9F8` (current vs last-broadcast).
    // Verified empirically: a clean 180° RMB-drag swings this by
    // ~π rad, and LMB-only camera orbits leave it untouched.
    // Used by `PLAYER_STARTED_TURNING` to detect actual character
    // rotation (vs the mouselook button merely being held).
    OFF_PLAYER_BODY_YAW = 0x9C4,

    // Camera object pointer chain. The game-state singleton lives
    // at `*0x00B4B2BC`; the camera object hangs off it at `+0x65B8`.
    // Verified via `Script_CameraZoomIn` (`0x0050B400`) which loads
    // this chain to call `FUN_0050FC60` on the camera. Returns the
    // camera struct or `nullptr` if the game state isn't yet
    // initialized (pre-login).
    VAR_GAME_STATE_PTR = 0x00B4B2BC,
    OFF_GAME_STATE_CAMERA_PTR = 0x65B8,

    // Camera yaw, relative to the character (radians). `0` =
    // camera-behind-character default. Accumulates as the user
    // LMB-orbits; stays at `0` during RMB-mouselook (camera rotates
    // *with* the character, so relative offset doesn't change).
    // Verified empirically: clean 180° LMB orbit moves this from
    // `0` to `~π`. Mirrored at `+0x210` and `+0x214` (smoothing /
    // last-applied buffers); we read the primary at `+0xF0`.
    OFF_CAMERA_RELATIVE_YAW = 0xF0,

    // UI input controller — heap-allocated struct holding the
    // engine's master input-state bitfield (mouselook / free-look /
    // turn-key / autorun / strafe-modifier / etc.). The slot at
    // `0x00BE1148` is a pointer; deref it to reach the controller,
    // which lives until the client tears down (rare in practice).
    // NULL during pre-login.
    VAR_UI_INPUT_CONTROLLER = 0x00BE1148,
    // Master input flags. Single u32 at `+0x04` of the controller,
    // touched by the engine's button-press/release handlers
    // (`FUN_00514840` / `FUN_00514B70`) on every mouse-button or
    // input-key transition.
    OFF_UI_INPUT_FLAGS = 0x04,
    // Bit 0 — `Script_IsMouselooking` checks this. Set when the
    // user is in mouselook mode (RMB-drag, or `MouselookStart`
    // called from Lua). Turning the character follows mouselook.
    INPUT_FLAG_MOUSELOOK = 0x01,
    // Bit 1 — set when the user is in free-look mode (LMB-drag).
    // Camera-only rotation; character doesn't turn.
    INPUT_FLAG_FREE_LOOK = 0x02,
    // Bits 4-7 — WASD movement-key state. Each `MoveForwardStart`
    // / `StrafeLeftStart` / etc. pushes one of these as the bit
    // mask into the engine's button-press handler. Bit `0x1000` is
    // the autorun-active flag (set by `ToggleAutoRun` and the
    // both-mouse-buttons combo). `INPUT_FLAGS_MOVING_ANY` is the
    // engine's own "is the user currently inputting translational
    // movement" mask, used to drive `PLAYER_STARTED_MOVING` —
    // matches modern's "STOPPED fires on key release, even
    // mid-air" semantics because no physics/airborne bit is
    // involved.
    INPUT_FLAG_MOVE_FORWARD = 0x10,
    INPUT_FLAG_MOVE_BACKWARD = 0x20,
    INPUT_FLAG_STRAFE_LEFT = 0x40,
    INPUT_FLAG_STRAFE_RIGHT = 0x80,
    INPUT_FLAG_AUTORUN = 0x1000,
    INPUT_FLAGS_MOVING_ANY =
        INPUT_FLAG_MOVE_FORWARD | INPUT_FLAG_MOVE_BACKWARD |
        INPUT_FLAG_STRAFE_LEFT | INPUT_FLAG_STRAFE_RIGHT |
        INPUT_FLAG_AUTORUN,

    // CGUnit -> MovementInfo pointer. Holds the unit's per-instance
    // position / orientation / movement-flags / per-direction-speed
    // block. Both the local player and synced remote units carry
    // valid pointers here (NPCs included, when they exist as
    // CGUnits). Verified at multiple call sites that pass
    // `[unit + 0x118]` to `FUN_MOVEMENT_GET_EFFECTIVE_SPEED` — e.g.
    // `0x00511920` (sprint-jump motion calculator).
    OFF_UNIT_MOVEMENT_INFO_PTR = 0x118,

    // Inside the MovementInfo block:
    OFF_MOVEMENT_FLAGS = 0x40,        // u32 — engine's MOVEFLAG_* bits
    OFF_MOVEMENT_WALK_SPEED = 0x88,
    OFF_MOVEMENT_RUN_SPEED = 0x8C,
    OFF_MOVEMENT_RUN_BACK_SPEED = 0x90,
    OFF_MOVEMENT_SWIM_SPEED = 0x94,
    OFF_MOVEMENT_SWIM_BACK_SPEED = 0x98,

    // Effective-speed selector. `__thiscall float(this = MovementInfo,
    // int forceWalk) → float`. Returns the speed the engine would
    // apply to this frame's movement step — accounts for movement
    // flags (walking, swimming, backward), taxi paths, and the
    // walk/run/swim selection. Returns 0 when the unit isn't
    // moving (movement-flag bits 0..3 all clear). Used by 1.12's
    // movement-prediction loop; we call it to provide modern's
    // `currentSpeed` first return from `GetUnitSpeed`.
    FUN_MOVEMENT_GET_EFFECTIVE_SPEED = 0x007C4C90,

    // ------------------------------------------------------------------
    // Get*ItemID companions to the engine's Get*ItemLink functions.
    // Each offset block is the data path the corresponding
    // `Script_Get*ItemLink` reads from before calling
    // `FUN_DBCACHE_ITEMSTATS_GET_RECORD` — same source, just stop at
    // the itemID instead of building the link string.
    // ------------------------------------------------------------------

    // Loot rolls: an intrusive linked list of in-progress group loot
    // rolls. `FUN_LOOT_ROLL_FROM_ID(rollID) → entry *` walks it.
    // `Script_GetLootRollItemLink` reads the itemID at `entry+0x1C`.
    FUN_LOOT_ROLL_FROM_ID = 0x0061B2E0,
    OFF_LOOT_ROLL_ITEM_ID = 0x1C,

    // Loot slots: flat array indexed by `slot-1`, 0x1C-byte stride.
    // `LOOTABLE_FLAG` mirrors the engine's `[VAR_LOOT_LOOTABLE]` test
    // that drives whether slot indices are 1-based or 0-based; we
    // match the engine's `dec` behavior. itemID is at `entry+0`.
    VAR_LOOT_SLOTS = 0x00B7196C,
    VAR_LOOT_GUID_LO = 0x00B71B48,
    VAR_LOOT_GUID_HI = 0x00B71B4C,
    VAR_LOOT_LOOTABLE = 0x00B71BA0,
    LOOT_SLOT_STRIDE = 0x1C,
    LOOT_MAX_SLOTS = 0x10,

    // Merchant inventory: flat array at `0xBDD118`, stride 0x1C,
    // itemID at `entry+4`. Two flag globals must both be non-zero
    // (engine's `OR; JZ` gate — set when a merchant interaction is
    // active).
    VAR_MERCHANT_ITEMS = 0x00BDD118,
    VAR_MERCHANT_FLAG_A = 0x00BDDFA0,
    VAR_MERCHANT_FLAG_B = 0x00BDDFA4,
    VAR_MERCHANT_COUNT = 0x00BDDFA8,
    MERCHANT_STRIDE = 0x1C,
    OFF_MERCHANT_ITEM_ID = 0x04,

    // Quest item resolver — used by `GetQuestItem`. Walks the active
    // quest-offer "reward"/"choice" arrays. Returns itemID directly
    // (no follow-up cache lookup needed by the caller).
    // `__fastcall(ECX = const char *type, EDX = int index0) → itemID`.
    // Returns 0 on bad type, OOR index, or empty slot.
    FUN_QUEST_ITEM_RESOLVER = 0x00501920,

    // Quest log record fields (within the data block returned by
    // `Quest::Cache::Lookup`). Reward and choice arrays each hold up
    // to N inline itemIDs (4 bytes per entry).
    OFF_QUEST_RECORD_REWARD_ITEMS = 0x3C,    // 4 entries (0..3)
    OFF_QUEST_RECORD_CHOICE_ITEMS = 0x5C,    // 6 entries (0..5)
    QUEST_RECORD_REWARD_COUNT = 4,
    QUEST_RECORD_CHOICE_COUNT = 6,

    // Trade window — local "player" side and remote "target" side.
    // Player side is a flat int32 array (just itemIDs). Target side
    // is a flat `{?, itemID}` array (8-byte stride). Both cap at 7
    // slots (vanilla's 6 trade slots + 1 non-tradeable slot).
    VAR_TRADE_PLAYER_SLOTS = 0x00B714F4,
    VAR_TRADE_TARGET_SLOTS = 0x00B716D0,
    TRADE_TARGET_STRIDE = 0x08,
    OFF_TRADE_TARGET_ITEM_ID = 0x04,
    TRADE_MAX_SLOTS = 7,

    // Auction house — three independent arrays of entry-pointers
    // covering the three views (list / owner / bidder). Each entry
    // carries the itemID at `+0x08`.
    VAR_AUCTION_LIST_ENTRIES = 0x00B72278,
    VAR_AUCTION_LIST_COUNT = 0x00B7261C,
    VAR_AUCTION_OWNER_ENTRIES = 0x00B72340,
    VAR_AUCTION_OWNER_COUNT = 0x00B72624,
    VAR_AUCTION_BIDDER_ENTRIES = 0x00B72470,
    VAR_AUCTION_BIDDER_COUNT = 0x00B7262C,
    OFF_AUCTION_ENTRY_ITEM_ID = 0x08,

    // Auction sell slot — holds a single CGItem GUID for the item
    // currently sitting in the "sell" frame. Engine resolves it via
    // a typed object lookup; pass `OBJECT_TYPE_ITEM` and the GUID.
    VAR_AUCTION_SELL_GUID_LO = 0x00B72608,
    VAR_AUCTION_SELL_GUID_HI = 0x00B7260C,
    // `__fastcall(ECX = typeCode, EDX = unused, [stack] = guidLo,
    //             [stack] = guidHi, [stack] = anyInt) → CGObject *`.
    // Returns NULL when no object matches or the type doesn't match.
    // The third stack arg (originally a `__FILE__/__LINE__` debug
    // hint) is read by debug paths only — anything works at runtime.
    FUN_GET_OBJECT_BY_GUID = 0x00468460,
    OBJECT_TYPE_ITEM = 2,

    // Inbox — array of mail-entry pointers behind a slot indirection.
    // The address itself holds a heap pointer (`MOV ECX, [imm32]` in
    // the engine), so reading the array requires one extra deref
    // beyond plain `(uint8_t **)`. itemID inline on each entry at
    // `+0x120`. (The engine's `Script_GetInboxItem` passes a per-
    // entry completion callback at `0x004AF0A0` to GetRecord; for
    // ID-only retrieval we skip the callback.)
    VAR_INBOX_ENTRIES = 0x00B6EF54,
    VAR_INBOX_COUNT = 0x00B6EFC0,
    OFF_INBOX_ENTRY_ITEM_ID = 0x120,

    // Craft window (engineering, alchemy, etc. before the "trade
    // skill" overhaul). Same slot-indirection shape as inbox: the
    // address holds a pointer to the heap-allocated entry-pointer
    // array, NOT the array itself. Each entry's `+0x00` is a
    // spellID (the recipe spell); the recipe's reagent list lives
    // on the spell record at `+0xA8`.
    VAR_CRAFT_ENTRIES = 0x00BDCF78,
    VAR_CRAFT_COUNT = 0x00BDCFC0,
    OFF_CRAFT_ENTRY_SPELL_ID = 0x00,

    // Trade skill window — different storage from craft, same shape
    // (slot-indirected entry-ptr array with a spellID inside). The
    // recipe's created item lives on the spell record at `+0x19C`
    // (the "produced item" field), and reagents at `+0xA8` (shared
    // with craft).
    VAR_TRADESKILL_ENTRIES = 0x00BDDFC0,
    VAR_TRADESKILL_COUNT = 0x00BDE04C,
    OFF_SPELL_RECORD_REAGENTS = 0xA8,        // int32 itemID[8]
    OFF_SPELL_RECORD_CREATED_ITEM = 0x19C,
    SPELL_RECIPE_MAX_REAGENTS = 8,

    // Gossip menu data. Populated by the SMSG_GOSSIP_MESSAGE packet
    // handler at 0x004E26E0 and reset by FUN_004E26A0 each time a new
    // gossip window opens. Two parallel storage arrays:
    //   Options: 16 slots, stride 0x80C, sentinel = signed `index` == -1.
    //   Quests:  32 slots, stride 0x20C, sentinel = u32 `questID` == 0.
    //            Available vs. active is partitioned by the +0x008
    //            `status` field — values 3 or 4 mark an active-quest
    //            row, anything else is a deliverable.
    VAR_GOSSIP_OPTIONS = 0x00BBBE90,
    GOSSIP_OPTIONS_STRIDE = 0x80C,
    GOSSIP_OPTIONS_MAX = 16,
    OFF_GOSSIP_OPTION_TEXT = 0x000,          // char[0x800] inline
    OFF_GOSSIP_OPTION_INDEX = 0x800,         // int32; -1 = unused slot
    OFF_GOSSIP_OPTION_BOX_CODED = 0x804,     // u8 (1 = needs password)
    OFF_GOSSIP_OPTION_ICON = 0x808,          // u8 icon type (0..10)

    VAR_GOSSIP_QUESTS = 0x00BB74C0,
    GOSSIP_QUESTS_STRIDE = 0x20C,
    GOSSIP_QUESTS_MAX = 32,
    OFF_GOSSIP_QUEST_ID = 0x000,             // u32 questID; 0 = unused slot
    OFF_GOSSIP_QUEST_LEVEL = 0x004,          // i32 questLevel
    OFF_GOSSIP_QUEST_STATUS = 0x008,         // u32 status; 3|4 = active
    OFF_GOSSIP_QUEST_TITLE = 0x00C,          // char[0x200] inline

    // Inline greeting buffer pushed by Script_GetGossipText. Populated
    // by the engine after the NPC_TEXT.dbc query for the gossip giver
    // resolves the textID embedded in SMSG_GOSSIP_MESSAGE.
    VAR_GOSSIP_GREETING_TEXT = 0x00BBB678,

    // Engine Lua C functions for the selector half of the gossip
    // surface. We translate modern (gossipOptionID/questID) into the
    // engine's 1-based-index Lua arg and tail-call these directly.
    FUN_SCRIPT_SELECT_GOSSIP_OPTION = 0x004E2A30,
    FUN_SCRIPT_SELECT_GOSSIP_AVAILABLE_QUEST = 0x004E2AA0,
    FUN_SCRIPT_SELECT_GOSSIP_ACTIVE_QUEST = 0x004E2AE0,
    FUN_SCRIPT_CLOSE_GOSSIP = 0x004E2B20,
};
