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
| Spell     | `C_Spell.GetSpellDescription`, `C_Spell.GetSpellInfo`, `C_Spell.GetSpellLink`, `C_Spell.GetSpellName`, `C_Spell.GetSpellTexture`, `C_Spell.IsSpellPassive`, `C_Spell.IsSpellUsable`, `FindSpellBookSlotByID`, `GetSpellInfo`, `GetSpellLink`, `GetSpellSchool`, `IsPassiveSpell`, `IsPlayerSpell`, `IsSpellKnown`, `IsUsableSpell` |
| GameTooltip | `GameTooltip:SetInventoryItemByID`, `GameTooltip:SetItemByID`, `GameTooltip:SetSpellByID`, `GameTooltip:SetTalentByID`, `GameTooltip:SetUnitAura` |
| Quest     | `C_QuestLog.GetTitleForQuestID`, `C_QuestLog.RequestLoadQuestByID` (+ `QUEST_DATA_LOAD_RESULT` event), `GetQuestIDFromLogIndex` |
| Faction   | `C_Reputation.SetWatchedFactionByID`, `GetFactionIDByIndex`, `GetFactionInfoByID`, `GetFactionParentID` |
| Item      | `C_Item.EquipItemByName`, `C_Item.GetItemCount`, `C_Item.GetItemFamily`, `C_Item.GetItemID`, `C_Item.GetItemInfoInstant`, `C_Item.IsBound`, `C_Item.IsEquippedItem`, `C_Item.IsItemDataCached(ByID)`, `C_Item.RequestLoadItemData(ByID)` (+ `ITEM_DATA_LOAD_RESULT` event), `GetInventoryItemDurability`, `GetInventoryItemID`, `GetItemIcon` / `C_Item.GetItemIcon` / `C_Item.GetItemIconByID`, `OffhandHasWeapon` |
| Container | `C_Container.GetContainerItemDurability`, `C_Container.GetContainerItemID`, `C_Container.GetContainerNumFreeSlots`, `C_Container.PlayerHasHearthstone`, `C_Container.UseHearthstone` |
| Class     | `FillLocalizedClassList` |
| Unit      | `UnitGUID`, `UnitIsAFK`, `UnitIsDND`, `UnitIsFeignDeath` |
| State     | `IsFalling`, `IsMounted`, `IsStealthed`, `IsSwimming` |
| AddOns    | `C_AddOns.DoesAddOnExist`, `C_AddOns.GetAddOnName`, `C_AddOns.GetAddOnNotes`, `C_AddOns.GetAddOnSecurity`, `C_AddOns.GetAddOnTitle`, `C_AddOns.IsAddOnLoadable` |
| Combat    | `InCombatLockdown` |
| Hooks     | `hooksecurefunc` |
| Input     | `IsLeftShiftKeyDown`, `IsRightShiftKeyDown`, `IsLeftControlKeyDown`, `IsRightControlKeyDown`, `IsLeftAltKeyDown`, `IsRightAltKeyDown`, `IsModifierKeyDown` (+ `MODIFIER_STATE_CHANGED` event) |
| Talent    | `GetTalentIDByIndex`, `GetTalentSpellID` |
| Time      | `GetServerTime` |
| Events    | `C_EventUtils.IsEventValid` |
| Globals   | `CLASSIC_API_VERSION`, `LE_EXPANSION_LEVEL_CURRENT`, `LE_EXPANSION_CLASSIC` … `LE_EXPANSION_MIDNIGHT`, `LE_ITEM_QUALITY_POOR` … `LE_ITEM_QUALITY_WOWTOKEN`, `LE_UNIT_STAT_STRENGTH` … `LE_UNIT_STAT_SPIRIT` |

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

Lua globals provided (backports of 3.3.5 helpers that don't exist in
1.12):

| Global | Purpose |
|--------|---------|
| `print(...)` | Backport of the 3.3.5 `print` — concats varargs with `" "` and pushes to `DEFAULT_CHAT_FRAME`. Routes through `setprinthandler`'s handler; falls back via `geterrorhandler` if the handler errors. |
| `setprinthandler(func)` / `getprinthandler()` | Install / query a custom print handler. Useful for redirecting print output in tests. |
| `tostringall(...)` | Apply `tostring()` to every vararg, preserving the count. Lua 5.0-compatible (uses `arg.n` since `select` doesn't exist in 5.0). |

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
