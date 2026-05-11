# ClassicAPI — TODO

Candidates for new DLL functions, prioritized. All are things the
[!!!ClassicAPI](C:\WoW\FrostmourneClient\Interface\AddOns\!!!ClassicAPI) Lua
addon polyfills badly, hackily, or not at all, and where engine access is the
right answer.

## ~~1. `C_QuestLog.IsQuestFlaggedCompleted(questID)`~~ — NOT FEASIBLE in 1.12

The original TODO note ("engine maintains a completed-quests bitfield in
client memory") was based on 3.3.5 / WotLK semantics. Vanilla 1.12 has no
equivalent: neither `QueryQuestsCompleted` nor `GetQuestsCompleted`
exists, the protocol has no `CMSG_QUERY_QUESTS_COMPLETED`, and the
client doesn't track historical completions. The polyfill at
`Util/C_QuestLog.lua:5-8` works on Frostmourne (3.3.5a) where those
functions exist; on the 1.12 Octo client they're absent.

Servers that want to expose this in 1.12 (notably Turtle WoW) do it
via the `CHAT_MSG_ADDON` channel — the addon sends `.queststatus` over
guild/whisper chat and the server replies with addon-channel messages
the addon parses. Fundamentally async, server-specific, and requires
SavedVariables-style persistence across sessions to amortize the
round-trip — all of which belong on the Lua side, not in this DLL.
There's nothing the DLL can usefully add here.

## ~~2. `C_Spell.GetSpellDescription(spellID)`~~ — DONE

Calls the engine's own spell-description formatter at `0x005075F0`,
which reads the description string from `Spell.dbc` (record `+0x228`,
locale-applied) and resolves `$s1`/`$s2`/`$o1`/`$d`-style placeholders
character-by-character. 8 callers in the engine (spell tooltip builder,
talent UI, trainer, etc.); we mirror the simplest one's args
(`contextFlag=0` = "no caster scaling, base-rank values"). Output goes
into a 1024-byte caller-provided buffer that the helper null-terminates.

The interesting find: I expected the description to need a CGUnit
context for substitution, but the engine has a "no scaling" code path
that produces `"14 to 22 Fire damage"` (Fireball Rank 1's actual base
range) without any unit reference. That's the right semantic for
"describe this spell" — modern WoW behaves the same when called outside
a unit context.

Side note: a global at `0x00BE0B80` is the formatter's parser cursor
(it walks the description string char-by-char, advancing the cursor in
place). It's set and managed entirely inside the helper — callers don't
touch it. Lua C functions are single-threaded, so the global state is
safe to share with whatever else might call the formatter (it's all on
the same thread).

See [src/spell/Description.cpp](src/spell/Description.cpp) and the
`FUN_FORMAT_SPELL_DESCRIPTION` block in [src/Offsets.h](src/Offsets.h)
for the full calling convention (8-arg `__fastcall`).

## ~~3. `C_Item.IsBound` / `isBound` from `GetContainerItemInfo`~~ — DONE

Implemented as `C_Item.IsBound(itemLocation)`. Reads the soulbound bit
(`ITEM_FIELD_FLAGS` bit 0) directly off the CGItem's UpdateField
descriptor — no scan-tooltip hack, no string comparison. Accepts the
modern `{equipmentSlotIndex=N}` and `{bagID=B, slotIndex=S}` location
shapes. See [src/item/Bound.cpp](src/item/Bound.cpp) for the soulbound
read; the slot-resolution chain it depends on lives in
[src/item/Location.cpp](src/item/Location.cpp).

## ~~4. `GetSpellInfo` for unlearned spells~~ — DONE

Implemented as the global `GetSpellInfo(spellID)`. Returns the full 9-tuple
matching 3.3.5 semantics, including `true`/`false` for `isFunnel` (verified
against the 3.3.5 client). See [src/SpellInfo.cpp](src/SpellInfo.cpp).

## ~~5. `GetServerTime()`~~ — DONE

Reads year/month/day/hour/minute from the engine's game-time struct at
`0x00CE8538` (populated by `SMSG_LOGIN_VERIFY_WORLD` /
`SMSG_LOGIN_SETTIMESPEED` and advanced by the internal tick handler)
and converts via `_mkgmtime` to a Unix epoch timestamp.

The interesting find: the engine has its own "to seconds since epoch"
helper at `0x00642320` that I almost reused — but decoding the magic
constant `0xC22E4507` after the `_mkgmtime` call revealed the helper
divides the result by `86400` (seconds in a day), so it returns
**days-since-epoch**, not seconds. Rolled our own `_mkgmtime` call
instead.

The 1.12 wire protocol carries time at minute granularity — there are
no seconds in `SMSG_LOGIN_SETTIMESPEED`'s packed gametime field — so
the engine's stored hour/minute steps once per minute. To make the
returned timestamp tick every second (which is what `GetServerTime`
callers actually need), we interpolate within the minute using
`GetTickCount`: whenever the engine's minute changes, we anchor to the
current tick and add `(now - anchor) / 1000` for subsequent calls. The
cold-start value is off by 0..59 seconds since we have no way to know
how far into a minute the engine is the first time we observe it, but
after the first minute rollover the anchor lands at the boundary and
the timestamp is accurate for the rest of the session.

See [src/time/Server.cpp](src/time/Server.cpp) and
`VAR_GAMETIME_STRUCT` / `OFF_GAMETIME_*` in
[src/Offsets.h](src/Offsets.h).

## ~~6. `C_QuestLog.GetTitleForQuestID(questID)`~~ — DONE

Reads the title out of the quest static-info cache at offset `+0x9C`
(inline null-terminated C string within the data block returned by the
cache's `_GetRecord`). Title-offset derivation: traced from the engine's
own quest-log title path at `0x004DF180`, which calls `_GetRecord` and
then does `add eax, 0x9C; ret` — the result is fed directly to
`lua_pushstring`, so `+0x9C` is the C string itself, not a pointer.

The original TODO note "engine has the title in `Quest.dbc`" turned out
to be wrong — there is no `Quest.dbc` in 1.12 (only `QuestInfo.dbc` for
reward types and `QuestSort.dbc` for category headers). All quest static
data is server-authoritative, fetched via `SMSG_QUEST_QUERY_RESPONSE` and
held in the runtime cache at `0x00C0E1B0`. The "async dance" the polyfill
does is the unavoidable cost of that — though now it's just one call to
`C_QuestLog.RequestLoadQuestByID` plus an `OnEvent` listener instead of
a hidden tooltip and a 180-second timeout.

See [src/quest/Title.cpp](src/quest/Title.cpp) and the shared cache
helper at [src/quest/Cache.h](src/quest/Cache.h). The data-block field
offsets (currently just `+0x9C` for title) live in `Cache.h` alongside
the `Lookup`/`Peek` wrappers, so additional cached fields like
description and objectives can be added there as we trace them.

## 7. `UnitAuraBySlot` / `UnitAuraSlots` — medium

Lua addon stubs these out entirely (commented as needing `LibAura`), so
`AuraUtil.ForEachAura` is broken in the addon as written. The per-unit aura
array is in memory; `UnitAura(idx)` already iterates it. Adding slot-stable
accessors is a thin wrapper over the same internal call. Worth doing because
modern addon code assumes these exist.

References: `Util/Aura.lua:311-321` (commented out), `Util/AuraUtil.lua:42-67`.

## ~~8. `GetItemInfo` for uncached items~~ — DONE (via #12 + #13)

The "cache-fill path with a proper async callback so addons stop polling
on a frame timer" goal landed as the request/event pair below:

- **#12** ships `C_Item.IsItemDataCachedByID(item)` /
  `C_Item.RequestLoadItemDataByID(item)` (and the ItemLocation
  variants), wrapping the engine's existing
  `DBCache_ItemStats_C_GetRecord` at `0x55BA30` with a non-NULL callback
  to trigger the SMSG_ITEM_QUERY_SINGLE network fetch.
- **#13** ships `ITEM_DATA_LOAD_RESULT(itemID, success)`, fired from our
  `__stdcall` callback when the engine drops the response into the
  cache (or synchronously if it was already loaded).

Together those replace the polyfill's 180-second polling frame
(`ItemEventListener` in `Util/ItemUtil.lua:254-372`) — addons can now
fire `RequestLoadItemDataByID(id)` and listen for
`ITEM_DATA_LOAD_RESULT` instead.

A single `GetItemInfo(item)` that "returns nil and triggers a load" in
one call (matching modern WoW's `GetItemInfo` semantics for uncached
items) would be a thin convenience wrapper on top, but the modern
preferred pattern is exactly the request/event pair we have, so it's
not pursued separately.

## ~~9. `C_Item.GetItemInfoInstant(item)`~~ — DONE (not in original list)

The always-available subset of `GetItemInfo` — fields that depend only on
classification, not on player-specific state. Accepts itemID or
`item:NNN` link strings (full chat links work too); returns nil for
uncached items rather than blocking on a server query. See
[src/item/Info.cpp](src/item/Info.cpp). Reading `ItemSubClass.dbc` was the
fiddly bit (parallel-storage hack at `+0x00`/`+0x04` for compound-key
lookup, plus a verbose→short locale-name fallback chain) — full notes in
[CLAUDE.md](CLAUDE.md) and [src/Offsets.h](src/Offsets.h).

## ~~10. `C_Item.GetItemID(itemLocation)`~~ — DONE (not in original list)

Returns the itemID for the item at a given equipment slot or bag/slot
tuple. Useful as the input to `GetItemInfoInstant`/`GetItemInfo` when you
only know which slot an item came from. Required factoring out the shared
slot-resolution code from `IsBound` into [src/item/Location.cpp](src/item/Location.cpp);
the GetItemID-specific bit (one extra deref) lives in
[src/item/ID.cpp](src/item/ID.cpp). The non-obvious part: the itemID
isn't in the CGItem's UpdateField descriptor (its `OBJECT_FIELD_ENTRY`
slot is empirically 0), but in a separate instance block at `CGItem+0x08`.

## ~~11. `C_EventUtils.IsEventValid(eventName)`~~ — DONE (not in original list)

Walks the engine's static event-name table at `0x00BE11D8` and returns
`true` if any populated slot contains the queried name. See
[src/event/Util.cpp](src/event/Util.cpp). The interesting bit: third-party
DLLs (notably SuperWoWhook on Turtle clients) inject custom events by
**overwriting `.rdata` event-name strings in place** rather than appending
to the table — so a runtime walk picks up their additions for free. Full
notes in [CLAUDE.md](CLAUDE.md).

## ~~12. `C_Item.IsItemDataCached*` / `RequestLoadItemData*`~~ — DONE (not in original list)

Wraps the engine's existing client-side item cache (`_GetRecord` at
`0x55BA30`, `requestIfMissing` flag). Both `ByID` and ItemLocation
variants ship. See [src/item/Data.cpp](src/item/Data.cpp).

## ~~13. `ITEM_DATA_LOAD_RESULT` event~~ — DONE

Fires with `(itemID, success)` when an item finishes loading after a
`RequestLoadItemData*` call (or synchronously when the data was already
cached). See [src/event/Custom.cpp](src/event/Custom.cpp) for the
custom-event plumbing and [src/item/Data.cpp](src/item/Data.cpp) for
the wiring to the cache callback.

The investigation needed three engine pieces nobody had documented:

- **Dispatcher** at `0x00703F50` — `__cdecl void(int eventID, const char *fmt, ...)`,
  149 callers, indexes the table at `[0xCEEF68] + id*16`, walks the
  subscriber chain. Format is printf-style (`%s`, `%d`, `%u`, `%f`); no
  bool code, pass `%d` with 0/1.
- **Registration table** at `[0xCEEF68]` (count at `[0xCEEF64]`),
  16-byte entries, name at `+0x00`, subscriber chain head at `+0x0C`.
  `Frame::RegisterEvent` at `0x00702140` strcmps event names against
  this table.
- **Teardown landmine.** `FrameScript_Initialize`'s reload path
  (`0x00701A40`) calls `SMemFree(entry.name)` on every entry; our static
  literals are not Storm-allocated and trip the safety check. There's a
  NULL-name shortcut (`jz` at `0x00701A45`) so we null our entries
  before the engine sees them.

Full notes in [CLAUDE.md](CLAUDE.md) under "Firing a custom event
end-to-end".

## ~~14. `C_QuestLog.RequestLoadQuestByID` + `QUEST_DATA_LOAD_RESULT`~~ — DONE (not in original list)

Fires `QUEST_DATA_LOAD_RESULT(questID, success)` when quest static info
(description, objectives, reward text) finishes loading. Synchronously
fired when the questID was already in the client cache; asynchronously
fired after `SMSG_QUEST_QUERY_RESPONSE` when the engine has to round-trip.

Implemented as a near-direct mirror of `Item::Data`: the quest cache
`_GetRecord` at `0x00562A40` has the same five-arg shape and the same
`callback != NULL` gate as the item cache `_GetRecord`. The interesting
investigation was confirming that `[0x00BB7480]` (passed as the cache
key in `Script_GetQuestLogQuestText`) is a *questID*, not a struct
pointer — verified by tracing the load site at `0x004DEF5D` back through
`mov eax, [esi + 0x00BB71C0]`, which reads field +0 of a quest log
entry (= questID per the quest log struct). The cache is then keyed by
questID just like the item cache is keyed by itemID.

See [src/quest/Data.cpp](src/quest/Data.cpp) and "Quest static-info
cache" in [CLAUDE.md](CLAUDE.md). The custom-event plumbing
([src/event/Custom.cpp](src/event/Custom.cpp)) is shared with
`ITEM_DATA_LOAD_RESULT` — no new infrastructure needed.

## ~~15. `GetFactionInfoByID` / `GetFactionIDByIndex`~~ — DONE (not in original list)

`GetFactionInfoByID(factionID)` returns the same 11 values as
`GetFactionInfo(factionIndex)` keyed by factionID — implemented as a thin
wrapper that walks the engine's index→ID resolver at `0x004D5FA0` to find
the displayed-list index for the requested ID, then replaces Lua arg 1 and
tail-calls `Script_GetFactionInfo` (`0x004D64F0`). Only works for factions
the player has rep with, matching modern WoW's semantics.

`GetFactionIDByIndex(factionIndex)` exposes the same resolver directly —
returns the factionID for real entries, `0` for headers (normalizing the
engine's mix of `0` for "Other" and `-1` for "Inactive"), `nil` for OOB.
Modern WoW (5.0+, including Classic Era 1.15.x) returns this as the 14th
value of `GetFactionInfo`, but 1.12-3.3.5 don't expose it at all.

See [src/faction/Info.cpp](src/faction/Info.cpp) and "Faction lookups" in
[CLAUDE.md](CLAUDE.md) for the resolver/dual-count details.

## ~~16. `C_Container.GetContainerItemID(bagIndex, slotIndex)`~~ — DONE

Shipped under the modern `C_Container.*` namespace (matching post-MoP
WoW). Wraps the existing bag→CGItem path: a new public
`Item::Location::ResolveBag(L, bagID, slotIndex)` exposes the bag
resolver that was previously private inside the table-shape
`Resolve()`, and a small `ItemIDFromCGItem` helper in
[src/item/ID.cpp](src/item/ID.cpp) factors out the CGItem → instance
block → itemID step now used by both `C_Item.GetItemID` and
`C_Container.GetContainerItemID`.

Note: ships under `C_Container` rather than as the historical legacy
global `GetContainerItemID` since the modern namespace is the canonical
home for bag/container APIs (the global was deprecated in 10.0). If
addons need the legacy global it's a one-line addition.

## ~~17. `GetItemIcon(itemID)` / `C_Item.GetItemIcon(itemLocation)` / `C_Item.GetItemIconByID(item)`~~ — DONE

Three Lua entry points that all share a `PushIconForItemID(L, itemID)`
helper in [src/item/Info.cpp](src/item/Info.cpp). The helper routes
through the existing `FetchItemRecord` (item cache) and `BuildIconPath`
(`ItemDisplayInfo.dbc` lookup at `+0x14`) — same code path
`GetItemInfoInstant` already runs, just stripped to the icon-only return.

The refactor that fell out: extracted `Item::ID::FromCGItem` (the
CGItem → instance-block → itemID step) into [src/item/ID.h](src/item/ID.h)
so both `Item::ID` (for `C_Item.GetItemID` and
`C_Container.GetContainerItemID`) and `Item::Info` (for
`C_Item.GetItemIcon`'s ItemLocation form) can share it. Was static-dup
across two files before this; now it's one definition.

Returns the path string (deviation from modern's `fileID:number` —
1.12 has no fileID system; same call-out as
`C_Spell.GetSpellTexture`'s docs).

## ~~18. `UnitGUID(unit)`~~ — DONE

Returns the unit's 64-bit GUID as `"0xHHHHHHHHLLLLLLLL"` (16 hex
digits, hi dword first). Reads `*(unit + OFF_UNIT_GUID_PTR)` to get
the 8-byte struct, formats with `snprintf`. Lives in
[src/unit/Identity.cpp](src/unit/Identity.cpp).

Two behavioral notes worth flagging in docs:

- **Vanilla format, not modern's prefix shape.** 1.12 GUIDs are plain
  64-bit integers (the `"Player-1234-..."` / `"Creature-0-..."`
  formatted GUIDs were a 6.0 addition). Addons backporting modern
  GUID parsers need to accept the `"0x..."` form.
- **Errors on totally-unknown unit tokens.** `ResolveUnitToken` itself
  raises `"Unknown unit name: <token>"` for strings outside the known
  unit-ID list. We don't suppress it — matches how every other 1.12
  unit-token function (`UnitName`, `UnitAffectingCombat`, etc.)
  behaves. Tokens that resolve to "no current unit" (e.g. `"target"`
  with nothing targeted) cleanly return nil via the GUID-is-zero
  short-circuit.

## ~~19. `IsPassiveSpell` / `C_Spell.IsSpellPassive`~~ — DONE

Reads `Attributes` (+0x18) bit 6 (`SPELL_ATTR_PASSIVE = 0x40`) off the
`Spell.dbc` record — same pattern [src/spell/Info.cpp](src/spell/Info.cpp)
already used for `isFunnel` (AttributesEx bit 6, on a different field).

Shipped both forms with shared `PushIsPassive` back-end:

- `IsPassiveSpell(spellID)` / `IsPassiveSpell(slot, bookType)` — the
  3.0-era global, supports both arg shapes via `ResolveLuaArgsToSpellID`
  (mirrors `GetSpellInfo` / `GetSpellLink`).
- `C_Spell.IsSpellPassive(spellID)` — the modern 10.0 form (word order
  flipped during the `C_Spell` namespace migration). spellID-only.

## ~~20. `GetSpellLink(spellID)` / `C_Spell.GetSpellLink(spellID)`~~ — DONE

Builds `|cff71d5ff|Hspell:ID:0|h[Name]|h|r` via `snprintf` after a
single `Spell.dbc` name read. The trailing `:0` after the spellID
matches modern's hyperlink shape so addons backporting from later
expansions can `gsub` with the standard `|Hspell:(%d+):` pattern; 1.12
ignores it during link parsing.

Both forms shipped:

- `GetSpellLink(spellID)` and `GetSpellLink(slot, bookType)` (global)
  return `(link, spellID)`. The slot-and-bookType overload reuses the
  same `Spell::Lookup::ResolveLuaArgsToSpellID` helper as
  `GetSpellInfo`.
- `C_Spell.GetSpellLink(spellID)` (table) returns just the link string
  — the caller already had the spellID on hand.

See [src/spell/Info.cpp](src/spell/Info.cpp) `Script_GetSpellLink` /
`Script_C_GetSpellLink` and the `BuildSpellLink` helper.

## ~~21. `InCombatLockdown()`~~ — DONE

Reads `UNIT_FLAG_IN_COMBAT` (bit 19, `0x00080000`) of the player's
UNIT_FIELD_FLAGS — same bit `Script_UnitAffectingCombat` at
`0x00517E4A`-`0x517E5C` tests via `mov eax, [fields+0xA0]; shr eax, 19;
test al, 1`. Modern WoW's "lockdown" gates secure-frame UI changes;
1.12 has no secure-frame system, so the function reduces to a plain
"is the player in combat" check (equivalent to
`UnitAffectingCombat("player")` but skips the unit-token resolution).

Lives in [src/unit/Combat.cpp](src/unit/Combat.cpp) — first occupant of
the new `src/unit/` directory. Future unit-flag accessors
(`UnitIsAFK`, `UnitIsDND`, `UnitIsFeignDeath`, `UnitClassBase`, etc.)
can land alongside without polluting the per-domain Lua-namespace
directories.

## 22. `UnitClassBase(unit)` — easy

Locale-independent class file string (`"WARRIOR"`, `"MAGE"`, etc.).
Modern addons use this for class detection because `UnitClass` returns
a localized name. Read from `UNIT_FIELD_BYTES_0` (the byte field that
holds class/race/gender/power-type), then map to one of the 9 vanilla
class strings. We already know the unit descriptor lives at
`[unit + 0x110]` (see `UnitPlayerControlled` discovery for
`GetInventoryItemID`).

## ~~23. `GetItemSpell(item)`~~ — DONE

Returns `(spellName, spellID)` for the on-use spell attached to an
item (potions, trinkets, scrolls, hearthstone, food/drink), or nil
for items without one. Reads the cached `ItemStats_C` record's
`m_spellId[5]` array at `+0x11C` and `m_spellTrigger[5]` at `+0x130`,
matching the first slot with `trigger == 0` (`ITEM_SPELLTRIGGER_ON_USE`).
Combines with `Spell.dbc.Name[locale]` at record `+0x1E0` for the
spell name.

Offsets computed from VanillaHelpers's fully-decoded
`ItemStats_C` struct; verified empirically against Hearthstone
(itemID 6948, slot-0 spell 8690, trigger 0). Other trigger codes
(`ON_EQUIP`=1, `CHANCE_ON_HIT`=2, `SOULSTONE`=4, `LEARN_SPELL`=6)
are deliberately ignored — matches modern WoW's `GetItemSpell`,
which only reports the on-use spell.

Same auto-warmup pattern as the other cache-backed accessors:
returns nil for uncached items and silently fires the cache fill,
so a follow-up call after `GET_ITEM_INFO_RECEIVED` lands the data.

See [src/item/Spell.cpp](src/item/Spell.cpp).

## ~~24. `GetInventoryItemDurability(invSlot)` / `C_Container.GetContainerItemDurability(containerIndex, slotIndex)`~~ — DONE

Returns `(current, max)` durability for the player's equipped item or
bag/bank slot, or nothing if the slot is empty or the item has no
durability concept (consumables, materials, rings, trinkets, shirts,
tabards). Both forms shipped:

- `GetInventoryItemDurability(invSlot)` — character-pane equipment slot
  (1-based, 1..19). Player-only by design, matches modern API; the
  original TODO line had `(unit, slot)` but modern
  `GetInventoryItemDurability` takes only the slot.
- `C_Container.GetContainerItemDurability(containerIndex, slotIndex)` —
  bag/bank slot. Goes through `Item::Location::ResolveBag` →
  `PackBagSlot` → `GetItemBySlot`, same chain
  `C_Container.GetContainerItemID` already used.

Both share the inner `PushDurabilityForItem` helper — only the
CGItem-resolution path differs. Reads ITEM_FIELD_DURABILITY (+0xA0) and
ITEM_FIELD_MAXDURABILITY (+0xA4) off the CGItem descriptor at `+0x114`
— the same descriptor [src/item/Bound.cpp](src/item/Bound.cpp) reads
FLAGS from. Field offsets verified by decoding
`Script_GetInventoryItemBroken` at `0x004C8590`, which tests
`[descriptor+0x3C] & 0x08` for the broken flag bit AND
`[descriptor+0xA4] > 0 && [descriptor+0xA0] == 0` as a fallback.
Modern semantics confirmed against 3.3.5's
`Script_GetInventoryItemDurability` at `0x005EA170`: returns nothing
(0 values) when max is 0.

See [src/item/Durability.cpp](src/item/Durability.cpp).

## ~~25. `FindSpellBookSlotByID(spellID)`~~ — DONE

Walks the player spellbook at `0x00B700F0` first, then the pet
spellbook at `0x00B6F098`, returning `(slot, bookType)` for the first
match. Bounds at `SPELLBOOK_MAX_SLOTS = 0x400` (the engine's own cap
in `Script_GetSpellName`); empty/zero entries past the populated count
are skipped naturally since spellID 0 doesn't match any positive
input. Result feeds directly into the slot-and-bookType API surface
(`GetSpellName`, `GameTooltip:SetSpell`, etc.).

Logic added to [src/spell/Lookup.cpp](src/spell/Lookup.cpp) as
`Spell::Lookup::FindSpellbookSlot`; the Lua C function lives next to
the other spell registrations in [src/spell/Info.cpp](src/spell/Info.cpp).

## 26. `IsEquippableItem(item)` — trivial

Boolean: can the item be equipped at all? One-line check: fetch the
ItemStats record (we already have `Item::Info::FetchItemRecord`) and
test `m_inventoryType > 0`. The INVTYPE field is at
`OFF_ITEMSTATS_INVENTORY_TYPE = 0x2C`, already wired up.

## 27. `IsEquippedItem(item)` — easy

Boolean: is the item currently equipped on the player? Walk equipment
slots 1..19 via the same `Item::Location::ResolveEquipmentSlot` chain
the existing accessors use, then `Item::ID::FromCGItem` for each one
and compare to the input itemID. Returns true on first match.

## ~~28. `C_Item.GetItemCount(itemInfo, [includeBank], [includeUses])`~~ — DONE (partial — `includeUses` deferred)

Total count of an item across the player's bags (and optionally bank).
Walks bags 0..4 always (live walk via `Item::Location::ResolveBag`).
Stack count read directly from `ITEM_FIELD_STACK_COUNT` at descriptor
+0x20 — verified by decoding `Script_GetContainerItemInfo`
(`0x004F9670`), which does `mov eax, [esi+0x114]; fild [eax+0x20]` for
the count return.

**Bank works cold** — no banker visit required. The 1.12 server sends
bank inventory at login (verified empirically on Turtle WoW: fresh
WoW.exe launch + cleared WDB folder shows GUIDs already populated in
main bank slots 39..62 with the gate at `VAR_BANK_GATE_GUID = 0`).
The engine's `GetItemBySlot` (`0x006228A0`) gates bank slots 39..68
on a non-zero banker-GUID at `0x00BDD038` — set when bank window
opens, cleared when it closes — but the underlying GUID data in
`invMgr+0x04` is always present from session start.

Bank-walk path bypasses the gate by reading GUIDs directly out of
the player invMgr's slot array and resolving each via
`FUN_OBJECT_RESOLVE_BY_GUID = 0x00468460` (same function the engine
itself calls inside `GetItemBySlot`, just without the gate wrapper).
Bank bags (slots 63..68) are followed via the bag's vtable +0x10 to
get its own invMgr, then walked by the same direct-read path. Cold
count of a bank-only item now works from login without ever
approaching a banker — confirmed in-game.

The 1.12 STACK_COUNT field index (0x8) puts it *before* the contained/
creator GUIDs, **opposite** the more commonly documented vanilla
layout (which has STACK_COUNT at index 0xE = +0x38). Trust the
binary: 0x20 is what the engine itself reads, and our test (cross-
checked against a manual `GetContainerItemInfo` walk) confirms it.

`includeUses` (charges-multiplier mode) is **accepted but currently
ignored** — both `true` and `false` produce the same value. Modern
semantics multiply each match by the item's spell-charges count (a
stack of 5 wands × 50 charges → 250). To finish: verify
`ITEM_FIELD_SPELL_CHARGES` offset (suspected +0x40 = field index
0x10 in 1.12's layout) by reading from a charged item's descriptor
and confirming against the cast counter.

See [src/item/Count.cpp](src/item/Count.cpp).

## ~~29. `C_Item.GetItemFamily(item)`~~ — DONE

Returns the BagFamily bitmask for the item, or `nil` if it isn't in
the cache. Reads `m_bagFamily` at +0x1D0 on the cached `ItemStats_C`
record (offset verified by decoding the engine's own
`GetItemBagFamily(itemID)` helper at `0x005DA050`, which calls
`_GetRecord` then `mov eax, [eax+0x1D0]; ret`).

**Encoding-shift gotcha.** 1.12 stores the raw 1-based BagFamily ID
(`arrow=1, bullet=2, soul shard=3, herb=6, …`); modern WoW (Wrath+)
flipped to the bitmask form `1 << (ID-1)` (`arrow=0x1, bullet=0x2,
soul shard=0x4, herb=0x20, …`). Addons backporting from later
expansions expect the bitmask, so we convert on the way out via
`bitmask = id ? (1 << (id - 1)) : 0`. Verified empirically on Turtle
WoW: Soul Shard (item 6265) reads raw 3 from the cache, returns 4
(`1 << 2`) to Lua callers — matches modern.

**Vanilla data sparsity.** 1.12 server data (at least on Turtle WoW)
populates `m_bagFamily` reliably on **individual loot items** but
leaves it 0 on most **bag containers** themselves. Soul Pouch,
Enchanting Bag, Herb Pouch all return 0 even though their
classification clearly implies a family. Only Light Quiver is
populated among the bags we tested. This means
`GetContainerNumFreeSlots`'s second return is reliable only for
quivers; for general bag categorization, addons should fall back to
checking `(class, subClass)` from `GetItemInfoInstant`. The
`GetItemFamily` *item* path is solid for routing loot to the right
specialty bag.

See [src/item/Info.cpp](src/item/Info.cpp).

## ~~30. `IsSpellKnown(spellID, [isPet])`~~ — DONE

Strict spellbook walk via `Spell::Lookup::FindSpellbookSlot`, filtered
by the requested book type. Returns true only when the spell is in
the player's (or pet's) spellbook UI arrays — does NOT cover talent
passives, profession recipes, or anything else that's "known" but not
displayable as a spellbook button.

### How we caught the bug pre-ship

Initially implemented `IsSpellKnown` using the same bitmap as
`IsPlayerSpell`, on the (wrong) assumption that the two functions
were equivalent in 1.12 since the bitmap was the engine's
authoritative knowledge store. Caught in review — `IsSpellKnown`
is meant to be the stricter check, distinct from `IsPlayerSpell`.

Cross-binary lookup confirmed: 3.3.5's `Script_IsSpellKnown` at
`0x0053C3A0` calls an inner helper at `0x0053B4E0` that does a strict
**spellbook array walk** (`[0x00BE6D88]` for player, `[0x00BE7D98]`
for pet) — not a bitmap. Modern WoW deliberately splits the two
functions: `IsSpellKnown` is the strict "spellbook button" check,
`IsPlayerSpell` (added in 5.0) is the broad "any kind of known"
query. We need both with their respective semantics.

### Behavior table (verified in-game)

| Spell type             | `IsPlayerSpell` | `IsSpellKnown` |
|------------------------|-----------------|----------------|
| Trained class ability  | true            | true           |
| Active talent grant    | true            | true           |
| Passive talent         | true            | **false**      |
| Profession recipe      | true            | **false**      |

Same split as modern WoW. The cross-binary technique paid off again
— caught the wrong implementation before it shipped.

See [src/spell/Info.cpp](src/spell/Info.cpp) and the worked example
in [CLAUDE.md](CLAUDE.md#cross-binary-reference--finding-112-implementations-via-newer-clients).

## ~~31. Unit flag bundle: `UnitIsAFK` / `UnitIsDND` / `UnitIsFeignDeath`~~ — DONE

All three shipped in [src/unit/Flags.cpp](src/unit/Flags.cpp):

- **`UnitIsAFK(unit)`** / **`UnitIsDND(unit)`** — read PLAYER_FLAGS at
  `[unit + 0xE68] + 0x08`, bits 1 (AFK = `0x02`) and 2 (DND = `0x04`).
  Works for any player-controlled unit (local self, target, party,
  raid, inspect targets). Gated on `UNIT_FLAG_PLAYER_CONTROLLED` first
  to avoid the crash hazard on creatures (where `+0xE68` is uninitialized
  garbage).
- **`UnitIsFeignDeath(unit)`** — reads `UNIT_FIELD_FLAGS` bit 29
  (`0x20000000`) at `[m_objectFields + 0xA0]`. Verified in-game with
  a Hunter using Feign Death — the function returns 1 while the
  feign-death buff is active, 0 otherwise.

**Where PLAYER_FLAGS lives — and the misleading first guess.** The
field index `0x8A` (= byte offset `0x228` in m_objectFields) is the
"natural" place to look (matches modern WoW's broadcast UpdateField),
but vanilla 1.12 *doesn't* broadcast PLAYER_FLAGS as a UpdateField —
reading `[descriptor + 0x228]` returns a different field entirely.
The actual storage is in a CGPlayer-side sub-struct at `[unit + 0xE68]`,
shared with the visible-items table at `+0x118`. Verified by
disassembling 1.12's nameplate AFK-prefix renderer at `0x005EC9E0`,
which is the *only* C-level code in stock 1.12 that decorates
nameplates with `<AFK>` — and which works for any unit, contrary to
the initial reading where I thought the JNE went the other way.

**Vanilla nameplate display.** Stock 1.12 shows `<AFK>` over **any**
nearby player's head, not just yourself — the function at `0x005EC9E0`
checks `[unit + 0xE68] + 0x08` bit 1 universally and only special-cases
the local player when a global at `[0x00B6E5CC]` is set (probably a
"hide my own AFK indicator" preference toggle). Verified in-game
on a non-Turtle stock 1.12.1 server.

## 32. `IsCurrentSpell(spellID)` — easy

Boolean: is the player currently casting / channeling this spell?
Needs a runtime "current cast spellID" global the engine maintains
for the spell-button UI. Likely lives near the cast bar state — a
short trace through `Script_GetCurrentCastingInfo`-style functions
(if any exist) or the cast-bar timer logic. Worth doing because
modern action-bar addons gate "highlight the active button" on this.

## 33. `C_Reputation.*` cluster — partial DONE

Modern table-shape and ID-based variants of the existing reputation
API. Most are thin wrappers around what we already have — useful for
addons backporting modern Lua-side code that prefers the C_Reputation
namespace.

- ✅ **`C_Reputation.SetWatchedFactionByID(factionID)`** — DONE.
  Calls the engine's inner watched-faction setter at
  `FUN_PLAYER_SET_WATCHED_FACTION = 0x004D6240` (`__fastcall(ecx =
  factionID) → void`) directly, bypassing
  `Script_SetWatchedFactionIndex`'s displayed-index round-trip.
  Works for unencountered factions too (the rep bar shows nothing
  until the player gains rep, then displays normally). factionID 0
  clears. Verified: Darnassus → 72 (Stormwind) → 0 (clear) all
  reflected by `GetWatchedFactionInfo()` immediately. Lives in
  `src/faction/Info.cpp`.
- **`C_Reputation.SetWatchedFactionByIndex(index)`** — modern alias
  for `SetWatchedFactionIndex`. One-line registration that points to
  the engine function via our existing wrapper.
- **`C_Reputation.GetFactionDataByIndex(index)`** /
  **`GetFactionDataByID(factionID)`** — table-returning variants of
  `GetFactionInfo` / `GetFactionInfoByID`. Pack the existing 11-tuple
  into named fields (`name`, `description`, `reaction`, `barMin`,
  `barMax`, `currentStanding`, `atWarWith`, `canToggleAtWar`,
  `isHeader`, `isCollapsed`, `hasRep`, `factionID`).
- **`C_Reputation.GetWatchedFactionData()`** — table form of
  `GetWatchedFactionInfo` (which 1.12 has natively). The watched
  factionID is stored at `[player + 0xE68] + 0x10C4` per the
  inner setter's compare-and-skip path — useful if we add a
  `IsWatchedFaction(id)` getter too (engine has one at
  `0x004D62F0` already).

## 34. `C_DateAndTime.GetSecondsUntilDailyReset()` — easy

Modern API returns seconds until the next "daily reset" — vanilla 1.12
has no daily reset concept (no daily quests, no LFG cooldown, etc.)
but the closest sensible analog is **seconds until midnight server
time**, which is what addons like RepBuddy actually use this for
(`Util/RepBuddy.lua:185` — refreshes the day's earned-rep snapshot).

Implementation: read server hour/minute from whatever global vanilla's
`Script_GetGameTime` (`0x...` — find via `raw_globals.txt`) reads,
then compute `(24-hour)*3600 - minute*60`. Returns an int. If the
server hour can't be resolved (e.g. before first time-sync packet),
fall back to `86400` (a full day) — better than 0, which would make
addons think a reset just happened and wipe daily snapshots.

Could share infrastructure with #5 `GetServerTime` if we wire that up
first — if we have the server epoch, "seconds until next UTC midnight"
is `86400 - (epoch % 86400)`. Otherwise the GetGameTime path stands
alone.

## 35. `GetPlayerInfoByGUID(guid)` — multi-day, deferred

Modern Classic Era / 3.3.5 returns
`localizedClass, englishClass, localizedRace, englishRace, sex, name, realm`
for any GUID the engine knows about. **In 1.12 a partial backport is
straightforward; the full version requires building our own packet-cache
layer.** Investigated and parked.

### What 1.12 has

- **Active object manager** (`ClntObjMgrObjectPtr` at `0x00468460`,
  signature `__fastcall(typeMask, dbgStr, guid_lo, guid_hi, dbgCode) →
  CGObject_C *`, `ret 0xC`). Resolves a GUID to a CGObject *only if the
  object is currently active in the client* — your party, raid, target,
  mouseover, nearby visible players. Returns NULL otherwise.
  - For an active CGPlayer_C, race/class/sex are byte-packed in
    `UNIT_FIELD_BYTES_0` at `[m_objectFields + 0x90]`
    (`m_objectFields = [unit + 0x110]`):
    - byte 0 = race, byte 1 = class, byte 2 = gender, byte 3 = power.
  - The player's name is reachable via the CGObject vftable's
    `GetName` slot (slot index ~28 per VanillaMinimapTracking's
    `CGObject_C_VfTable`).

- **NameCache** (instance at `0x00CE870C`, generic-cache class with
  init at `0x00760390`, allocator at `0x00760450`, entry-init helper
  at `0x006C6C90`). Entry size **0x38 bytes**, ~256 buckets initially.
  Layout (partial):
  - `+0x00..+0x07`: GUID (likely)
  - `+0x18`: state/flags (init = 3; bit 1 set = "in cache", bit 1 = ??)
  - `+0x1C`: free-list link
  - `+0x20..+0x2F`: ~16 bytes of payload — **fits a player name (≤12
    chars + null), nothing more**.
  - RTTI string `NameCache@@_KVCHashKeyGUID@@@@` at `0x00855EE3` confirms
    the GUID-keyed shape.

- The cache populator at `0x006C76A0` calls `ClntObjMgrObjectPtr` first
  and only allocates a NameCache entry if the GUID resolves to a
  CGObject with `[obj+0xD8] != 0`. **The engine's NameCache is not fed
  by SMSG_NAME_QUERY_RESPONSE** — it only caches names for objects we've
  already seen as live CGObjects. So even for offline guildies/friends,
  this cache is empty.

- **Friends/Guild/Who** caches do exist with GUID-bearing entries:
  friends list at `0x00C28168` (record fields `name@+0x04`, `level@+0x10`,
  `classID@+0x14`, `area@+0x18`), guild roster around `0x00C0E1EC`, who
  results count at `0x00C2A120`. These have **class but no race** — the
  SMSG_FRIEND_LIST and SMSG_GUILD_ROSTER wire formats simply don't carry
  race or sex.

### Two implementation paths

**A. Visible-only (1-2 hours, narrow coverage)**

Use `ClntObjMgrObjectPtr` + UNIT_FIELD_BYTES_0 + vftable->GetName.
Returns the full 7-tuple for live CGPlayer_C objects, `nil` otherwise.
Realm is whatever global holds the realm name (single-realm in 1.12).
Class/race byte → string via static maps (1=Warrior … 11=Druid;
1=Human … 8=Troll). Useful for tooltip/raid-frame use cases that
already have the player resolvable.

**B. Full async cache (3-5 days)**

Build our own player-info cache fed by hooking SMSG_NAME_QUERY_RESPONSE
(opcode `0x51`, wire format `GUID(8) + name(cstring) + realm(cstring) +
race(u32) + sex(u32) + class(u32)`). Send CMSG_NAME_QUERY (`0x50`) for
cache misses. Fire `NAME_QUERY_DATA_LOAD_RESULT(guid, success)` event on
response (reuse `Event::Custom`).

Two unsolved unknowns make this multi-day:

1. **Where is opcode dispatch in 1.12?** No clean
   `Net::RegisterPacketHandler` analog has been located. If dispatch is
   a giant switch (likely), we'd hook the dispatcher itself or the
   per-opcode handler we identify by static analysis. Need substantial
   disassembly time.
2. **What's the send-packet primitive?** Have to find a `CDataStore`
   build + `CWorld::Send` analog and figure out the calling convention.

If we ever take the dependency on SuperWoWhook (already loaded in the
user's setup), it exposes `RegisterPacketHandler` and `SendPacket` as
its own DLL ABI — would short-circuit both unknowns. But that introduces
a hard dependency on a third-party DLL, contrary to ClassicAPI's design.

### Recommended starting point when this gets picked up

Path A (visible-only) lands quickly and covers the most common
"resolve a GUID I'm already looking at" use case. Document the
limitation prominently in the API doc. Defer Path B until either a
concrete addon need surfaces or we end up doing the network-layer
disassembly for another reason (e.g. a packet-driven feature like
`C_FriendList` or `BNGetFriendInfo` polyfilling).

## ~~36. `GetTalentSpellID(tabIndex, talentIndex[, rank])`~~ — DONE

Bridges the talent system to the spell system: once you have the
spellID, every spell API chains naturally (`GetSpellInfo`,
`C_Spell.GetSpellDescription`, `GameTooltip:SetSpellByID`,
`IsPassiveSpell`, `C_Spell.GetSpellLink`, …). 1.12's `GetTalentInfo`
returns `(name, icon, tier, column, currentRank, maxRank, ...)` — no
spellID — so addons used to have to maintain their own
`(class, tab, idx) → spellID` lookup tables.

Reads from the engine's per-tab talent arrays at `[0x00BDCD28]`
(populated at login from `Talent.dbc` filtered by the player's class),
not Talent.dbc directly. Each `TabInfo *` exposes:

- `+0x00` TalentTab.dbc rowID
- `+0x04` pointsSpent
- `+0x08` numTalents
- `+0x0C` `TalentEntry *` array (stride `0x54`)

Each `TalentEntry`:

- `+0x00` talentID (Talent.dbc primary key)
- `+0x10..+0x33` `SpellRank[9]` (rank-N spellID at index N-1; vanilla
  uses 0..4, higher slots stay zero)

When `rank` is omitted, default = player's currentRank, derived by
delegating to `Script_GetTalentInfo` and reading its 5th return —
piggybacking on the engine's spell-knowledge logic. If currentRank is
0, falls back to rank 1.

### Two gotchas worth noting

1. **The engine's currentRank loop walks SpellRank backwards** — `lea
   edx, [ebx+0x30]` then `sub ecx, 4` per iteration, ending at +0x10.
   I initially read +0x30 as the start of `SpellRank[]`; it's actually
   the END (slot 9, always zero in vanilla). Confirmed empirically by
   a debug build returning 0 from +0x30 vs the right spellID from
   +0x10.

2. **Stock 1.12.1's `GetTalentInfo` returns 8 values, not the 6 I
   expected** based on naive disassembly. Reading the 5th return uses
   `stack[-(n - 4)]` (dynamic from the actual return count) instead of
   a hardcoded `stack[-2]`.

See [src/talent/Info.cpp](src/talent/Info.cpp) and the talent block
(`VAR_TALENT_TAB_*` / `OFF_TABINFO_*` / `OFF_TALENT_SPELL_RANK`) in
[src/Offsets.h](src/Offsets.h).

## ~~37. `GetSpellSchool(spellID)`~~ — DONE

Shipped as [src/spell/School.cpp](src/spell/School.cpp). Returns
`(schoolID, schoolName)` where schoolID is 1-based (1=Physical,
7=Arcane) and schoolName is the locale-independent English name.

### Spell.dbc School field — VERIFIED OFFSET

Vanilla 1.12 stores School as a **0-based integer at record `+0x04`**
(NOT the SchoolMask form the original TODO note speculated — that's
TBC+). Verified empirically via the probe Lua functions:

```
/dump _classicapi_FindSpellField(133, 116, 4, 16)   -- mask form: empty
/dump _classicapi_FindSpellField(133, 116, 2, 4)    -- 0-based int: offset 4 ✓
/dump _classicapi_FindSpellField(133, 116, 3, 5)    -- 1-based int: empty
```

Fireball (133) → School=2 (Fire); Frostbolt (116) → School=4 (Frost).

The probe helpers (`_classicapi_ProbeSpellRecord`,
`_classicapi_FindSpellField`) were temporarily exposed to derive the
offset and removed in the same commit that shipped `GetSpellSchool`.

## 38. `GetSpellRadius(spellID)` — easy

Returns the AOE radius in yards, or `nil` for non-AOE spells.
`Spell.dbc` has a `SpellRadiusIndex` field that points into
`SpellRadius.dbc` (instance at `0x00C0D7A8` per `docs/DBCs.md`,
already wired up via `VAR_SPELL_RANGE_RECORDS`-style helpers in
`src/spell/Info.cpp`).

Stride and field layout for `SpellRadius.dbc`: needs derivation
(should be `{id, radius_min, radius, radius_max}` per the public
schema, but verify). Sub-DBC lookup pattern matches what
`LookupSubRecord` already does in `Info.cpp` for icon / cast time /
range.

Useful for AOE damage trackers, ground-effect addons, and any aura
library that wants to render an indicator radius. Modern WoW exposes
this via `GetSpellRadius` (deprecated) or returns from
`C_Spell.GetSpellInfo`.

## ~~39. `GetFactionParentID(factionID)`~~ — DONE

Reads `Faction.dbc` `ParentFactionID` at record `+0x48` via the
existing `Faction::Info::FactionRecord` helper. Returns `0` for
top-level factions, `nil` for invalid IDs. Verified in-game:
`GetFactionParentID(72)` → 469 (Stormwind → Alliance Forces),
`GetFactionParentID(469)` → 0 (Alliance is top-level),
`GetFactionParentID(99999)` → nil.

See [src/faction/Info.cpp](src/faction/Info.cpp) and
`OFF_FACTION_PARENT_ID` in [src/Offsets.h](src/Offsets.h).

## 40. `UnitRaceBase(unit)` — easy

Sibling to TODO #22 `UnitClassBase` — locale-independent race string
(`"Human"`, `"Orc"`, `"Dwarf"`, `"NightElf"`, `"Scourge"`, `"Tauren"`,
`"Gnome"`, `"Troll"`). 1.12's `UnitRace(unit)` returns the localized
display name only; addons currently match against locale-specific
strings or maintain their own race-byte → name maps.

Read from `UNIT_FIELD_BYTES_0` (byte 0 = race). Same `[unit + 0x110]`
descriptor path `InCombatLockdown` already uses (see
`OFF_UNIT_DESCRIPTOR` / `OFF_UNIT_FIELD_FLAGS` for the pattern). The
9 race strings are static — encode as a small table.

Modern WoW added `UnitRace`'s second return as the locale-independent
form in 5.0; we surface it via this distinct call rather than
modifying `UnitRace`'s 1.12 signature.

## 41. `GetActiveTalentGroup()` / `GetCurrentSpec()` — easy

Vanilla has no formal "spec" system but every class has 3 talent tabs
and the player always invests in one primarily. Returns the
1-based tab index (1, 2, or 3) of the tab with the most points spent,
or `0` if the player has no points spent at all (low-level
characters, or after a respec).

Trivial implementation: walk `GetNumTalentTabs()` (= 3), call
`GetTalentTabInfo(tab)` for each, compare `pointsSpent`. Pick max.
No DBC reads, no engine internals — pure Lua-equivalent computation
that's just easier to call than write inline every time.

Modern WoW's `GetSpecialization()` returns 1..4 for the player's
active spec. This is the closest 1.12 analog, useful for class
addons that want to render different UI per spec ("am I tanking?
healing? DPS?") without having to do the talent-counting dance
themselves.

## ~~42. `GetTalentIDByIndex(tabIndex, talentIndex)`~~ — DONE

Reads `TalentEntry+0x00` from the per-tab talent arrays. The
talentID-at-+0x00 offset was already confirmed empirically during
the GetTalentSpellID debug pass (`entryFirstDword=174` matched
Inner Focus's row ID), so this was a 5-line addition piggybacking
on the existing struct walk. Verified in-game:
`GetTalentIDByIndex(1, 9)` → 174,
`GetTalentIDByIndex(1, 1)` → 166.

Unblocks #43 `GameTooltip:SetTalentByID` whenever someone wants the
modern by-ID tooltip dispatcher.

See [src/talent/Info.cpp](src/talent/Info.cpp).

## ~~43. `GameTooltip:SetTalentByID(talentID)`~~ — DONE

Shipped as [src/talent/Tooltip.cpp](src/talent/Tooltip.cpp). Works
for **any class's** talents, not just the player's. Two-tier
resolution:

1. **Player-class fast path** — walks the local player's TabInfo
   arrays for a matching talentID (same arrays #42 reads from,
   inverted). On hit, dispatches to `Script_GameTooltip_SetTalent`
   (registry slot 34, `0x00535170`) with the resolved `(tab, idx)`.
   Renders the full talent tooltip with rank counter, prereqs, and
   the "click to learn next rank" prompt.
2. **Cross-class fallback via `Talent.dbc`** — vanilla 1.12 only
   loads the local player's class talent data into TabInfo, so
   other classes' talents aren't findable in tier 1. Tier 2 looks
   up the talent record directly in `Talent.dbc` (records pointer
   `0x00C0D6E8`, count `0x00C0D6EC`, indexed by talentID), reads
   the rank-1 spellID at `+0x10`, and dispatches the spell tooltip
   via the same `Spell::Tooltip::ShowByID` helper that
   `SetSpellByID` uses. Renders the spell info (cast time, range,
   mana cost, description) without talent-specific decorations
   (rank counter, prereq lines).

Modern WoW's cross-class tooltip is slightly richer (talent name +
"Rank 0/N" header before the spell description). Building that
would require Lua-level `GameTooltip:AddLine` work; deferred since
the spell tooltip already answers "what does this talent do?".

### Refactoring done as part of this

Extracted `Spell::Tooltip::ShowByID(L, spellID)` into
[src/spell/Tooltip.h](src/spell/Tooltip.h) so the resolve-self +
`BuildSpellTooltip` chain is shared between
`GameTooltip:SetSpellByID` and the cross-class fallback in
`SetTalentByID`. Saves duplicating the
`FrameScript_PushObject`/`GetObject` inline-asm sequence.

### `Talent.dbc` record layout (verified standard vanilla schema)

| Offset | Field |
|--------|-------|
| `+0x00` | `uint32 ID` |
| `+0x04` | `uint32 TabID` |
| `+0x08` | `uint32 TierID` (row, 0-based) |
| `+0x0C` | `uint32 ColumnIndex` (0-based) |
| `+0x10` | `uint32 SpellRank[0..8]` — rank-1 spellID at `+0x10` |

The per-player `TalentEntry` (in `TabInfo->talents[]`) shares the
same `SpellRank` offset, so we reuse `OFF_TALENT_SPELL_RANK = 0x10`
for both. The `TalentEntry` stride (`0x54`) and the Talent.dbc
record stride differ — Talent.dbc records may be larger — but for
our purpose only the rank-1 spellID at `+0x10` matters.

### Verified in-game

Both tiers tested — own-class talents render the full talent
tooltip, cross-class IDs render the corresponding spell tooltip.

## ~~44. `GameTooltip:SetItemByID(itemID)`~~ — DONE

snprintf the hyperlink string and dispatch to the existing
`Script_GameTooltip_SetHyperlink`. Same registration pattern as
`SetSpellByID`.

### Caching gotcha caught in testing

Initial test showed only the item *name*, not the full tooltip, for
arbitrary itemIDs. Cause: 1.12 lazy-loads item data into the
client-side cache (the same cache `C_Item.GetItemInfoInstant` reads).
Items the player has never encountered show only what's in the link
itself (the name) until the cache is populated via
`SMSG_ITEM_QUERY_SINGLE_RESPONSE`. This isn't a bug in our
implementation — it's the same behavior as modern's `SetItemByID`,
which also requires `C_Item.RequestLoadItemDataByID` first for
arbitrary IDs.

Documented the caveat in [docs/API.md](docs/API.md) with the
recommended `IsItemDataCachedByID` / `RequestLoadItemDataByID` /
`ITEM_DATA_LOAD_RESULT` pattern callers should follow.

See [src/item/Tooltip.cpp](src/item/Tooltip.cpp).

## ~~45. `GameTooltip:SetUnitAura(unit, index, [filter])`~~ — DONE

Pure dispatch wrapper. Branches on filter string (`"HARMFUL"` →
`SetUnitDebuff`, anything else → `SetUnitBuff`) and forwards via
the existing script function. `filter` defaults to helpful when
omitted, matching modern.

See [src/unit/Tooltip.cpp](src/unit/Tooltip.cpp) and the
`FUN_SCRIPT_GAMETOOLTIP_SET_UNIT_*` block in
[src/Offsets.h](src/Offsets.h).

## 46. `GameTooltip:SetFrameStack([showHidden])` — medium-large, deferred

Renders a tooltip listing the chain of frames at the mouse cursor,
sorted by strata + level. The 3.0+ debug-tooling primitive used by
Blizzard's `Blizzard_DebugTools` and every layout-debugging addon.

Our bundled `DebugTools/` backport already implements its own walk
over `EnumerateFrames()` (see the comment in
[DebugTools/DebugTools.lua](DebugTools/DebugTools.lua):
"No GameTooltip:SetFrameStack -- a basic walk over global frames is
used instead"). The Lua version works but is slow (thousands of
frames each call) and misses anonymous frames the engine sees but
doesn't expose to globals.

A C implementation would walk the engine's internal frame list
directly (faster, complete) and use the engine's hit-testing to
filter to frames actually at the cursor. Significant scope:

- Locate the engine's frame list (`CSimpleFrame::s_frameList` or
  similar) — not yet identified.
- Find the cursor hit-test path — engine has one for the click
  dispatcher, but locating it needs disassembly time.
- Strata/level sort.
- Format and feed into `GameTooltip:AddLine` — we don't currently
  call AddLine from our DLL, would need to add that path.

Skip until either the DebugTools backport's slowness becomes a real
pain point, or we end up doing frame-list disassembly for another
reason (e.g. exposing a `GetMouseFocus()` polyfill).

## ~~47. `IsPlayerSpell(spellID)`~~ — DONE

Single bitmap lookup at `[VAR_PLAYER_SPELL_BITMAP] = [0x00B710FC]`.
The engine maintains a dword bitmap with one bit per spellID — set
when the player learns a spell (any source: training, talent
investment, profession recipe, racial unlock, etc.) and cleared on
unlearn. We just consult the same bit the engine itself reads via
the helper at `0x0060C740`:

```cpp
return (bitmap[spellID >> 5] & (1 << (spellID & 31))) != 0;
```

### How we found it

The naive walk-based implementation (originally drafted) failed for
profession recipes — turns out vanilla 1.12's `VAR_PLAYER_SPELLBOOK`
arrays DON'T contain profession recipe spellIDs (verified
empirically: a tailor with Bolt of Linen Cloth fails
`FindSpellBookSlotByID(2963)`). Surfaced during in-game testing.

To confirm whether 1.12 had a unified spell-knowledge data structure,
I dumped 5.4.8's `Script_IsPlayerSpell` (5.4 = the version where the
function was first added) and saw a clean bitmap pattern:

```
mov ecx, eax
and ecx, 0x1F            ; bitPos = spellID & 31
shl edx, cl              ; mask = 1 << bitPos
mov ecx, [imm32]         ; bitmap base ptr
shr eax, 5               ; idx = spellID / 32
and eax, [ecx + eax*4]   ; bitmap[idx] & mask
```

Searched 1.12 for the same pattern → matched at the helper
`0x0060C740`, which `Script_GetTalentInfo` calls for currentRank
derivation. Same shape, same purpose, just at a different bitmap
address (`[0x00B710FC]`).

### Modern-parity semantics

Verified against 1.15.x: only the player's currentRank spellID has
its bit set; lower-rank IDs return false. A 5/5 Unbreakable Will
player sees `IsPlayerSpell(14791)` (rank 5) = true but rank 4 / 3 /
2 / 1 = false — same in 1.12 with our implementation, same in 1.15.

This means our implementation matches modern exactly, including the
"only current rank counts" quirk. Addons backporting from later
expansions that key on currentRank's spellID work unmodified.

### Spillover wins for the rest of TODO

The same bitmap makes #30 `IsSpellKnown(spellID, isPet)` trivial —
ship it as a thin wrapper using the same lookup (or split: player
scan via bitmap, pet via existing pet spellbook walk). The
`Spell::Lookup::FindSpellbookSlot` walk is now only needed for the
slot-and-bookType return shape, not for knowledge queries.

See [src/spell/Info.cpp](src/spell/Info.cpp) and
`VAR_PLAYER_SPELL_BITMAP` in [src/Offsets.h](src/Offsets.h).

## 48. `GetSpellCooldown(spellID)` overload — easy

Modern signature accepts a spellID directly. 1.12 only has
`GetSpellCooldown(slot, bookType)` which forces callers to first
resolve a spellID into a spellbook slot (and fails for spellIDs not
in the spellbook — talent passives, recipes, etc.).

The engine clearly tracks cooldowns by spellID internally (the
existing slot-based version reads spellbook[slot] and feeds the
spellID to a downstream cooldown manager). Cross-binary technique
applies cleanly:

1. Find `Script_GetSpellCooldown` in 5.4.8 — the modern overload
   that takes spellID directly.
2. Decode it to identify the cooldown manager's data structure
   (likely a hashmap/array indexed by spellID with `(start, duration,
   enable)` triple per entry).
3. Search 1.12 for the same access pattern.
4. Wrap as a global `GetSpellCooldown(spellIDOrSlot, [bookType])` that
   detects the arg shape and routes accordingly.

Useful for any cooldown-tracking addon that wants to query talent
abilities or off-spellbook spells.

## ~~49. `IsUsableSpell(spell)` / `C_Spell.IsSpellUsable(spellID)`~~ — DONE

Both shipped. `IsUsableSpell` accepts spellID or `(slot, bookType)`
via the same `Spell::Lookup::SpellbookSlotToID` resolver other
spell-API functions use. Returns `(1, nil)` / `(nil, 1)` /
`(nil, nil)` per legacy convention. `C_Spell.IsSpellUsable` is the
modern shape — same logic, returns proper booleans.

**Two engine-cache approaches investigated and discarded:**

1. **Per-spell cache at `0x004F0F40` returning fields at +0x564
   / +0x568** — initially looked promising (the action-bar
   dispatcher at `0x004E5BA0` reads these), but the writers
   (`0x004EFF17` cluster) showed they're parsed config integers,
   not real-time usability bools. False trail.
2. **Action-bar per-slot caches at `0x00BC67A0` (noMana) and
   `0x00BC6B60` (usable)** — these ARE real-time, but only updated
   for the 120 action-bar slots. Spells off the bar stay
   uninitialized. Action-bar-only is the wrong abstraction since
   modern `IsUsableSpell(spellID)` is action-bar-agnostic.

Settled on a **manual five-check approach**:
1. Player knows the spell (`VAR_PLAYER_SPELL_BITMAP` lookup —
   covers profession recipes, talents, racials, etc.).
2. Player is alive (`HEALTH > 0`).
3. Spell is not on cooldown (`FUN_SPELL_QUERY_COOLDOWN` —
   the engine helper `Script_GetSpellCooldown` calls internally
   after slot resolution, queried with `bookType=0` for player).
4. Player has ≥ `ManaCost` of the spell's `PowerType` —
   only this failure flips `noMana=true`.
5. Player has all required reagents in bags
   (`Spell.dbc` Reagent[8] / ReagentCount[8] at `+0x110` /
   `+0x130`, walked via `Item::Location::ResolveBag`).

**Not checked** (deliberate, different concerns): silence, GCD,
stance/form, range, target type, line-of-sight, casting state.

**Verified on Turtle WoW:**
- Mana check: Renew rank 3 (cost 105) reports usable at 144 mana
  and unusable at 39 mana, transitioning exactly at the cost
  boundary.
- Reagent check: SHIPPED BUT UNVERIFIED — the Reagent[8] /
  ReagentCount[8] offsets at `+0x110` / `+0x130` are the
  CMaNGOS-documented vanilla layout, and we know spell records
  match CMaNGOS docs (PowerType=+0x7C and ManaCost=+0x80 both
  match), so high confidence — but no in-game verification yet.
  Should test with a reagent spell (e.g., Mage Teleport spell 3565
  needs Rune of Teleportation 17031). Failure mode if offsets are
  wrong: reagent check becomes a no-op (always passes).
- Cooldown check: SHIPPED BUT UNVERIFIED in this implementation —
  the helper signature was traced via `Script_GetSpellCooldown` at
  `0x004B40A0` but not exercised in-game. Should test with a
  cooldown'd spell.

**Important offset correction along the way.** Initial implementation
read POWER1 at descriptor `+0x5C` based on the CMaNGOS-documented
field index `0x17`. That was *max* mana — current was at `+0x44`
(field index `0x11`, six fields earlier than the standard layout).
The whole UNIT_FIELD layout in 1.12.1 is shifted 0x18 / 6 fields
earlier than CMaNGOS documents:

| Field         | CMaNGOS docs | 1.12.1 actual |
|---------------|-------------:|--------------:|
| HEALTH        | +0x58        | +0x40         |
| POWER1 (mana) | +0x5C        | +0x44         |
| POWER2..5     | +0x60..+0x6C | +0x48..+0x54  |
| MAXHEALTH     | +0x70        | +0x58         |
| MAXPOWER1..5  | +0x74..+0x80 | +0x5C..+0x6C  |
| LEVEL         | +0x88        | +0x70         |
| FLAGS         | +0xA0        | +0xA0 ✓       |

FLAGS still lands at +0xA0 (verified separately by
`Script_UnitPlayerControlled` and `Script_UnitAffectingCombat`).
Trust the binary, verify with `_classicapi_DescDump` if you need
to derive new ones.

See [src/spell/Usable.cpp](src/spell/Usable.cpp).

## 50. Cross-binary technique applications worth picking up — meta

The `IsPlayerSpell` discovery proved out the cross-binary disassembly
technique (documented in [CLAUDE.md](CLAUDE.md#cross-binary-reference--finding-112-implementations-via-newer-clients)).
Other pending TODOs that look like good fits for the same approach:

- **#7 `UnitAuraBySlot` / `UnitAuraSlots`** — modern slot-based aura
  iteration. Aura state is in CGUnit's UpdateField data; modern
  `UnitAura` reads slot offsets we can decode from 5.4.8 and use in
  1.12.
- **#32 `IsCurrentSpell(spellID)`** — runtime cast-state global. The
  engine has a "currently casting spellID" global (the cast-bar UI
  reads it). Modern's `IsCurrentSpell` reads the same value.
- **#42 `GetTalentIDByIndex`** — already easy with the talent struct
  we now know (talentID at TalentEntry+0x00); cross-binary not needed
  here but could verify against 5.4.8's `GetTalentInfoByID` for
  semantics.

Tag here so we remember the technique exists when picking up these
items.

## ~~51. `C_Container.GetContainerNumFreeSlots(bagID)`~~ — DONE

Returns `(numberOfFreeSlots, bagType)` for the player's backpack
(`bagID = 0`) or one of the four equipped bag slots (`bagID = 1..4`).
`bagType` is the BagFamily bitfield — vanilla 1.12 has only three
non-zero families (quiver=1, ammo pouch=2, soul shard bag=4); modern
expansions added 20+ more, but reading the bitfield gives addons a
forward-compatible shape regardless. Always returns two values, falling
back to `(0, 0)` for unsupported bag IDs (bank/keyring/out-of-range) —
matches 3.3.5's behavior.

Implementation walks the bag and counts empties via
`Item::Location::ResolveBag`. Bag metadata (slot count + BagFamily) for
real bags 1..4 comes from the equipped bag's `ItemStats_C` cache
record: `m_containerSlots` at +0x64, `m_bagFamily` at +0x1D0. Backpack
short-circuits to `(16, 0)` since vanilla's main backpack is fixed-size
and unfamilied. Field offsets derived from VanillaHelpers's
`struct ItemStats_C` layout, cross-checked against the existing
`m_stackable` at +0x60 (already in `Offsets.h`).

The slot-walk uses ResolveBag, which clobbers Lua stack[1]/[2] each
call (a requirement of the engine's `PackBagSlot`). That's safe here —
we own the stack inside the callback and the user's bagID arg is
copied to a local before the loop. Cross-checked in-game against a
manual `GetContainerItemID` walk; counts match for all five bags.

**bagType return is sparse on vanilla data.** Empirically, only Light
Quiver carries a non-zero `m_bagFamily` field among the bags we tested
on Turtle WoW; Soul Pouch, Enchanting Bag, Herb Pouch all return 0.
The same field on individual items (arrows, shards, herbs) is
reliable — see TODO #29's notes. Addons that need bag-category
discrimination should look at `(class, subClass)` from
`GetItemInfoInstant` rather than `bagType`. The conversion from
1.12's raw-ID encoding to the modern bitmask
(`1 << (ID-1)`) still applies for the rare populated cases.

See [src/item/Bag.cpp](src/item/Bag.cpp).

## ~~52. `C_Container.PlayerHasHearthstone()` / `C_Container.UseHearthstone()`~~ — DONE

Modern convenience pair for the most-used bag walk in addons. Walks
bags 0..4 looking for the hearthstone (vanilla itemID 6948 — the only
one in 1.12; modern WoW recognizes many hearthstone-toy itemIDs but
none exist in this client). Stops on first match.

`PlayerHasHearthstone()` returns the itemID (always 6948 or nil).
`UseHearthstone()` invokes the engine's `Script_UseContainerItem`
(`0x004FA0E0`) with `(bagID, slot)` set up on the Lua stack — same
secure-action path a regular `UseContainerItem(bag, slot)` Lua call
would take, so cooldown / combat / movement-cancel handling is
identical. Returns `true` if a hearthstone was found and the call
dispatched, `false` otherwise (the cast itself can still fail
downstream — same caveat as manually calling `UseContainerItem`).

Walking helper is shared between the two via `FindHearthstone(L,
*outBag, *outSlot)`. Slot counts come from delegating to the engine's
own `Script_GetContainerNumSlots` (`0x004F9560`) rather than a
hardcoded vanilla cap, so custom servers with larger-than-24-slot
bags work without code changes.

See [src/item/Hearthstone.cpp](src/item/Hearthstone.cpp).

## ~~53. `LE_EXPANSION_*` constants~~ — DONE

Twelve expansion-level enum globals (`LE_EXPANSION_CLASSIC` through
`LE_EXPANSION_MIDNIGHT`) plus `LE_EXPANSION_LEVEL_CURRENT`, all
matching the canonical retail `Enum.ExpansionLevel` table values.
On 1.12, `LE_EXPANSION_LEVEL_CURRENT` resolves to
`LE_EXPANSION_CLASSIC` (= 0). Lets addons backported from later
expansions use the modern `if LE_EXPANSION_LEVEL_CURRENT < ...`
gating idiom without conditionally defining the constants
themselves.

Set via the same Lua C API path `CLASSIC_API_VERSION` uses (no
`FrameScript_Execute` source). One per entry, written to
`_G[name] = value` from the module's `RegisterLuaFunctions` hook
which fires after the engine's `LoadScriptFunctions` boot phase.

See [src/expansion/Constants.cpp](src/expansion/Constants.cpp).

## ~~54. `C_AddOns.*` namespace — convenience wrappers~~ — DONE (Tier 2)

Modern WoW (10.x) moved most addon API into the `C_AddOns` namespace.
1.12 already exposes most of the underlying functions as globals;
we deliberately do *not* duplicate them under `C_AddOns` (Tier 1 in
the prior version of this entry — pure namespace mirror, low value).
What's worth adding is the post-vanilla *new* functionality.

### Tier 1 — DROPPED

Bulk wrapping of 17 existing 1.12 globals (`IsAddOnLoaded`,
`LoadAddOn`, etc.) under `C_AddOns.*`. Skipped — addons that need
the namespace can `C_AddOns = C_AddOns or {} ; C_AddOns.IsAddOnLoaded
= IsAddOnLoaded` themselves in two lines.

### ~~Tier 2 — convenience wrappers around `GetAddOnInfo`~~ — DONE

Shipped as `src/addons/Info.cpp`. Six wrappers, all bypassing
`Script_GetAddOnInfo` and reading directly from the engine's
addon-registry helpers we found at `0x0051DF00..0x0051E050`:

- `C_AddOns.GetAddOnName(indexOrName)` — direct via `GetByIndex`
  (numeric: returns the entry's inline name buffer; string:
  echoes the input after validating existence).
- `C_AddOns.GetAddOnTitle(indexOrName)` — calls
  `GetTitleByName` (`0x0051DF20`) directly.
- `C_AddOns.GetAddOnNotes(indexOrName)` — calls
  `GetNotesByName` (`0x0051E050`) directly.
- `C_AddOns.IsAddOnLoadable(arg [, character, demandLoaded])` —
  returns `(loadable, reason)`. Still dispatches through
  `Script_GetAddOnInfo` (5th + 6th return) because the engine's
  per-field accessor is buried behind a locale-derived field key.
  Gated on a `ResolveAddOnName` existence check first to avoid
  the `lua_error` `Script_GetAddOnInfo` raises for out-of-range
  numeric indices.
- `C_AddOns.GetAddOnSecurity(indexOrName)` — same dispatch
  shape as `IsAddOnLoadable`.
- `C_AddOns.DoesAddOnExist(indexOrName)` — direct via
  `ResolveAddOnName`; correctly returns `false` for unknown
  names rather than the dispatch-based heuristic which
  false-positived on garbage strings (engine echoes the input
  back as ret1 even on miss).

The underlying `FUN_ADDON_GET_BY_INDEX` / `_TITLE_BY_NAME` /
`_NOTES_BY_NAME` accessors share a hash-table walk: they take a
char* name (entry pointer doubles as a name string thanks to
the inline 12-byte buffer), hash it to find the registry entry,
then walk a per-entry metadata hash table keyed by the field
name (`"Title"`, `"Notes"`) to extract the value. We piggyback
on the same name → entry lookup for all wrappers.

Notable bug caught in testing: the original implementation used
`PushValue` + `Insert(L, 1)` + `SetTop(L, 1)` to pull the
desired return down to the bottom of the stack. That combo
silently returned the LAST pushed value of `Script_GetAddOnInfo`
regardless of `returnPos` — same family of broken-stack-state
issues as the `RegisterTableFunction` gotcha in CLAUDE.md.
Replacing with `SetTop(L, 1 + returnPos)` (truncate to keep
the desired return on top, return 1 to Lua) fixed it.

### Tier 3 — research / new work

- **`GetAddOnInterfaceVersion(index)`** — reads `## Interface:` from
  the .toc. The engine parses this at addon-load time; we'd need
  to find where it stores the per-addon value. (Note: vanilla also
  exposes `GetAddOnMetadata(name, "Interface")` natively, which
  may already cover this — verify before implementing.)
- **`GetAddOnLocalTable(index)`** — NOT FEASIBLE in 1.12. The
  modern API returns the addon's private namespace passed as the
  second `...` arg to each addon file (`local name, addon = ...`).
  Verified empirically: vanilla 1.12 calls `lua_pcall(L, 0, 0, -2)`
  on every addon chunk — `nargs = 0`, so `...` is always empty.
  Confirmed at four pcall sites in the addon-loader region
  (0x704B68, 0x704D22, 0x704E79, 0x705199), all with the same
  `xor edx, edx; mov ecx, L; call lua_pcall` pattern. The
  convention was added in 3.0+ when WoW moved to Lua 5.1; the
  underlying engine machinery (per-addon table allocation,
  vararg injection) doesn't exist in 1.12. Adding it would
  require hooking the pcall sites to inject args AND maintaining
  a per-addon-name table map ourselves — a feature add, not just
  an API exposure. Skip unless an addon explicitly needs it.
- **`DoesAddOnHaveLoadError(index)`** — load-error tracking. May
  not exist in 1.12 in a queryable form.
- **`IsAddOnDefaultEnabled(index)`** — initial-enabled state from
  the .toc `## DefaultState:` tag (not in 1.12 .toc syntax).

### Skip

- `GetScriptsDisallowedForBeta` — beta-specific; no 1.12 analogue.

### Why this is worth doing

Almost every modern addon released in the last few years uses
`C_AddOns.IsAddOnLoaded` instead of the global. Backporting an
arbitrary modern addon to 1.12 currently means find/replace
across the codebase. Tier 1 alone (one afternoon) makes that
unnecessary for the most common 17 entry points.

### Investigation findings — direct AddOnInfo struct reads (deferred)

**Goal:** make Tier 2 convenience wrappers (GetAddOnName/Title/
Notes/Loadable/Security/DoesAddOnExist) read directly from the
engine's per-addon struct instead of dispatching to
`Script_GetAddOnInfo` and discarding 6 of 7 returns. **Status:
deferred** — struct layout is more complex than a clean inline-
string packing, and time-to-derive exceeds the perf upside for
the typical caller.

**What we learned:**

- Addon **array** lives at `[0x00BE1B94]` (pointer to array of
  4-byte pointers) and **count** at `[0x00BE1B90]`. Confirmed
  via `Script_GetAddOnInfo` disassembly: lookup is
  `if (idx >= [0x00BE1B90]) return null; return ((void**)
  [0x00BE1B94])[idx];`.
- Each entry points to a per-addon struct (heap-allocated, sized
  per-addon).
- **Struct fields we identified (relative to struct base):**

| Offset      | Content                                            |
|-------------|---------------------------------------------------|
| `+0x00..+0x0B` | Name buffer — inline, 12 bytes, null-terminated. Both 8-char "Atlas-TW" and 9-char "aux-addon" fit. |
| `+0x0C`     | Heap pointer (Storm allocator addr like `0x29B4D908`) **OR** null. Non-null for Atlas-TW; null for aux-addon. Derefs to non-printable bytes — likely a struct/array, not a direct string. |
| `+0x10..+0x17` | Often zeros across multiple addons. |
| `+0x18`     | Constant `0x909` across all addons probed. Format/version marker? |
| `+0x1C`     | Constant `5` across all addons probed. Schema version? |
| `+0x20+`    | Variable — for shorter addons this is unrelated adjacent heap memory (saw "ItemSync" appear once and "Notes" appear in another session for aux-addon, both clearly not part of aux-addon's data). |

- **Title and Notes are NOT inline-packed** at the start of the
  struct as I initially hypothesized. Atlas-TW's actual title is
  `"Atlas-TW"` (matches name) and notes is the long
  `"Atlas-TW: instance Map Browser..."` string. Neither appears
  in the first 0x60 bytes of the struct except where the inline
  *name* field happens to look like the title.

- **The pointer at `+0x0C`** is the most likely indirection to
  the title/notes data, but it points to a binary blob (non-
  ASCII first bytes), suggesting another layer of indirection
  (struct of pointers, or a tagged section list). Would require
  a second-level scan to chase.

- **Engine helper functions** for field extraction live around
  `0x0051DF00`–`0x0051E0xx` (LookupByIndex, GetTitle, GetNotes,
  etc.), reachable via call-target arithmetic from
  `Script_GetAddOnInfo`. Initial decoding was confused by
  off-by-one arithmetic on my part — the targets land on what
  appear to be mid-instruction addresses, suggesting the
  helpers may use non-standard prologues or are tail-call
  thunks. Worth tracing more carefully.

- **Diagnostic that worked:** `_classicapi_AddonStructDump(idx)`
  with SEH-wrapped pointer deref (the bare pointer-deref
  approach crashes with `0xC0000005` when interpreting inline
  string bytes as pointers — `"s-TW"` happened to fall in
  `0x10000000..0xF0000000` range and AVed). The
  `SafeReadAsciiString` helper using `__try`/`__except` is the
  pattern to keep if we resume this work.

**Recommended next step** if revisiting: extend the diagnostic
to recursively chase the `+0x0C` pointer (read 32 bytes there,
check if any look like further string pointers), and trace
1.12's `Script_GetAddOnMetadata` (`0x0048E530`) which is more
self-contained than `GetAddOnInfo` and might reveal a cleaner
field-access pattern.

Tier 3 (interface version, addon local table, etc.) still
deferred — none of those have a proven 1.12 storage location.

## ~~55. `IsMounted` / `IsStealthed` / `IsFalling` / `IsSwimming`~~ — DONE

Shipped as `src/unit/State.cpp`. Modern globals that 1.12
doesn't bind to Lua, despite the underlying state being tracked
by the engine. All four read directly off the local player
struct or its descriptor — no dispatch:

- `IsMounted()` — checks `UNIT_FIELD_MOUNTDISPLAYID` at
  descriptor `+0x1FC`. Non-zero means the engine is rendering
  a mount.
- `IsStealthed()` — checks bit `0x02` of the player visibility
  byte at descriptor `+0x17C`, AND-gated with `MountDisplayID
  == 0` (mount also sets that bit). May false-positive on
  shapeshift/possess/etc. — untested, fix by walking the
  player's aura list looking for the actual stealth/prowl
  spell if it turns up in practice.
- `IsFalling()` — checks `MOVEFLAG_FALLING | MOVEFLAG_FALLING_FAR`
  (`0x2000 | 0x4000`) on the local player's movement-flags u32
  at `+0x9E8`. This is client-side state (outbound MSG_MOVE_*
  data), not broadcast — only meaningful for the local player.
- `IsSwimming()` — same movement-flags word, `MOVEFLAG_SWIMMING`
  (`0x200000`).

### How we found the offsets

A general-purpose probe system in `src/debug/` (Probe.cpp +
Log.cpp + Log.h) writes labeled descriptor / player-object
snapshots to `<WoW dir>\Logs\classicapi_debug.log` (alongside
the engine's `FrameXML.log`, `Sound.log`, etc.). Path is
resolved at runtime via `GetModuleFileNameA(nullptr)` so the
DLL works against any client install. User runs
`_classicapi_DescLog("baseline", 0, 0x100)` then
`_classicapi_DescLog("mounted", 0, 0x100)` etc.; agent reads
the log file directly and diffs.

Findings:

- **MountDisplayID** found by diffing baseline vs mounted
  descriptor — only `+0x1FC` changed from 0 to a creature
  display ID like `0x4306`.
- **Stealth bit** found by diffing baseline vs stealthed —
  `+0x17C` byte 0 went `0 → 0x02`. Mounted state went
  `0 → 0x1A` (= bits 1, 3, 4). Bit 1 is shared, bits 3+4
  are mount-specific.
- **Movement flags** found by diffing standing/swimming/falling
  player-object dumps. `+0x9E8` went `0 → 0x200000` (swimming)
  and `0 → 0xE001` (falling: bits 0x1 FORWARD + 0x2000 FALLING
  + 0x4000 FALLING_FAR + 0x8000 PENDING_STOP). Bit values
  match documented vanilla `MOVEFLAG_*` constants exactly.

The probe helpers (`_classicapi_DescDump`, `_classicapi_PlayerDump`,
`_classicapi_DescLog`, `_classicapi_PlayerLog`,
`_classicapi_LogClear`, `_classicapi_LogAppend`,
`_classicapi_LogPrintf`, `_classicapi_HexDump`) are kept in
the build as a reusable offset-finding scaffold. They write
labeled blocks to `<WoW dir>\Logs\classicapi_debug.log`,
sharing the location convention engine logs and other
injected DLLs (nampower, etc.) use. Path is computed at
runtime — no hardcoded install location.

The C++ `Debug::Log` API (in `src/debug/Log.h`) backs the
Lua bindings and is available to any module via
`#include "debug/Log.h"`:

- `Debug::Log::Clear()` — truncate the file.
- `Debug::Log::Append(label, content)` — labeled block, content
  written verbatim. Best for snapshot diffs.
- `Debug::Log::Printf(fmt, ...)` — printf-style one-line trace.
  Auto-newlines if the caller didn't include one.
- `Debug::Log::HexDump(label, ptr, len, [base])` — xxd-style
  hex+ASCII dump of a memory range, 16 bytes per row, with
  the offset column showing `base + i`. SEH-wrapped: if a row
  faults (page boundary, freed memory) the dump stops cleanly
  and notes the failed offset rather than crashing the host.

`HexDump` is the most useful for arbitrary-VA inspection — call
it on a struct pointer to scan layouts, or from Lua via
`_classicapi_HexDump(label, addr, len)` to inspect engine
globals without a per-investigation probe module.

## ~~56. `OffhandHasWeapon()`~~ — DONE

Shipped as `src/item/Equipment.cpp`. Resolves the off-hand
equipment slot (slot 17) via `Item::Location::ResolveEquipmentSlot`,
reads the cached `ItemStats_C` record's `m_inventoryType` at
`+0x2C`, and returns true iff it's `INVTYPE_WEAPON` (13) or
`INVTYPE_WEAPONOFFHAND` (22). Shields, holdables (tomes/orbs),
empty slots, and uncached items all return false.

No async load is fired — modern API behavior matches; if the
off-hand item isn't cached the function silently returns false
until something else warms the cache. Typically only matters on
first login before the player's own gear is in the cache, which
in practice is loaded eagerly anyway.

## 57. `IsFlying()` — TESTED, DROPPED

Modern `IsFlying()` returns true when on a flying mount. Vanilla
1.12 has no flying mounts, so the closest semantic mapping is
"on a flightpath" (i.e., `UnitOnTaxi("player") == 1`).

Initial implementation tested `MOVEFLAG_FLYING = 0x01000000` on
the local movement-flags word at `[CGPlayer + 0x9E8]`. The bit
is documented in CMaNGOS as `MOVEMENTFLAG_FLYING`, but verified
empirically that 1.12 does NOT set it during taxi flights —
`/dump IsFlying()` stayed false through an entire flightpath
on which `/dump UnitOnTaxi("player")` correctly returned 1.

Vanilla taxi rides are server-driven splines that animate the
mount/character without flipping a local movement-flag bit;
the FLYING bit only ever gets set on flying mounts (which
don't exist) or possibly GM fly mode (untested). Bit 0x01000000
in vanilla may instead be `MOVEMENTFLAG_SPLINE_ENABLED` per
some references, but if so, that bit also stays clear during
normal taxi.

Conclusion: there is no useful interpretation of `IsFlying()`
in vanilla. `UnitOnTaxi("player")` covers the only flight-like
state the client tracks. Function and `MOVEFLAG_FLYING`
constant both removed; `Offsets.h` carries a comment noting
the empirical result so this doesn't get re-attempted.

## ~~58. `FillLocalizedClassList(table [, isFemale])`~~ — DONE

Shipped as `src/classes/Info.cpp`. Iterates `ChrClasses.dbc`
records and writes `tbl[classToken] = localizedClassName` for
each, returning the same table for chaining.

ChrClasses record layout (derived from `Script_GetSelectedClass`
at `0x004716E0`):

- `+0x14` — `char *Name[9]` localized class names (one per locale)
- `+0x38` — `char *Filename` class token (`"WARRIOR"`, `"MAGE"`, etc.)

Vanilla 1.12 has no separate female-name array — `Name[9]` is
exactly the 36 bytes between `+0x14` and `+0x38` (= 9 locales × 4
byte ptr), with the token immediately after. The modern
`isFemale` arg is accepted for signature parity but ignored;
same names returned regardless. Most locales (English included)
don't differentiate gender forms anyway, so callers won't typically
notice. Sparse classIDs (vanilla skips classID 6 — Death Knight
didn't exist) have NULL records and are silently skipped.

Stack discipline: takes the table at idx 1, drops any extra args
via `SetTop(L, 1)`, then loops `PushString(token); PushString(name);
SetTable(L, 1)` per record. `SetTable` consumes both stack values
so the table sits alone at idx 1 throughout the loop. Returns 1
to push the (same) table back to Lua.

## 59. `C_Item.EquipItemByName` cursor-free path — DEFERRED, partial

Currently shipped in [src/item/Equipment.cpp](src/item/Equipment.cpp)
as a cursor-state guard: refuses to operate when `CursorHasItem()` is
true rather than clobber the held item. Modern WoW's `EquipItemByName`
silently preserves the cursor by routing through an engine-internal
"swap by GUID" helper that doesn't touch cursor state. The 1.12 engine
has the same primitive at the binary level but it's not reachable from
addon space without significant additional reverse-engineering.

### Why we can't just call the obvious helper

A standalone-looking CMSG_SWAP_INV_ITEM (opcode 0x10D) builder exists
at `0x005E0B50` — `int __stdcall(int slot1, int slot2)`, builds a
local Packet, writes opcode + 2 slot bytes, ends with an indirect
call through vtable `0x007FF9E4`. **It's dead code.** Verified two
ways:

- Zero callers in the binary (full xref scan). The function entry
  is intact but nothing reaches it.
- The vtable at `0x007FF9E4` is a memory-buffer vtable, NOT a
  network-packet vtable. `vtable+4 = 0x00417BD0` reads as a "Send"
  by name pattern but its body just calls `SMemFree(buffer)` —
  it's a destructor, not a transmitter.

Calling `0x005E0B50` directly was tested empirically: with that
path active, `EquipItemByName(item, slot)` produced no observable
effect. Consistent with the dead-code analysis — the function
builds a packet in memory and immediately frees it.

### Where the production CMSG_SWAP_INV_ITEM actually goes

Real `0x10D` traffic flows through a different chain:

```
0x501130  gate function (10 callers; reads cursor-state globals)
  ↓
0x468460  generic action dispatcher (1090 total callers, 538 with ECX=0x10)
  ↓
0x464870  thin 2-arg wrapper
  ↓
0x464890  inner (GCC-style stack-aligned prologue, complex body)
```

Two structural traps caught me:

1. **The `edx = 0x0084ECA0` arg looks like a class table.** It's not —
   it's a `__FILE__` debug string (`"E:\build\buildWoW\WoW\Source\Ui\
   QuestFrame.cpp"`). Just metadata for engine logging.
2. **`0x468460` with `ECX=0x10` is called 538 times across the binary
   with 100+ distinct "opcode" values pushed.** It's a generic
   action register, not a packet send. The actual transmission is
   deeper.

### State coupling that prevents direct invocation

`0x501130` reads slot values from globals `[0xBE0810]` (src) and
`[0xBE0814]` (dst). These are populated by upstream cursor machinery
— writes traced to:

- `0x500D86` / `0x500D88` — cleanup path of an "item action" handler
  (writes both globals from struct fields `[ebx+8]` / `[ebx+0xC]`)
- `0x501201` / `0x501207` — internal to `0x501130` itself

To call `0x501130` from outside, we'd need to set up the action-
handler struct AND the globals. Reproducing that state machine
without breaking it is the bottleneck.

### What's needed to finish

A focused RE pass to find the actual `Connection::Send(packet,
size)` (or equivalent) at the bottom of the packet pipeline. Once
that's identified, build CMSG_SWAP_INV_ITEM bytes ourselves:

```
[opcode: u32 0x10D]  (or u16 + size header — verify by sniffing
                      what the engine actually transmits)
[dst_slot: u8]
[src_slot: u8]
```

Linear-slot encoding (already documented in `Offsets.h`):
- `0..18`  paperdoll equipment (head..tabard)
- `19..22` equipped bag containers
- `23..38` backpack contents
- For backpack source from Lua bag/slot: `linearSrc = 22 + slot`
- For dst equipment slot: `linearDst = dstSlot - 1`

Items inside equipped bags (bagID 1..4) need CMSG_SWAP_ITEM (0x10E)
instead — `[srcBag, srcSlot, dstBag, dstSlot]` payload, where
`dstBag = 0xFF` for paperdoll. Add when we have the send path
identified.

### Risk if we get it wrong

Bad opcodes or malformed payloads can disconnect the client mid-
session ("disconnected from server"). Worth packet-sniffing a real
drag-and-drop equip via Wireshark or similar before committing —
don't trust the static analysis alone for protocol details.

### Cross-reference

2.4.3 has a clean engine-internal helper at `0x5E8310`
(`ItemMgr::EquipByGUID` — `__thiscall(this=invMgr, guidLo, guidHi,
dstSlot, 0)`) that bundles the find-and-move cleanly, called from
`Script_EquipItemByName` at `0x4A0D80`. 1.12's equivalent (if it
exists at all as a single function) hasn't surfaced through xref or
pattern search. The 2.4.3 disassembly is in the conversation
history; useful as a reference for what the bundled call looked
like before Blizzard refactored.

### Status

Cursor-guard implementation ships. Tested with hearthstone-on-
cursor + `EquipItemByName("Robes of Antiquity", 5)` — refuses
gracefully, cursor preserved. To work around: caller drops the
cursor first (`ClearCursor()` or `PutItemInBag(0)`), then calls
`EquipItemByName`.

## 60. `GameTooltip:AddSpellByID(spellID)` — easy/medium

Modern method that **appends** spell info to the current tooltip
without clearing existing lines — natural pair to `SetSpellByID`,
which clears + rebuilds. Lets callers compose tooltips like
"talent name (line 1) + AddSpellByID(rankSpell) (rest)".

Direct upgrade for our cross-class `GameTooltip:SetTalentByID`
fallback (#43): the current tier-2 path renders just the spell
tooltip, losing the talent name. With `AddSpellByID` we could do
`ClearLines + AddLine(talentName) + AddDoubleLine("Rank 0/N", "")
+ AddSpellByID(spellID)` to match modern's richer cross-class
tooltip exactly.

Implementation question: 1.12's `BuildSpellTooltip` (the inner
helper at `0x0052E610` we use for `SetSpellByID`) almost certainly
clears the buffer first. We'd need a different entry point that
appends, OR find where the line-emission helper sits inside
`BuildSpellTooltip` and call that directly.

Approach: disassemble `Script_GameTooltip_SetSpell` (`0x00532D10`)
and `BuildSpellTooltip` to identify the "ClearLines" prologue and
the "emit description / cost / range / cooldown" loop. If those
are factored apart, the loop is callable standalone and `AddSpellByID`
is a thin wrapper. If they're inlined, we'd either need to skip
the clear ourselves (call after some no-op state setup) or
manually call the line-emission primitives.

## 61. `GameTooltip:GetItem()` / `GetSpell()` / `GetUnit()` — easy if reachable

Modern query methods that return what the tooltip is currently
showing:

- `GetItem()` → `(name, link)`
- `GetSpell()` → `(name, rank, spellID)`
- `GetUnit()` → `(name, unitToken)`

1.12 has `SetUnit` (slot 31), `SetSpell` (slot 18), and various
`SetInventoryItem`/`SetBagItem`/`SetMerchantItem` (slots 19, 30,
27) but no `Get*` counterparts. Modern addons that scrape tooltip
state — rank parsers, link extractors, debug overlays — currently
have to wrap every `SetX` call in their own state tracking.

The engine almost certainly maintains "what am I currently
displaying?" fields on the GameTooltip frame for its own redraw
logic. Finding those fields is the work; once located, each `Get*`
is a one-line read.

Investigation steps:
1. Set a known item via `SetItemByID(6948)` (Hearthstone)
2. Search the GameTooltip frame's instance memory for the value
   `6948` after the call
3. Repeat with `SetSpellByID(133)` (Fireball) — find spellID slot
4. Repeat with `SetUnit("player")` — find the unit-token or GUID
   slot

Each of these will reveal an offset on the frame's instance struct
that holds the current state. Symmetric for the others.

`GetItem()` returns a link — slightly more involved since we'd
need to format the item link string (we have `Item::Tooltip` for
this pattern).

## 62. `GameTooltip:NumLines()` — trivial if reachable

Returns the count of FontStrings currently rendered in the
tooltip. The frame internally maintains a list of lines (it has
to, for layout). If there's a count field, this is a single
deref + push.

Investigation: dump the GameTooltip frame instance after
`AddLine` calls and look for an integer that increments. Probably
near the FontStrings array pointer.

Useful for tooltip-parsing addons that want to walk all lines
without iterating until `_GetTooltipLine(i)` returns nil.

## ~~65. `hooksecurefunc(name|table, [name,] callback)`~~ — DONE

Shipped as [src/HookSecureFunc.cpp](src/HookSecureFunc.cpp). Pure C
implementation using a `lua_pushcclosure` with two upvalues
(`orig`, `callback`). The wrapper calls orig with `LUA_MULTRET`,
then callback (returns discarded), then returns orig's full result
list with no count cap.

### Why pure C, not embedded Lua

Originally drafted as a Lua source string executed via
`FrameScript_Execute` at boot. Reworked to pure C to keep all
implementation in one language and avoid embedded source strings;
the refactor added three Lua C API offsets we hadn't wrapped before
(`lua_gettop` at `0x6F3070`, `lua_remove`, `lua_call` at
`0x6F4180`), plus the `LUA_UPVALUEINDEX(i) = GLOBALS_INDEX - i`
helper.

### Lua API offset corrections discovered along the way

While diagnosing why `hooksecurefunc("GetSpellInfo", fn)` was returning
"target field is not a function", a series of `_classicapi_*Probe`
helpers traced the failure to **three misidentified offsets** in
`docs/LuaCAPI.md` (and `Offsets.h`):

| VA | Doc'd as | Actually is |
|----|----------|-------------|
| `0x006F30D0` | `lua_pushvalue` | **`lua_remove`** (shift-down loop + decr_top) |
| `0x006F32B0` | `lua_remove`    | **`lua_replace`** (TValue copy + decr_top)   |
| `0x006F3350` | `lua_replace`   | **`lua_pushvalue`** (TValue copy → top + incr_top) |

How we caught it: `_classicapi_PushValueProbe` showed `GetTop` going
from `1 → 0` after `PushValue(L, 1)` — the supposed `lua_pushvalue`
was actually popping (lua_remove behavior). Disassembly of `0x6F30D0`
confirmed the shift-down loop + `add [ecx+8], -0x10`, vs `0x6F3350`
which has the standard `add [ecx+8], 0x10` incr_top after a single
TValue copy.

`Offsets.h` and `docs/LuaCAPI.md` both updated. The misidentified
offsets had no observable effect previously because no code in the
project used `PushValue` outside `HookSecureFunc.cpp` — every
existing call path uses `PushString`/`PushNumber` for fresh pushes
and `SetTable`/`GetTable` for stack manipulation.

### Why 1.12 needs this even without taint

Modern's "secure" label is about taint propagation in 3.x+ protected
frames. 1.12 has no taint system; the function is functionally a
plain after-hook with return preservation. We ship it for **modern
API parity** — addons being backported from later expansions
frequently use `hooksecurefunc(GameTooltip, "SetInventoryItem", ...)`
and similar patterns, expecting the original to run first and their
callback to fire afterward. Both forms (global by name, table+method)
are supported.

### Unhookable globals

Modern WoW rejects `hooksecurefunc("<core lua/secure fn>", ...)` outright
to keep callers from clobbering language primitives or breaking taint
propagation. We mirror that with a name blacklist in `Script_HookSecureFunc`
that fires `lua_error` with `"hooksecurefunc: function is unhookable"`
when the two-arg form targets one of: `getfenv`, `getmetatable`,
`hooksecurefunc`, `ipairs`, `issecurevalue`, `issecurevariable`, `next`,
`pairs`, `pcall`, `pcallwithenv`, `rawget`, `rawset`, `scrub`,
`securecall`, `securecallfunction`, `secureexecuterange`, `select`,
`setfenv`, `setmetatable`, `type`, `unpack`, `wipe`, `xpcall`.

Three-arg form (table + name + callback) is exempt: hooking
`MyLib.pairs` is fine even if `_G.pairs` is not — the blacklist is
specifically about replacing functions on the globals table. A user who
passes `_G` explicitly as the table target is opting in deliberately.

### Cross-version reference

2.4.3 has the strings `hooksecurefunc`/`securecall`/`issecure`
clustered together at `FO=0x4D2CF8`/`0x4D2D08`/`0x4D2D28` in `.rdata`,
suggesting they're a "secure environment helpers" string table for
the engine's taint-propagation code. The `hooksecurefunc` Lua
implementation itself is in `FrameXML/RestrictedFrames.lua` even in
modern WoW — the engine never had an internal C version we could
mirror.

## ~~63. `GameTooltip:SetInventoryItemByID(itemID)`~~ — DONE

Shipped in [src/item/Tooltip.cpp](src/item/Tooltip.cpp).
Confirmed empirically distinct from `SetItemByID`: with run-speed
enchanted boots equipped, `SetInventoryItemByID` shows the
enchant line; `SetItemByID` shows the base item only.

Implementation walks character-pane slots 1..19 looking for an
equipped item matching `itemID`; on hit, dispatches to the
engine's existing `Script_GameTooltip_SetInventoryItem` (registry
slot 19, `0x00532EE0`) with `("player", slot)` rewritten onto the
Lua stack. Silent no-op when the item isn't currently equipped —
caller falls back to `SetItemByID` for unworn items.

### Duplicate-item slot ordering — verified

When the player has duplicates of the same itemID equipped, modern
client renders the **lower-numbered slot** first: MAINHAND (16)
before OFFHAND (17), FINGER1 (11) before FINGER2 (12), TRINKET1
(13) before TRINKET2 (14). Our ascending 1..19 walk + first-match-
break naturally produces the same ordering — lucky, since the
INVSLOT constants happen to be ordered with the "primary" slot
numbered lower than its peer. Locked in with a code comment so
the loop direction doesn't accidentally drift in future refactors.
## 64. `OnTooltipSetItem` / `OnTooltipSetSpell` / `OnTooltipSetUnit` script handlers — DEFERRED, scoped

Modern WoW fires `OnTooltipSetItem` / `Spell` / `Unit` script handlers
on `GameTooltip` after each `SetX` finishes filling the tooltip. The
canonical addon pattern:

```lua
GameTooltip:HookScript("OnTooltipSetItem", function(self)
    local _, link = self:GetItem()  -- depends on TODO #61
    -- augment tooltip
end)
```

1.12 has the tooltip script-handler infrastructure but only fires
**three** specific names: `OnTooltipAddMoney`, `OnTooltipCleared`,
`OnTooltipSetDefaultAnchor`. The modern set is missing.

### Architecture (verified)

- 1.12 has **`GetScript` (`0x00774780`) and `SetScript` (`0x007748D0`)**
  natively, but **no native `HookScript`**. Addons polyfill `HookScript`
  Lua-side — confirmed via:
  - `!!!ClassicAPI/api/api.lua:458`
  - `pfUI/modules/eqcompare.lua:224`

  Both polyfills compose through `GetScript` + `SetScript`, so
  hooking those two makes every existing `HookScript` polyfill in
  the wild work for our new names. We don't need to write one
  ourselves.

- The script-name → handler-storage mapping lives in a virtual
  method on `GameTooltip` (resolved via `vtable[?]`), pointed at by
  one entry in `.rdata` at `FO=0x408F6C` → matcher function at
  `0x005295D0`. The matcher's body:

  ```
  push 0x7FFFFFFF; push "OnTooltipSetDefaultAnchor"; push scriptName
  call SStrCmpI → if match: lea eax, [frame+0x444]; ret
  push 0x7FFFFFFF; push "OnTooltipCleared"; push scriptName
  call SStrCmpI → if match: lea eax, [frame+0x44C]; ret
  push 0x7FFFFFFF; push "OnTooltipAddMoney"; push scriptName
  call SStrCmpI → if match: lea eax, [frame+0x454]; ret
  xor eax, eax; ret  // unknown name
  ```

  Frame storage layout: 8 bytes per script handler slot (probably
  `{lua_func_ref, flags}`).

- `GetScript`'s "doesn't have a 'X' script" error fires when this
  matcher returns 0 AND the name isn't in the standard frame
  scripts (OnEvent, OnEnter, etc.).

### Implementation plan

**Phase 1 — Script handler intercept** (~half-day)

- MinHook on `Script_GetScript` (`0x00774780`) and
  `Script_SetScript` (`0x007748D0`). Both are real functions
  (already addressable, not just registry slots).
- For each:
  1. Read scriptName from Lua stack.
  2. If name ∈ `{"OnTooltipSetItem", "OnTooltipSetSpell", "OnTooltipSetUnit"}`:
     - Validate stack[1] is a frame.
     - For Set: store handler in our registry table.
     - For Get: push from our registry table or `nil`.
     - Return without invoking original.
  3. Else: tail-call original.
- Storage: Lua registry table (allocated once at boot, kept alive
  via a known registry index). Keys: `frame_va * "/" * scriptName`.
  Handler lookup is `lua_gettable(L, our_table_idx)`.
- Lua ref management: 5.0 uses `lua_ref`/`lua_getref`/`lua_unref`.
  Need to find these in the binary (probably near the existing
  Lua C API offsets at `0x6F3xxx`); else use a pure Lua-table
  approach (set `our_table[key] = function`, retrieve with
  `lua_gettable`).

**Phase 2 — Fire from our existing entry points** (~1 hour)

After `Spell::Tooltip::ShowByID` and `Item::Tooltip::Set*ByID`,
call a `FireTooltipScript(L, frame, "OnTooltipSetItem")` helper
that does `lua_gettable` for the handler and `lua_pcall(L, 1, 0)`
with `frame` as arg.

This gives a working but incomplete OnTooltipSetItem — fires for
our two ByID functions but not for the engine's
`SetInventoryItem`/`SetBagItem`/etc.

**Phase 3 — Cover engine `Set*Item` functions** (~2-4 hours)

Either hook each individually (~19 `Script_*Item` functions in 1.12,
list in `docs/raw_methods.txt`) or find a unified inner builder:

- Item: no unified `BuildItemTooltip` apparent — each
  `Script_GameTooltip_Set*Item` builds independently. Hooking each
  via MinHook is straightforward but adds 19 hook points.
  Alternative: find the deeper inner helper they all converge on.
  The Set*Item functions all have similar prologues; trace one
  thoroughly to find the shared call.
- Spell: **`BuildSpellTooltip` at `0x0052E610`** is the unified
  builder (we already use it from `SetSpellByID`). Single hook.
- Unit: find inner of `Script_GameTooltip_SetUnit`
  (slot 31, `0x005349B0`).

**Phase 4 — GetItem/GetSpell/GetUnit** (TODO #61, related)

For handlers to be useful, addons typically need to know *what*
was set. Find frame state offsets where the engine stores the
current item/spell/unit. Approach:

1. Set known value via `SetItemByID(6948)` / `SetSpellByID(133)` /
   `SetUnit("player")`.
2. Dump GameTooltip frame instance memory.
3. Search for `6948` / `133` / player GUID.
4. Each match reveals the offset for that state field.

Once located, `GetItem()`, `GetSpell()`, `GetUnit()` are trivial
deref+push. Implementation roughly the same as `OffhandHasWeapon`
or `IsEquippedItem` patterns.

### Risks

- **Hot-path hooks:** `GetScript` and `SetScript` are called by
  every frame for every script registration. Mistakes in our hooks
  can break the entire UI. Test extensively before shipping.
- **Lua ref leaks:** if a frame is destroyed while we hold a ref
  to its handler, we leak. Need a "frame destroyed" hook (or
  rely on garbage collection of a weak-keyed table).
- **Engine internals path:** finding the unified item builder
  (Phase 3) might require deep RE; falling back to 19 individual
  hooks is safe but ugly.

### Alternative deferred design — Option B (in case Phase 1 turns
out unworkable in practice)

Skip the SetScript/GetScript intercept entirely. Provide:

```lua
GameTooltip.OnTooltipSetItem = function(self) ... end
```

(field-assignment, no HookScript polyfill needed). We fire from
all the same places as Phase 2-3, but instead of `lua_gettable`
on our registry table, we do `lua_getfield(frame, "OnTooltipSetItem")`
directly off the frame's Lua table. Simpler, but doesn't compose
with `HookScript` polyfills automatically (each addon would
clobber the previous).

## ~~66. Left/right modifier keys + `MODIFIER_STATE_CHANGED` event~~ — DONE

Shipped in [src/input/Modifier.cpp](src/input/Modifier.cpp). Adds
the seven query functions (`IsLeftShiftKeyDown`, `IsRightShiftKeyDown`,
`IsLeftControlKeyDown`, `IsRightControlKeyDown`, `IsLeftAltKeyDown`,
`IsRightAltKeyDown`, `IsModifierKeyDown`) plus the 2.4.3+
`MODIFIER_STATE_CHANGED(key, down)` event. All seven match the
modern WoW return shape (`1` for down, `nil` for not-down).

### Why this needed a Win32 hook

The engine has **no L/R distinction anywhere in its state**. 1.12's
own `IsShiftKeyDown` chain bottoms out at `GetKeyState(VK_SHIFT)` —
the merged virtual key `0x10` — at `0x00436024`, never the L/R-aware
`VK_LSHIFT` / `VK_RSHIFT` (`0xA0` / `0xA1`). Helper3 at `0x00436D80`
returns a struct with only 3 modifier slots (shift/ctrl/alt), confirmed
by inspection. No amount of digging in engine memory finds L/R state
because **it never existed there**.

There IS a 6-bit field at `[*[0x00C0ED38] + 0x269C]` that looks
structurally identical to 2.4.3's L/R modifier bitmask at `[+0x130]`
— same shift-and-mask helper at `0x005936C0`, same 6-bit dispatch
limit. We chased it for a while. Empirically the field is something
else entirely (some kind of input-device-flags bitmask) — it stays at
`0x3BF` regardless of which modifier is held. The 2.4.3 → 1.12
structural similarity is misleading; the field's meaning evolved.

The OS-level keystate **does** have the L/R distinction (VK_LSHIFT /
VK_RSHIFT etc.), and 1.12 already uses Win32 `GetKeyState` for its own
modifier checks — so calling `GetAsyncKeyState(VK_LSHIFT)` ourselves
is the same primitive the engine already uses, just with the L/R-aware
VK codes.

### Why a `WH_GETMESSAGE` hook over `SetWindowLongPtr` subclass

We initially subclassed WoW's `WNDPROC` (via `EnumWindows` to find the
`GxWindowClassD3d`/`GxWindowClassOpenGl` HWND, then `SetWindowLongPtrA`
to install a wrapper). Worked for L/R queries and the event. Then the
user toggled vsync and the event went silent — WoW destroys and
recreates its main window on some renderer state changes, so the
subclass was left dangling on a destroyed HWND.

Switched to `SetWindowsHookExA(WH_GETMESSAGE, ..., GetCurrentThreadId())`.
The hook is per-thread, not per-HWND, so it survives any number of
window recreations. Decodes `WM_KEY{,SYS}{DOWN,UP}` messages,
maintains a 6-bit cached bitmap that the queries read, and fires
`MODIFIER_STATE_CHANGED` on transitions.

### L/R decoding from WM_KEY messages

- `VK_SHIFT` (the merged virtual key the OS delivers for either key)
  → `MapVirtualKeyA(scancode, MAPVK_VSC_TO_VK_EX)` returns
  `VK_LSHIFT` or `VK_RSHIFT` based on the hardware scancode.
- `VK_CONTROL` / `VK_MENU` → the `KF_EXTENDED` flag (bit 24) of
  `lParam` discriminates: extended = right, plain = left.
- `VK_L*` / `VK_R*` delivered directly (some keyboards, SendInput) →
  accepted as-is.

### `MODIFIER_STATE_CHANGED` and the custom-event saga

Documented as part of the broader event-table investigation in
CLAUDE.md's "Events" section. Short version: we tried three approaches
to inject custom event names into the engine's event table.

**Approach A — slot-claim (original Event::Custom)**: walk the live
table for NULL slots, write our `SMemAlloc`+memcpy'd name. Works.
Downside: first NULL slots were near the head of the table (slot 1
got `MINIMAP_BLIP_TRACKING_CHANGED` from VanillaMinimapTracking,
which felt off).

**Approach B — rebuild-hook + tail-injection**: hook
`RebuildEventTable` at `0x00703D90`, pad the input array to 1024
entries, place our names at the tail (slots 1020+). Engine's own
SStrDup handles allocation, so we never write to engine memory.
Worked for a single DLL. **Crashed when SuperWoWhook, nampower,
transmogfix, and VanillaMinimapTracking all hooked the same
function**: their layered modifications to `(names, count)` produced
a count→buffer-size mismatch, and the engine's fill loop wrote off
the end of the allocated buffer on its last iteration.

**Approach C — slot-claim + backward walk (final)**: same as A but
walk the table from `count-1` downward looking for NULL slots, so
high indices are claimed first. Each DLL works on the table
independently — no hook chaining, no argument modification, no
crashes. Custom events naturally group at the tail (verified: slots
695..699 with all five custom events when ClassicAPI +
VanillaMinimapTracking are both loaded). Also swapped
`SMemAlloc + memcpy` for the engine's own `SStrDup` at `0x0064A620`
— cleaner allocator, exactly matches what the engine's bulk
rebuild does.

The intermediate rebuild-hook approach also uncovered the engine's
**hardcoded post-rebuild slot writes**: the bulk rebuild populates
slots 0..548 from its `0x00BE1198` name array, then engine code
writes specific events to hardcoded slot indices in the 549..699
range (`SPELL_DAMAGE_EVENT_SELF` at slot 549,
`SPELL_DAMAGE_EVENT_OTHER` at 550, etc.). Documented in CLAUDE.md.
This is why claiming slots from low indices early-on would have
gotten clobbered — the slot-claim approach only fires after Lua's
first `RegisterEvent` call, by which point the engine's writes have
settled.

## 67. `BAG_UPDATE_DELAYED` event - ATTEMPTED, REVERTED

Modern 5.4.8+ event that fires once per game frame in which any
`BAG_UPDATE` fired. The 5.4.8 engine sets an
`m_bagUpdateDelayedPending` flag from the bag-change pipeline and
drains it from `CContainerMgr::Update` (its per-frame routine),
producing exactly-one coalesced fire per frame.

### What we tried

A pure C++ hook chain:

- Naked thunk over `FUN_FIRE_EVENT` (`0x00703F50`) — peek at the
  event ID, set a pending flag on match against the cached
  `BAG_UPDATE` slot, `jmp` to the trampoline so the variadic stack
  passes through untouched.
- Normal `__thiscall`-style hook over the per-frame OnUpdate
  dispatcher at `0x00772890` — drain the pending flag and fire
  `BAG_UPDATE_DELAYED` once per frame.

Reservation via the existing `Event::Custom::AutoReserve` slot-claim
machinery. The DLL compiled cleanly and registered the event;
`IsEventValid("BAG_UPDATE_DELAYED")` returned `true`.

### Why we reverted

The DLL crashed during the login-screen XML load — `EIP=0x15FF075B`
inside MinHook trampoline space, with zero bytes at that address,
`ESP=0x001AEFEB` (misaligned), and stack ret-addr `0x00772954` inside
the patched function. Classic signature of a corrupted MinHook
trampoline.

The user's Octo install loads SuperWoWhook, nampower, transmogfix,
UnitXP_SP3, VanillaHelpers, VanillaMinimapTracking, and
VanillaMultiMonitorFix alongside ours. Each links its own MinHook
copy. Hooking high-traffic engine functions like the event dispatcher
or per-frame OnUpdate sits squarely in the path where trampoline
regions can collide between DLLs — same class of problem as the
`RebuildEventTable` hook-chaining failure we already documented in
CLAUDE.md.

We removed `src/bag/UpdateDelayed.{cpp,h}`, the two `HOOK_FUNCTION`
calls in `DllMain.cpp`, and `FUN_FRAMESCRIPT_FRAME_ONUPDATE` from
`Offsets.h`.

### Earlier dead end: Lua-source bootstrap from C++

Before the hook attempt, I shipped a version that used
`FUN_FRAME_SCRIPT_EXECUTE` (`0x00704CD0`) to inject a Lua snippet
creating a hidden frame with `RegisterEvent("BAG_UPDATE")` /
`SetScript("OnUpdate", ...)`. The user pushed back: embedding Lua
source in the DLL is architecturally wrong for this project. The
right answer is always a C++ engine hook. `FUN_FRAME_SCRIPT_EXECUTE`
is gone from the codebase; pretend it doesn't exist. (See the
saved feedback memory.)

### Where to go next

Three possible non-conflicting paths:

1. **Find an obscure single-purpose engine function** that other
   DLLs demonstrably don't touch — call sites of `FUN_FIRE_EVENT`
   from the specific bag-update code path (e.g., the
   `Script_GetBagName` / `Script_GetContainerItemInfo` neighborhood
   in `0x004F8xxx`-`0x004FAxxx`), or whatever fires BAG_UPDATE from
   network packet processing. A hook there avoids the popular targets.

2. **Read-only mechanism** — poll a global that tracks bag dirty
   state. The engine maintains `[ItemMgr + 0x10]` "bank-aware mode"
   and similar flags around the inventory manager; there may be a
   dirty/sequence counter we can read once per frame from a
   non-hooked surface (e.g., piggybacking on a tick that the rest of
   our code already runs in). No new hook, no collision risk.

3. **Document the limitation** and skip the polyfill — addons that
   want this behavior on Octo-style multi-DLL clients can fall back
   to debouncing `BAG_UPDATE` themselves via an `OnUpdate` timer.
   Ship-quality, but admits defeat.

Prefer #2 if a read-only signal exists; #1 only if a verifiably
quiet hook target turns up; #3 as the honest fallback.

## 68. `UnitChannelInfo(unit)` / `UnitCastingInfo(unit)` — medium

Modern signatures return `(name, _, texture, startMs, endMs, _, _,
spellID)` for the unit's currently channeled / cast spell. 1.12
exposes nothing equivalent — the closest is `SPELLCAST_CHANNEL_*`
events, which only fire for the local player and don't carry
spellID payloads.

The data is sitting in the broadcast descriptor:

- `UNIT_FIELD_CHANNEL_SPELL` at descriptor `+0x228` (already in
  `Offsets.h` as `OFF_UNIT_FIELD_CHANNEL_SPELL`). Verified via the
  warlock/clicker/fishing diffs in the `IsAssistingRitual` session
  — non-zero = currently channeling that spell.
- `UNIT_FIELD_CHANNEL_OBJECT` at descriptor `+0x38/+0x3C`
  (`OFF_UNIT_FIELD_CHANNEL_OBJECT`). 64-bit GUID of the channel
  target — bobber for fishing, lock for lockpicking, etc. Zero for
  channels that don't bind to a GameObject (warlock spells, Ritual
  of Summoning).

That gives us spellID immediately, and `name` + `texture` via the
existing `GetSpellInfo(spellID)` chain. Worth shipping a partial
function that omits the `startMs`/`endMs` returns and let callers
do their own duration estimation off `SPELLCAST_CHANNEL_START` —
or hunt for the per-unit cast-time fields the engine uses for the
local castbar.

What's missing for the full modern signature: the start/end-time
fields. The local player has `Script_SpellStopCasting`-adjacent
state at fixed VAs (`m_castingSpellId`, `m_castStartMs`,
`m_castEndMs`) — but for *other* units we'd need to either find
broadcast UpdateFields holding the same, or compute end-time on
the client from `Spell.dbc`'s cast time when we see a fresh
UNIT_FIELD_CHANNEL_SPELL value. The 5.4.8/3.3.5 binaries are the
right next stop for this.

Companion field to investigate while we're in here: `desc 0x1320`
captures the *prior* channeled spellID after the channel ends —
see entry #71 for what that's about.

## 69. `IsGathering()` — easy (local player only)

No-arg local-player boolean that fires while the player is
mid-gather: herb pick, ore mine, fishing-bobber-loot, lockpicking,
skinning. Modern WoW doesn't expose this exact concept but addons
that care (auto-attack suppression, mouse-look reversion, autorun
toggles) reverse-engineer it from `LOOT_OPENED` /
`UNIT_SPELLCAST_*` events. A direct read is cleaner.

State machine from the `IsAssistingRitual` session, verified by
diffing the player while gathering:

- `[player + 0x1D28]` / `[player + 0x1D2C]` — 64-bit GUID of the
  GameObject the player is gathering from. `0xF110xxxx` high half
  = vanilla GameObject prefix. Set when the gather animation
  starts, cleared when it ends or breaks.
- `[player + 0xD48]` — small int, `0` baseline, `1` during gather.
  Confirmed for herb gather; mining sets the same field. Cleanest
  single-bit signal we have.
- `UNIT_FIELD_FLAGS` (descriptor `+0xA0`) bit `0x1000` is cleared
  during gather — unclear what the bit semantically means (other
  emulator sources don't agree on this bit's name in vanilla), but
  it's another way to detect the state.

Three implementation options worth picking between:

1. **`IsGathering()`** boolean, `[player+0xD48] != 0`. One line.
2. **`GatheringObjectGUID()`** returning `(lo, hi)` from
   `[player+0x1D28]/+0x1D2C`. Lets addons distinguish herb-from-
   herb / chest-from-chest. Modest extra surface.
3. **Family of `IsHerbing()` / `IsMining()` / `IsFishing()`** —
   requires looking up the GameObject's type/sub-class from its
   GUID, which means walking the engine's GO cache. Significantly
   more work; defer unless someone has a use case.

Fishing is the trickiest case — `/cast Fishing` is a regular spell
cast (sets UNIT_CHANNEL_SPELL = 7620), the bobber sits in water
during a separate wait phase (UNIT_CHANNEL_OBJECT = bobber GUID),
then the right-click bobber action triggers the gather/loot
machinery (sets `[player+0x1D28]/+0x1D2C` to the bobber GUID, but
NOT `0xD48`). So `[player+0xD48]` is "gather-cast active"
(herbing/mining), and `[player+0x1D28]` is the broader "engaged
with a lootable GO" pointer. Pick which semantic you want before
naming.

## 70. `UnitStandState(unit)` — easy

Modern `GetUnitStandStateValue(unit)` or `UnitStandState(unit)` (the
name varies by expansion) returns an integer describing whether
the unit is standing / sitting / sleeping / kneeling. Vanilla 1.12
exposes only `IsSitOrStanding()` (local boolean) and lacks any
"is target sitting" check.

Verified directly via the `IsAssistingRitual` session: descriptor
`+0x210` is `UNIT_BYTES_1` (CMaNGOS field 132 in the 1.12.1
layout). The standstate is the **low byte** of that u32. Observed
transition during a `/sit` on a chair: `0xEE00 → 0xEE05`,
i.e. low byte 0 → 5 (`STANDSTATE_SIT_MEDIUM_CHAIR`).

Vanilla standstate enum (from emulator sources, low byte of
`UNIT_BYTES_1`):

| Value | Meaning             |
|------:|---------------------|
| 0     | STANDING            |
| 1     | SITTING             |
| 2     | SITTING_CHAIR       |
| 3     | SLEEPING            |
| 4     | SITTING_LOW_CHAIR   |
| 5     | SITTING_MEDIUM_CHAIR|
| 6     | SITTING_HIGH_CHAIR  |
| 7     | DEAD                |
| 8     | KNEELING            |

Implementation: resolve unit → descriptor → `[+0x210] & 0xFF`.
Broadcast field, so it works for any synced unit (player, target,
party/raid, inspect targets) — not local-only.

While we're documenting `UNIT_BYTES_1`, the other bytes of the same
u32 are worth deriving when needed (CMaNGOS docs say byte 1 =
`PetTalents`, byte 2 = `VisFlag`, byte 3 = `AnimTier`), but none
have an obvious modern-API hook in vanilla.

## 71. Descriptor `+0x1320` — "last channeled spell" parking lot

During the `IsAssistingRitual` investigation, the player's
descriptor showed `+0x1320` flip from `0` to `0x2BA` (= 698 =
Ritual of Summoning) **immediately after** the channel ended,
where the active channel was at `+0x228`. So `+0x228` =
UNIT_CHANNEL_SPELL (live), and `+0x1320` looks like a "previously
channeled" stash that's populated on transition-to-zero.

Not enough data to commit to a semantic yet. Open questions:

- Does it also stash for fishing / lockpicking / mining channels?
  We have one data point (the portal click) showing the behavior.
- Does it clear, and if so when? (`/reload`? Next channel? Never?)
- Is it a broadcast UpdateField or local-only?

If it turns out to be a stable "last completed channel" field, it
unlocks polyfills for the modern `UNIT_SPELLCAST_CHANNEL_STOP`
event payload (which carries the spellID that just ended) without
hooking the channel-stop code path. Lower-priority; useful but
not load-bearing for any specific API in the polyfill addon
today.

Investigation plan when picked up: snap `+0x1320` before any
channeled action, perform a clean channel, snap again immediately
after channel end (use `_classicapi_DescDump(0x1320, 4)` for a
targeted read). Repeat across several channel types and confirm
the pattern.
