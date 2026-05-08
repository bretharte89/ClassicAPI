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

## 23. `GetItemSpell(item)` — easy

Returns the on-use spell name + spellID for items that have one
(potions, trinkets, scrolls). Reads ItemStats fields that we haven't
mapped yet — the `m_spellId[5]` array on the cached item record.
Combine the spellID with `GetSpellInfo` for the name.

## 24. `GetInventoryItemDurability(unit, slot)` — easy

Returns `(current, max)` durability for an equipped item. Reads
ITEM_FIELD_DURABILITY and ITEM_FIELD_MAXDURABILITY off the CGItem
descriptor at `+0x114` — same descriptor we read FLAGS from in
`C_Item.IsBound`. Just two more dword reads at known offsets.

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

## 28. `GetItemCount(item, [includeBank], [includeCharges])` — easy

Total count of an item across the player's bags. Walks all bag slots
(backpack 0 + bags 1..4) using `Item::Location::ResolveBag` and counts
matching itemIDs, accumulating `ITEM_FIELD_STACK_COUNT` from each
CGItem's descriptor. The `includeBank` flag adds bank bag slots
(20..23 plus 28..63 for the main bank slots); `includeCharges` is
charges-aware for items with multiple uses (rare in 1.12 — e.g.,
mana stones).

## 29. `GetItemFamily(item)` — trivial

Bag-family bitmask (e.g. soul shards = `0x100`, herbs = `0x800`). One
field read on the cached ItemStats record. Used by addons that
auto-route loot to specialty bags. Field offset isn't yet mapped in
`Offsets.h` — small tracing job inside `Script_GetItemFamily` (which
has the same offset for both 1.12 and 3.3.5 since the field
predates Wrath).

## 30. `IsSpellKnown(spellID)` — trivial

Boolean: does the player have this spell? Reuse the existing
`Spell::Lookup::FindSpellbookSlot` and check that the resulting
bookType is `0` (player). Three-line wrapper.

Modern WoW also supports an `isPet` second arg for pet-spell checks;
we'd take the same shape and route to bookType `1`.

## 31. Unit flag bundle: `UnitIsAFK` / `UnitIsDND` / `UnitIsFeignDeath` — trivial

Three single-bit checks on UNIT_FIELD_FLAGS / PLAYER_FLAGS, mechanically
identical to `InCombatLockdown` (item #21):

- `UnitIsAFK(unit)` — PLAYER_FLAGS bit (`0x02` per the protocol).
  PLAYER_FLAGS lives at a different field offset than UNIT_FLAGS
  (likely `+0xE8` of m_objectFields, but needs verification).
- `UnitIsDND(unit)` — PLAYER_FLAGS bit `0x04`.
- `UnitIsFeignDeath(unit)` — UNIT_FIELD_FLAGS bit `0x10000000` (bit 28),
  same field as the combat flag.

Could ship as one file `src/unit/Flags.cpp` with all three (they
share the field-resolve path). The PLAYER_FLAGS offset is the only
new data to derive — UNIT_FIELD_FLAGS is already known from
`Item::InventoryID`'s `UnitPlayerControlled` work.

## 32. `IsCurrentSpell(spellID)` — easy

Boolean: is the player currently casting / channeling this spell?
Needs a runtime "current cast spellID" global the engine maintains
for the spell-button UI. Likely lives near the cast bar state — a
short trace through `Script_GetCurrentCastingInfo`-style functions
(if any exist) or the cast-bar timer logic. Worth doing because
modern action-bar addons gate "highlight the active button" on this.

## 33. `C_Reputation.*` cluster — easy

Modern table-shape and ID-based variants of the existing reputation
API. Most are thin wrappers around what we already have — useful for
addons backporting modern Lua-side code that prefers the C_Reputation
namespace.

- **`C_Reputation.SetWatchedFactionByID(factionID)`** — walks the
  faction-index resolver to find the index for `factionID`, then calls
  the engine's `Script_SetWatchedFactionIndex`. Same approach as our
  `GetFactionInfoByID`. Highest-value of the cluster — RepBuddy
  explicitly takes the fast path here when available
  (`Util/RepBuddy.lua:286`).
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
  `GetWatchedFactionInfo` (which 1.12 has natively).

All five share the same faction-index resolver
(`FUN_RESOLVE_FACTION_INDEX`) and walk pattern we already use in
`src/faction/Info.cpp`. Could land as a single `src/faction/CInfo.cpp`
adding the C_Reputation registrations.

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
