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
| Spell     | `GetSpellInfo` (with `(slot, bookType)` overload), `C_Spell.GetSpellInfo`, `C_Spell.GetSpellName`, `C_Spell.GetSpellTexture`, `GameTooltip:SetSpellByID`, `C_Spell.GetSpellDescription` |
| Quest     | `GetQuestIDFromLogIndex`, `C_QuestLog.RequestLoadQuestByID` (+ `QUEST_DATA_LOAD_RESULT` event), `C_QuestLog.GetTitleForQuestID` |
| Faction   | `GetFactionIDByIndex`, `GetFactionInfoByID` |
| Item      | `C_Item.IsBound`, `C_Item.GetItemID`, `GetInventoryItemID`, `C_Item.GetItemInfoInstant`, `C_Item.IsItemDataCached(ByID)`, `C_Item.RequestLoadItemData(ByID)` (+ `ITEM_DATA_LOAD_RESULT` event) |
| Events    | `C_EventUtils.IsEventValid` |
| Global    | `CLASSIC_API_VERSION` |

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
