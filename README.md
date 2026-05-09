# ClassicAPI

A small DLL for World of Warcraft 1.12.1 (Vanilla / Turtle WoW) that adds a
collection of Lua API calls Blizzard never exposed in 1.12 but which make
addon authoring noticeably less painful — primarily for backporting addons
written against later API versions (3.3.5+) where these calls do exist.

The DLL hooks the FrameScript engine after WoW boots and registers its
extensions through the same in-engine mechanisms WoW uses for its own Lua
functions. No companion addon is required.

## What's added

Full per-function reference: **[docs/API.md](docs/API.md)**.

Quick index:

| Namespace | Calls |
|-----------|-------|
| Spell     | `GetSpellInfo` (with `(slot, bookType)` overload), `C_Spell.GetSpellInfo`, `C_Spell.GetSpellName`, `C_Spell.GetSpellTexture`, `GetSpellLink` (with `(slot, bookType)` overload), `C_Spell.GetSpellLink`, `FindSpellBookSlotByID`, `C_Spell.GetSpellDescription`, `IsPassiveSpell` (with `(slot, bookType)` overload), `C_Spell.IsSpellPassive`, `IsPlayerSpell`, `IsSpellKnown` |
| GameTooltip | `GameTooltip:SetSpellByID`, `GameTooltip:SetItemByID`, `GameTooltip:SetUnitAura` |
| Quest     | `GetQuestIDFromLogIndex`, `C_QuestLog.RequestLoadQuestByID` (+ `QUEST_DATA_LOAD_RESULT` event), `C_QuestLog.GetTitleForQuestID` |
| Faction   | `GetFactionIDByIndex`, `GetFactionInfoByID`, `GetFactionParentID` |
| Item      | `C_Item.IsBound`, `C_Item.GetItemID`, `GetInventoryItemID`, `GetInventoryItemDurability`, `C_Item.GetItemFamily`, `C_Item.GetItemInfoInstant`, `GetItemIcon` / `C_Item.GetItemIcon` / `C_Item.GetItemIconByID`, `C_Item.IsItemDataCached(ByID)`, `C_Item.RequestLoadItemData(ByID)` (+ `ITEM_DATA_LOAD_RESULT` event) |
| Container | `C_Container.GetContainerItemID`, `C_Container.GetContainerItemDurability`, `C_Container.GetContainerNumFreeSlots`, `C_Container.PlayerHasHearthstone`, `C_Container.UseHearthstone` |
| Unit      | `UnitGUID` |
| Combat    | `InCombatLockdown` |
| Talent    | `GetTalentSpellID`, `GetTalentIDByIndex` |
| Time      | `GetServerTime` |
| Events    | `C_EventUtils.IsEventValid` |
| Global    | `CLASSIC_API_VERSION` |

## Installation

Use [VanillaFixes](https://github.com/hannesmann/vanillafixes) to load the
DLL.

1. Install VanillaFixes if it isn't already.
2. Copy `ClassicAPI.dll` into your game directory.
3. Add `ClassicAPI.dll` to `dlls.txt`.
4. Launch the game with `VanillaFixes.exe`.

## Bundled addon: DebugTools

A 1.12.1 / Lua 5.0 backport of Blizzard's `Blizzard_DebugTools` addon
(originally shipped with the 3.0 client) lives in [`DebugTools/`](DebugTools/).
It's an independent addon — it doesn't use anything ClassicAPI adds, and
ClassicAPI works fine without it. It's bundled here because it's the
natural companion for testing and debugging anything written against the
1.12 Lua surface (with or without the ClassicAPI extensions).

Slash commands provided:

| Command | Purpose |
|---------|---------|
| `/dump <expr>` | Pretty-print any Lua value, including tables and multi-return tuples. The right tool for inspecting return values from `GetSpellInfo`, `C_Item.GetItemInfoInstant`, etc. |
| `/etrace` | Event tracer window. `/etrace start`, `/etrace stop`, `/etrace add EVENT`, etc. |
| `/framestack` (or `/fstack`) | Tooltip showing the frame hierarchy under the mouse cursor. |
| `/luaerrors` (or `/scripterrors`) | Lua error display window. |

To install: copy [`DebugTools/`](DebugTools/) into your
`Interface/AddOns/` directory like any other addon.

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
