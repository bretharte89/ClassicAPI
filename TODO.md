# ClassicAPI — TODO

Candidates for new DLL functions, prioritized. All are things the
[!!!ClassicAPI](C:\WoW\FrostmourneClient\Interface\AddOns\!!!ClassicAPI) Lua
addon polyfills badly, hackily, or not at all, and where engine access is the
right answer.

## 1. `C_QuestLog.IsQuestFlaggedCompleted(questID)` — trivial

Lua addon hits the server (`QueryQuestsCompleted`) on every call and scans
`GetQuestsCompleted()`. Slow and chatty. Engine maintains a completed-quests
bitfield in client memory after the first query — read it directly, no
server round-trip.

Reference: `Util/C_QuestLog.lua:5-8`.

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

## 6. `C_QuestLog.GetTitleForQuestID(questID)` — easy

Lua addon linear-scans the quest log, then if the questID isn't currently
in-log it falls back to an async `SetHyperlink("quest:..")` against a hidden
tooltip with a 180-second timeout (whole `QuestEventListener` subsystem in
`QuestUtil.lua`). Engine has the title in `Quest.dbc` indexed by questID —
one lookup replaces the entire async dance.

References: `Util/C_QuestLog.lua:51-65`, `Util/QuestUtil.lua`.

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

## 13. `ITEM_DATA_LOAD_RESULT` event — hard

Modern API fires this event with `(itemID, success)` when a previously
uncached item finishes loading after a `RequestLoadItemData*` call. We
ship the cache-check + request functions in #12, but addons currently
have to poll `IsItemDataCachedByID` to detect completion.

To wire up the real event we need:

1. **Engine fire-event-by-id function.** Vanilla dispatches events by
   integer ID, not by name — verified by greping the binary for pushes
   of every well-known event name string and finding zero call sites
   outside the static-init function. The dispatcher is well-buried; no
   debug strings (`SignalEvent`/`FireEvent`/etc. all return zero hits).
   Best path forward is probably tracing back from a specific packet
   handler whose dispatch we already understand (e.g. find the
   `SMSG_ADDON_INFO` handler and trail forward to see how it fires
   `ADDON_LOADED`).
2. **A way to advertise `ITEM_DATA_LOAD_RESULT` as a known event.** The
   `_GetRecord` callback parameter (3rd arg) is probably how the engine
   wires SMSG response → cache fill → notify observer. If we can pass
   our own callback, we can hook the cache-fill notification directly
   without touching the engine's event table.
3. **A NULL slot in the event-name table** if firing requires the name
   to be registered. Static analysis found ~221 gap slots in
   `0x00BE11D8..0x00BE1C24` that no observed write targets — pick one
   and patch in `"ITEM_DATA_LOAD_RESULT"`.

Notes are in [CLAUDE.md](CLAUDE.md) under "Events (verified)".
