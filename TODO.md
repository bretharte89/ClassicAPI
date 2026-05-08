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

## 2. `C_Spell.GetSpellDescription(spellID)` — easy

Lua addon creates a hidden `__CAPIScanTooltip`, calls `SetHyperlink("spell:"..ID)`,
then reads `TextLeft<N>:GetText()` of the last line. Fragile, GC-heavy, and
collides with anything else using the scan tooltip. Engine has the
description string in `Spell.dbc` directly — and we already know how to
reach the spell-tooltip internals from `SetSpellByID`, so resolving any
description-format placeholders is a known path.

Reference: `Util/C_Spell.lua:22-30`.

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

## 8. `GetItemInfo` for uncached items — medium

Lua addon polls `GetItemInfo` after firing `SetHyperlink("item:..")` with a
180s timeout (`ItemEventListener` in `ItemUtil.lua`). Engine has an
item-cache request function with callback hooks, and `Item.dbc` has the
static fields directly — replaces the entire polling frame.

The instant-only subset is already covered by `C_Item.GetItemInfoInstant`
(see below); what's left for this task is the cache-fill path with a
proper async callback so addons stop polling on a frame timer.

Reference: `Util/ItemUtil.lua:254-372`.

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
