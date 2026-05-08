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

## 5. `GetServerTime()` — trivial

Lua addon returns `GetTime()` (frame-relative seconds-since-login). Useless
for anything that needs wall-clock seconds (calendar, cooldown sync, log
timestamps). The client receives time-sync packets and tracks a real server
epoch internally — find that global and expose it.

Reference: `Util/API.lua:144-146`.

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

## 16. `GetContainerItemID(bagID, slotIndex)` — trivial

Modern positional accessor for the same data `C_Item.GetItemID({bagID=B,
slotIndex=S})` returns. We already have the full bag→CGItem→itemID chain
in [src/item/Location.cpp](src/item/Location.cpp); this is just a
two-positional-arg wrapper over it.

## 17. `GetItemIcon(item)` — trivial

itemID → icon path (or fileID in modern). Same value `GetItemInfoInstant`
returns at position 5; this is the single-field accessor. Reads
`ItemDisplayInfo.dbc` at +0x14 — exact same code path
[src/item/Info.cpp](src/item/Info.cpp) already runs.

## 18. `UnitGUID(unit)` — easy

Returns the unit's 64-bit GUID as a string (`"0x%016X"` formatted). Used
heavily by modern addon code that tracks units across events (combat
log, threat tables, raid frames). 1.12 stores GUIDs the same way the
3.3.5 engine does — `[unit + 0x08]` is a pointer to an 8-byte GUID
struct (verified in `Script_GetInventoryItemLink`'s GUID-compare path
at `0x004C8CB0`-`0x004C8CC7`). Worth the small format helper because
addons backporting from 3.3.5+ assume this exists.

## 19. `IsPassiveSpell(spellID)` — trivial

Spell.dbc Attributes bit. Same one-byte read pattern
[src/spell/Info.cpp](src/spell/Info.cpp) already does for `isFunnel`
(AttributesEx bit 6) — just a different bit on a different field. The
public Spell.dbc schema has `Attributes` at `+0x18` with bit 6 (`0x40`)
= passive in vanilla. Used by aura libraries and talent code.

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

## 21. `InCombatLockdown()` — easy

Boolean: are we currently in combat for purposes of Blizzard's secure
UI restrictions. Modern addons gate action-bar/binding edits on this.
Maps to the same combat-state flag the existing 1.12 `UnitAffectingCombat("player")`
reads — likely just one global to expose, or one PLAYER_FLAGS bit.

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
