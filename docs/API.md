# ClassicAPI — Lua Reference

Per-function reference for the calls ClassicAPI adds to the WoW 1.12.1 Lua
environment. See the [project README](../README.md) for installation and
build instructions.

## Contents

- [Spell](#spell)
  - [`GetSpellInfo(spellID)` / `GetSpellInfo(slot, bookType)`](#getspellinfospellid--getspellinfoslot-booktype)
  - [`C_Spell.GetSpellInfo(spellID)`](#c_spellgetspellinfospellid)
  - [`C_Spell.GetSpellName(spellID)`](#c_spellgetspellnamespellid)
  - [`C_Spell.GetSpellTexture(spellID)`](#c_spellgetspelltexturespellid)
  - [`FindSpellBookSlotByID(spellID)`](#findspellbookslotbyidspellid)
  - [`GetSpellLink(spellID)` / `GetSpellLink(slot, bookType)`](#getspelllinkspellid--getspelllinkslot-booktype)
  - [`C_Spell.GetSpellLink(spellID)`](#c_spellgetspelllinkspellid)
  - [`GameTooltip:SetSpellByID(spellID)`](#gametooltipsetspellbyidspellid)
  - [`C_Spell.GetSpellDescription(spellID)`](#c_spellgetspelldescriptionspellid)
- [Quest](#quest)
  - [`GetQuestIDFromLogIndex(index)`](#getquestidfromlogindexindex)
  - [`C_QuestLog.RequestLoadQuestByID(questID)`](#c_questlogrequestloadquestbyidquestid)
  - [`C_QuestLog.GetTitleForQuestID(questID)`](#c_questloggettitleforquestidquestid)
- [Faction](#faction)
  - [`GetFactionIDByIndex(factionIndex)`](#getfactionidbyindexfactionindex)
  - [`GetFactionInfoByID(factionID)`](#getfactioninfobyidfactionid)
- [Item](#item)
  - [`C_Item.IsBound(itemLocation)`](#c_itemisbounditemlocation)
  - [`C_Item.GetItemID(itemLocation)`](#c_itemgetitemiditemlocation)
  - [`GetInventoryItemID(unit, slot)`](#getinventoryitemidunit-slot)
  - [`C_Item.GetItemInfoInstant(item)`](#c_itemgetiteminfoinstantitem)
  - [`C_Item.IsItemDataCachedByID(item)` / `C_Item.IsItemDataCached(itemLocation)`](#c_itemisitemdatacachedbyiditem--c_itemisitemdatacacheditemlocation)
  - [`C_Item.RequestLoadItemDataByID(item)` / `C_Item.RequestLoadItemData(itemLocation)`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation)
- [Events](#events)
  - [`C_EventUtils.IsEventValid(eventName)`](#c_eventutilsiseventvalideventname)
- [Globals](#globals)
  - [`CLASSIC_API_VERSION`](#classic_api_version)

## Spell

### `GetSpellInfo(spellID)` / `GetSpellInfo(slot, bookType)`

Returns the same nine values as 3.3.5's `GetSpellInfo`, **plus a 10th
value: `spellID`**, for **any** spell ID — including spells the player
has not learned. Stock 1.12 has no `GetSpellInfo` Lua function at all
(only `GetSpellName`/`GetSpellTexture`, both of which take a spellbook
*slot* rather than an ID), so addons that need spell metadata for
arbitrary IDs (raid frames, debuff trackers, aura libraries) currently
can't get it.

Returns `name, rank, icon, cost, isFunnel, powerType, castTime,
minRange, maxRange, spellID`. All read directly from `Spell.dbc` (with
`SpellIcon.dbc`, `SpellCastTimes.dbc`, and `SpellRange.dbc` for the
indirected fields). Cast time is in milliseconds; ranges are floats in
yards. `isFunnel` is a real boolean (`true`/`false`), matching 3.3.5's
behavior. Returns `nil` if the spell ID is out of range.

Two input forms are accepted:

- **`GetSpellInfo(spellID)`** — direct DBC lookup by ID.
- **`GetSpellInfo(slot, bookType)`** — same shape as 1.12's
  `GetSpellName(slot, bookType)`. `slot` is 1-based, `bookType` is
  `"spell"` (player) or `"pet"`. The slot is resolved to a spellID via
  the engine's spellbook array, then the same DBC reads run. Returns
  `nil` for empty / out-of-range slots.

```lua
local name, rank, icon, _, _, _, _, _, _, spellID = GetSpellInfo(133)
-- name="Fireball", rank="Rank 1", icon="Spell\\Fire\\...", spellID=133

-- spellbook overload
local _, _, _, _, _, _, _, _, _, id = GetSpellInfo(1, "spell")
-- id is the spellID at player spellbook slot 1
```

> **Note on the 10th return.** Modern WoW (5.0+) added the spellID as
> the 14th return of its slimmer signature. We kept the existing 9
> returns (so addons that worked against the previous signature still
> work) and just appended `spellID` at position 10.

### `C_Spell.GetSpellInfo(spellID)`

Modern table-style accessor for the same data. Returns a Lua table of
the spell's metadata, or `nil` if the spell ID is out of range.

Table fields:

| Field        | Type    | Notes |
|--------------|---------|-------|
| `name`       | string  | Localized name |
| `iconID`     | string  | Icon **path** (e.g. `"Interface\\Icons\\Spell_Fire_FlameBolt"`). See note below. |
| `castTime`   | number  | Base cast time in milliseconds, or 0 for instant |
| `minRange`   | number  | Yards, or 0 if not applicable |
| `maxRange`   | number  | Yards, or 0 if not applicable |
| `spellID`    | number  | Echo of the input |
| `rank`       | string  | Localized rank (e.g. `"Rank 1"`) — vanilla extra, not in modern's spec |
| `cost`       | number  | Base ManaCost — vanilla extra |
| `isFunnel`   | boolean | True for funnel-channeled spells — vanilla extra |
| `powerType`  | number  | 0=mana, 1=rage, 2=focus, 3=energy, 4=happiness — vanilla extra |

```lua
local info = C_Spell.GetSpellInfo(133)
-- info.name = "Fireball"
-- info.iconID = "Interface\\Icons\\Spell_Fire_FlameBolt"
-- info.castTime = 1500
-- info.spellID = 133
-- info.rank = "Rank 1"
-- ... etc.
```

> **Deviation from modern.** Modern WoW returns `iconID` as a
> `fileID:number`. Vanilla 1.12 has no fileID system — assets are
> referenced by path strings. We surface the icon path here so it's
> directly usable with `texture:SetTexture(info.iconID)`. If you're
> backporting code that expects a number, this is the field to adjust.

> **Vanilla extras.** The four fields beyond the modern spec
> (`rank`/`cost`/`isFunnel`/`powerType`) are present because 1.12 has
> them in `Spell.dbc` and the previous-generation `GetSpellInfo` exposed
> them. Including them costs nothing and helps addons backporting from
> 3.3.5 where the same data was returned positionally.

Equivalent to the function of the same name introduced in 4.0.

### `C_Spell.GetSpellName(spellID)`

Returns the localized name of `spellID`, or `nil` if the spell ID is out
of range or has no name in the current locale. Convenience accessor for
the `name` field of [`C_Spell.GetSpellInfo`](#c_spellgetspellinfospellid)
when that's all you need — single field read, no DBC indirection beyond
the locale lookup.

```lua
local name = C_Spell.GetSpellName(133)  -- "Fireball"
```

Equivalent to the function of the same name introduced in 10.0.

### `C_Spell.GetSpellTexture(spellID)`

Returns the icon path string for `spellID` (read from `SpellIcon.dbc`
via the spell's `SpellIconID` field), or `nil` if the spell ID is out
of range or the icon record is empty.

> **Path string, not fileID.** Modern WoW returns this as a
> `fileID:number`. Vanilla 1.12 has no fileID system — see the same
> note on [`C_Spell.GetSpellInfo`](#c_spellgetspellinfospellid)'s
> `iconID` field. Pass directly to `texture:SetTexture(...)`.

```lua
local path = C_Spell.GetSpellTexture(133)
-- path = "Interface\\Icons\\Spell_Fire_FlameBolt"
```

Equivalent to the function of the same name introduced in 10.0.

### `FindSpellBookSlotByID(spellID)`

Inverse of 1.12's `GetSpellName(slot, bookType)`. Given a spellID,
returns the 1-based slot it occupies in the player or pet spellbook,
along with the matching bookType so the result feeds directly into
slot-and-bookType APIs (`GetSpellName`, `GetSpellTexture`,
`GameTooltip:SetSpell`, etc.).

```
slot, bookType = FindSpellBookSlotByID(spellID)
```

- Returns `(slot, "spell")` if the spell is in the player spellbook.
- Returns `(slot, "pet")` if it's only in the pet spellbook.
- Returns `nil` if the spellID isn't currently in either book.

Player book is searched first, so if a spell somehow appeared in both
(unusual but possible for special pet-shared abilities), the player
slot wins.

```lua
local slot, book = FindSpellBookSlotByID(133)
if slot then
    GameTooltip:SetSpell(slot, book)  -- shows full caster-scaled tooltip
end
```

Equivalent to the legacy function of the same name introduced in 3.0
(later renamed to `FindSpellBookSlotBySpellID` in 5.x).

### `GetSpellLink(spellID)` / `GetSpellLink(slot, bookType)`

Returns the chat-style spell hyperlink and the spellID:

```
link, spellID = GetSpellLink(spellID)
              = GetSpellLink(slot, bookType)
```

Format is `|cff71d5ff|Hspell:ID:0|h[Name]|h|r` — the standard 1.12
spell-link wrapper. The trailing `:0` after the spellID matches modern
WoW's hyperlink shape (where the field is a sub-data slot for
pet-spellbook flags etc.); 1.12 ignores it during link parsing, but
addons grepping with `|Hspell:(%d+):` patterns will pick it up
correctly.

Two input forms, mirroring [`GetSpellInfo`](#getspellinfospellid--getspellinfoslot-booktype):

- `GetSpellLink(spellID)` — direct DBC lookup.
- `GetSpellLink(slot, bookType)` — resolves the spellbook slot to a
  spellID first. Useful when iterating the player's known spells:
  caller gets back both the link AND the underlying ID without a
  separate lookup.

Returns `nil` if the spellID/slot doesn't resolve to a real spell.

```lua
local link = GetSpellLink(133)
DEFAULT_CHAT_FRAME:AddMessage("Cast " .. link .. "!")

-- Walking the spellbook to print every learned spell:
for slot = 1, GetNumSpellTabs() and 100 or 100 do
    local link, id = GetSpellLink(slot, "spell")
    if not link then break end
    -- ...
end
```

### `C_Spell.GetSpellLink(spellID)`

Modern table-namespace variant. Same link string as
[`GetSpellLink(spellID)`](#getspelllinkspellid--getspelllinkslot-booktype),
but returns only the link — no spellID echo since the caller already
had it on hand to make the call.

```lua
local link = C_Spell.GetSpellLink(133)  -- "|cff71d5ff|Hspell:133:0|h[Fireball]|h|r"
```

Equivalent to the function of the same name introduced in 4.0.

### `GameTooltip:SetSpellByID(spellID)`

Renders a spell tooltip for any `spellID`, including spells the player has
not learned. The stock 1.12 `GameTooltip:SetSpell(slot, bookType)` only works
for entries in the player's spellbook (or pet book). `SetSpellByID` bypasses
the spellbook indirection and calls WoW's internal tooltip builder directly.

```lua
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
GameTooltip:SetSpellByID(133)  -- Fireball
GameTooltip:Show()
```

Equivalent to the function of the same name introduced in 3.0.

### `C_Spell.GetSpellDescription(spellID)`

Returns the formatted spell description for any `spellID` — including
spells the player has not learned. `$s1`/`$s2`/`$o1`/`$d`-style
placeholders are resolved to base-rank values; ranges and durations
appear as actual numbers (e.g. `"14 to 22 Fire damage"`, not
`"$s1 to $s2 Fire damage"`). Returns `nil` if the spell ID is out of
range or has no description in the current locale.

The 1.12 client doesn't expose this from Lua at all — the only existing
path is the scan-tooltip hack (set a hidden tooltip via
`SetHyperlink("spell:"..ID)` and read each `TextLeftN:GetText()` line).
That's slow, GC-heavy, and shares the global scan tooltip with every
other addon. This function calls the engine's own description formatter
directly — same code path the in-game tooltip uses, no UI side effects.

```lua
local desc = C_Spell.GetSpellDescription(133)  -- Fireball Rank 1
-- "Hurls a fiery ball that causes 14 to 22 Fire damage and an additional
--  2 Fire damage over 4 sec."
```

Equivalent to the function of the same name introduced in 4.0.

> **No caster scaling.** Values reflect the spell's base rank — caster
> level / spell power / talents are not applied. Modern WoW behaves the
> same way when called outside a unit context. If you need the
> "currently displayed" tooltip text with caster scaling, use
> `GameTooltip:SetSpellByID` and read line strings from there.

## Quest

### `GetQuestIDFromLogIndex(index)`

Returns the questID (Quest.dbc row ID) for the entry at the given 1-based
quest log index. In 3.3.5 this came as the 9th return of `GetQuestLogTitle`;
in 1.12 it isn't returned at all, even though the engine has it internally.

- Returns the questID for real quests.
- Returns `0` for header rows (zone / category dividers).
- Returns `nil` if the index is out of range.

```lua
for i = 1, GetNumQuestLogEntries() do
    local title, level, questTag, isHeader, isCollapsed, isComplete
        = GetQuestLogTitle(i)
    local questID = GetQuestIDFromLogIndex(i)  -- 0 for headers
    -- ...
end
```

### `C_QuestLog.RequestLoadQuestByID(questID)`

Asks the engine to fetch the static data for `questID` (title, description,
objectives, reward text) from the server if not already cached. Returns no
values — fire-and-forget, matching modern WoW's signature.

Fires `QUEST_DATA_LOAD_RESULT(questID, success)` when the data lands in the
cache. Synchronously fired when the data was already cached
(so polling code paths still work), asynchronously fired after
`SMSG_QUEST_QUERY_RESPONSE` lands when the engine had to round-trip to the
server.

> **Vanilla quirk:** the engine has no native bool format code — `success`
> arrives as `1`/`0` (number), not `true`/`false`, just like
> `ITEM_DATA_LOAD_RESULT`. Idiomatic check is `if success == 1 then ...`.

> **Vanilla limitation:** for *invalid* questIDs (ones the server doesn't
> have), the 1.12 server silently drops the query — it doesn't send a
> "not found" response packet. The engine's pending callback never
> resolves, so `QUEST_DATA_LOAD_RESULT` doesn't fire with `success=0`
> either. Modern Classic Era servers explicitly respond with an error
> for invalid IDs, which is why modern `RequestLoadQuestByID` reliably
> fires `success=false`; vanilla doesn't. Addons that need to handle
> "request timed out" should use their own timer (the Lua polyfill at
> `!!!ClassicAPI/Util/QuestUtil.lua` uses a 180-second wait).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("QUEST_DATA_LOAD_RESULT")
f:SetScript("OnEvent", function()
    -- vanilla 1.12: event payload is in `arg1`, `arg2`, ... globals
    if event == "QUEST_DATA_LOAD_RESULT" and arg2 == 1 then
        local questID = arg1
        -- title/description/objectives are now in the engine's quest cache
    end
end)
C_QuestLog.RequestLoadQuestByID(2)
```

### `C_QuestLog.GetTitleForQuestID(questID)`

Returns the title (string) for `questID` from the engine's quest static-info
cache, or `nil` if the data isn't loaded. Doesn't require the quest to be
in the player's quest log — works for any questID once its data has been
fetched. Header rows are excluded (their titles live in `QuestSort.dbc`,
not in this cache); for those you'd use the existing `GetQuestLogTitle`.

The cache is populated lazily — by the engine's own quest-log path when
the player has the quest, or explicitly by
`C_QuestLog.RequestLoadQuestByID`. If the title isn't there yet, queue a
load and read on the event:

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("QUEST_DATA_LOAD_RESULT")
f:SetScript("OnEvent", function()
    if event == "QUEST_DATA_LOAD_RESULT" and arg2 == 1 then
        local title = C_QuestLog.GetTitleForQuestID(arg1)
        if title then
            -- title is now available
        end
    end
end)
C_QuestLog.RequestLoadQuestByID(215)
```

Equivalent to the function of the same name introduced in 5.0.

## Faction

### `GetFactionIDByIndex(factionIndex)`

Returns the factionID (Faction.dbc row ID) for the entry at the given 1-based
displayed-faction index. Modern WoW (5.0+, including Classic Era 1.15.x)
returns this as the 14th value of `GetFactionInfo`; older clients (1.12
through 3.3.5) don't expose it from Lua at all, even though the engine
uses it internally to look up `Faction.dbc`.

- Returns the factionID for real factions.
- Returns `0` for header / category rows (`"Other"`, `"Inactive"`, etc.) —
  matching the modern Classic Era client, which puts `0` in
  `GetFactionInfo`'s `factionID` slot for those rows.
- Returns `nil` if the index is out of range.

The "headers normalize to `0`" rule deliberately matches modern WoW
(5.0+) behavior. The 1.12 engine actually returns `0` for some header
types (`"Other"`) and `-1` for others (`"Inactive"`-style pseudo-rows);
we collapse both to `0` so the user-facing convention is consistent.

```lua
for i = 1, GetNumFactions() do
    local name, _, _, _, _, _, _, _, isHeader = GetFactionInfo(i)
    if not isHeader then
        local factionID = GetFactionIDByIndex(i)
        -- ...
    end
end
```

### `GetFactionInfoByID(factionID)`

Returns the same nine-to-eleven values as `GetFactionInfo(factionIndex)`,
keyed by factionID instead of displayed index:

```
name, description, standingID, barMin, barMax, barValue,
atWarWith, canToggleAtWar, isHeader, isCollapsed, hasRep
```

Returns `nil` for factionIDs the player has no reputation with (not in the
displayed list). This matches modern WoW's behavior — `GetFactionInfoByID`
is fundamentally a "look up rep info I already have, by ID" call, not a
DBC reader for arbitrary factions.

```lua
local name, _, standing = GetFactionInfoByID(69)  -- Darnassus
-- name = "Darnassus", standing = 5 (Friendly), etc.
```

Equivalent to the function of the same name introduced in 3.0.

## Item

### `C_Item.IsBound(itemLocation)`

Returns `true` if the item at the given location is soulbound, `false` otherwise
(including when the slot is empty or the location is malformed). The 1.12
client tracks the soulbound bit on each item instance directly; previously
the only way to read it from Lua was a scan-tooltip hack
(`SetBagItem` + string-compare against the localized `ITEM_SOULBOUND`
constant) — slow, locale-fragile, and one of the hottest paths during bag
updates.

`itemLocation` is a table with one of these shapes (matching the modern
`ItemLocation` mixin):

```lua
{ equipmentSlotIndex = N }     -- 1-based, character pane
{ bagID = B, slotIndex = S }   -- both required
```

```lua
if C_Item.IsBound({equipmentSlotIndex = INVSLOT_HEAD}) then ... end
if C_Item.IsBound({bagID = 0, slotIndex = 1}) then ... end
```

### `C_Item.GetItemID(itemLocation)`

Returns the itemID of the item at the given location, or `nil` if the slot
is empty or the location is malformed. Useful as the input to
`GetItemInfoInstant`/`GetItemInfo` when you only know which slot an item
came from (rather than its link or ID).

Accepts the same `itemLocation` shapes as `IsBound`:

```lua
local id = C_Item.GetItemID({equipmentSlotIndex = INVSLOT_HEAD})
if id then
    local _, type, subtype = C_Item.GetItemInfoInstant(id)
    -- ...
end
```

### `GetInventoryItemID(unit, slot)`

Returns the itemID of the item equipped at `slot` (1-based) on `unit`,
or `nil` if the slot is empty / the unit isn't valid / the unit doesn't
expose its equipment to the local client. Same arg shape as 1.12's
`GetInventoryItemLink(unit, slot)` — drop-in for code that just needs
the ID and would otherwise have to parse the link string.

- For `"player"` (and any token resolving to the local player, e.g.
  `"target"` when self-targeted): walks the private inventory manager.
  Supports the full slot range (equipment 1..19, bag slots 20..23,
  bank slots, etc.) — same range `GetItemBySlot` accepts internally.
- For other player-controlled units (`"target"`, `"party1"..party4"`,
  inspect targets): reads the unit's broadcast visible-items array.
  Equipment slots 1..19 only.
- For NPCs / creatures: returns `nil`. The visible-items array isn't
  populated for non-player-controlled units in 1.12, so we gate this
  on `UnitPlayerControlled` to avoid the engine crash that
  `GetInventoryItemLink` itself would trigger on the same input.

```lua
local id = GetInventoryItemID("player", INVSLOT_HEAD)
if id then
    local _, type, subtype = C_Item.GetItemInfoInstant(id)
    -- ...
end

-- Inspect a party member without parsing a hyperlink:
local headID = GetInventoryItemID("party1", INVSLOT_HEAD)
```

### `C_Item.GetItemInfoInstant(item)`

Modern-style accessor for the always-available subset of item info — the
fields that depend only on classification, not on player-specific state.
Returns immediately from the client-side item cache; no server round-trip,
no async polling. Returns `nil` if the item isn't in cache.

Accepts a numeric `itemID` or a string containing `"item:NNN"` (matches both
the bare `"item:1234"` shorthand and full chat links like
`"|cff...|Hitem:1234:...|h[Name]|h|r"`). Item names are not accepted —
vanilla itself has no name → ID resolver, and it's rarely the form addon code
actually has on hand.

Returns seven values:

```
itemID, itemType, itemSubType, itemEquipLoc, icon, classID, subClassID
```

- `itemType` / `itemSubType` are the localized class / subclass names
  (e.g. `"Weapon"` / `"One-Handed Swords"`), read from `ItemClass.dbc` and
  `ItemSubClass.dbc`.
- `itemEquipLoc` is the `"INVTYPE_*"` constant (e.g. `"INVTYPE_HEAD"`), or
  `""` for non-equippable items.
- `icon` is a path string (`"Interface\\Icons\\..."`), matching what the
  rest of the 1.12 API returns. Modern WoW returns a numeric fileID here,
  but 1.12 has no fileID system, so a path is the only meaningful value.
- `classID` / `subClassID` are the raw enum integers (e.g. `2`, `7` for
  one-handed swords).

```lua
local id, type, subtype, equipLoc, icon, classID, subClassID
    = C_Item.GetItemInfoInstant(6948)  -- Hearthstone
-- type="Miscellaneous", subtype="Junk", equipLoc="",
-- icon="Interface\\Icons\\INV_Misc_Rune_01", classID=15, subClassID=0
```

The actual class/subclass values reflect 1.12.1's data, which differs from
modern WoW. For example, vanilla had no Cloth subclass under Trade Goods —
Silk Cloth lives at `(7, 0)` in this client, not the modern `(7, 5)`.

### `C_Item.IsItemDataCachedByID(item)` / `C_Item.IsItemDataCached(itemLocation)`

Returns `true` if the item's static data is currently in the client-side
item cache, `false` otherwise. The "ByID" variant takes an itemID or
"item:NNN"-style string; the location variant takes the modern
`{equipmentSlotIndex=}` / `{bagID=, slotIndex=}` table.

These read the cache without firing a server query — pair with
`RequestLoadItemData(ByID)` if you need to ensure the data is loaded
before checking.

```lua
if not C_Item.IsItemDataCachedByID(itemID) then
    C_Item.RequestLoadItemDataByID(itemID)
    -- (poll IsItemDataCachedByID on a timer until true)
end
```

### `C_Item.RequestLoadItemDataByID(item)` / `C_Item.RequestLoadItemData(itemLocation)`

Asks the engine to fetch the item's data from the server if not already
cached. Returns `true` if the request was initiated (or the input was
parseable to an itemID), `false` for malformed input. Fire-and-forget —
the engine handles the round-trip; the data lands in the cache when the
server responds.

Fires `ITEM_DATA_LOAD_RESULT(itemID, success)` when the data lands in
the cache, matching the modern API. Synchronously fired when the item
was already cached (so polling code paths still work), asynchronously
fired when the engine's SMSG response handler completes a network
fetch.

> **Vanilla quirk:** the engine has no native bool format code — `success`
> arrives as `1`/`0` (number), not `true`/`false`. Idiomatic check is
> `if success == 1 then ...`.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("ITEM_DATA_LOAD_RESULT")
f:SetScript("OnEvent", function()
    -- vanilla 1.12: event payload is in `arg1`, `arg2`, ... globals
    if event == "ITEM_DATA_LOAD_RESULT" and arg2 == 1 then
        local _, type = C_Item.GetItemInfoInstant(arg1)
        -- ...
    end
end)
C_Item.RequestLoadItemDataByID(2589)
```

## Events

### `C_EventUtils.IsEventValid(eventName)`

Returns `true` if the named event exists in the engine's event-name table
and can be registered for, `false` otherwise. Equivalent to the modern
function of the same name; useful for addons that gate code behind feature
detection (e.g. registering for events that exist in some client builds
but not others).

```lua
if C_EventUtils.IsEventValid("PLAYER_LOGIN") then ... end       -- true
if C_EventUtils.IsEventValid("COMBAT_LOG_EVENT") then ... end   -- false (added in 3.x)
```

The check walks the engine's static event-name table at runtime, so it
sees events added by **any** loaded DLL that injects names by overwriting
`.rdata` strings in place — for example, on a Turtle WoW client running
SuperWoWhook, `IsEventValid("UNIT_CASTEVENT")` returns `true` because
SuperWoWhook patches the binary's event names with its own. This matches
what `frame:RegisterEvent` will actually accept, which is what addon code
needs to know.

## Globals

### `CLASSIC_API_VERSION`

Defined once FrameScript has booted. Addons can use this to detect that
the DLL is loaded and which version is in use. The value is
`X*10000 + Y*100 + Z` for a tag of `vX.Y.Z` passed to CMake at configure
time via `-DCLASSICAPI_TAG=vX.Y.Z`. Unset versions resolve to `0`.

```lua
if CLASSIC_API_VERSION and CLASSIC_API_VERSION >= 10200 then
    -- ClassicAPI v1.2.0 or newer is loaded
end
```
