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
  - [`C_Spell.GetSpellDescription(spellID)`](#c_spellgetspelldescriptionspellid)
  - [`IsPassiveSpell(spellID)` / `IsPassiveSpell(slot, bookType)`](#ispassivespellspellid--ispassivespellslot-booktype)
  - [`C_Spell.IsSpellPassive(spellID)`](#c_spellisspellpassivespellid)
  - [`IsPlayerSpell(spellID)`](#isplayerspellspellid)
  - [`IsSpellKnown(spellID, [isPet])`](#isspellknownspellid-ispet)
  - [`IsUsableSpell(spell)` / `IsUsableSpell(slot, bookType)`](#isusablespellspell--isusablespellslot-booktype)
  - [`C_Spell.IsSpellUsable(spellID)`](#c_spellisspellusablespellid)
  - [`GetSpellSchool(spellID)`](#getspellschoolspellid)
- [GameTooltip](#gametooltip)
  - [`GameTooltip:SetSpellByID(spellID)`](#gametooltipsetspellbyidspellid)
  - [`GameTooltip:SetTalentByID(talentID)`](#gametooltipsettalentbyidtalentid)
  - [`GameTooltip:SetInventoryItemByID(itemID)`](#gametooltipsetinventoryitembyiditemid)
- [Quest](#quest)
  - [`GetQuestIDFromLogIndex(index)`](#getquestidfromlogindexindex)
  - [`C_QuestLog.RequestLoadQuestByID(questID)`](#c_questlogrequestloadquestbyidquestid)
  - [`C_QuestLog.GetTitleForQuestID(questID)`](#c_questloggettitleforquestidquestid)
- [Faction](#faction)
  - [`GetFactionIDByIndex(factionIndex)`](#getfactionidbyindexfactionindex)
  - [`GetFactionInfoByID(factionID)`](#getfactioninfobyidfactionid)
  - [`GetFactionParentID(factionID)`](#getfactionparentidfactionid)
  - [`C_Reputation.SetWatchedFactionByID(factionID)`](#c_reputationsetwatchedfactionbyidfactionid)
- [Item](#item)
  - [`C_Item.IsBound(itemLocation)`](#c_itemisbounditemlocation)
  - [`C_Item.GetItemID(itemLocation)`](#c_itemgetitemiditemlocation)
  - [`GetInventoryItemID(unit, slot)`](#getinventoryitemidunit-slot)
  - [`GetInventoryItemDurability(invSlot)`](#getinventoryitemdurabilityinvslot)
  - [`C_Item.GetItemFamily(item)`](#c_itemgetitemfamilyitem)
  - [`C_Item.GetItemCount(itemInfo, [includeBank], [includeUses])`](#c_itemgetitemcountiteminfo-includebank-includeuses)
  - [`GetItemIcon(itemID)` / `C_Item.GetItemIcon(itemLocation)` / `C_Item.GetItemIconByID(item)`](#getitemiconitemid--c_itemgetitemiconitemlocation--c_itemgetitemiconbyiditem)
  - [`C_Item.GetItemInfoInstant(item)`](#c_itemgetiteminfoinstantitem)
  - [`C_Item.IsItemDataCachedByID(item)` / `C_Item.IsItemDataCached(itemLocation)`](#c_itemisitemdatacachedbyiditem--c_itemisitemdatacacheditemlocation)
  - [`C_Item.RequestLoadItemDataByID(item)` / `C_Item.RequestLoadItemData(itemLocation)`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation)
  - [`OffhandHasWeapon()`](#offhandhasweapon)
  - [`C_Item.IsEquippedItem(item)`](#c_itemisequippeditemitem)
  - [`C_Item.EquipItemByName(itemInfo [, dstSlot])`](#c_itemequipitembynameiteminfo--dstslot)
- [Container](#container)
  - [`C_Container.GetContainerItemID(bagIndex, slotIndex)`](#c_containergetcontaineritemidbagindex-slotindex)
  - [`C_Container.GetContainerItemDurability(containerIndex, slotIndex)`](#c_containergetcontaineritemdurabilitycontainerindex-slotindex)
  - [`C_Container.GetContainerNumFreeSlots(bagID)`](#c_containergetcontainernumfreeslotsbagid)
  - [`C_Container.PlayerHasHearthstone()`](#c_containerplayerhashearthstone)
  - [`C_Container.UseHearthstone()`](#c_containerusehearthstone)
- [Class](#class)
  - [`FillLocalizedClassList(table [, isFemale])`](#filllocalizedclasslisttable-isfemale)
- [Unit](#unit)
  - [`UnitGUID(unit)`](#unitguidunit)
  - [`UnitIsAFK(unit)`](#unitisafkunit)
  - [`UnitIsDND(unit)`](#unitisdndunit)
  - [`UnitIsFeignDeath(unit)`](#unitisfeigndeathunit)
  - [`UnitIsInMyGuild(unitOrName)`](#unitisinmyguildunitorname)
  - [`UnitIsPossessed(unit)`](#unitispossessedunit)
- [State](#state)
  - [`IsMounted()`](#ismounted)
  - [`IsStealthed()`](#isstealthed)
  - [`IsFalling()`](#isfalling)
  - [`IsSwimming()`](#isswimming)
- [AddOns](#addons)
  - [`C_AddOns.GetAddOnName(indexOrName)`](#c_addonsgetaddonnameindexorname)
  - [`C_AddOns.GetAddOnTitle(indexOrName)`](#c_addonsgetaddontitleindexorname)
  - [`C_AddOns.GetAddOnNotes(indexOrName)`](#c_addonsgetaddonnotesindexorname)
  - [`C_AddOns.IsAddOnLoadable(indexOrName)`](#c_addonsisaddonloadableindexorname)
  - [`C_AddOns.GetAddOnSecurity(indexOrName)`](#c_addonsgetaddonsecurityindexorname)
  - [`C_AddOns.DoesAddOnExist(indexOrName)`](#c_addonsdoesaddonexistindexorname)
- [Combat](#combat)
  - [`InCombatLockdown()`](#incombatlockdown)
- [Input](#input)
  - [`IsLeftShiftKeyDown()` / `IsRightShiftKeyDown()`](#isleftshiftkeydown--isrightshiftkeydown)
  - [`IsLeftControlKeyDown()` / `IsRightControlKeyDown()`](#isleftcontrolkeydown--isrightcontrolkeydown)
  - [`IsLeftAltKeyDown()` / `IsRightAltKeyDown()`](#isleftaltkeydown--isrightaltkeydown)
  - [`IsModifierKeyDown()`](#ismodifierkeydown)
  - [`MODIFIER_STATE_CHANGED` event](#modifier_state_changed-event)
- [Talent](#talent)
  - [`GetTalentSpellID(tabIndex, talentIndex, [rank])`](#gettalentspellidtabindex-talentindex-rank)
  - [`GetTalentIDByIndex(tabIndex, talentIndex)`](#gettalentidbyindextabindex-talentindex)
- [Time](#time)
  - [`GetServerTime()`](#getservertime)
- [Events](#events)
  - [`C_EventUtils.IsEventValid(eventName)`](#c_eventutilsiseventvalideventname)
- [Globals](#globals)
  - [`CLASSIC_API_VERSION`](#classic_api_version)
  - [`LE_EXPANSION_*`](#le_expansion_)
  - [`LE_ITEM_QUALITY_*`](#le_item_quality_)
  - [`LE_UNIT_STAT_*`](#le_unit_stat_)

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

### `IsPassiveSpell(spellID)` / `IsPassiveSpell(slot, bookType)`

Returns `true` if the spell is passive (no cast bar — applies its effect
as soon as it's learned/equipped), `false` otherwise. Returns `nil` for
invalid spell IDs / out-of-range slots.

Reads `Attributes` bit 6 (`SPELL_ATTR_PASSIVE = 0x40`) directly off the
`Spell.dbc` record. Used by aura libraries and talent code to decide
whether a spell needs cast-bar tracking.

```lua
IsPassiveSpell(6603)   -- true  (Auto Attack)
IsPassiveSpell(133)    -- false (Fireball)

-- spellbook overload mirrors GetSpellInfo's
IsPassiveSpell(1, "spell")   -- true/false depending on slot 1
```

Equivalent to the function of the same name introduced in 3.0. Modern
WoW renamed it to [`C_Spell.IsSpellPassive(spellID)`](#c_spellisspellpassivespellid)
(word order flipped) when it moved into the `C_Spell` namespace; we
ship both forms.

### `C_Spell.IsSpellPassive(spellID)`

Modern table-namespace form of [`IsPassiveSpell`](#ispassivespellspellid--ispassivespellslot-booktype).
Same return semantics, but takes a spellID only — `C_Spell.*` calls
don't accept the older spellbook slot + bookType shape.

```lua
C_Spell.IsSpellPassive(6603)   -- true (Auto Attack)
```

Equivalent to the function of the same name introduced in 10.0.

### `IsPlayerSpell(spellID)`

Returns `true` if the player currently knows the given spellID, `false`
otherwise. Covers everything granted by `SMSG_LEARNED_SPELL`:

- Trained class abilities (the obvious case)
- Racial abilities
- Talent passives the player has invested in
- **Profession recipes** — including ones from vendors / discovered
  via tradeskill — without needing to have the trade skill window open
- Anything else the engine considers "known"

```lua
IsPlayerSpell(133)     -- true if you have Fireball
IsPlayerSpell(2963)    -- true for a tailor who knows Bolt of Linen
                       --   (works without opening the Tailoring window)
```

> **Only the current rank counts.** For ranked spells (passives,
> trained ranks), only the spellID of the **player's current rank**
> returns `true` — lower-rank IDs return `false` even though the player
> conceptually "has" them. Matches the same semantic Classic Era 1.15.x
> uses: a player with 5/5 Unbreakable Will sees `IsPlayerSpell(14791)`
> (rank 5) as true but rank 4 / rank 3 / etc. as false.
>
> This is by design — the engine's spell-knowledge bitmap stores one
> bit per spellID, and the highest-rank spellID is the one set when
> you train up. To check "do I have at least rank N", use the
> rank-N spellID specifically.

Reads a single bit from the engine's spell-knowledge bitmap at
`[0x00B710FC]` — `(bitmap[spellID >> 5] & (1 << (spellID & 31))) != 0`.
The same lookup the engine itself does internally. No spellbook walk,
no talent walk, no profession-window dependency.

Equivalent to the function of the same name introduced in 5.0.

### `IsSpellKnown(spellID, [isPet])`

Returns `true` if the given spellID is currently in the player's (or
pet's) spellbook arrays — the same source the in-game spellbook UI
displays. **Strict semantics**: only counts spells that have a
spellbook button. `isPet` defaults to `false`.

```lua
IsSpellKnown(2050)         -- true if Lesser Heal is trained
IsSpellKnown(2649, true)   -- true if your hunter pet has Growl
IsSpellKnown(133)          -- false on a Priest (Fireball is a Mage spell)
```

> **Not the same as `IsPlayerSpell`.** Modern WoW deliberately splits
> these two: `IsSpellKnown` is the strict "in the spellbook UI" check,
> `IsPlayerSpell` is the broad "any kind of known" query. The split
> matters because:
>
> | Spell type | `IsPlayerSpell` | `IsSpellKnown` |
> |---|---|---|
> | Trained class ability (Fireball) | `true` | `true` |
> | Active talent grant (Mortal Strike) | `true` | `true` |
> | Passive talent (Wand Spec, Imp Fireball) | `true` | **`false`** |
> | Profession recipe (Bolt of Linen) | `true` | **`false`** |
>
> Use `IsPlayerSpell` for "do I have access to this effect at all";
> use `IsSpellKnown` for "is this in my spellbook so I can put it on
> an action bar".

Implementation walks `VAR_PLAYER_SPELLBOOK` (`0x00B700F0`) when
`isPet=false` or `VAR_PET_SPELLBOOK` (`0x00B6F098`) when `isPet=true`.
Verified to match 3.3.5's `Script_IsSpellKnown` semantics — that
function does the same spellbook walk in its inner helper at
`0x0053B4E0` (player array `[0x00BE6D88]`, pet array `[0x00BE7D98]`,
same shape just different addresses).

Equivalent to the function of the same name introduced in 3.0.

### `IsUsableSpell(spell)` / `IsUsableSpell(slot, bookType)`

Returns `(usable, noMana)` for a spell, matching the modern
3.0+ signature. Returns `(1, nil)` when the spell is castable,
`(nil, 1)` when only mana is preventing it, `(nil, nil)` for any
other reason (unknown spell, dead, etc.). Matches the 1.12-style
`1`/`nil` return convention used by the existing
`Script_IsUsableAction`.

Two arg shapes accepted:

- **`IsUsableSpell(spellID)`** — direct spellID lookup.
- **`IsUsableSpell(slot, bookType)`** — `bookType` is `"spell"`
  (player) or `"pet"`. Resolves to a spellID via the same engine
  spellbook arrays `GetSpellInfo`/`GetSpellLink` walk.

```lua
IsUsableSpell(133)           -- Fireball: 1, nil if you have mana, nil, 1 if not
IsUsableSpell(1, "spell")    -- player spellbook slot 1
```

> **What this function checks:**
>
> 1. Player knows the spell (engine's spell-knowledge bitmap —
>    covers trained class abilities, talent passives, racials,
>    profession recipes).
> 2. Player is alive (HEALTH > 0).
> 3. Spell is not on cooldown (engine's per-spell cooldown helper).
> 4. Player has enough of the spell's power type for the base cost
>    (mana / rage / focus / energy / happiness) — *only* this
>    failure sets `noMana=true`.
> 5. Player has all required reagents in bags (Spell.dbc
>    Reagent[8] / ReagentCount[8]).
>
> **What this function doesn't check** (different concerns or
> post-vanilla concepts): silence, GCD, stance/form, range, target
> type, line-of-sight, casting state.
>
> Verified empirically on Turtle WoW for the mana branch: Renew rank
> 3 (cost 105) is reported usable at 144 mana and unusable at 39
> mana, transitioning at exactly the cost boundary. Cooldown and
> reagent checks ship in the same implementation but haven't been
> exercised in-game; if you find an inconsistency, the reagent
> offsets (+0x110 / +0x130) and cooldown helper (`0x006E2EA0`) are
> the components to verify.

Equivalent to the function of the same name introduced in 3.0.

### `C_Spell.IsSpellUsable(spellID)`

Modern table-namespace form. Same logic as
[`IsUsableSpell(spellID)`](#isusablespellspell--isusablespellslot-booktype)
but returns proper booleans (`isUsable`, `insufficientPower`) per
the `C_Spell.*` convention rather than `1`/`nil` pairs.

```lua
local usable, noMana = C_Spell.IsSpellUsable(133)
-- usable=true, noMana=false  → cast it
-- usable=false, noMana=true  → drink up
-- usable=false, noMana=false → unknown spell, dead, or other block
```

Equivalent to the function of the same name introduced in 10.x.

### `GetSpellSchool(spellID)`

Returns `(schoolID, schoolName)` for any spell — known to the player
or not. `schoolID` is 1-based; `schoolName` is the locale-independent
English name.

| schoolID | schoolName |
|---------:|------------|
| 1 | `"Physical"` |
| 2 | `"Holy"` |
| 3 | `"Fire"` |
| 4 | `"Nature"` |
| 5 | `"Frost"` |
| 6 | `"Shadow"` |
| 7 | `"Arcane"` |

Reads directly from `Spell.dbc` record `+0x04`. Vanilla 1.12 stores
School as a single 0-based integer (no multi-school `SchoolMask`
combinations — that's a TBC+ thing), so a spell belongs to exactly
one school.

Returns `nil` for invalid spellIDs (out of range or unpopulated
`Spell.dbc` slot).

```lua
local id, name = GetSpellSchool(133)   -- 3, "Fire"   (Fireball)
local id, name = GetSpellSchool(116)   -- 5, "Frost"  (Frostbolt)
local id, name = GetSpellSchool(635)   -- 2, "Holy"   (Holy Light)
```

Useful for combat-log breakdown addons, dispel-eligibility checks,
resistance-aware aura libraries, and damage-meter school tagging.
Previously addons either maintained hardcoded `spellID → school`
tables or scanned tooltips for the first-line color tag.

## GameTooltip

### `GameTooltip:SetItemByID(itemID)`

Modern method that renders an item tooltip from just an itemID. The
1.12 workaround was constructing an item hyperlink and calling
`SetHyperlink` — `tooltip:SetHyperlink("item:" .. id .. ":0:0:0:0:0:0:0")`
— which works but forces every caller to know the hyperlink format.

```lua
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
GameTooltip:SetItemByID(6948)  -- Hearthstone
GameTooltip:Show()
```

Implementation: snprintf the hyperlink string and dispatch to the
existing `Script_GameTooltip_SetHyperlink` (registry slot 12).

> **Item cache caveat.** 1.12 lazy-loads item data into the
> client-side cache at `0x00C0E2A0` — the cache is fed by
> `SMSG_ITEM_QUERY_SINGLE_RESPONSE` only when the player encounters
> an item. For an itemID the player has never seen, the tooltip
> renders only the name; full data appears once the cache is warm.
>
> The fix is to ensure the item is cached before opening the tooltip:
>
> ```lua
> if C_Item.IsItemDataCachedByID(itemID) then
>     GameTooltip:SetItemByID(itemID)
> else
>     C_Item.RequestLoadItemDataByID(itemID)
>     -- register for ITEM_DATA_LOAD_RESULT(loadedItemID, success)
>     -- and call SetItemByID once the matching itemID arrives
> end
> ```
>
> This caching behavior matches what `C_Item.GetItemInfoInstant`
> documents — same underlying cache. Modern WoW (5.0+) has the same
> caveat, just with `C_Item.RequestLoadItemData(itemLocation)` /
> `Item:OnItemLoad`-style continuation.

Equivalent to the function of the same name introduced in 8.0.

### `GameTooltip:SetUnitAura(unit, index, [filter])`

Modern unified-aura method. 1.12 splits this into `SetUnitBuff` and
`SetUnitDebuff`; we dispatch to the right one based on the `filter`
string (`"HARMFUL"` → `SetUnitDebuff`, anything else → `SetUnitBuff`).
`filter` defaults to helpful when omitted, matching modern.

```lua
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
GameTooltip:SetUnitAura("player", 1, "HELPFUL")  -- first buff
GameTooltip:SetUnitAura("player", 1, "HARMFUL")  -- first debuff
GameTooltip:SetUnitAura("player", 1)              -- defaults to HELPFUL
GameTooltip:Show()
```

Pure dispatcher — no engine changes; the underlying logic is whatever
1.12's `SetUnitBuff` / `SetUnitDebuff` already does. Just lets you use
the modern call shape (which most aura libraries backport from)
without conditionally splitting on filter.

Equivalent to the function of the same name introduced in 6.0.

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

### `GameTooltip:SetTalentByID(talentID)`

Renders a tooltip for the talent identified by `Talent.dbc` primary
key — the natural pair to
[`GetTalentIDByIndex`](#gettalentidbyindextabindex-talentindex).
Works for any class's talents, not just the player's.

Two-tier resolution:

| Tier | When it applies | Tooltip rendered |
|------|-----------------|------------------|
| Player class (rich) | `talentID` belongs to one of the player's loaded tabs | Full talent tooltip — name, "Rank N/M", description, prereqs, "click to learn" prompts |
| Cross-class (fallback) | `talentID` is from another class | Spell tooltip for the talent's rank-1 spellID — name, cast time, range, mana cost, description |

The fallback exists because vanilla 1.12 only loads the local
player's class talent data into the engine's per-player TabInfo
arrays. For other classes, we look up the talent in `Talent.dbc`
directly and dispatch the rank-1 spell tooltip — functionally
"what does this talent do?" without the rank counter. Modern WoW
adds talent name and "Rank 0/N" decorations on top of the spell
description for cross-class; we don't replicate that here yet.

Silent no-op (no tooltip change) when:

- `talentID` doesn't match any record in `Talent.dbc`
- `talentID` is `nil`, non-numeric, or non-positive

```lua
-- Player's own class — rich tooltip
local talentID = GetTalentIDByIndex(1, 9)        -- player's tab 1, talent 9
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
GameTooltip:SetTalentByID(talentID)
GameTooltip:Show()

-- Other class — spell tooltip for the talent's primary ability
GameTooltip:SetTalentByID(2065)                   -- works regardless of player class
GameTooltip:Show()
```

Equivalent to the function of the same name introduced in 5.0.

### `GameTooltip:SetInventoryItemByID(itemID)`

Renders the tooltip for the **equipped instance** of `itemID` —
walks character-pane slots 1..19, finds the matching item, and
shows it with its actual enchants, random-suffix stats, and
broken/locked state. Distinct from
[`SetItemByID`](#gametooltipsetitembyiditemid), which shows the
clean ItemSparse data with no instance-specific decorations.

For example, on a pair of boots with a run-speed enchant equipped:

| Method | Renders |
|--------|---------|
| `SetItemByID(<bootsID>)` | Base boots tooltip — name, armor, durability, level req. **No enchant.** |
| `SetInventoryItemByID(<bootsID>)` | Same plus `Enchanted: Minor Speed` and any random-suffix lines. |

Silent no-op if the item isn't currently equipped — fall back to
`SetItemByID` for unworn items, or check via
[`C_Item.IsEquippedItem`](#c_itemisequippeditemitem) first.

When the player has duplicates of the same itemID equipped
(matched MH/OH weapons, identical rings, identical trinkets), the
**lower-numbered slot wins** — MAINHAND before OFFHAND, FINGER1
before FINGER2, TRINKET1 before TRINKET2. Matches modern client
behavior (verified empirically).

```lua
local _, _, _, _, _, _, _, _, _, _, _, _, _, link = GetItemInfo(itemID)
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
if C_Item.IsEquippedItem(itemID) then
    GameTooltip:SetInventoryItemByID(itemID)  -- shows enchants/suffix
else
    GameTooltip:SetItemByID(itemID)            -- shows base stats
end
GameTooltip:Show()
```

Equivalent to the function of the same name introduced in 8.0.

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

Returns the same eleven values as `GetFactionInfo(factionIndex)`, keyed by
factionID instead of displayed index:

```
name, description, standingID, barMin, barMax, barValue,
atWarWith, canToggleAtWar, isHeader, isCollapsed, hasRep
```

Works for any factionID present in `Faction.dbc`, not just factions the
player has rep with:

- **In the player's reputation list** — full data, identical to
  `GetFactionInfo(displayedIndex)`.
- **Not in the reputation list** — name and description from `Faction.dbc`,
  Neutral defaults for the rep fields: `standingID = 4`, `barMin = 0`,
  `barMax = 3000`, `barValue = 0`, all flags `nil`. Matches what 3.3.5's
  `GetFactionInfoByID` returns for unencountered factions.
- **Invalid factionID** (out of range or empty DBC slot) — `nil`.

```lua
local name, _, standing = GetFactionInfoByID(69)  -- Darnassus
-- name = "Darnassus", standing = 5 (Friendly), etc. (encountered)

local name = GetFactionInfoByID(574)  -- Caer Darrow (faction that can't be encountered in standard Vanilla)
-- name = "Caer Darrow" — works even if you never had rep with it
```

Equivalent to the function of the same name introduced in 3.0.

### `GetFactionParentID(factionID)`

Returns the parent factionID for a faction in a hierarchy (e.g.
Stormwind's parent is Alliance Forces; The Defilers's parent is
Horde Forces). Returns `0` for top-level factions with no parent,
or `nil` for invalid factionIDs.

```lua
GetFactionParentID(72)     -- 469 (Stormwind → Alliance)
GetFactionParentID(469)    -- 0   (Alliance is top-level)
GetFactionParentID(99999)  -- nil
```

Modern WoW returns this as the 13th value of `GetFactionInfoByID`;
we expose it as its own getter since 1.12's `GetFactionInfo` doesn't
have the slot.

Reads `Faction.dbc` `ParentFactionID` at record `+0x48` directly —
no displayed-list dependency, works for any faction in the DBC
regardless of whether the player has rep with it.

Equivalent to the function of the same name introduced in 3.0 (as
the 13th return of `GetFactionInfoByID`).

### `C_Reputation.SetWatchedFactionByID(factionID)`

Sets the faction shown above the XP bar by ID. The vanilla
`SetWatchedFactionIndex(displayedIndex)` accepts only a 1-based
displayed-list index, which forces addons to walk the index list
themselves. This wrapper takes a `factionID` directly.

- `factionID > 0` — sets the watched faction. Works even for
  factions the player hasn't yet encountered (the rep bar will
  show nothing until the player gains rep, then displays
  normally).
- `factionID == 0` — clears the watched faction.
- `factionID < 0` — silent no-op.

Returns nothing. The engine's UI / event machinery refreshes
automatically — `GetWatchedFactionInfo()` reflects the new state
on the next call.

```lua
C_Reputation.SetWatchedFactionByID(72)  -- Stormwind
print(GetWatchedFactionInfo())          -- "Stormwind", ...

C_Reputation.SetWatchedFactionByID(0)   -- clear
print(GetWatchedFactionInfo())          -- (empty)
```

Implementation calls the engine's inner watched-faction setter
directly, bypassing `Script_SetWatchedFactionIndex`'s
displayed-index round-trip.

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

### `GetInventoryItemDurability(invSlot)`

Returns `(current, maximum)` durability for the player's equipped item
at `invSlot` (1-based, character-pane slots 1..19), or nothing if the
slot is empty or the item has no durability concept (rings, trinkets,
necks, backs, shirts, tabards, etc.).

```
current, maximum = GetInventoryItemDurability(invSlot)
```

```lua
local cur, max = GetInventoryItemDurability(INVSLOT_CHEST)
if cur then
    -- cur, max are positive integers (e.g. 65, 65 for full chest)
end

-- Items without durability return nothing — both locals are nil:
local cur, max = GetInventoryItemDurability(INVSLOT_FINGER1)
-- cur == nil, max == nil
```

> **Player-only.** Matches modern API: 3.3.5+ `GetInventoryItemDurability`
> takes only the slot, no unit token. Inspect targets / party members'
> equipment durability isn't broadcast in 1.12, so we couldn't expose it
> for other units even if we wanted to.

> **`(0, max)` vs nothing.** Items that have a durability concept but
> are currently broken (`current == 0`, `max > 0`) still return
> `(0, max)`. The "nothing" return is reserved for items that have no
> durability fields populated at all — `max` is the discriminator,
> matching the engine's own `GetInventoryItemBroken` logic.

Reads ITEM_FIELD_DURABILITY (+0xA0) and ITEM_FIELD_MAXDURABILITY
(+0xA4) directly off the CGItem's m_objectFields descriptor at `+0x114`
— same descriptor [`C_Item.IsBound`](#c_itemisbounditemlocation) reads
FLAGS from. No DBC indirection.

Equivalent to the function of the same name introduced in 3.0.

### `C_Item.GetItemFamily(item)`

Returns the BagFamily bitmask for an item — i.e., what kind of
specialty bag it belongs in. `0` means "general-purpose" (no specialty
bag preference). Returns `nil` if the item isn't in the cache.

```
familyBitmask = C_Item.GetItemFamily(item)
```

`item` accepts a numeric `itemID` or a string containing `"item:NNN"`
(the bare shorthand and full chat links both parse correctly), same as
[`C_Item.GetItemInfoInstant`](#c_itemgetiteminfoinstantitem).

```lua
C_Item.GetItemFamily(2512)   -- 1   (Wooden Arrow → quiver-family)
C_Item.GetItemFamily(2516)   -- 2   (Light Shot → ammo pouch-family)
C_Item.GetItemFamily(6265)   -- 4   (Soul Shard → soul bag-family)
C_Item.GetItemFamily(2447)   -- 32  (Peacebloom → herb bag-family)
C_Item.GetItemFamily(6948)   -- 0   (Hearthstone → general-purpose)
```

Bitmask values match modern WoW's encoding (`1 << (familyID - 1)`):

| Bit | Value | Family |
|-----|-------|--------|
| 0   | 1     | Quiver (arrows) |
| 1   | 2     | Ammo Pouch (bullets) |
| 2   | 4     | Soul Shard Bag |
| 5   | 32    | Herb Bag |
| 6   | 64    | Enchanting Bag (modern) |

> **Encoding deviation under the hood.** 1.12 actually stores the raw
> 1-based BagFamily ID (`arrow=1, bullet=2, soul shard=3, herb=6, …`).
> Modern WoW (Wrath+) flipped to the bitmask form for the same field.
> We convert on the way out via `bitmask = 1 << (rawID - 1)`, so
> callers backporting from modern see the encoding they expect — addons
> can `band(family, FAMILY_BAG_HERB_BAG)` directly.

> **Item-level data is reliable; bag-level is sparse.** Individual
> loot items (arrows, bullets, shards, herbs) carry the field
> reliably, so `C_Item.GetItemFamily` works for the auto-routing use
> case. **Bag containers** themselves (Soul Pouch, Enchanting Bag,
> Herb Pouch, etc.) leave the field at 0 in vanilla server data — at
> least on Turtle WoW. If you need to know "is this bag specialty",
> read the bag's `subClass` via
> [`C_Item.GetItemInfoInstant`](#c_itemgetiteminfoinstantitem)
> instead. The Quiver class is fully populated; everything else needs
> the subClass fallback.

> **`nil` vs `0`.** Modern WoW returns `0` for items the cache lookup
> fails on; we return `nil` so callers can distinguish "item not
> cached, retry after the event lands" from "item exists but has no
> family preference." Both are safe to treat as `0` for routing-logic
> purposes; the distinction helps debugging.

> **Auto-warmup on cache miss.** First call for an uncached item
> returns `nil` AND triggers the engine's cache fill in the
> background. Listen for `GET_ITEM_INFO_RECEIVED(itemID, success)` and
> retry once the matching `itemID` arrives. This matches Classic Era
> 1.15's observed behavior of "nil first call, value second call." We
> diverge from 1.15 in one detail: 1.15's `GetItemFamily` doesn't fire
> any event when the cache lands (silent fill), whereas we fire
> `GET_ITEM_INFO_RECEIVED` to stay consistent with our other implicit-
> warmup paths (`GetItemInfo`, `SetItemByID`). Addons that already
> listen for that event get the notification for free.

Equivalent to the legacy global `GetItemFamily` (since 3.0) and the
modern `C_Item.GetItemFamily` introduced in 10.x.

### `C_Item.GetItemCount(itemInfo, [includeBank], [includeUses])`

Returns the player's total count of `itemInfo` across bags (and
optionally bank slots).

```
count = C_Item.GetItemCount(itemInfo [, includeBank [, includeUses]])
```

- `itemInfo` — numeric `itemID` or string containing `"item:NNN"`
  (full chat links work). Item names are NOT accepted (vanilla has
  no name → ID resolver).
- `includeBank` *(optional, default false)* — also walk bank slots
  (bag `-1` for the main bank, bags `5..10` for bank-bag slots).
- `includeUses` *(optional, default false)* — **accepted but
  currently ignored.** Returns the same value as `false` regardless.
  See note below.

```lua
local n = C_Item.GetItemCount(2589)               -- Linen Cloth in bags
local n = C_Item.GetItemCount(2589, true)         -- bags + bank
local n = C_Item.GetItemCount("item:2589")        -- string form works too
```

> **`includeUses` not yet supported.** Modern semantics: when `true`,
> multiplies each match by the item's spell-charges count (a stack
> of 5 wands × 50 charges each → 250). Implementing this needs the
> `ITEM_FIELD_SPELL_CHARGES` descriptor offset, which hasn't been
> verified empirically yet. The arg is accepted for API parity so
> backported code doesn't break, but currently both `true` and
> `false` produce the same number (sum of stack counts). Track
> [TODO #28] for charges support.

> **Equipped items don't count.** Matches modern API. A wand in your
> ranged slot won't be counted; only items in bags and (optionally)
> bank.

> **Bank works cold — no banker visit required.** The 1.12 server
> sends bank inventory at login alongside the rest of the player's
> data; only the engine's own `GetItemBySlot` gates bank slots until
> the window opens. We bypass that gate by reading the GUID array
> directly out of the player invMgr and resolving each entry via the
> engine's object resolver — same path `GetItemBySlot` would take
> internally if the gate let us through. Counts are correct from
> session start.

Walks via the same `Item::Location::ResolveBag` chain
[`C_Container.GetContainerItemID`](#c_containergetcontaineritemidbagindex-slotindex)
uses. Slot counts come from the engine's `Script_GetContainerNumSlots`
so custom server bag sizes work transparently. Stack counts read
directly off the CGItem's m_objectFields descriptor at +0x20
(`ITEM_FIELD_STACK_COUNT`, verified by decoding
`Script_GetContainerItemInfo` at `0x004F9670`).

Equivalent to the legacy global `GetItemCount` (since 3.0) and the
modern `C_Item.GetItemCount` introduced in 10.x.

### `GetItemIcon(itemID)` / `C_Item.GetItemIcon(itemLocation)` / `C_Item.GetItemIconByID(item)`

Three accessors that all return the icon path for an item, differing only in
how you address the item:

| Function                             | Input                     |
|--------------------------------------|---------------------------|
| `GetItemIcon(itemID)`                | numeric itemID (global)   |
| `C_Item.GetItemIcon(itemLocation)`   | `{equipmentSlotIndex=N}` or `{bagID=B, slotIndex=S}` |
| `C_Item.GetItemIconByID(item)`       | numeric itemID OR `"item:NNN"` string (full chat links work) |

All three return the icon path string (e.g. `"Interface\\Icons\\INV_Misc_Rune_01"`),
or `nil` if the item isn't in the client-side cache, the slot is empty, or
the display info record is missing. The path is suitable for direct use
with `texture:SetTexture(...)`.

```lua
local path = GetItemIcon(6948)                                -- "Interface\\Icons\\INV_Misc_Rune_01"
local path = C_Item.GetItemIcon({equipmentSlotIndex = INVSLOT_HEAD})
local path = C_Item.GetItemIconByID("|cff...|Hitem:6948:0:0:0|h[Hearthstone]|h|r")
```

> **`iconID`-vs-path deviation.** Modern WoW returns these as a
> `fileID:number` (specifically `iconFileID` in Classic Era 1.15.x). 1.12
> has no fileID system — same situation as
> [`C_Spell.GetSpellTexture`](#c_spellgetspelltexturespellid) and
> [`C_Spell.GetSpellInfo`](#c_spellgetspellinfospellid)'s `iconID` field.
> We surface the path string everywhere for consistency.

Equivalent to the legacy global `GetItemIcon` (since 1.x) and the
`C_Item.GetItemIcon` / `GetItemIconByID` family added in 10.x.

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

### `OffhandHasWeapon()`

Returns `true` if the player has a one-handed weapon (or off-hand-only
weapon) equipped in the off-hand slot, `false` otherwise. Used by
dual-wield checks and any addon backporting modern weapon-equipment
logic.

Returns `false` for:

- Empty off-hand
- Shields (`INVTYPE_SHIELD`)
- Held items — tomes, orbs, librams (`INVTYPE_HOLDABLE`)
- Off-hand item data not yet cached (typically only on first login,
  before the engine has the item record; warms up after a peek)

Returns `true` for `INVTYPE_WEAPON` (any one-handed weapon — sword,
axe, mace, dagger, fist) and `INVTYPE_WEAPONOFFHAND` (off-hand-only
weapons). Two-handers occupy the main-hand slot exclusively, so they
never apply.

```lua
if OffhandHasWeapon() then
    -- Apply mainhand+offhand poison, refresh dual-wield rotation, etc.
end
```

Equivalent to the function of the same name introduced in 3.0.

### `C_Item.IsEquippedItem(item)`

Returns `true` if `item` is currently equipped in any of the 19
character-pane slots, `false` otherwise. Walks slots 1..19 in order and
short-circuits on the first match.

`item` accepts the same shapes as `GetItemInfo`:

| Form | Example | Match strategy |
|------|---------|----------------|
| itemID | `2589` | exact `itemID` equality |
| bare link | `"item:2589:0:0:0"` | parses the first numeric field |
| chat link | `\124cffffffff\124Hitem:2589:0:0:0\124h[Linen Cloth]\124h\124r` | extracts itemID after `\124Hitem:` |
| name | `"Linen Cloth"` | case-insensitive match against the cached `m_name[0]` of each equipped item |

Returns `false` (no Lua error) for invalid input — `nil`, empty string,
or a string that doesn't parse as any of the above.

The name-match path requires the candidate equipped item to be in the
client item cache; equipped items are always cached, so this is a
non-issue in practice.

```lua
if C_Item.IsEquippedItem(2589) then
    -- Linen Cloth is equipped (silly example — pick a wearable item)
end

-- From a chat link click:
if C_Item.IsEquippedItem(itemLink) then ...

-- By localized name:
if C_Item.IsEquippedItem("Thunderfury, Blessed Blade of the Windseeker") then ...
```

Equivalent to the function of the same name introduced in 3.x.

### `C_Item.EquipItemByName(itemInfo [, dstSlot])`

Finds the first item in the player's bags matching `itemInfo` and
equips it. With `dstSlot` (a 1-based character-pane slot, 1..19),
equips to that specific slot; without, the engine auto-picks based on
the item's inventory type.

`itemInfo` accepts the same shapes as
[`C_Item.IsEquippedItem`](#c_itemisequippeditemitem) — itemID number,
bare `"item:N"` string, full chat link, or a localized item name. Name
matches are case-insensitive against the cached `m_name[0]`.

Returns nothing. Silently no-ops when:

- the input is `nil`, an empty string, or otherwise unparseable
- no matching item is in bags (already-equipped items aren't moved —
  matches modern API behavior)
- the engine refuses the equip — combat, locked item, type mismatch
  with `dstSlot`, locked equipment slot, etc.

Walks bags 0..4 in order and stops on the first match, then dispatches
to the engine's `PickupContainerItem` + `EquipCursorItem` /
`AutoEquipCursorItem` path. The dispatch goes through the same secure-
action flow a direct Lua call would, so combat lockdown and the
standard equip-failure error messages behave identically.

**Cursor-state guard.** Because the dispatch path uses
`PickupContainerItem` internally, an item already on the cursor would
get swapped into the source bag slot. To avoid that side effect, the
function checks `CursorHasItem()` first and refuses to operate (no-op
return) when the cursor is non-empty. Callers must drop the cursor
before calling — `ClearCursor()` or completing the held action.

Background: vanilla 1.12 has a CMSG_SWAP_INV_ITEM (0x10D) packet
builder that would let us bypass the cursor entirely (matching 2.4.3's
`ItemMgr::EquipByGUID` semantic), but the only clean wrapper for it
in the binary is dead code with zero callers. Other 0x10D build sites
read from the engine's internal cursor-tracking globals, so they
can't be called safely from outside that state machine. Refusing the
swap when cursor is dirty is the safest available option until a
working bypass is found.

```lua
-- By itemID, auto-pick slot:
C_Item.EquipItemByName(2589)

-- By name, force into off-hand (slot 17):
C_Item.EquipItemByName("Linen Cloth", 17)

-- From a chat link:
C_Item.EquipItemByName(itemLink)
```

Equivalent to the function of the same name (as a global) introduced
in 3.x. The `C_Item.` namespace placement matches Dragonflight-era
addon conventions.

## Container

### `C_Container.GetContainerItemID(bagIndex, slotIndex)`

Returns the itemID at the given bag/slot, or `nil` if the slot is empty
or the indices are out of range. Modern positional-arg form of the same
lookup `C_Item.GetItemID({bagID=B, slotIndex=S})` performs.

- `bagIndex = 0` — the player's main backpack.
- `bagIndex = 1..4` — the player's equipped bag slots.
- `slotIndex` — 1-based, capped at the bag's actual slot count (the
  engine's `PackBagSlot` rejects out-of-range slots and returns nil
  cleanly).

```lua
for slot = 1, 16 do
    local id = C_Container.GetContainerItemID(0, slot)
    if id then
        local _, type, subtype = C_Item.GetItemInfoInstant(id)
        -- ...
    end
end
```

### `C_Container.GetContainerItemDurability(containerIndex, slotIndex)`

Bag/bank variant of [`GetInventoryItemDurability`](#getinventoryitemdurabilityinvslot).
Same `(current, maximum)` return shape and the same "nothing for items
without durability" rule.

```
current, maximum = C_Container.GetContainerItemDurability(containerIndex, slotIndex)
```

- `containerIndex = 0` — the player's main backpack.
- `containerIndex = 1..4` — the player's equipped bag slots.
- `containerIndex = -1, 5..11` — bank-frame slots, only addressable
  while the bank window is open.
- `slotIndex` — 1-based, capped at the bag's actual slot count.

```lua
for slot = 1, GetContainerNumSlots(0) do
    local cur, max = C_Container.GetContainerItemDurability(0, slot)
    if cur then
        -- item at backpack slot has durability
    end
end
```

Goes through the same bag-resolve chain
[`C_Container.GetContainerItemID`](#c_containergetcontaineritemidbagindex-slotindex)
uses (engine's `PackBagSlot` → `GetItemBySlot`), then reads the same
durability fields off the descriptor.

Equivalent to the function of the same name introduced in 10.0.

### `C_Container.GetContainerNumFreeSlots(bagID)`

Returns the number of empty slots in the given bag, plus the bag's
`BagFamily` bitfield (the type of items the bag is restricted to).

```
numberOfFreeSlots, bagType = C_Container.GetContainerNumFreeSlots(bagID)
```

- `bagID = 0` — the player's main backpack. Always reports
  `(freeCount, 0)` — the backpack is unfamilied (general-purpose).
- `bagID = 1..4` — the player's equipped bag slots. Slot count and
  bagType come from the bag item's cached `ItemStats_C` record —
  `m_containerSlots` and `m_bagFamily` respectively. If no bag is
  equipped (or the item somehow isn't cached): `(0, 0)`.
- Other `bagID` values (bank, keyring, out-of-range): `(0, 0)`. Always
  returns two values, never nil.

```lua
local free, bagType = C_Container.GetContainerNumFreeSlots(0)
-- free = number of empty slots in backpack, bagType = 0

for bag = 0, 4 do
    local free = C_Container.GetContainerNumFreeSlots(bag)
    -- ...
end
```

> **Vanilla `bagType` is sparse.** On Turtle WoW (1.12.1), only Light
> Quiver actually carries a populated BagFamily field among the bags
> we tested. Soul Pouch, Enchanting Bag, Herb Pouch, etc. all return
> 0 here even though their classification clearly implies a family —
> the vanilla server data simply doesn't fill the field for those.
> For bag-category discrimination, prefer reading `(class, subClass)`
> from [`C_Item.GetItemInfoInstant`](#c_itemgetiteminfoinstantitem).
> The same field on **individual loot items** (arrows, bullets,
> shards, herbs) IS populated reliably — use
> [`C_Item.GetItemFamily`](#c_itemgetitemfamilyitem) for routing
> decisions.

> **The bitmask encoding matches modern.** `bagType` is the bit
> position (`1 << (familyID - 1)`), not the raw 1.12-stored familyID,
> so callers can bitwise-AND with itemFamily values from
> [`C_Item.GetItemFamily`](#c_itemgetitemfamilyitem) directly. We
> convert internally — see that function's notes for the
> encoding-shift story.

Implementation walks the bag using
[`Item::Location::ResolveBag`](#c_containergetcontaineritemidbagindex-slotindex)
internally and counts slots that resolve to a null `CGItem *` (i.e.,
empty). Cross-checked in-game against a manual
`C_Container.GetContainerItemID` walk; counts match.

Equivalent to the function of the same name introduced in 3.0.

### `C_Container.PlayerHasHearthstone()`

Returns the itemID of the hearthstone if one is in the player's bags,
or `nil` otherwise. Vanilla 1.12 only has a single hearthstone item
(itemID `6948`), so the return is always either `6948` or `nil`.

```
itemID = C_Container.PlayerHasHearthstone()
```

```lua
if C_Container.PlayerHasHearthstone() then
    -- player can hearth
end
```

> **Modern WoW returns the actual itemID found.** Modern clients
> recognize a list of hearthstone-equivalent toys (Garrison
> Hearthstone, Dalaran Hearthstone, etc.); whichever is found in bags
> is returned. None of those items exist in 1.12, so the original
> Hearthstone (`6948`) is the only possible result here. Code
> backporting from modern that does
> `local id = C_Container.PlayerHasHearthstone()` works without
> modification — `id` is just always `6948` when truthy.

Walks bags 0..4 via the same chain
[`C_Container.GetContainerItemID`](#c_containergetcontaineritemidbagindex-slotindex)
uses internally. Stops on first match.

Equivalent to the function of the same name introduced in 9.0.

### `C_Container.UseHearthstone()`

Locates the hearthstone in bags and uses it. Returns `true` if the
hearthstone was found and the use call dispatched, `false` if no
hearthstone is in bags.

```
used = C_Container.UseHearthstone()
```

```lua
if not C_Container.UseHearthstone() then
    print("No hearthstone in bags!")
end
```

> **`true` doesn't guarantee the cast started.** The return reflects
> "we had a hearthstone to try with and called the engine's
> `UseContainerItem` on it." Whether the cast actually starts depends
> on cooldown, combat, movement, etc. — same downstream rules as
> calling `UseContainerItem(bag, slot)` manually. If you need to
> know whether the hearth completed, listen for the appropriate cast
> events (`SPELLCAST_START` / `SPELLCAST_STOP`).

Internally locates the hearthstone with the same walk
`PlayerHasHearthstone` uses, then dispatches to the engine's existing
`Script_UseContainerItem` Lua C function with `(bagID, slot)` set up
on the stack. No new use-item logic is introduced — this is a
convenience wrapper.

Equivalent to the function of the same name introduced in 9.0.

## Class

### `FillLocalizedClassList(table [, isFemale])`

Fills the passed-in table with `[classToken] = localizedClassName`
pairs for every class in `ChrClasses.dbc`, and returns the same
table for chaining.

The table is mutated in place. Existing keys are overwritten;
unrelated keys are preserved.

```lua
local classes = FillLocalizedClassList({})
-- classes.WARRIOR = "Warrior"
-- classes.MAGE    = "Mage"
-- classes.PRIEST  = "Priest"
-- ...
```

Modern API supports an optional `isFemale` boolean to fetch female-
form names. Vanilla 1.12 has no separate female-name array in
`ChrClasses.dbc` — `Name[9]` (one localized string per locale)
sits exactly between offsets `+0x14` and `+0x38`, with the class
token immediately after, leaving no room. The arg is accepted for
signature parity but ignored; the same names are returned either way.
Most locales (English included) wouldn't differentiate the two
anyway, so callers won't typically notice.

Sparse class IDs (vanilla skips classID 6 — Death Knight didn't
exist yet — and a few others) have NULL records and are silently
skipped.

Equivalent to the function of the same name introduced in 3.0.

## Unit

### `UnitGUID(unit)`

Returns the unit's 64-bit GUID formatted as a hex string
`"0xHHHHHHHHLLLLLLLL"` (16 hex digits, hi dword first), or `nil` if the
resolved unit's GUID is empty (NULL pointer or all zeroes).

```lua
local guid = UnitGUID("player")  -- "0x0000000000000777" (low IDs are local-realm characters)
local guid = UnitGUID("target")  -- "0xF13000059A002553" (the F130... prefix tags creatures)
```

> **Vanilla format, not modern.** Vanilla GUIDs are plain 64-bit
> integers — there's no `"Player-RealmID-CharacterID"` /
> `"Creature-0-0-MapID-..."` prefix system that modern WoW uses
> (introduced in 6.0). Addons backporting modern GUID-parsing code
> will need to either accept the `"0x..."` form or extract the raw
> hex.

> **Errors on invalid unit tokens.** Same behavior as vanilla's other
> unit-token functions (`UnitAffectingCombat`, `UnitName`, etc.) —
> passing a string that doesn't match a known unit ID raises a Lua
> error rather than returning nil. Modern WoW silently returns nil;
> we match the engine's existing convention here. Unit tokens that
> resolve to "no current unit" (like `"target"` with nothing
> targeted) return nil cleanly via the GUID = 0 check.

### `UnitIsAFK(unit)`

Returns `true` if the unit is currently AFK (toggled via `/afk` or
auto-set after idle timeout). Works for any player-controlled unit
— local self, target, party*, raid*, inspect targets. NPCs always
return `false`.

```lua
UnitIsAFK("player")   -- true if you've /afk'd
UnitIsAFK("target")   -- true if the targeted player is AFK
UnitIsAFK("party1")   -- true if party member 1 is AFK
UnitIsAFK("npc")      -- always false
```

> **How it works under the hood.** Vanilla 1.12 doesn't broadcast
> PLAYER_FLAGS as a UpdateField (modern WoW does — that field was
> added 3.0+), but every nearby player's CGPlayer-side info struct
> at `[unit + 0xE68]` carries it at byte +0x08. Same struct the
> engine reads when rendering the `<AFK>` prefix above a player's
> head. Verified against the in-game nameplate behavior.

Equivalent to the function of the same name introduced in 3.0.

### `UnitIsDND(unit)`

Returns `true` if the unit is currently in DND mode ("Do Not Disturb",
toggled via `/dnd`). Same unit-coverage as `UnitIsAFK` — any
player-controlled unit, false for NPCs.

```lua
UnitIsDND("player")
UnitIsDND("target")
```

Equivalent to the function of the same name introduced in 3.0.

### `UnitIsFeignDeath(unit)`

Returns `true` if the unit is feigning death (Hunter's `Feign Death`).
Reads `UNIT_FIELD_FLAGS` bit 29 (`0x20000000`) from the broadcast
descriptor — works for any unit since UNIT_FIELD_FLAGS is broadcast
in object updates.

```lua
UnitIsFeignDeath("target")   -- true if a feigning hunter
```

Equivalent to the function of the same name introduced in 3.0.

### `UnitIsInMyGuild(unitOrName)`

Returns `1` if the unit/character shares the player's guild, `nil`
otherwise. Accepts either a unit token (`"player"`, `"target"`,
`"party1"`, etc.) or a literal character name (`"Bob"`), matching
3.3.5's `Script_UnitIsInMyGuild` (`0x0060C4B0` in the Frostmourne
client).

```lua
UnitIsInMyGuild("player")    -- 1 if you're in any guild
UnitIsInMyGuild("target")    -- 1 if your target is a guildmate
UnitIsInMyGuild("party1")    -- 1 if party member 1 is a guildmate
UnitIsInMyGuild("Bob")       -- 1 if there's a guildmate named Bob
```

Resolution strategy:

1. Calls `UnitName(input)` via `lua_pcall` to canonicalize the input.
   Tokens resolve cleanly; literal names hit the engine's "Unknown
   unit name" error which `pcall` swallows.
2. For valid tokens, attempts a fast direct comparison against the
   player's guild-key field at `[unit + 0xE68 + 0x0C]` — same field
   `GetGuildInfo` reads, populated immediately on guild join for the
   local player and for any synced player-controlled unit. No roster
   fetch needed for nearby/loaded units.
3. Falls back to walking the engine's guild roster array (same
   backing storage `GetGuildRosterInfo` reads) by name. Required for
   out-of-range party members, distant raid members, and literal
   name input — needs `GuildRoster()` to have been called and the
   server's response to have arrived.

Return convention matches 3.3.5: `1.0` / `nil`, not boolean. Both
work for `if UnitIsInMyGuild(x) then` checks.

The slow path reads the engine's full roster count
(`[0x00B73118]`, the same value `GetNumGuildMembers(true)` returns),
not the online-only count, so offline guildmates resolve too — the
"show offline" toggle doesn't affect lookup. The only requirement
is that `GuildRoster()` has been called and the server's
SMSG_GUILD_ROSTER response has arrived.

### `UnitIsPossessed(unit)`

Returns `true` if the unit is currently possessed (priest's `Mind
Control`, warlock's `Subjugate Demon`). Reads `UNIT_FIELD_FLAGS` bit
24 (`0x01000000`) — the standard vanilla `UNIT_FLAG_POSSESSED` per
emulator sources — directly off the unit's m_objectFields descriptor.
Works for any unit token since UNIT_FIELD_FLAGS is broadcast in
object updates.

```lua
UnitIsPossessed("target")   -- true if mind-controlled
```

Distinct from `UnitIsCharmed`: charm covers any charm-type effect
(including pets summoned via mob-charm spells), possess is the
specific spell-driven take-over effect modern WoW splits out.

## State

Player movement / visibility state queries that modern WoW exposes
as no-arg globals. 1.12 doesn't bind them to Lua despite the engine
tracking the underlying state — broadcast in UpdateFields for some
(mount, stealth visibility), local-only for others (falling,
swimming).

### `IsMounted()`

Returns `true` if the player is currently mounted, `false` otherwise.

Reads `UNIT_FIELD_MOUNTDISPLAYID` from the player's broadcast
descriptor. The field holds a creature display ID (the model the
engine renders under the player) when mounted, and `0` otherwise.

```lua
if not IsMounted() then
    CastSpellByName("Summon Dreadsteed")
end
```

Equivalent to the function of the same name introduced in 1.10.

### `IsStealthed()`

Returns `true` if the player is currently in Stealth (Rogue) or
Prowl (Druid), `false` otherwise.

Reads bit `0x02` of the player visibility byte at descriptor
`+0x17C` and AND-gates with `MountDisplayID == 0` to disambiguate
mount (which sets the same bit). Untested for Druid shapeshift
forms — if you find a false-positive there, file an issue and we'll
switch to walking the player's aura array for the actual stealth
spell.

```lua
if IsStealthed() then
    -- defer the spell that would break stealth
end
```

### `IsFalling()`

Returns `true` if the player is currently mid-jump or falling,
`false` otherwise.

Reads the local CGPlayer movement-flags word at `+0x9E8` and tests
`MOVEFLAG_FALLING | MOVEFLAG_FALLING_FAR` (`0x2000 | 0x4000`). This
is client-side state (outbound `MSG_MOVE_*` data, never visible for
remote units), so the function only meaningfully applies to the
local player.

```lua
if not IsFalling() then
    -- safe to bind ground-targeted spell
end
```

### `IsSwimming()`

Returns `true` if the player is currently swimming (in liquid, with
the swim animation/movement set), `false` otherwise.

Same movement-flags word as `IsFalling`, testing `MOVEFLAG_SWIMMING`
(`0x200000`). Local-player only.

```lua
if IsSwimming() then
    -- breath bar logic, mount-failure suppression, etc.
end
```

## AddOns

Six modern `C_AddOns.*` getters that splat the legacy
`GetAddOnInfo(arg)` 7-tuple `(name, title, notes, enabled,
loadable, reason, security)` into single-field accessors. Most
bypass `GetAddOnInfo` entirely and call the engine's per-field
helpers directly.

All accept either a 1-based index (`1..GetNumAddOns()`) or an
addon directory name string.

### `C_AddOns.GetAddOnName(indexOrName)`

Returns the addon's directory name as the engine sees it (the
folder name on disk). For numeric input, returns the engine's
canonical casing; for string input, echoes the input verbatim
once existence is confirmed. Returns `nil` for missing addons.

```lua
C_AddOns.GetAddOnName(1)            -- "Atlas-TW"
C_AddOns.GetAddOnName("DebugTools") -- "DebugTools"
C_AddOns.GetAddOnName("garbage")    -- nil
```

### `C_AddOns.GetAddOnTitle(indexOrName)`

Returns the `## Title:` from the addon's `.toc` file, with WoW
color-code escapes applied. `nil` for missing addons or addons
without a title field.

```lua
C_AddOns.GetAddOnTitle("DebugTools") -- "UI Debug Tools"
```

### `C_AddOns.GetAddOnNotes(indexOrName)`

Returns the `## Notes:` from the `.toc`. `nil` for missing
addons or addons without notes.

```lua
C_AddOns.GetAddOnNotes("DebugTools")
-- "Tools for developing addons (backport of Blizzard_DebugTools 3.3.5 to 1.12.1 / Lua 5.0)"
```

### `C_AddOns.IsAddOnLoadable(indexOrName)`

Returns `loadable, reason` — a real boolean and a status string
(or `nil`).

`reason` comes from a small status table the engine consults when
populating `GetAddOnInfo`'s 6th return: `"DISABLED"`, `"BANNED"`,
`"CORRUPT"`, `"INSECURE"`, `"NOT_DEMAND_LOADED"`,
`"INTERFACE_VERSION"`, `"MISSING"`. `nil` when the addon is
loadable. The full modern signature accepts optional `character`
and `demandLoaded` arguments — those are ignored here since vanilla
1.12 has no per-character addon enable state.

```lua
C_AddOns.IsAddOnLoadable("DebugTools")        -- true, nil
C_AddOns.IsAddOnLoadable("HardcoreTooltips")  -- false, "DISABLED"
C_AddOns.IsAddOnLoadable("garbage")           -- false, nil
```

### `C_AddOns.GetAddOnSecurity(indexOrName)`

Returns `"SECURE"` for Blizzard-signed addons or `"INSECURE"` for
user addons. Returns `nil` for missing addons. Vanilla addons
loaded from `Interface/AddOns/` are always insecure.

```lua
C_AddOns.GetAddOnSecurity("DebugTools") -- "INSECURE"
```

### `C_AddOns.DoesAddOnExist(indexOrName)`

Returns `true` iff the engine's addon registry has a matching
entry, `false` otherwise. Cheap existence probe used by addons
doing soft-dependency checks.

The implementation goes through the registry directly rather than
dispatching to `GetAddOnInfo` — the engine echoes its input name
back as ret1 unconditionally (before the lookup), so a
`GetAddOnInfo("garbage") ~= nil` heuristic returns true for any
string. This wrapper avoids that.

```lua
C_AddOns.DoesAddOnExist("DebugTools")  -- true
C_AddOns.DoesAddOnExist("garbage")     -- false
```

## Combat

### `InCombatLockdown()`

Returns `true` if the local player is currently in combat, `false`
otherwise. Modern WoW gates secure-frame UI manipulation on this; 1.12
has no secure-frame system, so the function reduces to a plain
"is the player in combat" check. Useful for addons backporting modern
code that does e.g. `if not InCombatLockdown() then SetBindingClick(...) end`.

```lua
if not InCombatLockdown() then
    -- safe to make UI changes
end
```

Equivalent to `UnitAffectingCombat("player")` but faster — reads the
`UNIT_FLAG_IN_COMBAT` bit directly off the player CGUnit's
m_objectFields, no token-resolution roundtrip.

## Hooks

### `hooksecurefunc(name, callback)` / `hooksecurefunc(table, name, callback)`

Modern post-call hook: the original function runs first, then
`callback` runs with the same args (return values discarded). The
original's return values propagate to the caller.

```lua
hooksecurefunc("GetSpellInfo", function(spellID)
    print("GetSpellInfo called with " .. tostring(spellID))
end)

hooksecurefunc(GameTooltip, "SetInventoryItem", function(self, unit, slot)
    -- runs after the engine fills the tooltip
    print("inventory tooltip:", unit, slot)
end)
```

The "secure" label refers to taint-propagation behavior introduced in
3.0 for protected-frame manipulation. Vanilla 1.12 has no taint
system, so the function is functionally equivalent to a plain
"after-hook" — just preserves modern API parity for addons being
backported from later expansions.

Implemented in pure C: builds a Lua C closure with `(orig, callback)`
as upvalues; the wrapper calls orig with `LUA_MULTRET`, then callback,
then returns orig's full result list. No return-count cap — works
correctly for functions returning any number of values.

Errors via `lua_error` on:
- Non-string `name`
- Non-function `callback`
- `target[name]` not resolvable to a function (covers typos and
  hooking unknown frame methods)

## Input

1.12 ships `IsShiftKeyDown` / `IsControlKeyDown` / `IsAltKeyDown` but
they only report "any shift/ctrl/alt" — there's no built-in way to tell
left from right. These seven functions add the missing distinction
plus an `IsModifierKeyDown()` rollup.

### `IsLeftShiftKeyDown()` / `IsRightShiftKeyDown()`
### `IsLeftControlKeyDown()` / `IsRightControlKeyDown()`
### `IsLeftAltKeyDown()` / `IsRightAltKeyDown()`

Each returns `1` when the corresponding key is physically down, `nil`
otherwise — matching the convention 1.12's own `IsShiftKeyDown` etc.
use.

```lua
if IsLeftShiftKeyDown() and not IsRightShiftKeyDown() then
    -- left-shift-only binding
end
```

### `IsModifierKeyDown()`

Returns `1` if **any** of the six modifier keys is down, `nil`
otherwise. Equivalent to
`IsShiftKeyDown() or IsControlKeyDown() or IsAltKeyDown()` but in one
call.

### `MODIFIER_STATE_CHANGED` event

Fires on every modifier-key press and release with `(key, down)`:

| arg1 (`key`) | `LSHIFT`, `RSHIFT`, `LCTRL`, `RCTRL`, `LALT`, `RALT` |
| arg2 (`down`) | `1` on press, `0` on release |

Only transitions fire — key autorepeat does not. Matches 2.4.3+
semantics.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("MODIFIER_STATE_CHANGED")
f:SetScript("OnEvent", function()
    -- in vanilla 1.12 OnEvent receives no args; engine sets `event` and
    -- `arg1..argN` as globals before invoking the handler.
    if event == "MODIFIER_STATE_CHANGED" and arg1 == "LSHIFT" then
        print("left shift", arg2 == 1 and "down" or "up")
    end
end)
```

**Implementation notes**

L/R modifier distinction doesn't exist anywhere in the 1.12 engine's
own state — its `IsShiftKeyDown` chain bottoms out at
`GetKeyState(VK_SHIFT)` (the merged virtual key, `0x10`), never the
L/R-aware `VK_LSHIFT` / `VK_RSHIFT`. The OS-level keystate *does* have
the distinction; we capture it by installing a `WH_GETMESSAGE` Win32
thread hook on the engine's message-pump thread, decoding each
`WM_KEY{,SYS}{DOWN,UP}` message (using `MapVirtualKeyA(scancode,
MAPVK_VSC_TO_VK_EX)` to resolve `VK_SHIFT` to L/R and the `KF_EXTENDED`
bit in `lParam` for `VK_CONTROL` / `VK_MENU`), and maintaining a
6-bit cached bitmap that the seven query functions read.

The thread-message hook is per-thread, not per-`HWND` — it survives
renderer-state changes that recreate WoW's main window (e.g. toggling
vertical sync), where an `SetWindowLongPtr`-style `WNDPROC` subclass
would be left dangling.

## Talent

### `GetTalentSpellID(tabIndex, talentIndex, [rank])`

Returns the spellID for the talent at the given `(tabIndex, talentIndex)`
and rank, or `nil` if the talent index is out of range or the rank slot
is empty. 1.12's `GetTalentInfo` returns `(name, icon, tier, column,
currentRank, maxRank, ...)` with no spellID; this fills the gap so
addons can chain into the spell APIs without maintaining their own
`(class, tab, idx) → spellID` lookup tables.

```lua
GetTalentSpellID(1, 9)           -- 14751  (Inner Focus, single rank)
GetTalentSpellID(1, 1)           -- 14525  (Wand Specialization, current rank)
GetTalentSpellID(1, 1, 1)        -- 14524  (Wand Specialization rank 1)
GetTalentSpellID(1, 1, 2)        -- 14525  (Wand Specialization rank 2)

-- chains into the spell APIs cleanly
GameTooltip:SetSpellByID(GetTalentSpellID(1, 9))
print(C_Spell.GetSpellName(GetTalentSpellID(1, 9)))  -- "Inner Focus"
```

`rank` is optional. When omitted, defaults to the player's currentRank
by delegating to the engine's existing `GetTalentInfo` (5th return) —
that's the same player-state derivation the engine uses for talent UI
rendering, including the spell-knowledge checks that account for
talent-granted abilities. If currentRank is 0 (no points spent), falls
back to rank 1 so the function still produces the talent's canonical
spellID for unallocated talents.

When provided explicitly, returns the spellID at that rank regardless
of how many points the player has invested. Useful for tooltip-on-hover
scenarios where you want to preview "what rank 5 would do" without
respec'ing.

Returns `nil` for:

- non-numeric or non-positive `tabIndex` / `talentIndex`
- `tabIndex` or `talentIndex` out of range for the player's class
- explicit `rank` exceeding the talent's allocated max — e.g. asking
  for rank 5 on a 1-rank talent like Inner Focus

Reads the engine's per-tab talent arrays at `[0x00BDCD28]` (populated
at login from `Talent.dbc` filtered by class). The `SpellRank[]` array
lives at offset `+0x10` of each `TalentEntry` (stride `0x54`), with
one spellID per rank — vanilla populates indices 0..4 (ranks 1..5),
the higher slots stay zero.

Equivalent to one of `GetTalentInfo`'s extended returns in modern WoW
(varies by version; the talent's spellID has been part of the tuple
since 5.0+).

### `GetTalentIDByIndex(tabIndex, talentIndex)`

Returns the engine's hidden talentID — the primary key of the
`Talent.dbc` row — for the talent at the given (tab, idx). Returns
`nil` for out-of-range indices.

```lua
GetTalentIDByIndex(1, 9)   -- 174  (Inner Focus, Discipline tier 3)
GetTalentIDByIndex(1, 1)   -- 166  (first Discipline talent)
```

1.12's `GetTalentInfo(tab, idx)` returns
`(name, icon, tier, column, currentRank, maxRank, ...)` but NOT the
talentID. Modern WoW exposes it (and uses talentIDs as the natural
key for `GetTalentInfoByID`, talent build sharing strings, etc.); we
add this getter so addons that key on talentIDs from later expansions
work unmodified.

Reads `TalentEntry+0x00` from the per-tab talent arrays at
`[0x00BDCD28]`. Same struct walk as `GetTalentSpellID`, just reads
a different field.

Equivalent to the talentID return slot of `GetTalentInfo` in modern
WoW (5.0+; not exposed at all in 1.12).

## Time

### `GetServerTime()`

Returns the current server clock as a Unix epoch timestamp (seconds since
1970-01-01 UTC), or `nil` before login (while the engine's game-time
struct is still BSS-zero).

```lua
local now = GetServerTime()
-- now = 1778260148 (Fri 2026-05-08 17:09:08 UTC)
```

Reads year/month/day/hour/minute from the engine's game-time struct at
`0x00CE8538` (populated from `SMSG_LOGIN_VERIFY_WORLD` /
`SMSG_LOGIN_SETTIMESPEED` and advanced by the internal tick handler) and
converts via `_mkgmtime`. Stock `GetTime()` returns frame-relative
seconds-since-login and is useless for wall-clock alignment; this is the
right call for calendar / log-timestamp / cooldown-sync use cases.

> **Sub-minute accuracy.** The 1.12 wire protocol carries time at
> minute granularity — the packed gametime field has no seconds — so
> the engine's clock only steps every minute. We interpolate within the
> minute using `GetTickCount`: whenever we observe the engine's minute
> change, we anchor to the current tick and add `(now - anchor) / 1000`
> seconds for subsequent calls in that minute.
>
> The very first call after login lands at second `:00` of the current
> minute (we have no way to know how far in we are when we first see
> it), so the cold-start value can be off by 0..59 seconds. After the
> first minute rollover we observe, the anchor lands at the rollover
> boundary and the timestamp is accurate to within a second of the
> engine's clock for as long as the session continues.

Equivalent to the function of the same name introduced in 3.0.

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

### `LE_EXPANSION_*`

The retail / Classic Era expansion-level enum, exposed as Lua globals
so addons backporting from later expansions don't have to gate on
`if LE_EXPANSION_CLASSIC then` (the constant being defined is itself
the version probe). Values match the modern `Enum.ExpansionLevel`
table.

| Constant                              | Value |
|---------------------------------------|------:|
| `LE_EXPANSION_LEVEL_CURRENT`          | `0` *(this is Classic)* |
| `LE_EXPANSION_CLASSIC`                | `0` |
| `LE_EXPANSION_BURNING_CRUSADE`        | `1` |
| `LE_EXPANSION_WRATH_OF_THE_LICH_KING` | `2` |
| `LE_EXPANSION_CATACLYSM`              | `3` |
| `LE_EXPANSION_MISTS_OF_PANDARIA`      | `4` |
| `LE_EXPANSION_WARLORDS_OF_DRAENOR`    | `5` |
| `LE_EXPANSION_LEGION`                 | `6` |
| `LE_EXPANSION_BATTLE_FOR_AZEROTH`     | `7` |
| `LE_EXPANSION_SHADOWLANDS`            | `8` |
| `LE_EXPANSION_DRAGONFLIGHT`           | `9` |
| `LE_EXPANSION_WAR_WITHIN`             | `10` |
| `LE_EXPANSION_MIDNIGHT`               | `11` |

```lua
-- Classic version check using modern idiom
if LE_EXPANSION_LEVEL_CURRENT < LE_EXPANSION_BURNING_CRUSADE then
    -- pure-1.x (vanilla / Classic Era) code path
end

-- Probe for ClassicAPI presence
if LE_EXPANSION_LEVEL_CURRENT then
    -- the constants are defined → ClassicAPI is loaded
end
```

### `LE_ITEM_QUALITY_*`

The item-quality enum (modern `Enum.ItemQuality`), exposed as Lua
globals so addons backporting modern code can do
`if quality >= LE_ITEM_QUALITY_RARE then ...` against the integer
quality returned by `GetItemInfo` / `C_Item.GetItemInfoInstant`.

| Constant                       | Value | Color in tooltip |
|--------------------------------|------:|------------------|
| `LE_ITEM_QUALITY_POOR`         | `0`   | gray (junk) |
| `LE_ITEM_QUALITY_COMMON`       | `1`   | white |
| `LE_ITEM_QUALITY_UNCOMMON`     | `2`   | green |
| `LE_ITEM_QUALITY_RARE`         | `3`   | blue |
| `LE_ITEM_QUALITY_EPIC`         | `4`   | purple |
| `LE_ITEM_QUALITY_LEGENDARY`    | `5`   | orange |
| `LE_ITEM_QUALITY_ARTIFACT`     | `6`   | gold *(TBC+ only)* |
| `LE_ITEM_QUALITY_HEIRLOOM`     | `7`   | light blue *(WotLK+ only)* |
| `LE_ITEM_QUALITY_WOWTOKEN`     | `8`   | orange *(WoD+ only)* |

Values 0..5 (POOR..LEGENDARY) correspond to actual qualities present
in vanilla 1.12. Higher values are exposed for source compatibility
with modern addons — vanilla items will never carry those quality
values, so a comparison like `quality == LE_ITEM_QUALITY_HEIRLOOM`
trivially never matches and the rest of the code path is unreachable.

```lua
local _, _, quality = GetItemInfo(itemID)
if quality and quality >= LE_ITEM_QUALITY_RARE then
    -- highlight in UI
end
```

### `LE_UNIT_STAT_*`

The primary-stat enum (modern `Enum.UnitStat`), exposed as Lua globals
so addons can index `UnitStat(unit, statIndex)` symbolically:

| Constant                | Value | Stat |
|-------------------------|------:|------|
| `LE_UNIT_STAT_STRENGTH`  | `1`  | Strength |
| `LE_UNIT_STAT_AGILITY`   | `2`  | Agility |
| `LE_UNIT_STAT_STAMINA`   | `3`  | Stamina |
| `LE_UNIT_STAT_INTELLECT` | `4`  | Intellect |
| `LE_UNIT_STAT_SPIRIT`    | `5`  | Spirit |

Values are stable across every WoW expansion — `UnitStat("player", 1)`
has always returned strength.

```lua
local _, effective = UnitStat("player", LE_UNIT_STAT_AGILITY)
```
