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
