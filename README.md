# ClassicAPI

A small DLL for World of Warcraft 1.12.1 (Vanilla / Turtle WoW) that adds a
couple of Lua API calls Blizzard never exposed in 1.12 but which make addon
authoring noticeably less painful — primarily for backporting addons written
against later API versions (3.3.5+) where these calls do exist.

The DLL hooks the FrameScript engine after WoW boots and registers its
extensions through the same in-engine mechanisms WoW uses for its own Lua
functions. No companion addon is required.

## Lua API

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

### `GetSpellInfo(spellID)`

Returns the same nine values as 3.3.5's `GetSpellInfo`, for **any** spell ID
— including spells the player has not learned. Stock 1.12 has no
`GetSpellInfo` Lua function at all (only `GetSpellName`/`GetSpellTexture`,
both of which take a spellbook *slot* rather than an ID), so addons that
need spell metadata for arbitrary IDs (raid frames, debuff trackers, aura
libraries) currently can't get it.

Returns `name, rank, icon, cost, isFunnel, powerType, castTime, minRange,
maxRange`. All read directly from `Spell.dbc` (with `SpellIcon.dbc`,
`SpellCastTimes.dbc`, and `SpellRange.dbc` for the indirected fields). Cast
time is in milliseconds; ranges are floats in yards. `isFunnel` is a real
boolean (`true`/`false`), matching 3.3.5's behavior. Returns `nil` if the
spell ID is out of range.

```lua
local name, rank, icon = GetSpellInfo(133)  -- Fireball Rank 1
-- name="Fireball", rank="Rank 1", icon="Spell\\Fire\\..."
```

## Global

The DLL also defines a Lua global `CLASSIC_API_VERSION` once FrameScript has
booted, which addons can use to detect that the DLL is loaded and which
version is in use.

## Installation

Use [VanillaFixes](https://github.com/hannesmann/vanillafixes) to load the
DLL.

1. Install VanillaFixes if it isn't already.
2. Copy `ClassicAPI.dll` into your game directory.
3. Add `ClassicAPI.dll` to `dlls.txt`.
4. Launch the game with `VanillaFixes.exe`.

## Building

Requires CMake (3.10+) and an MSVC toolchain that can target 32-bit Windows.
WoW.exe is x86, so the DLL must be built as Win32; an x64 build will not
load.

```powershell
git submodule update --init --recursive   # fetches MinHook
cmake -B build -A Win32
cmake --build build --config Release
```

The output is `build/Release/ClassicAPI.dll`.

To stamp a version into `CLASSIC_API_VERSION`, pass `-DCLASSICAPI_TAG=vX.Y.Z`
at configure time; the value exposed to Lua will be `X*10000 + Y*100 + Z`.

## License

LGPL v3 or later. See the headers in `src/` for the full notice.
