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
  - [`IsHarmfulSpell(spell)` / `IsHelpfulSpell(spell)`](#isharmfulspellspell--ishelpfulspellspell)
  - [`C_Spell.IsSpellHarmful(spellID)` / `C_Spell.IsSpellHelpful(spellID)`](#c_spellisspellharmfulspellid--c_spellisspellhelpfulspellid)
  - [`C_SpellBook.GetSpellLevelLearned(spellID)`](#c_spellbookgetspelllevellearnedspellid)
  - [`C_SpellBook.GetCurrentLevelSpells([level])`](#c_spellbookgetcurrentlevelspellslevel)
  - [`C_SpellBook.GetSpellSkillLine(spellID)`](#c_spellbookgetspellskilllinespellid)
  - [`GetSpellSchool(spellID)`](#getspellschoolspellid)
  - [`CastSpellNoToggle(name | spellID)`](#castspellnotogglename--spellid)
- [Macros](#macros)
  - [Numeric spellIDs in `/cast` and `CastSpellByName`](#numeric-spellids-in-cast-and-castspellbyname)
  - [`CastSpellNoToggle` as a macro cast line](#castspellnotoggle-as-a-macro-cast-line)
- [GameTooltip](#gametooltip)
  - [`GameTooltip:SetSpellByID(spellID)`](#gametooltipsetspellbyidspellid)
  - [`GameTooltip:SetTalentByID(talentID)`](#gametooltipsettalentbyidtalentid)
  - [`GameTooltip:SetInventoryItemByID(itemID)`](#gametooltipsetinventoryitembyiditemid)
  - [`GameTooltip:GetItem()`](#gametooltipgetitem)
  - [`GameTooltip:GetSpell()`](#gametooltipgetspell)
  - [`GameTooltip:HasItem()` / `GameTooltip:HasSpell()`](#gametooltiphasitem--gametooltiphasspell)
- [Quest](#quest)
  - [`C_QuestLog.GetQuestIDFromLogIndex(index)`](#c_questloggetquestidfromlogindexindex)
  - [`C_QuestLog.RequestLoadQuestByID(questID)`](#c_questlogrequestloadquestbyidquestid)
  - [`C_QuestLog.GetTitleForQuestID(questID)`](#c_questloggettitleforquestidquestid)
- [Gossip](#gossip)
  - [`C_GossipInfo.GetText()`](#c_gossipinfogettext)
  - [`C_GossipInfo.GetOptions()`](#c_gossipinfogetoptions)
  - [`C_GossipInfo.GetAvailableQuests()`](#c_gossipinfogetavailablequests)
  - [`C_GossipInfo.GetActiveQuests()`](#c_gossipinfogetactivequests)
  - [`C_GossipInfo.GetNumOptions()` / `GetNumAvailableQuests()` / `GetNumActiveQuests()`](#c_gossipinfogetnumoptions--getnumavailablequests--getnumactivequests)
  - [`C_GossipInfo.SelectOption(gossipOptionID)` / `SelectOptionByIndex(orderIndex)`](#c_gossipinfoselectoptiongossipoptionid--selectoptionbyindexorderindex)
  - [`C_GossipInfo.SelectAvailableQuest(questID)`](#c_gossipinfoselectavailablequestquestid)
  - [`C_GossipInfo.SelectActiveQuest(questID)`](#c_gossipinfoselectactivequestquestid)
  - [`C_GossipInfo.CloseGossip()`](#c_gossipinfoclosegossip)
- [Faction](#faction)
  - [`GetFactionIDByIndex(factionIndex)`](#getfactionidbyindexfactionindex)
  - [`GetFactionInfoByID(factionID)`](#getfactioninfobyidfactionid)
  - [`GetFactionParentID(factionID)`](#getfactionparentidfactionid)
  - [`C_Reputation.GetFactionStandings()`](#c_reputationgetfactionstandings)
  - [`C_Reputation.GetWatchedFactionData()`](#c_reputationgetwatchedfactiondata)
  - [`C_Reputation.GetFactionDataByIndex(factionSortIndex)`](#c_reputationgetfactiondatabyindexfactionsortindex)
  - [`C_Reputation.SetWatchedFactionByID(factionID)`](#c_reputationsetwatchedfactionbyidfactionid)
  - [`C_Reputation.GetLastStandingChange()`](#c_reputationgetlaststandingchange)
  - [`FACTION_STANDING_CHANGED` event](#faction_standing_changed-event)
- [FriendList](#friendlist)
  - [`C_FriendList.SendWhoQueryByName(name)`](#c_friendlistsendwhoquerybynamename)
  - [`C_FriendList.IsWhoQueryPending()`](#c_friendlistiswhoquerypending)
- [Item](#item)
  - [`C_Item.IsBound(itemLocation)`](#c_itemisbounditemlocation)
  - [`C_Item.GetItemID(itemLocation)`](#c_itemgetitemiditemlocation)
  - [`GetInventoryItemID(unit, slot)`](#getinventoryitemidunit-slot)
  - [`GetInventoryItemDurability(invSlot)`](#getinventoryitemdurabilityinvslot)
  - [`GetInventoryItemRepairCost(invSlot)`](#getinventoryitemrepaircostinvslot)
  - [`C_Item.GetItemFamily(item)`](#c_itemgetitemfamilyitem)
  - [`C_Item.GetItemCount(itemInfo, [includeBank], [includeUses])`](#c_itemgetitemcountiteminfo-includebank-includeuses)
  - [`GetItemIcon(itemID)` / `C_Item.GetItemIcon(itemLocation)` / `C_Item.GetItemIconByID(item)`](#getitemiconitemid--c_itemgetitemiconitemlocation--c_itemgetitemiconbyiditem)
  - [`C_Item.GetItemInfoInstant(item)`](#c_itemgetiteminfoinstantitem)
  - [`C_Item.IsItemDataCachedByID(item)` / `C_Item.IsItemDataCached(itemLocation)`](#c_itemisitemdatacachedbyiditem--c_itemisitemdatacacheditemlocation)
  - [`C_Item.RequestLoadItemDataByID(item)` / `C_Item.RequestLoadItemData(itemLocation)`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation)
  - [`C_Item.GetItemSpell(item)`](#c_itemgetitemspellitem)
  - [`OffhandHasWeapon()`](#offhandhasweapon)
  - [`C_Item.IsEquippableItem(item)`](#c_itemisequippableitemitem)
  - [`C_Item.IsEquippedItem(item)`](#c_itemisequippeditemitem)
  - [`C_Item.EquipItemByName(itemInfo [, dstSlot])`](#c_itemequipitembynameiteminfo--dstslot)
  - [`C_Item.DoesItemExist(itemLocation)` / `C_Item.DoesItemExistByID(item)`](#c_itemdoesitemexititemlocation--c_itemdoesitemexistbyiditem)
  - [`C_Item.GetItemName(itemLocation)` / `C_Item.GetItemNameByID(item)`](#c_itemgetitemnameitemlocation--c_itemgetitemnamebyiditem)
  - [`C_Item.GetItemQuality(itemLocation)` / `C_Item.GetItemQualityByID(item)`](#c_itemgetitemqualityitemlocation--c_itemgetitemqualitybyiditem)
  - [`C_Item.GetItemSellPrice(itemLocation)` / `C_Item.GetItemSellPriceByID(item)`](#c_itemgetitemsellpriceitemlocation--c_itemgetitemsellpricebyiditem)
  - [`C_Item.GetCurrentItemLevel(itemLocation)` / `C_Item.GetDetailedItemLevelInfo(item)`](#c_itemgetcurrentitemlevelitemlocation--c_itemgetdetaileditemlevelinfoitem)
  - [`C_Item.GetItemMaxStackSize(itemLocation)` / `C_Item.GetItemMaxStackSizeByID(item)`](#c_itemgetitemmaxstacksizeitemlocation--c_itemgetitemmaxstacksizebyiditem)
  - [`C_Item.GetItemLink(itemLocation)`](#c_itemgetitemlinkitemlocation)
  - [`C_Item.GetItemInventoryType(itemLocation)` / `C_Item.GetItemInventoryTypeByID(item)`](#c_itemgetiteminventorytypeitemlocation--c_itemgetiteminventorytypebyiditem)
  - [`C_Item.IsLocked(itemLocation)`](#c_itemislockeditemlocation)
  - [`C_Item.GetItemGUID(itemLocation)`](#c_itemgetitemguiditemlocation)
  - [`C_Item.GetItemLocation(itemGUID)`](#c_itemgetitemlocationitemguid)
  - [`Get*ItemID` — companions to the engine's `Get*ItemLink` family](#getitemid--companions-to-the-engines-getitemlink-family)
- [Container](#container)
  - [`C_Container.GetContainerItemID(bagIndex, slotIndex)`](#c_containergetcontaineritemidbagindex-slotindex)
  - [`C_Container.GetContainerItemDurability(containerIndex, slotIndex)`](#c_containergetcontaineritemdurabilitycontainerindex-slotindex)
  - [`C_Container.GetContainerItemRepairCost(containerIndex, slotIndex)`](#c_containergetcontaineritemrepaircostcontainerindex-slotindex)
  - [`C_Container.GetContainerNumFreeSlots(bagID)`](#c_containergetcontainernumfreeslotsbagid)
  - [`C_Container.PlayerHasHearthstone()`](#c_containerplayerhashearthstone)
  - [`C_Container.UseHearthstone()`](#c_containerusehearthstone)
- [MerchantFrame](#merchantframe)
  - [`C_MerchantFrame.GetItemInfo(slot)`](#c_merchantframegetiteminfoslot)
  - [`C_MerchantFrame.GetBuybackItemID(slot)`](#c_merchantframegetbuybackitemidslot)
  - [`C_MerchantFrame.GetNumJunkItems()`](#c_merchantframegetnumjunkitems)
  - [`C_MerchantFrame.SellAllJunkItems()`](#c_merchantframesellalljunkitems)
  - [`C_MerchantFrame.IsMerchantItemRefundable(slot)`](#c_merchantframeismerchantitemrefundableslot)
  - [`C_MerchantFrame.IsSellAllJunkEnabled()`](#c_merchantframeissellalljunkenabled)
- [EquipmentSet](#equipmentset)
  - [Overview & file format](#overview--file-format)
  - [`C_EquipmentSet.CanUseEquipmentSets()`](#c_equipmentsetcanuseequipmentsets)
  - [`C_EquipmentSet.GetNumEquipmentSets()`](#c_equipmentsetgetnumequipmentsets)
  - [`C_EquipmentSet.GetEquipmentSetIDs()`](#c_equipmentsetgetequipmentsetids)
  - [`C_EquipmentSet.GetEquipmentSetID(name)`](#c_equipmentsetgetequipmentsetidname)
  - [`C_EquipmentSet.GetEquipmentSetInfo(setID)`](#c_equipmentsetgetequipmentsetinfosetid)
  - [`C_EquipmentSet.GetIgnoredSlots(setID)`](#c_equipmentsetgetignoredslotssetid)
  - [`C_EquipmentSet.GetItemIDs(setID)`](#c_equipmentsetgetitemidssetid)
  - [`C_EquipmentSet.GetItemLocations(setID)`](#c_equipmentsetgetitemlocationssetid)
  - [`C_EquipmentSet.CreateEquipmentSet(name [, icon])`](#c_equipmentsetcreateequipmentsetname--icon)
  - [`C_EquipmentSet.SaveEquipmentSet(setID [, icon])`](#c_equipmentsetsaveequipmentsetsetid--icon)
  - [`C_EquipmentSet.ModifyEquipmentSet(setID, newName)`](#c_equipmentsetmodifyequipmentsetsetid-newname)
  - [`C_EquipmentSet.DeleteEquipmentSet(setID)`](#c_equipmentsetdeleteequipmentsetsetid)
  - [`C_EquipmentSet.IgnoreSlotForSave(slot)` / `UnignoreSlotForSave` / `IsSlotIgnoredForSave` / `ClearIgnoredSlotsForSave`](#c_equipmentsetignoreslotforsaveslot--unignoreslotforsave--isslotignoredforsave--clearignoredslotsforsave)
  - [`C_EquipmentSet.EquipmentSetContainsLockedItems(setID)`](#c_equipmentsetequipmentsetcontainslockeditemssetid)
  - [`C_EquipmentSet.UseEquipmentSet(setID)`](#c_equipmentsetuseequipmentsetsetid)
  - [`EQUIPMENT_SETS_CHANGED` event](#equipment_sets_changed-event)
  - [`EQUIPMENT_SWAP_PENDING` event](#equipment_swap_pending-event)
  - [`EQUIPMENT_SWAP_FINISHED` event](#equipment_swap_finished-event)
- [Class](#class)
  - [`FillLocalizedClassList(table [, isFemale])`](#filllocalizedclasslisttable-isfemale)
- [Unit](#unit)
  - [`UnitGUID(unit)`](#unitguidunit)
  - [`GetUnitSpeed(unit)`](#getunitspeedunit)
  - [`UnitClassBase(unit)`](#unitclassbaseunit)
  - [`UnitIsAFK(unit)`](#unitisafkunit)
  - [`UnitIsDND(unit)`](#unitisdndunit)
  - [`UnitIsFeignDeath(unit)`](#unitisfeigndeathunit)
  - [`UnitIsInMyGuild(unitOrName)`](#unitisinmyguildunitorname)
  - [`UnitIsPossessed(unit)`](#unitispossessedunit)
  - [`UnitStandState(unit)`](#unitstandstateunit)
- [UnitAuras](#unitauras)
  - [`C_UnitAuras.GetAuraDataByIndex(unit, index [, filter])`](#c_unitaurasgetauradatabyindexunit-index--filter)
  - [`C_UnitAuras.GetBuffDataByIndex(unit, index)` / `GetDebuffDataByIndex(unit, index)`](#c_unitaurasgetbuffdatabyindexunit-index--getdebuffdatabyindexunit-index)
  - [`C_UnitAuras.GetUnitAuraBySpellID(unit, spellID [, filter])`](#c_unitaurasgetunitaurabyspellidunit-spellid--filter)
  - [`C_UnitAuras.GetPlayerAuraBySpellID(spellID)`](#c_unitaurasgetplayeraurabyspellidspellid)
  - [`C_UnitAuras.GetUnitAuras(unit [, filter])`](#c_unitaurasgetunitaurasunit--filter)
  - [`C_UnitAuras.GetAuraDispelTypeColor(dispelName)`](#c_unitaurasgetauradispeltypecolordispelname)
- [NameCache](#namecache)
  - [`GetPlayerInfoByGUID(guid)`](#getplayerinfobyguidguid)
  - [`C_PlayerCache.GetPlayerInfoByName(name)`](#c_playercachegetplayerinfobynamename)
  - [`C_PlayerInfo.GUIDIsPlayer(guid)` / `GUIDIsCreature` / `GUIDIsPet` / `GUIDIsGameObject`](#c_playerinfoguidisplayerguid--guidiscreature--guidispet--guidisgameobject)
  - [`C_CreatureInfo.GetCreatureID(guid)`](#c_creatureinfogetcreatureidguid)
  - [`C_PlayerCache.RememberPlayer(guid, name, classToken)`](#c_playercacherememberplayerguid-name-classtoken)
  - [`C_PlayerCache.SetEnabled(enabled)`](#c_playercachesetenabledenabled)
  - [`C_PlayerCache.IsEnabled()`](#c_playercacheisenabled)
  - [`C_PlayerCache.SetScanEnabled(enabled)`](#c_playercachesetscanenabledenabled)
  - [`C_PlayerCache.IsScanEnabled()`](#c_playercacheisscanenabled)
- [State](#state)
  - [`IsMounted()`](#ismounted)
  - [`IsStealthed()`](#isstealthed)
  - [`IsFalling()`](#isfalling)
  - [`IsSwimming()`](#isswimming)
  - [`IsAssistingRitual()`](#isassistingritual)
- [CharacterList](#characterlist)
  - [`GetSavedCharacterOrder(realm)` / `SetSavedCharacterOrder(realm, order)` — GlueXML only](#getsavedcharacterorderrealm--setsavedcharacterorderrealm-order--gluexml-only)
- [Chat](#chat)
  - [`GetCurrentChatGUID()`](#getcurrentchatguid)
- [AddOns](#addons)
  - [`C_AddOns.GetAddOnName(indexOrName)`](#c_addonsgetaddonnameindexorname)
  - [`C_AddOns.GetAddOnTitle(indexOrName)`](#c_addonsgetaddontitleindexorname)
  - [`C_AddOns.GetAddOnNotes(indexOrName)`](#c_addonsgetaddonnotesindexorname)
  - [`C_AddOns.IsAddOnLoadable(indexOrName)`](#c_addonsisaddonloadableindexorname)
  - [`C_AddOns.GetAddOnSecurity(indexOrName)`](#c_addonsgetaddonsecurityindexorname)
  - [`C_AddOns.DoesAddOnExist(indexOrName)`](#c_addonsdoesaddonexistindexorname)
- [Combat](#combat)
  - [`InCombatLockdown()`](#incombatlockdown)
- [CVar](#cvar)
  - [`C_CVar.GetCVarBool(cvar)`](#c_cvargetcvarboolcvar)
- [Table](#table)
  - [`table.wipe(t)`](#tablewipet)
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
  - [`C_Timer.After(seconds, callback)`](#c_timeraftersseconds-callback)
  - [`C_Timer.NewTimer(seconds, callback)`](#c_timernewtimerseconds-callback)
  - [`C_Timer.NewTicker(seconds, callback, [iterations])`](#c_timernewtickerseconds-callback-iterations)
  - [`C_DateAndTime` overview](#c_dateandtime-overview)
  - [`C_DateAndTime.GetCurrentCalendarTime()`](#c_dateandtimegetcurrentcalendartime)
  - [`C_DateAndTime.GetCalendarTimeFromEpoch(epoch)`](#c_dateandtimegetcalendartimefromepochepoch)
  - [`C_DateAndTime.AdjustTimeByDays(date, days)` / `AdjustTimeByMinutes(date, minutes)`](#c_dateandtimeadjusttimebydaysdate-days--adjusttimebyminutesdate-minutes)
  - [`C_DateAndTime.CompareCalendarTime(lhs, rhs)`](#c_dateandtimecomparecalendartimelhs-rhs)
  - [`C_DateAndTime.GetServerTimeLocal()`](#c_dateandtimegetservertimelocal)
  - [`C_DateAndTime.GetSecondsUntilDailyReset()`](#c_dateandtimegetsecondsuntildailyreset)
- [UIColor](#uicolor)
  - [`C_UIColor.GetColors()`](#c_uicolorgetcolors)
- [Events](#events)
  - [`C_EventUtils.IsEventValid(eventName)`](#c_eventutilsiseventvalideventname)
  - [`BAG_UPDATE_DELAYED` event](#bag_update_delayed-event)
  - [Player input-state events (`PLAYER_STARTED_MOVING` / `LOOKING` / `TURNING` + `STOPPED_*`)](#player-input-state-events)
  - [`GLOBAL_MOUSE_DOWN` / `GLOBAL_MOUSE_UP` events](#global_mouse_down--global_mouse_up-events)
- [Globals](#globals)
  - [`CLASSIC_API_VERSION`](#classic_api_version)
  - [`LE_EXPANSION_*`](#le_expansion_)
  - [`LE_ITEM_QUALITY_*`](#le_item_quality_)
  - [`LE_UNIT_STAT_*`](#le_unit_stat_)
  - [`Enum.AddOnSecurityStatus`](#enumaddonsecuritystatus)

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
other reason (unknown spell, dead, etc.). Matches the `1`/`nil`
return convention of the existing `Script_IsUsableAction`.

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

### `C_Item.GetWeaponEnchantInfo()`

Returns the modern 12-tuple matching WotLK+'s extended
`GetWeaponEnchantInfo` signature, including the **temp-enchant IDs**
that vanilla 1.12's 8-return global omits.

```
hasMain, mainExpire, mainCharges, mainEnchantID,
hasOff,  offExpire,  offCharges,  offEnchantID,
hasRanged, rangedExpire, rangedCharges, rangedEnchantID
   = C_Item.GetWeaponEnchantInfo()
```

```lua
-- Apply Brilliant Mana Oil to mainhand, then:
local has, expireMs, charges, enchantID = C_Item.GetWeaponEnchantInfo()
-- has = true, expireMs ≈ 1800000, charges = 5, enchantID = <oil's enchant>
```

Reads the **temporary** enchant slot (`ITEM_FIELD_ENCHANTMENT`
slot 1 at descriptor `+0x4C`) — the same slot oils, sharpening
stones, and poisons populate and the engine drains as they expire.
This is what modern's `GetWeaponEnchantInfo` measures.

The permanent enchant (Crusader, Mongoose, etc., in slot 0 at
`+0x40`) is **not** reported here — that's a separate field and
modern's `GetWeaponEnchantInfo` doesn't expose it either. Get the
permanent enchant ID by parsing `GetInventoryItemLink("player",
slot)` (the link includes it as the 2nd numeric field).

Vanilla 1.12's global `GetWeaponEnchantInfo` is unchanged — old
addons reading positions 4..8 by index still work.

Equivalent to the extension of `GetWeaponEnchantInfo` introduced
in 3.x.

### `IsHarmfulSpell(spell)` / `IsHelpfulSpell(spell)`

Classify a spell as offensive (`IsHarmfulSpell`) or non-offensive
(`IsHelpfulSpell`) without parsing its tooltip. Accept either form
the spellbook-aware globals use:

```lua
IsHarmfulSpell(spellID)             -- by ID
IsHarmfulSpell(slot, "spell")       -- player spellbook slot
IsHarmfulSpell(slot, "pet")         -- pet spellbook slot
-- same calling shapes for IsHelpfulSpell
```

Examples:

```lua
IsHarmfulSpell(133)     -- true  (Fireball)
IsHarmfulSpell(118)     -- true  (Polymorph)
IsHelpfulSpell(2061)    -- true  (Flash Heal)
IsHelpfulSpell(1243)    -- true  (Power Word: Fortitude)
IsHarmfulSpell(2061)    -- false
```

Reads `Spell.dbc.AttributesEx` (record `+0x1C`) bit `0x80` — the
same `SPELL_ATTR_EX_NEGATIVE` bit CMaNGOS uses to mark spells as
debuffs server-side. `IsHarmfulSpell` is true iff that bit is set;
`IsHelpfulSpell` is true iff the spell exists and the bit is NOT
set. Both return `false` for invalid spellIDs.

> Vanilla 1.12 doesn't have a dedicated "positive" flag, so the
> helpful side is the rough complement of harmful. For utility
> spells with no clear orientation (Aspect of the Cheetah,
> Stealth, ground-targeted AOEs), modern WoW sometimes returns
> false for both; we return `true` for helpful as a safer default
> for addons gating on "is this castable on me?" logic. Compute
> precise modern semantics by also inspecting effect implicit
> targets if you need them.

Equivalent to the functions of the same name introduced in 3.x
(when WoW first exposed them as globals).

### `C_Spell.IsSpellHarmful(spellID)` / `C_Spell.IsSpellHelpful(spellID)`

Same classification logic as the globals above, but exposed in the
modern `C_Spell` namespace. Both take a numeric `spellID` only —
no `(slot, bookType)` form.

```lua
C_Spell.IsSpellHarmful(133)     -- true (Fireball)
C_Spell.IsSpellHelpful(2061)    -- true (Flash Heal)
```

Equivalent to `C_Spell.IsSpellHarmful` / `C_Spell.IsSpellHelpful`
introduced in 11.x.

### `C_SpellBook.GetSpellLevelLearned(spellID)`

Returns the level at which a spell becomes available — the
`BaseLevel` field in `Spell.dbc` (record `+0x70`). Direct read off
the cached record; no class/race filtering.

```lua
C_SpellBook.GetSpellLevelLearned(133)    -- Fireball rank 1 → 4
C_SpellBook.GetSpellLevelLearned(25306)  -- Fireball rank 12 → 60
C_SpellBook.GetSpellLevelLearned(2061)   -- Flash Heal rank 1 → 20
```

Returns `0` for invalid spellIDs, spells with no level requirement
(most non-class utility spells), or records the engine hasn't
cached. Matches modern semantics — unknown / utility spells
return 0 rather than nil.

### `C_SpellBook.GetCurrentLevelSpells([level])`

Returns a 1-based table of spellIDs the player's class/race can
learn at the given level. Without arguments, defaults to the
player's current level.

```lua
C_SpellBook.GetCurrentLevelSpells()    -- spells trainable at your level
C_SpellBook.GetCurrentLevelSpells(20)  -- preview what's available at 20
C_SpellBook.GetCurrentLevelSpells(60)  -- preview at 60
```

Walks `SkillLineAbility.dbc` filtering by the player's class bit
(`1 << (classID - 1)`) and race bit, respecting both the include
masks (entries with mask = 0 are "all classes"/"all races", which
also pass) and the exclude masks. For each surviving entry, looks
up the spell's `BaseLevel` and includes it if it matches the
queried level.

> **Vanilla is trainer-driven.** Modern `GetCurrentLevelSpells`
> (added in 5.x when trainers were removed) returns *auto-learned*
> spells. Vanilla 1.12 requires visiting a trainer to actually
> learn most class spells. We return the closest available analog:
> **what's *trainable* at this level**. Useful for "what's new
> this level" UI panels and level-preview tooling ported from MoP+.

Class/race come from the local player — there's no
`(class, race, level)` form because vanilla doesn't expose a
clean class-string→classID lookup. Returns an empty table at
character select / pre-login (no CGPlayer yet) and for levels
where no class/race spells match.

### `C_SpellBook.GetSpellSkillLine(spellID)`

Returns `(name, skillID)` — the SkillLine.dbc row that the given spell
belongs to. The name is the user-facing skill/tab string in the engine's
current locale: spellbook tab names for class spells (`"Protection"`,
`"Fire"`, `"Holy"`), profession headers (`"Tailoring"`, `"Engineering"`),
weapon-skill rows (`"Swords"`, `"Bows"`), etc.

```lua
C_SpellBook.GetSpellSkillLine(1671)   -- "Protection", 26 (Shield Bash)
C_SpellBook.GetSpellSkillLine(133)    -- "Fire", 8        (Fireball)
C_SpellBook.GetSpellSkillLine(2050)   -- "Holy", 56       (Lesser Heal)
C_SpellBook.GetSpellSkillLine(2480)   -- "Bows", 45       (Shoot Bow)
C_SpellBook.GetSpellSkillLine(3273)   -- "First Aid", 129 (Anesthetic)
```

Walks `SkillLineAbility.dbc` for any row whose `spellID` matches, then
reads the localized `Name[locale]` field at offset `+0x0C` of the
referenced `SkillLine.dbc` record. For spells with multiple SLA rows
(race-locked variants, Turtle WoW's faction-specific entries) the
lookup prefers a row whose class/race masks match the local player —
so the result reflects "what tab is this spell in *for me*" when one
exists. Falls back to the first matching row otherwise, so the
function still answers for spells the local player can't actually use
(inspecting other classes, parsing combat-log entries from unfamiliar
specs).

Returns `(nil, nil)` for:
- non-numeric or non-positive input
- spellIDs with no `SkillLineAbility.dbc` row (most temporary auras,
  proc-only spells, item-on-use effects, GM-debug spells)
- rows whose `skillID` doesn't resolve in `SkillLine.dbc`

Vanilla's `GetSpellTabInfo(tabIndex)` enumerates spellbook tabs by
1-based index — it can't answer "what tab is *this spellID* in".
Addons that need a spellID→tab mapping have historically had to walk
every tab's slot range and string-match against `GetSpellName`. This
function reads it directly off the DBC for any spellID, including
spells the player hasn't learned.

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

### `CastSpellNoToggle(name | spellID)`

Spam-safe variant of `CastSpellByName` that won't toggle off an
already-active spell. Covers both kinds of vanilla-toggle abilities:

- **Auto-repeat** — Shoot, Auto-Shot, Wand. Tracked via the engine's
  active-auto-repeat global.
- **Self-aura toggles** — shapeshift (Cat/Bear/Travel/Moonkin/
  Shadowform), stance (Battle/Defensive/Berserker), aspect (Hunter),
  seal (Paladin), blessing-on-self, etc. Tracked via the unit
  descriptor's aura array.

If either toggle is already on for the requested spell, the call is
a no-op — exactly what `/cast !SpellName` does in 2.3.2+ clients,
but expressed as a vanilla Lua call.

| Engine state                  | Behavior            | Return  |
|-------------------------------|---------------------|---------|
| Nothing toggled               | Starts the cast     | `true` if the spell is now active, else `false` |
| This spell already auto-repeating | No-op           | `true`  |
| This spell's aura already on (form / stance / aspect) | No-op | `true`  |
| A *different* auto-repeat is active | No-op         | `false` (don't disrupt the other auto-repeat) |

Replaces both the action-bar scan-and-skip pattern and the
form-already-active check:

```lua
-- Before (auto-repeat case)
for i = 1, 120 do
    if IsAutoRepeatAction(i) then return end
end
CastSpellByName("Shoot")

-- Before (shapeshift case)
if not (UnitBuff("player", "Cat Form") or GetShapeshiftFormID() == CAT_FORM) then
    CastSpellByName("Cat Form")
end

-- After (both cases)
CastSpellNoToggle("Shoot")
CastSpellNoToggle("Cat Form")
```

Accepts either a name string or a spellID number:

```lua
CastSpellNoToggle("Shoot")         -- by name (matches what CastSpellByName accepts)
CastSpellNoToggle(75)              -- by spellID (Auto Shot rank 1)
CastSpellNoToggle("Aspect of the Hawk")
CastSpellNoToggle("Battle Stance")
```

String input matches case-insensitively and tolerates a trailing
`(Rank N)` suffix the same way `CastSpellByName` itself does —
`"Shoot"` and `"Shoot(Rank 1)"` both compare equal to a Shoot that's
already auto-repeating.

Reads `[VAR_ACTIVE_AUTO_REPEAT_SPELL]` (`0x00CEAC30`) for the auto-
repeat check, and the engine's `FUN_SPELL_IS_TOGGLE_AURA_ACTIVE`
(`0x004B36F0`) for the aura-active check. Delegates to
`Script_CastSpellByName` (`0x004B4AB0`) for the actual cast — same
resolution semantics (rank picking, target rules, etc.) as the
engine's own global.

Using this from inside a macro action slot? See
[`CastSpellNoToggle` as a macro cast line](#castspellnotoggle-as-a-macro-cast-line) below for the additional
parser support that makes the slot tag correctly for action-bar UIs.

## Macros

Engine-level extensions to how macros are parsed and dispatched. These
don't add new Lua functions — they teach the engine to recognize input
forms it didn't accept in stock 1.12. Macro authors get them for free
once `ClassicAPI.dll` is loaded.

Vanilla 1.12 doesn't support `[target=...]`-style macro conditionals
natively; we don't add those. If you have a separate DLL/addon that
does (nampower's conditional macros, SuperWoWhook, etc.), the
extensions below compose with it — that layer strips the bracket
clause and forwards the cleaned tail to `CastSpellByName`, which then
flows through our additions.

### Numeric spellIDs in `/cast` and `CastSpellByName`

Pure-numeric input is now accepted anywhere a spell name would be:

```
/cast 5019                     -- in a macro line; casts Shoot if known
CastSpellByName("5019")        -- same effect from Lua / /run / chat
```

The spellID is resolved through `Spell.dbc` to the locale-resolved
name, then handed to the engine's existing name resolver — so all the
normal downstream behavior applies: the spellbook lookup gates on
"player knows this spell," action-bar UI sees the cast as if it had
been named, macros containing `/cast 5019` get tagged with the right
spellID in the engine's spell-state cache (so `IsCurrentAction(slot)`
and `IsAutoRepeatAction(slot)` work for the macro slot).

| Input        | Outcome                                                |
|--------------|--------------------------------------------------------|
| `/cast 5019` | Casts Shoot if you know it (any rank, highest known)   |
| `/cast 1234` | Falls through as "unknown spell," same as `/cast Foo`  |
| `/cast Shoot`| Unchanged from vanilla — names still work              |
| `/cast 5019(Rank 1)` | Falls through — rank suffix with a numeric stem isn't supported (use the name form if you need a specific rank) |

Implemented as a single hook on the engine's name → spellbook-slot
resolver at `0x004B3950` — covers `/cast`, `/castsequence` aliases,
`CastSpellByName`, the macro parser's tagging path, and anywhere
else the engine resolves a spell name. Numeric input with garbage
trailing characters falls through unchanged.

### `CastSpellNoToggle` as a macro cast line

Putting `CastSpellNoToggle("Shoot")` in a macro body now tags the
macro with Shoot's spellID the same way `/cast Shoot` or
`CastSpellByName("Shoot")` would. Without this, the macro casts
correctly but vanilla's macro parser doesn't know to associate the
slot with any spell — so action-bar UIs that call
`IsCurrentAction(slot)` / `IsAutoRepeatAction(slot)` (pfUI's
actionbar, ShaguTweaks, etc.) never light up the slot.

Recognized forms in a macro body:

```
CastSpellNoToggle("Shoot")        -- by name (string)
CastSpellNoToggle("Cat Form")     -- shapeshift / aspect / stance all tag the same way
CastSpellNoToggle(5019)           -- not yet — macro parser only recognizes the string form
```

The numeric form **works for the cast itself** (via the function's
runtime resolution), but the macro parser only scans for the string
form when tagging. To get the slot to highlight when using the
spellID, write `CastSpellNoToggle("Shoot")` instead.

The engine's first-match-wins rule still applies — if your macro is:

```
/cast Wand
CastSpellNoToggle("Shoot")
```

…the macro is tagged with Wand (the engine recognized `/cast Wand`
on line 1; our additional pattern is only consulted when the engine
didn't find any of its own patterns).

Macro tagging happens at macro edit/save time. Existing macros need
to be opened in the Macro UI and re-saved once after dropping in the
new DLL to pick up the new parser behavior.

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

### `GameTooltip:GetItem()`

Returns `(name, link, itemID)` for whichever item the tooltip is
currently displaying, or nothing if it isn't showing an item. Modern
WoW returns only `(name, link)`; we extend with `itemID` as a third
return so callers don't have to gsub-extract it from the link.

The engine stashes two fields per Set* item call:
- `tooltip+0x398` ← itemID (always populated)
- `tooltip+0x380/+0x384` ← item GUID (only when there's a real
  CGItem — `SetBagItem`, `SetInventoryItem`, `SetLootItem`,
  `SetMerchantItem`, etc. Zero for `SetItemByID` / `SetHyperlink`
  which have no instance.)

Both fields are zeroed by the per-tooltip Clear on Hide / before the
next Set*.

```lua
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")

-- SetItemByID — no per-instance data, basic link returned
GameTooltip:SetItemByID(6948)
local name, link, id = GameTooltip:GetItem()
-- name = "Hearthstone"
-- link = "|cffffffff|Hitem:6948:0:0:0:0:0:0:0|h[Hearthstone]|h|r"
-- id   = 6948

-- SetInventoryItem — full dressed link with enchant + random suffix
GameTooltip:SetInventoryItem("player", INVSLOT_BACK)
local name, link, id = GameTooltip:GetItem()
-- name = "Superior Cloak"
-- link = "|cff1eff00|Hitem:9805:247:843:0|h[Superior Cloak of the Eagle]|h|r"
-- id   = 9805
```

Two link paths:

| Set* path | Link form | Dressing |
|---|---|---|
| `SetBagItem`, `SetInventoryItem`, `SetLootItem`, `SetMerchantItem`, … | Full dressed link via the engine's own builder at `0x0052AE00` | Enchant ID, random-suffix factor, unique ID, suffix-decorated name |
| `SetItemByID`, `SetHyperlink` (item:N with no instance data) | Basic colored link | itemID + cached quality + cached name only |

The dressed-link path works for items not in player inventory
(merchant, loot, mailbox, trade window, etc.) — the engine's
resolver finds any CGItem the client has loaded.

Returns nothing for: non-item tooltip, uncached itemID on the
no-GUID path (fires a background cache warmup), or empty name.

Equivalent to the function of the same name introduced in 3.0,
with the added `itemID` third return.

### `GameTooltip:GetSpell()`

Returns `(name, rank, spellID)` for whichever spell the tooltip is
currently displaying, or nothing if it isn't showing a spell. The
engine stashes the spellID on the tooltip frame at `+0x39C` whenever
any `SetX` spell path runs (`SetSpell`, `SetSpellByID`,
`SetUnitBuff`/`SetUnitDebuff`, `SetTalent`, etc.) and zeroes it on
the next `Clear` / `Hide`.

```lua
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
GameTooltip:SetSpellByID(25306)
local name, rank, spellID = GameTooltip:GetSpell()
-- name = "Fireball"
-- rank = "Rank 12"
-- spellID = 25306
```

`rank` is the empty string (not nil) for spells whose Spell.dbc rank
slot is blank — most racials, talent passives, and proc-triggered
spells. This matches the modern semantics where the rank position is
always populated.

Equivalent to the function of the same name introduced in 3.0.

### `GameTooltip:HasItem()` / `GameTooltip:HasSpell()`

Boolean companions to `GetItem` / `GetSpell`. Return `true` if the
tooltip is currently displaying an item / spell, `false` otherwise.
Cheaper than `GetItem` / `GetSpell` when all you need is the
predicate — single `uint32` read against `OFF_TOOLTIP_ITEM_ID` /
`OFF_TOOLTIP_SPELL_ID`, no DBC lookup or link-build.

```lua
if GameTooltip:HasItem() then
    local _, link = GameTooltip:GetItem()
    -- only do the GetItem work when we actually need it
end
```

Equivalent to the functions of the same name introduced in 3.0
(`HasItem`) and 6.0 (`HasSpell`).

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

### `C_QuestLog.GetQuestIDFromLogIndex(index)`

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
    local questID = C_QuestLog.GetQuestIDFromLogIndex(i)  -- 0 for headers
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

## Gossip

Retail-shaped wrappers around vanilla's flat `GetGossipText` /
`GetGossipOptions` / `GetGossipAvailableQuests` / `GetGossipActiveQuests`
/ `SelectGossipOption` / `SelectGossipAvailableQuest` /
`SelectGossipActiveQuest` / `CloseGossip` surface. The data is the
same — these calls just read the engine's two gossip-state arrays
(`0x00BBBE90` for options, `0x00BB74C0` for quests, both filled by the
SMSG_GOSSIP_MESSAGE handler at `0x004E26E0`) and shape it into the
modern struct-tables that addons ported from retail expect.

**Fields the 1.12 server simply doesn't send** — and therefore aren't
in any of the returned tables — include `rewards` and `spellID` (added
after the post-vanilla quest/spell system rework), per-option
`status` (Available / Unavailable / Locked / AlreadyComplete — vanilla
servers don't compute this), and modern UX hints like `overrideIconID`
and `selectOptionWhenOnlyOption`.

The `icon` field is the raw icon-type byte from SMSG_GOSSIP_MESSAGE
(server-extensible — pservers can add custom NPC types past the
Blizzard 0..10 range). Default Blizzard types resolve to
`Interface\GossipFrame\<Type>GossipIcon`:

| Value | Type | Value | Type |
|------:|:-----|------:|:-----|
| `0` | Gossip       | `6`  | Banker       |
| `1` | Vendor       | `7`  | Petition     |
| `2` | Taxi         | `8`  | Tabard       |
| `3` | Trainer      | `9`  | BattleMaster |
| `4` | Healer       | `10` | Auctioneer   |
| `5` | Binder (innkeeper) | | |

### `C_GossipInfo.GetText()`

Returns the gossip greeting string the engine resolved from the NPC's
referenced `NPC_TEXT.dbc` row. Same value `GetGossipText()` returns;
provided so addons that prefer the `C_GossipInfo` namespace don't have
to mix the two surfaces.

```lua
print(C_GossipInfo.GetText())
```

### `C_GossipInfo.GetOptions()`

Returns an array of gossip-option tables in display order. Each entry
contains:

| Field            | Type    | Notes |
|------------------|---------|-------|
| `gossipOptionID` | number  | Vanilla `optionIndex`. The same value `C_GossipInfo.SelectOption` expects; arbitrary integer assigned by the server. |
| `name`           | string  | Option text (locale-applied by the server). |
| `icon`           | number  | Raw icon-type byte from SMSG_GOSSIP_MESSAGE — passed through unmapped so pserver-added types (anything past the default `0..10` range) survive. See the type table above for the default Blizzard categories and the `Interface\GossipFrame\<Type>GossipIcon` path each one resolves to. |
| `flags`          | number  | Bit 0 set = `boxCoded` (the option asks for confirmation text — `Are you sure?` boxes). |
| `orderIndex`     | number  | 1-based position in the emitted list. Matches the index `SelectGossipOption` (legacy) expects. |

```lua
for _, opt in ipairs(C_GossipInfo.GetOptions()) do
    print(opt.gossipOptionID, opt.name, "icon", opt.icon)
end
```

### `C_GossipInfo.GetAvailableQuests()`

Returns an array of deliverable-quest tables (quests the giver is
offering to start), in display order. Filter mirrors vanilla's
`GetGossipAvailableQuests` — status field at `+0x008` not in `{3, 4}`.

| Field        | Type   | Notes |
|--------------|--------|-------|
| `questID`    | number | Quest.dbc row ID. |
| `title`      | string | Localized quest title. |
| `questLevel` | number | Quest level. |

### `C_GossipInfo.GetActiveQuests()`

Returns an array of active-quest tables (quests the giver wants you
to turn in, in-progress or complete). Status field at `+0x008` in
`{3, 4}`.

| Field        | Type    | Notes |
|--------------|---------|-------|
| `questID`    | number  | Quest.dbc row ID. |
| `title`      | string  | Localized quest title. |
| `questLevel` | number  | Quest level. |
| `isComplete` | boolean | `true` when the engine's status byte is `4` (ready to turn in); `false` when it's `3` (still in progress). |

### `C_GossipInfo.GetNumOptions()` / `GetNumAvailableQuests()` / `GetNumActiveQuests()`

Convenience counters — return the length of each of the three lists
above without building the table.

### `C_GossipInfo.SelectOption(gossipOptionID)` / `SelectOptionByIndex(orderIndex)`

Picks a gossip option. `SelectOption` resolves the modern
`gossipOptionID` to vanilla's 1-based slot, then tail-calls the
engine's native `SelectGossipOption`. `SelectOptionByIndex` is a thin
passthrough for callers that already have the slot index (e.g. from
`opt.orderIndex` on a `GetOptions()` entry, or from a UI button bound
directly to the slot).

Both end in the same CMSG; the option-ID variant exists so addons can
drive selections off the modern `gossipOptionID` key without keeping
their own slot mapping. Returns nothing.

Vanilla's `SelectGossipOption` doesn't accept a confirmation-text
argument, so for `boxCoded` options the engine's own confirm dialog
runs as usual — there is no way to send the password from the script.

### `C_GossipInfo.SelectAvailableQuest(questID)`

Picks a deliverable quest by `questID`. Walks the active-vs-available
filter the same way `GetAvailableQuests` does, locates the matching
1-based slot, and tail-calls the engine's `SelectGossipAvailableQuest`.
Returns nothing.

### `C_GossipInfo.SelectActiveQuest(questID)`

Picks an in-progress quest by `questID`. Same shape as
`SelectAvailableQuest` but walks the active filter and tail-calls
`SelectGossipActiveQuest`. Returns nothing.

### `C_GossipInfo.CloseGossip()`

Closes the gossip window. Direct passthrough to the engine's
`CloseGossip`. Returns nothing.

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

### `C_Reputation.GetFactionStandings()`

Returns a flat `{ [factionID] = currentStanding }` table covering
every faction in the player's reputation list. `currentStanding` is
the same value `GetFactionInfo` puts in its `barValue` slot — `base
+ delta` from the rep slot, signed.

Always returns a table (possibly empty); never nil.

```lua
local standings = C_Reputation.GetFactionStandings()
for factionID, standing in pairs(standings) do
    print(GetFactionInfoByID(factionID), standing)
end
```

Unlike a `GetNumFactions` + `GetFactionInfo` walk, this skips header
rows entirely and doesn't depend on the player having opened the
reputation pane recently — it reads straight out of the per-faction
rep-slot array, which the engine keeps populated for every faction
the player has rep with.

If you need names instead of IDs, layer `GetFactionInfoByID` on top:

```lua
local byName = {}
for factionID, standing in pairs(C_Reputation.GetFactionStandings()) do
    byName[GetFactionInfoByID(factionID)] = standing
end
```

This is a ClassicAPI-only call; modern WoW's closest equivalent is
the 11.x `C_Reputation.GetFactions()`, which returns an array of
struct tables rather than a flat map.

### `C_Reputation.GetWatchedFactionData()`

Returns a table describing the faction shown above the XP bar, or
`nil` if no faction is being watched. Backports the modern
struct-style accessor — vanilla's `GetWatchedFactionInfo()` returns
the same data as a 5-tuple without the factionID, which is the field
modern callers rely on most.

Returns the same `FactionData` table shape as
[`GetFactionDataByIndex`](#c_reputationgetfactiondatabyindexfactionsortindex)
— see that section for the full field list. `isWatched` is forced
`true` (this faction IS the watched one by definition).

```lua
local data = C_Reputation.GetWatchedFactionData()
if data then
    print(string.format("%s (%d): %d / %d",
        data.name, data.factionID,
        data.currentStanding - data.currentReactionThreshold,
        data.nextReactionThreshold - data.currentReactionThreshold))
end
```

Equivalent to the function of the same name introduced in 10.x.

Implementation reads the watched `RepListID` from the player's
`+0xE68` sub-struct, indexes the per-faction rep slot array at
`0x00B73290` to recover the factionID, then runs the shared
`ReadFactionData` chain (Faction.dbc lookup, reaction band, rep slot
flags, header/collapsed checks).

### `C_Reputation.GetFactionDataByIndex(factionSortIndex)`

Returns a `FactionData` table for the faction at the given 1-based
displayed-list index, or `nil` for an out-of-range index or a
pseudo-row ("Other" / "Inactive" placeholders that don't have a
`Faction.dbc` record). The index range matches what `GetNumFactions`
covers — real factions plus category headers.

Fields:

| Field                      | Type    | Notes |
|----------------------------|---------|-------|
| `factionID`                | number  | `Faction.dbc` record ID. |
| `name`                     | string  | Locale-applied. |
| `description`              | string  | Locale-applied (may be `""`). |
| `reaction`                 | number  | `1`=Hated .. `8`=Exalted. |
| `currentReactionThreshold` | number  | Band min standing value. |
| `nextReactionThreshold`    | number  | Band max standing value. |
| `currentStanding`          | number  | Current standing (`base + delta`). |
| `atWarWith`                | boolean | Rep slot flags bit `0x02`. |
| `canToggleAtWar`           | boolean | `currentStanding ≥ -3000` AND not peace-forced (flags bit `0x10`). |
| `isHeader`                 | boolean | Faction is a category header in the displayed list. |
| `isHeaderWithRep`          | boolean | Always `false` in vanilla — parent factions don't aggregate rep. |
| `isCollapsed`              | boolean | UI state: user has collapsed this header. |
| `isWatched`                | boolean | This faction is shown above the XP bar. |
| `canSetInactive`           | boolean | True when `!isHeader && repListIndex ≥ 0` — i.e. the engine's `SetFactionInactive`/`SetFactionActive` will accept this faction. |
| `isChild`                  | boolean | Always `false` (parent-child rep introduced post-vanilla). |
| `hasBonusRepGain`          | boolean | Always `false` (added in MoP). |
| `isAccountWide`            | boolean | Always `false` (added in Dragonflight). |

```lua
for i = 1, GetNumFactions() do
    local d = C_Reputation.GetFactionDataByIndex(i)
    if d and not d.isHeader and d.currentStanding < d.nextReactionThreshold then
        -- still grindable rep
    end
end
```

Implementation: resolves the 0-based index to a factionID via the
engine's `FUN_RESOLVE_FACTION_INDEX` at `0x004D5FA0`, then runs the
shared `ReadFactionData` chain — no Lua-side round-trip through
`Script_GetFactionInfo`. `isHeader` / `isCollapsed` come from the
displayed-list header array at `0x00B736C0` (count at `0x00B736B0`)
and the per-character bitmask at `0x0084A0A4`.

Equivalent to the function of the same name introduced in 10.x.

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

### `C_Reputation.GetLastStandingChange()`

Returns `factionID, newStanding, repGained` for the rep change
currently being fired, or `nil` if called outside that window.

Companion to [`FACTION_STANDING_CHANGED`](#faction_standing_changed-event).
The triple is the same payload as the event's `arg1, arg2, arg3`, but
the getter is also live during the engine's
`CHAT_MSG_COMBAT_FACTION_CHANGE` dispatch on the same hook — useful
for addons that want to enrich the chat line with the factionID
(which vanilla's chat-event payload doesn't carry) without
double-registering for `FACTION_STANDING_CHANGED`:

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("CHAT_MSG_COMBAT_FACTION_CHANGE")
f:SetScript("OnEvent", function()
    local factionID, newStanding, repGained = C_Reputation.GetLastStandingChange()
    if factionID then
        -- factionID is the real ID, not a parsed-out name string
        print(factionID, newStanding, repGained)
    end
end)
```

Outside the in-flight `FireNotify` hook stack (i.e. anywhere except
inside a `CHAT_MSG_COMBAT_FACTION_CHANGE` or `FACTION_STANDING_CHANGED`
handler), returns nothing. There is no "last change" memory — the
state is cleared as soon as the dispatch returns.

This is a ClassicAPI-only call; modern WoW has no equivalent (modern
addons get the factionID from `FACTION_STANDING_CHANGED` directly, so
they don't need a separate getter).

### `FACTION_STANDING_CHANGED` event

Fires once per reputation change with `(factionID, newStanding, repGained)`:

| arg1 (`factionID`)    | Faction.dbc row ID of the faction whose standing changed |
| arg2 (`newStanding`)  | New total standing value (post-change `barValue`)        |
| arg3 (`repGained`)    | Signed delta — positive on gain, negative on loss        |

Polyfills the modern event of the same name. Vanilla 1.12 exposes only
`CHAT_MSG_COMBAT_FACTION_CHANGE`, whose `arg1` is the localized chat
string (`"Your Stormwind reputation has increased by 100."`) — addons
have to parse the text and reverse-resolve the faction name back to
an ID. This event lets addons skip that work.

Does **not** fire for the initial faction sync at login or `/reload`
— matches modern semantics. Only fires when a real reputation gain or
loss arrives from the server (`SMSG_SET_FACTION_STANDING`).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("FACTION_STANDING_CHANGED")
f:SetScript("OnEvent", function()
    -- in vanilla 1.12 OnEvent receives no args; engine sets `event`
    -- and `arg1..argN` as globals before invoking the handler.
    if event == "FACTION_STANDING_CHANGED" then
        local name = GetFactionInfoByID(arg1)
        print(string.format("%s: %+d  (now %d)", name, arg3, arg2))
    end
end)
```

`repGained` is signed: positive for gains, negative for the rare loss
case (e.g. killing a Goblin Commodity Exchange NPC drops your rep
with Booty Bay).

**Implementation notes**

Hooks the engine's per-rep-change notify dispatcher at `0x0062C5F0`
— the chokepoint called once per `(factionID, signedDelta)` from
the per-slot setter at `0x004D6330`, gated by "value-actually-changed
AND notify-flag-set". The setter has already written the new delta
into the per-slot storage by the time the hook fires, so we read the
total back via `FUN_REPUTATION_GET_STANDING(factionID)` (`0x004D6370`)
to produce the `newStanding` payload.

The hook calls the original before firing our event so the engine's
`CHAT_MSG_COMBAT_FACTION_CHANGE` still fires first — no behavior
change for addons that depended on the chat text. A per-call
snapshot of `(factionID, newStanding, repGained)` is captured before
forwarding, which [`C_Reputation.GetLastStandingChange`](#c_reputationgetlaststandingchange)
exposes — so addons can read the structured payload from inside the
chat event without re-parsing the localized string.

## FriendList

### `C_FriendList.SendWhoQueryByName(name)`

Issues a /who name-filter query for a specific player. Results
buffer into the engine's WhoList so a normal `WHO_LIST_UPDATE` +
`GetWhoInfo(i)` flow can read them — no chat output, no
`"Found N players matching..."` system message.

Vanilla 1.12 exposes `SendWho(query)` and `SetWhoToUI(flag)` only as
separate primitives, so addons that want to silently look up a
single player's class/level/zone (e.g. to color an unknown name in
chat) have to manage state, cooldown timing, and friends-panel
suppression themselves. This collapses the invocation half of that
dance to one call with a clean true/false return.

Returns `true` if the query was sent, `false` if any of:

- the name was empty / nil
- the call is within the 5-second cooldown of a previous send
- the engine's WhoSystem isn't initialized yet (pre-login)

A `false` return is a no-op — the engine state isn't touched, no
pending flag is set, and the cooldown isn't extended. Safe to call
on every chat line that mentions an unknown player; the call will
naturally rate-limit.

The cooldown matches the server's: vanilla's CMSG_WHO is silent-
dropped server-side at roughly 5-second granularity, so a faster
client just wastes queries that won't get a response.

```lua
if C_FriendList.SendWhoQueryByName("Bob") then
    -- query sent; result will arrive via WHO_LIST_UPDATE within ~1s
end
```

This call alone does **not** suppress the friends-panel popup on
response — for that, wrap `FriendsFrame_OnEvent` and gate on
[`C_FriendList.IsWhoQueryPending()`](#c_friendlistiswhoquerypending):

```lua
local original = FriendsFrame_OnEvent
_G.FriendsFrame_OnEvent = function()
    if event == "WHO_LIST_UPDATE" and C_FriendList.IsWhoQueryPending() then
        return
    end
    return original()
end
```

(Auto-suppressing the popup from C++ would require hooking the
engine's `FrameScript_SignalEvent` or the SMSG_WHO opcode handler —
both high-traffic / high-risk sites. The wrap above is reliable and
takes 5 lines, so it stays addon-side for now.)

Implementation reads the WhoSystem singleton from `0x00C28168` and
calls the inner sender at `0x005AEBB0`
(`__thiscall(this = WhoSystem, queryStr)`), the same chokepoint
`Script_SendWho` tail-calls. The `whoToUI` flag at `0x00C2A12C` is
flipped to `1` first so the SMSG_WHO handler routes results into the
list (`WHO_LIST_UPDATE` path) instead of printing to chat.

### `C_FriendList.IsWhoQueryPending()`

Returns `true` within ~5 seconds of the most recent
[`SendWhoQueryByName`](#c_friendlistsendwhoquerybynamename) that
returned `true`; otherwise `false`. Time-based — the engine doesn't
expose a "response arrived" hook to C++ yet, so the window is a
conservative upper bound on the response RTT, which in practice
lands within a few hundred ms.

The intended use is gating an addon-side `FriendsFrame_OnEvent`
wrap so the user-issued `/who` window still works while addon-
issued silent queries don't pop the panel — see the example under
`SendWhoQueryByName`.

Concurrent callers see a shared pending flag — if pfUI and another
addon both call `SendWhoQueryByName` close in time, both queries
contribute to the same pending window. This is acceptable for the
"suppress popup for any in-flight DLL-issued query" use case;
addons that need per-call tracking should manage their own ticket
state on top.

## Item

> ### `itemLocation` argument shapes
>
> Every `C_Item.*` function on this page that takes `itemLocation` accepts
> any of three forms, matching the modern `ItemLocation` mixin plus a
> GUID-string convenience form:
>
> ```lua
> { equipmentSlotIndex = N }     -- 1-based, character pane
> { bagID = B, slotIndex = S }   -- both required
> "0xHHHHHHHHLLLLLLLL"           -- a GUID string from C_Item.GetItemGUID
> ```
>
> Table forms are O(1) — the engine knows the slot. The GUID-string form
> is O(~80) — the function walks equipment slots 1..19 and bags 0..4
> comparing each CGItem's stored GUID against the requested one. Bank and
> keyring are not walked. Fine for sporadic addon use; avoid for per-frame
> polling.

### `C_Item.IsBound(itemLocation)`

Returns `true` if the item at the given location is soulbound, `false` otherwise
(including when the slot is empty or the location is malformed). The 1.12
client tracks the soulbound bit on each item instance directly; previously
the only way to read it from Lua was a scan-tooltip hack
(`SetBagItem` + string-compare against the localized `ITEM_SOULBOUND`
constant) — slow, locale-fragile, and one of the hottest paths during bag
updates.

```lua
if C_Item.IsBound({equipmentSlotIndex = INVSLOT_HEAD}) then ... end
if C_Item.IsBound({bagID = 0, slotIndex = 1}) then ... end
if C_Item.IsBound(itemGUID) then ... end
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

### `GetInventoryItemRepairCost(invSlot)`

Returns the cost in copper to repair the player's equipped item at
`invSlot`. Same value `GameTooltip:SetInventoryItem` returns as its
third out-parameter.

```
copperCost = GetInventoryItemRepairCost(invSlot)
```

```lua
local cost = GetInventoryItemRepairCost(INVSLOT_CHEST)
if cost > 0 then
    -- e.g. cost == 12345
end
```

Returns `0` for slots that are empty, items without a durability
concept, fully-repaired items, or items whose stats aren't cached yet
(rare — happens briefly after login before SMSG_ITEM_QUERY_RESPONSE
arrives for newly-seen items). The "no return" path is reserved for
invalid input (non-numeric slot).

> **Player-only**, same constraint as
> [`GetInventoryItemDurability`](#getinventoryitemdurabilityinvslot)
> — other units' durability isn't broadcast in 1.12.

> **Discount is vendor-context-dependent.** The faction-rep + PvP
> rank discount is only applied when the player has a merchant
> window open (i.e. has received `SMSG_LIST_INVENTORY`). Called from
> anywhere else, you get the raw, undiscounted base cost.
>
> The engine tracks the current merchant via globals
> `DAT_00BDDFA0/A4` (the merchant's GUID), set when the merchant
> frame opens and zeroed when it closes. The helper short-circuits
> the discount path when those globals are zero.
>
> For consistent "what will I pay" semantics inside addons, only
> trust this value when `MerchantFrame:IsShown()` is true — or call
> it once per merchant-visit and cache.

Calls the engine's own per-item repair-cost helper at `0x004FAF30`,
which is the same function `Script_GameTooltip_SetInventoryItem`
calls for its repairCost return. The raw cost comes from
`DurabilityCosts.dbc` (indexed by item subclass and slot type) ×
`DurabilityQuality.dbc` (indexed by item quality); the discount —
when applicable — stacks faction reputation with the merchant
(Friendly+ unlocks a base 5%) and PvP rank (Sergeant Major+ adds
another 5%, Knight-Lieutenant+ stacks one more).

This is a ClassicAPI addition — modern WoW has no standalone Lua
function for per-item repair cost; the only way to read it there is
the tooltip's third return value. We expose it directly because the
underlying calculation is already in the engine and a Lua function
is the natural surface.

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

Returns the player's total count of `itemInfo` — equipped items +
bags, and optionally bank.

```
count = C_Item.GetItemCount(itemInfo [, includeBank [, includeUses]])
```

- `itemInfo` — numeric `itemID` or string containing `"item:NNN"`
  (full chat links work). Item names are NOT accepted (vanilla has
  no name → ID resolver).
- `includeBank` *(optional, default false)* — also walk bank slots
  (bag `-1` for the main bank, bags `5..10` for bank-bag slots).
- `includeUses` *(optional, default false)* — when `true`, multiplies
  each match by the item's spell-charges count. A wand with 50 charges
  contributes 50 instead of 1. Items without charges pass their plain
  stack count through unchanged, so the flag is a no-op for them.

```lua
local n = C_Item.GetItemCount(2589)               -- Linen Cloth in bags + equipped
local n = C_Item.GetItemCount(2589, true)         -- + bank
local n = C_Item.GetItemCount("item:2589")        -- string form works too

-- Equipped items count toward the total:
local trinketID = GetInventoryItemID("player", INVSLOT_TRINKET1)
C_Item.GetItemCount(trinketID)                    -- 1

-- includeUses multiplies by charges for charged items:
C_Item.GetItemCount(wandID, false, true)          -- 50 for one 50-charge wand
C_Item.GetItemCount(linenID, false, true)         -- same as stack count (no charges)
```

> **Bank works cold — no banker visit required.** The 1.12 server
> sends bank inventory at login alongside the rest of the player's
> data; only the engine's own `GetItemBySlot` gates bank slots until
> the window opens. We bypass that gate by reading the GUID array
> directly out of the player invMgr and resolving each entry via the
> engine's object resolver — same path `GetItemBySlot` would take
> internally if the gate let us through. Counts are correct from
> session start.

Walks the 19 equipment slots via
`Item::Location::ResolveEquipmentSlot`, then bags via the same
`Item::Location::ResolveBag` chain
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

### `C_Item.GetItemSpell(item)`

Returns `(spellName, spellID)` for the on-use spell attached to an
item (potions, trinkets, scrolls, hearthstone, food/drink, etc.),
or `nil` for items without one (vendor trash, regular gear, weapons
with passive procs).

`item` accepts the same input shapes as `GetItemInfo` — a numeric
itemID, a chat-link `"|cffffffff|Hitem:6948:0:0:0|h[...]|h|r"`
fragment, or a `"item:NNN"` short form.

```lua
C_Item.GetItemSpell(6948)
-- "Hearthstone", 8690

C_Item.GetItemSpell(13442)
-- "Mighty Rage Potion", 17528

C_Item.GetItemSpell(11288)  -- a soulstone (trigger=4, not ON_USE)
-- nil

C_Item.GetItemSpell(2589)   -- Linen Cloth, no spell
-- nil
```

Returns the **ON_USE** spell only. Vanilla items can carry up to 5
spell entries in their `ItemStats_C` record, each with its own
trigger code:

| Trigger | Meaning | Surfaced by `GetItemSpell`? |
|---|---|---|
| 0 | `ON_USE` | **yes** |
| 1 | `ON_EQUIP` (passive aura on gear) | no |
| 2 | `CHANCE_ON_HIT` (weapon procs) | no |
| 4 | `SOULSTONE` (on-death) | no |
| 5 | `ON_USE_NO_DELAY` | no (TODO: should we add this?) |
| 6 | `LEARN_SPELL` (recipes) | no |

This matches modern WoW's `GetItemSpell`, which only reports on-use
triggers. Addons that need the other trigger types (proc auras,
recipe targets) should reach into the cache directly — the spell
slots are at `ItemStats +0x11C` (5 spell IDs) and `+0x130` (5 trigger
codes).

**Auto-warmup on cache miss.** Items not yet in the local cache
return `nil` and silently kick off an `SMSG_ITEM_QUERY_SINGLE`
request. A second call after `GET_ITEM_INFO_RECEIVED` lands the
data. Same warmup pattern as `C_Item.GetItemFamily` and the rest of
our cache-backed accessors.

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

### `C_Item.IsEquippableItem(item)`

Returns `true` if `item` can be equipped in any character-pane slot,
`false` otherwise. Reads `m_inventoryType` from the cached ItemStats
record — INVTYPE_NON_EQUIP (value `0`) is the only "not equippable"
value, so any non-zero inventory type passes (head, neck, weapon,
shield, holdable, …).

`item` is an itemID number or `"item:N..."` link. Item names aren't
accepted — vanilla has no name→ID resolver, and equippability is an
itemID-keyed property anyway.

Returns `false` for uncached items (no async load fired). If you
need it to wait for the cache, call `C_Item.RequestLoadItemDataByID`
first and re-check on `ITEM_DATA_LOAD_RESULT`.

```lua
C_Item.IsEquippableItem(12640)  -- Lionheart Helm → true
C_Item.IsEquippableItem(6948)   -- Hearthstone → false
```

Equivalent to the function of the same name introduced in 1.10
(global in 4.3.4, namespaced under `C_Item` in modern Classic Era).

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

Two paths based on `dstSlot`:

- **Explicit `dstSlot` (1..19): cursor-free direct swap.** Calls the
  engine's own `FUN_INVENTORY_SWAP` (`0x005E0C40`) — the same
  primitive `Script_EquipCursorItem` dispatches to after resolving
  cursor state. We hand it the source item GUID, source container
  GUID, source linear slot, and target paperdoll slot; the engine
  builds and sends the CMSG_SWAP_INV_ITEM (or CMSG_AUTOEQUIP_ITEM
  for cross-container) packet through its normal pipeline. **The
  cursor is never read or written.** An item already on the cursor
  stays on the cursor.
- **No `dstSlot` (engine auto-picks slot from inventory type):**
  falls back to the cursor-pickup + `AutoEquipCursorItem` path
  because 1.12's auto-pick logic reads off cursor state. For this
  path only, the function refuses to operate (no-op) when
  `CursorHasItem()` is already true, to avoid clobbering whatever's
  held.

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

### `C_Item.DoesItemExist(itemLocation)` / `C_Item.DoesItemExistByID(item)`

Existence checks straight off the engine's inventory manager (location
form) and item cache (ID form). No `GetItemInfo` chaining — both
functions read directly from the structures they need.

- `C_Item.DoesItemExist(itemLocation)` — `true` iff the equipment-slot
  or `(bagID, slotIndex)` location resolves to a populated inventory
  slot on the active player. Empty slots, missing bags, or malformed
  tables return `false`.
- `C_Item.DoesItemExistByID(item)` — `true` iff the cache currently
  has data for `item`. Cache miss returns `false` and kicks off the
  network query in the background; a follow-up call after
  [`GET_ITEM_INFO_RECEIVED`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation)
  will succeed. Accepts a numeric `itemID`, a bare `"item:NNN..."`
  string, or a full chat link.

```lua
if C_Item.DoesItemExist({equipmentSlotIndex = INVSLOT_HEAD}) then ... end
if C_Item.DoesItemExistByID(6948) then ... end
```

### `C_Item.GetItemName(itemLocation)` / `C_Item.GetItemNameByID(item)`

Returns the cached display name as a string, or `nil` for empty /
uncached / invalid inputs. Reads `m_name[0]` straight off the
`ItemStats_C` cache record (`+0x08`) — no `GetItemInfo` round-trip.

Cache miss on the `ByID` form returns `nil` and fires the cache fill
so the next call (after `GET_ITEM_INFO_RECEIVED`) succeeds.

```lua
local name = C_Item.GetItemName({bagID = 0, slotIndex = 1})
local name = C_Item.GetItemNameByID(6948)   -- "Hearthstone"
```

### `C_Item.GetItemQuality(itemLocation)` / `C_Item.GetItemQualityByID(item)`

Returns the item's quality as an integer (0=Poor, 1=Common, 2=Uncommon,
3=Rare, 4=Epic, 5=Legendary), matching the `LE_ITEM_QUALITY_*`
constants. Single `uint32` read at cache record `+0x1C`.

```lua
if C_Item.GetItemQualityByID(itemID) >= LE_ITEM_QUALITY_RARE then
    -- highlight rare-or-better drop
end
```

### `C_Item.GetItemSellPrice(itemLocation)` / `C_Item.GetItemSellPriceByID(item)`

Returns the vendor sell price in copper, **per unit** (multiply by
stack count for the per-stack value). Matches the 11th return of
modern WoW's `GetItemInfo`. Returns `nil` on cache miss / invalid
input. Cache miss fires a background fill so a follow-up call after
`GET_ITEM_INFO_RECEIVED` returns the value.

```lua
local unit = C_Item.GetItemSellPriceByID(2589)   -- Linen Cloth → 25 (copper)
local stack = unit * C_Item.GetItemMaxStackSizeByID(2589)
```

Single `uint32` read at cache record `+0x28` (`m_sellPrice`). Vanilla
1.12 doesn't surface this in tooltips — the field is populated on
every sellable item but the engine's tooltip code never reads it —
so this function exposes data that's been sitting in the cache
unused.

### `C_Item.GetCurrentItemLevel(itemLocation)` / `C_Item.GetDetailedItemLevelInfo(item)`

Returns the item's base ilvl from `m_itemLevel` (cache record `+0x38`).
Vanilla 1.12 has no per-instance scaling (no upgrades, no warforging),
so "current" and "base" item level are always identical — both APIs
return the same single value. Modern `GetDetailedItemLevelInfo` is
spec'd to return `(current, isPreview, base)`; we push only the
current level, callers that care about the extra returns will see
`nil` for them.

```lua
local ilvl = C_Item.GetCurrentItemLevel({equipmentSlotIndex = INVSLOT_HEAD})
```

### `C_Item.GetItemMaxStackSize(itemLocation)` / `C_Item.GetItemMaxStackSizeByID(item)`

Returns the item type's max stack size — what you'd find as the 8th
return of `GetItemInfo(item)`. `1` for non-stackable items; `5`, `20`,
`200`, etc. for stackables. Different from `C_Item.GetStackCount` /
the engine's `GetContainerItemInfo`, which return the **current**
count in a specific slot.

```lua
local cap = C_Item.GetItemMaxStackSizeByID(2589)  -- Linen Cloth → 20
```

Single `uint32` read at cache record `+0x60` (`m_stackable`). By-ID
form fires a background cache fill on miss and returns nil; re-call
after `GET_ITEM_INFO_RECEIVED`.

### `C_Item.GetItemLink(itemLocation)`

Returns the fully-decorated per-instance hyperlink for the item at
the location — same string `GetContainerItemLink(bag, slot)` or
`GetInventoryItemLink("player", slot)` would return for the same
slot. Enchant ID, random-suffix, and any other per-instance data
attached to the CGItem are preserved.

```lua
local link = C_Item.GetItemLink({bagID = 0, slotIndex = 1})
-- "|cffa335ee|Hitem:16539:911:::::::70::::::::::|h[General's Silk Boots]|h|r"
--                          ^^^ — enchant ID (Speed +15%) preserved
```

Implemented by reading the location table's fields and tail-calling
the engine's link script function (`Script_GetContainerItemLink` at
`0x004F9930` for bag locations, `Script_GetInventoryItemLink` at
`0x004C8C10` for equipment slots). The link string is built by the
engine off the live CGItem, so it always matches what other
addons see via the older positional-arg APIs.

### `C_Item.GetItemInventoryType(itemLocation)` / `C_Item.GetItemInventoryTypeByID(item)`

Returns the numeric `Enum.InventoryType` straight off the cache
record's `m_inventoryType` field (`+0x2C`) — the integer sibling of
`GetItemInfoInstant`'s 4th return (which gives the `INVTYPE_*` string).

| Value | Constant              | Slot                        |
|------:|-----------------------|-----------------------------|
| 0     | `INVTYPE_NON_EQUIP_IGNORE` | non-equippable        |
| 1     | `INVTYPE_HEAD`        | head                         |
| 2     | `INVTYPE_NECK`        | neck                         |
| …     | …                     | …                            |
| 20    | `INVTYPE_ROBE`        | chest (full-body robes)      |
| 26    | `INVTYPE_RANGEDRIGHT` | ranged                       |
| 27    | `INVTYPE_QUIVER`      | quiver/ammo pouch            |

Vanilla items only produce values `0..28`; the higher modern
constants (`INVTYPE_PROFESSION_*`, `INVTYPE_EQUIPABLESPELL_*`, etc.)
were introduced post-vanilla and are never returned. Modern
backport code that compares against the higher enum values still
resolves correctly because vanilla items just don't carry those
types.

```lua
local t = C_Item.GetItemInventoryTypeByID(19019)  -- Thunderfury: 17 = INVTYPE_2HWEAPON
if t == 1 then -- head
    ...
end
```

### `C_Item.IsLocked(itemLocation)`

Returns `true` if the equipped/bagged item is in a server-side locked
state — mid-trade, mid-mail-attach, picked up onto the cursor, etc.
Direct descriptor read: CGItem → `m_objectFields` (`+0x114`) →
`ITEM_FIELD_FLAGS` (`+0x3C`) → bit 2 (`0x04`).

```lua
if C_Item.IsLocked({bagID = 0, slotIndex = 1}) then
    -- skip the action; slot is mid-transaction
end
```

**No `LockItem` / `UnlockItem` companion.** Vanilla 1.12 doesn't
provide a public Lua API for setting the lock flag — it's
exclusively server-driven via the `SMSG_ITEM_TIME_UPDATE` /
`SMSG_ITEM_PUSH_RESULT` packet family in response to actions like
`PickupContainerItem`. Modern backport code that calls
`C_Item.LockItem` / `UnlockItem` will hit the
`!!!ClassicAPI/Util/ItemUtil.lua` no-op shim and silently continue
— there's nothing useful we can do on the C side without a fake
state machine that wouldn't match server behavior.

### `C_Item.GetItemGUID(itemLocation)`

Returns the per-instance 64-bit GUID of the item at the location,
formatted as `"0xHHHHHHHHLLLLLLLL"` (16 hex digits, hi dword first).
Same format `UnitGUID` uses — 1.12 GUIDs are plain qwords with no
`"Item-Server-..."`-style prefix scheme (modern's prefix format
arrived in 6.x). Returns `nil` for empty / invalid locations.

```lua
local guid = C_Item.GetItemGUID({equipmentSlotIndex = INVSLOT_HEAD})
-- "0x40000000000DEFCA"
```

Reads CGItem instance block at `+0x08` → GUID qword at `+0x00`.
The GUID is stable per-character-session and survives moves between
bags / character pane, so it's the right identifier for "this exact
item instance" — equipment-set tracking, soulbind matching, or any
addon code that needs to follow a single item across slot moves.

### `C_Item.GetItemLocation(itemGUID)`

Reverse lookup from a GUID string (the format `C_Item.GetItemGUID`
returns) to an `itemLocation` table identifying the slot currently
holding that item. Returns `nil` if the GUID isn't resident in the
walked scope.

```lua
local guid = C_Item.GetItemGUID({equipmentSlotIndex = INVSLOT_HEAD})
-- player moves item to bag, drops it, etc. — guid stays valid across
-- bag/equipment rearrangements until the item leaves the session
local loc = C_Item.GetItemLocation(guid)
-- loc might now be { bagID = 1, slotIndex = 4 }, or nil if sold
```

Modern WoW returns an `ItemLocation` mixin object; we return a plain
table with the same field shape every other `C_Item.*` API in
ClassicAPI accepts as input (`{equipmentSlotIndex=N}` or
`{bagID=B, slotIndex=S}`), so the result pipes straight into
`C_Item.GetItemQuality(loc)`, `C_Item.GetItemLink(loc)`, etc.

**Walked scope.** Character-pane equipment (slots 1..19) + backpack
(bagID 0) + the four equipped bags (bagIDs 1..4). Bank and keyring
are NOT walked — same scope as the rest of the `C_Container.*`
family. Worst case is ~80 slot lookups; fine for sporadic use, not
appropriate for per-frame polling.

**Implementation.** We don't go through the engine's typed
object-by-GUID helper at `0x00468460` (the one used for the auction-
sell slot) because that path asserts on bad input — stale GUIDs
from addon-side caching would crash the client. Instead we walk
equipment + bag slots, read each CGItem's GUID, and compare. Misses
return nil; hits never need a fallback.

### `Get*ItemID` — companions to the engine's `Get*ItemLink` family

The 1.12 engine ships ~14 `Get*ItemLink` functions covering every
frame that lets you mouse over an item (loot, merchant, quests,
auction, trade, mail, tradeskill, craft). To get the itemID, the
standard pattern is to call the `Link` function and scrape the
number out of the returned string with `gsub` / `match`. Modern
WoW only has direct-ID accessors for a handful of these
(`GetLootSlotItemID`, `GetInboxItemID`, `GetQuestItemID`,
`GetMerchantItemID`, …), and the rest of the addon ecosystem still
scrapes the link.

These backport the modern ones where they exist and fill in the
gaps for the rest, so the whole `Get*ItemLink` family has a
1-to-1 ID companion. Each reads the same itemID the engine reads
when building the link, with no string parsing required.

All return `nil` for an empty slot, an out-of-range index, or when
the relevant UI frame isn't open.

| Function | Companion to | Notes |
|----------|--------------|-------|
| `GetLootRollItemID(rollID)` | `GetLootRollItemLink` | Group-loot roll ID (from `START_LOOT_ROLL`). Walks the engine's in-progress roll list. |
| `GetLootSlotItemID(slot)` | `GetLootSlotLink` | 1-based slot. Returns `nil` for coin slots. |
| `GetMerchantItemID(index)` | `GetMerchantItemLink` | 1-based; the active merchant's inventory. |
| `GetQuestItemID(type, index)` | `GetQuestItemLink` | `type` is `"reward"` or `"choice"`; active quest-offer / completion UI. |
| `GetQuestLogItemID(type, index)` | `GetQuestLogItemLink` | Same args; reads the currently selected quest-log entry. |
| `GetTradePlayerItemID(slot)` | `GetTradePlayerItemLink` | 1..7; your side of the trade window. |
| `GetTradeTargetItemID(slot)` | `GetTradeTargetItemLink` | 1..7; the other player's side. |
| `GetAuctionItemID(type, index)` | `GetAuctionItemLink` | `type` is `"list"`, `"owner"`, or `"bidder"`. |
| `GetAuctionSellItemID()` | `GetAuctionSellItemInfo` | The item currently in the sell slot. No args. |
| `GetInboxItemID(mailID)` | `GetInboxItem` | 1-based mail index; nil for gold-only mail. |
| `GetTradeSkillItemID(index)` | `GetTradeSkillItemLink` | The recipe's *created item* itemID. |
| `GetTradeSkillReagentItemID(index, reagentIndex)` | `GetTradeSkillReagentItemLink` | reagentIndex is 1-based; reagents are densely packed (no skipping empty slots). |
| `GetCraftReagentItemID(craftIndex, reagentIndex)` | `GetCraftReagentItemLink` | Craft frame (enchanting / beast training) — same shape as tradeskill reagents. |
| `GetCraftSpellID(craftIndex)` | `GetCraftItemLink` | The craft frame's link is `Hspell:`, not `Hitem:` — so the companion is a **spellID**, not an itemID. Addons scraping for itemID get nil today; this returns the actual identifier. |

```lua
-- Drop-in for the typical scrape:
--   local id = tonumber((GetLootRollItemLink(rollID)):match("item:(%d+)"))
local id = GetLootRollItemID(rollID)

-- Plays nice with the existing data APIs:
if id then
    local _, _, _, _, _, classID = C_Item.GetItemInfoInstant(id)
    -- ...
end
```

> **Quest UI distinction.** `GetQuestItemID` reads the *active quest
> offer* (the QuestFrame you see when accepting/turning in a quest);
> `GetQuestLogItemID` reads the currently-selected entry in the
> *quest log*. The two cover different states intentionally — modern
> WoW does the same split with `GetQuestItemLink` /
> `GetQuestLogItemLink`.

> **`GetTradeSkillItemID` vs `GetCraftSpellID`.** Vanilla splits
> recipes across two UI frames: TradeSkill (smithing / alchemy /
> tailoring / etc., which always produce a finished item) and Craft
> (enchanting formulas / beast-training tomes, where the spell IS
> the deliverable). `GetTradeSkillItemID` returns the produced
> itemID; `GetCraftSpellID` returns the recipe spellID — because
> there's no produced item to point at.

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

### `C_Container.GetContainerItemRepairCost(containerIndex, slotIndex)`

Bag/bank variant of
[`GetInventoryItemRepairCost`](#getinventoryitemrepaircostinvslot).
Same `copperCost` single-int return and the same "0 for nothing to
repair" semantics.

```
copperCost = C_Container.GetContainerItemRepairCost(containerIndex, slotIndex)
```

`containerIndex` accepts the same values as
[`C_Container.GetContainerItemDurability`](#c_containergetcontaineritemdurabilitycontainerindex-slotindex)
(`0` = backpack, `1..4` = equipped bags, `-1`/`5..11` = bank, etc.).

```lua
for slot = 1, GetContainerNumSlots(0) do
    local cost = C_Container.GetContainerItemRepairCost(0, slot)
    if cost > 0 then
        -- broken/damaged item sitting in the backpack
    end
end
```

Useful for "repair only items above N copper" smart-repair logic
without scanning tooltips. ClassicAPI addition; modern WoW has no
direct equivalent.

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
or `nil` otherwise.

```
itemID = C_Container.PlayerHasHearthstone()
```

```lua
if C_Container.PlayerHasHearthstone() then
    -- player can hearth
end
```

**Match logic.** An item counts as a hearthstone if **either**:
1. Its itemID is `6948` (the vanilla Hearthstone — fast path, no
   cache lookup needed), **or**
2. Its on-use spell is spell `8690` (the "Hearthstone" cast itself).

Rule 2 lets custom servers (Turtle WoW, etc.) ship reskinned
hearthstone items with different itemIDs and still have them
recognized — as long as their on-use is the Hearthstone spell.
The return is the **actual matched itemID**, not a hardcoded
constant, so code that wants to do something with the found id
(`SetItemRef` for chat-linking, `GetItemInfo` for the item's
name, etc.) gets the right value for any variant.

> **Cache dependency for rule 2.** The on-use-spell check requires
> the item's `ItemStats_C` record to be in the local cache. Items
> currently in the player's bags are always cached (the engine
> pre-fills the cache during bag sync), so the check is reliable
> for this code path. The fallback to rule 1 (vanilla itemID
> equality) covers the moment-of-login window before the cache is
> fully populated.

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

## MerchantFrame

Backports the six `C_MerchantFrame.*` calls retail addons use when
interacting with a vendor. All entry points read the engine's
merchant/buyback storage directly — no Lua-roundtrip through
`GetMerchantItemInfo` / `GetBuybackItemLink` etc. — and the
`SellAllJunkItems` dispatcher bypasses `UseContainerItem` entirely
by calling the engine's internal `MerchantSellItem` packet builder.

A "merchant frame is currently open" gate (`VAR_MERCHANT_NPC_GUID_*`
non-zero) applies to every function — the same gate retail enforces.
`GetNumJunkItems` returns `0` away from a vendor and the sell
functions are no-ops; the per-slot getters return `nil`.

### `C_MerchantFrame.GetItemInfo(slot)`

Returns a table describing the merchant's item at the given 1-based
slot, or `nil` if no merchant is open / the slot is out of range.

Table fields:

| Field             | Type    | Notes |
|-------------------|---------|-------|
| `itemID`          | number  | Item record key, suitable for `C_Item.GetItemInfoInstant` / cache lookups. |
| `price`           | number  | Copper cost per stack (multiply by `stackCount` for unit price). |
| `stackCount`      | number  | Units delivered per purchase (1 for most equipment, e.g. 5 for stacks of cloth). |
| `numAvailable`    | number  | Limited-supply count, or `-1` for unlimited stock. |
| `isPurchasable`   | boolean | Always `true` in this build — vanilla has no "blocked from buying" flag. |
| `isThrottled`     | boolean | Always `false` — modern's anti-spam concept doesn't apply in vanilla. |
| `hasExtendedCost` | boolean | Always `false` — currency/honor-cost merchants don't exist in vanilla. |

```lua
local info = C_MerchantFrame.GetItemInfo(1)
if info then
    print(info.itemID, info.price, info.stackCount)
end
```

Reads directly from the 28-byte merchant entry at
`VAR_MERCHANT_ITEMS + (slot-1) * MERCHANT_STRIDE` (the same flat
array `Script_GetMerchantItemInfo` walks). Compared to the existing
[`GetMerchantItemID`](#getitemid--companions-to-the-engines-getitemlink-family),
this returns the wider modern struct shape in one call.

Equivalent to the function of the same name introduced in 10.x.

### `C_MerchantFrame.GetBuybackItemID(slot)`

Returns the itemID of the merchant's buyback slot at the given
1-based index (1..12), or `nil` if the slot is empty / no merchant
is open.

```lua
for slot = 1, 12 do
    local id = C_MerchantFrame.GetBuybackItemID(slot)
    if id then
        -- buybackable item at slot
    end
end
```

Resolution chain (engine-direct, no Lua-side `GetBuybackItemLink`
roundtrip):

1. Read the buyback slot's stored invMgr index from
   `VAR_BUYBACK_SLOTS + (slot-1)*4`.
2. Look up that index in the player invMgr's flat GUID array (the
   engine keeps sold items alive in the player's inventory storage,
   not in a separate buyback pool).
3. Resolve the GUID to a `CGItem*` via
   `Item::Location::ResolveByGUID` — same engine helper
   `C_EquipmentSet.GetItemLocations` uses.
4. Read the itemID from the CGItem's instance block at `+0x08 + 0x0C`.

### `C_MerchantFrame.GetNumJunkItems()`

Returns the count of grey-quality (`LE_ITEM_QUALITY_POOR`) items in
the player's bags 0..4 that `SellAllJunkItems` would sell. Returns
`0` when no merchant frame is open — matching retail's behavior of
gating the count on merchant context, since the count is meant as
a "what would the sell-junk button do right now" signal rather than
a passive inventory query.

```lua
-- Inside a MERCHANT_SHOW handler:
local junk = C_MerchantFrame.GetNumJunkItems()
if junk > 0 then
    print("Selling " .. junk .. " junk item(s)")
    C_MerchantFrame.SellAllJunkItems()
end
```

Quality is read from each item's `ItemStats` cache record at the
`m_quality` field (`OFF_ITEMSTATS_QUALITY = 0x1C`). Items with
unloaded cache records (rare for items in your own bags) are
skipped — same conservative behavior the sell path uses.

### `C_MerchantFrame.SellAllJunkItems()`

Sells every quality-0 item in the player's bags to the open
merchant. No-op when no merchant frame is open.

Sells are dispatched **one per frame** via the shared
`WorldTick` subscriber, not in a tight loop within the call —
vanilla's network path drops packets when CMSG_SELL_ITEM is
flooded (a 10-item burst consistently lost the 2nd-to-last sell
in testing). Calling pace matches click-by-click selling. For
10 junk items, expect the queue to drain in ~10 frames (~150ms
at 60fps).

```lua
C_MerchantFrame.SellAllJunkItems()
```

If the player closes the merchant frame or opens a different
vendor mid-drain, the remaining queue is discarded rather than
mis-routed to the new merchant.

Each sell is delivered via the engine's internal
`MerchantSellItem` helper (`FUN_MERCHANT_SELL_ITEM`, opcode
`CMSG_SELL_ITEM`/`0x1A0`) called with `count = 0` ("sell whole
stack"). Bypasses `Script_UseContainerItem` entirely — addons
hooking `UseContainerItem` will not see these sells, and there's
no risk of the use-not-sell dispatch branch firing if the merchant
state changes mid-loop.

### `C_MerchantFrame.IsMerchantItemRefundable(slot)`

Always returns `false`. Vanilla 1.12 has no refund mechanic
(retail's 2-hour buy-back-for-full-price system was introduced
post-vanilla); the function exists for API parity so retail
addons that gate behavior on refundability don't break.

```lua
if not C_MerchantFrame.IsMerchantItemRefundable(slot) then
    -- always taken in this build
end
```

### `C_MerchantFrame.IsSellAllJunkEnabled()`

Always returns `true`. Retail exposes an optional client setting to
disable the sell-all-junk button; vanilla has no such setting, so
the feature is always on. Function exists so retail addons that
gate `SellAllJunkItems` on this don't no-op silently.

## EquipmentSet

Backports the modern `C_EquipmentSet.*` namespace on top of a
client-side persistent store. Vanilla 1.12 had no equipment-set
functionality at all (Blizzard introduced it in 3.1.2 as
`SaveEquipmentSet` etc., then namespaced it into `C_EquipmentSet` in
Legion) — and even when it shipped natively, the data lived
server-side, synced via `SMSG_EQUIPMENT_SET_LIST`. Vanilla servers
don't speak that opcode and won't ever, so each character's sets are
kept in a per-character file under `WTF\Account\...`. The format
matches what `VanillaMinimapTracking` does for its tracking config.

### Overview & file format

Sets are stored in
`WTF\Account\<account>\<realm>\<character>\ClassicAPI_EquipmentSets.txt`
in a line-oriented, human-readable format:

```
# ClassicAPI Equipment Sets v1
set 1
  name=Tanking
  icon=INV_Shield_06
  slot 1 guid=0x0000000040000123
  slot 2 ignored
  slot 5 guid=0x0000000040000789
set 2
  name=Healing
  icon=Spell_Holy_HolyBolt
  ...
```

Identity is by **item GUID**, snapshotted at `CreateEquipmentSet` /
`SaveEquipmentSet` time. `UseEquipmentSet` searches every player-owned
container for those GUIDs and dispatches pickup→equip pairs for items
that aren't already where they belong. The search reads the
underlying GUID arrays directly (same trick `C_Item.GetItemCount`
uses for its `includeBank=true` path), so bank items resolve **even
without the bank window being open** — the bank gate at
`VAR_BANK_GATE_GUID` only suppresses `GetItemBySlot`, not the
underlying data.

One limitation worth knowing about:

- **Equipping from the bank** is not supported by vanilla's protocol
  — `UseEquipmentSet` skips bank items rather than try and fail.
  Retrieve the items first, then re-run.

Modern's signatures take a numeric `iconFileID`; we accept icon path
strings (e.g. `"INV_Shield_06"`) since vanilla has no fileDataID
system. Same string-or-default fallback semantic as 4.3.4 native.

Cap is **10 sets per character**, matching 4.3.4. The full list re-
serializes on every mutation; a corrupted file is harmless (parse
errors leave the in-memory list empty and the next save rewrites the
file from scratch).

> Note: this is a **fresh client-side namespace**, not a polyfill of
> some specific Blizzard build's behavior. The shape mirrors Classic
> Era 1.15.x's `C_EquipmentSet.*` where it can, but anything that
> requires server-side state (cross-character sharing, the
> "Equipment Manager" specialization tab) isn't supported.

### `C_EquipmentSet.CanUseEquipmentSets()`

Returns `true` unconditionally. Vanilla has no banker/feature gate
on equipment-set storage; we ship the feature for every character.
Equivalent to Classic Era's behavior.

### `C_EquipmentSet.GetNumEquipmentSets()`

Returns the count of sets stored for the current character. Loads
the file on first call after login.

### `C_EquipmentSet.GetEquipmentSetIDs()`

Returns a numeric-keyed table of every setID in storage order
(insertion-order; not alphabetical). Empty table when nothing's
saved.

### `C_EquipmentSet.GetEquipmentSetID(name)`

Returns the numeric setID for a set with the given name, or `nil` if
no set by that name exists. Names are exact (case-sensitive, no
trimming).

### `C_EquipmentSet.GetEquipmentSetInfo(setID)`

Returns the nine values modern ships:

```
name, icon, setID, isEquipped,
numItems, numEquipped, numInInventory, numMissing, numIgnored
```

`isEquipped` is `true` when every resolvable item in the set is in
its target slot (missing items don't disqualify — useful so a set
that includes a bank-stored cloak still shows as equipped after you
swap in the rest). `numItems` excludes ignored slots; `numIgnored`
counts them separately. Returns nothing for an unknown setID.

### `C_EquipmentSet.GetIgnoredSlots(setID)`

Returns a numeric-keyed table of slot indices (1..19) that the set
has flagged ignored. Empty table when no slots are ignored. Ignored
slots are recorded **per-set at save time**, by reading the global
`IgnoreSlotForSave` state — not retroactively editable on a saved set.

### `C_EquipmentSet.GetItemIDs(setID)`

Returns a hash table `{ [slot] = itemID }` for every set slot whose
item is currently resolvable. Missing items (GUID stored but client
can't find a CGItem) are omitted because vanilla doesn't keep an
itemID separate from the live CGItem record.

### `C_EquipmentSet.GetItemLocations(setID)`

Returns a hash table `{ [slot] = locationCode }`. Location codes use
the same bit-packed encoding Blizzard's FrameXML
`EquipmentManager_UnpackLocation` decodes (constants in
`Blizzard_FrameXMLBase/Classic/Constants.lua`):

| Bit/field | Meaning |
|-----------|---------|
| `0x00100000` (PLAYER) | Item is in a player slot (equipped or in a player bag) |
| `0x00200000` (BAGS)   | Item is inside a bag (player or bank) |
| `0x00400000` (BANK)   | Item is in the bank (main or bank bag) |
| bits 0..7             | Slot (1-based) within the container |
| bits 8..15            | Bag index — present only when BAGS bit is set |

PLAYER and BANK are **mutually exclusive** in the encoding (the
unpack uses `if player elseif bank`). Bank bags subtract 4 from the
bagID before storing so the field fits cleanly; unpack reverses this
to give back bag IDs 5..10. Composition:

| Location | Bits | Example |
|----------|------|---------|
| Equipped (paperdoll slot 1..19) | `PLAYER | slot` | `0x100007` = legs (slot 7) |
| Backpack / player bag 1..4 | `PLAYER | BAGS | (bag<<8) | slot` | `0x300205` = bag 2 slot 5 |
| Main bank slot 1..28 | `BANK | slot` | `0x400003` = bank slot 3 |
| Bank bag 5..10 | `BANK | BAGS | ((bag-4)<<8) | slot` | `0x600101` = bank bag 5 slot 1 |

Special values returned in the table entry:
- `1` — slot is ignored (`GetIgnoredSlots` lists these)
- `-1` — item is missing (was in the set, can't find now)

The packed codes pass cleanly through Blizzard's
`EquipmentManager_UnpackLocation` if you want to use the
shared-FrameXML helpers in your addon UI.

### `C_EquipmentSet.CreateEquipmentSet(name [, icon])`

Snapshots the player's currently-equipped items into a new set and
returns its setID. Honors `IgnoreSlotForSave` — slots flagged ignored
at call time get the ignored marker instead of the equipped item's
GUID.

Returns the new setID on success. Returns nothing if:
- the name is empty or already in use
- the cap of 10 sets is reached

`icon` defaults to `"INV_Misc_QuestionMark"` if omitted.

### `C_EquipmentSet.SaveEquipmentSet(setID [, icon])`

Overwrites an existing set's contents with the player's currently-
equipped items. Same ignored-slot handling as `CreateEquipmentSet`.
If `icon` is provided it replaces the set's previous icon; otherwise
the icon is left unchanged.

### `C_EquipmentSet.ModifyEquipmentSet(setID, newName)`

Renames an existing set. Fails silently if the new name is empty,
already in use by a different set, or the setID doesn't exist.

### `C_EquipmentSet.DeleteEquipmentSet(setID)`

Removes the set. SetIDs are not reused — the next `Create` call
allocates one higher than any seen.

### `C_EquipmentSet.IgnoreSlotForSave(slot)` / `UnignoreSlotForSave` / `IsSlotIgnoredForSave` / `ClearIgnoredSlotsForSave`

Global "skip this slot the next time `CreateEquipmentSet` or
`SaveEquipmentSet` runs" state, indexed by 1-based slot (1..19).
Persists for the rest of the session; not written to the WTF file.
Use it when building a set that should leave (say) the tabard slot
free — set the ignore flag, then call `CreateEquipmentSet`, then
optionally `ClearIgnoredSlotsForSave()` afterward.

### `C_EquipmentSet.EquipmentSetContainsLockedItems(setID)`

Returns `true` if any item in the set is currently flagged "locked"
by the engine — a pending pickup or use is in flight that
`UseEquipmentSet` would race with. Reads bit 2 (`0x04`) of
`ITEM_FIELD_FLAGS` for each resolvable item in the set.

### `C_EquipmentSet.UseEquipmentSet(setID)`

Walks the set and dispatches one **atomic server-side swap** per item
that isn't already in its target slot. Items in the bank are skipped
silently (vanilla can't equip from bank). Missing items are skipped
silently. Returns `true` if the call ran (the set existed), `false`
otherwise.

Implementation uses the same `FUN_INVENTORY_SWAP` primitive
[`C_Item.EquipItemByName`](#c_itemequipitembynameiteminfo--dstslot)
uses for its explicit-slot path. Each swap is a single
CMSG_SWAP_INV_ITEM (or CMSG_AUTOEQUIP_ITEM) packet that the server
applies atomically — the two-cycle "ring A in slot 11, ring B in
slot 12, set swaps them" case resolves in one packet because the
opcode swaps both slots in a single server transaction. The cursor
is never touched; an item held on the cursor when
`UseEquipmentSet` is called stays on the cursor.

Longer dependency chains (3+ items rotating) are rare with 1.12's
slot set — the realistic conflicts are paired slots (rings 11/12,
trinkets 13/14, weapons 16/17), all 2-cycles.

> **`CURSOR_UPDATE` fires per item moved.** The engine's swap
> primitive (`FUN_005E0C40`) runs a generic cursor-state cleanup at
> the end of each call, which fires `CURSOR_UPDATE` regardless of
> whether the cursor was actually touched. So one
> `UseEquipmentSet` call that moves N items will fire N
> `CURSOR_UPDATE` events in quick succession. Addons that react to
> `CURSOR_UPDATE` (cursor-attached tooltips, drag-state tracking)
> should debounce — or check `CursorHasItem()` before doing work —
> since most of those fires won't reflect a real cursor change.
> This is engine behavior, not a bug in the implementation; the
> old cursor-based path actually fired more (one per pickup, one
> per equip, one per cursor-clear).

### `EQUIPMENT_SETS_CHANGED` event

Fires (with no payload) after any mutation: `Create`, `Save`,
`Modify`, `Delete`, and the four `*IgnoredSlot*` calls. Addon UI
should re-read its set list / button state when this fires.

### `EQUIPMENT_SWAP_PENDING` event

Fires with a single payload arg — `setID` — at the **start** of
`UseEquipmentSet`, right after the set-exists check passes and
before any pickup/equip work begins. Modern addons use this to
gate "swap in progress" UI state (grey out the set button, show
a spinner, etc.) until `EQUIPMENT_SWAP_FINISHED` arrives.

Doesn't fire if `UseEquipmentSet` is called with an unknown
setID — in that case only `EQUIPMENT_SWAP_FINISHED(false, setID)`
fires.

### `EQUIPMENT_SWAP_FINISHED` event

Fires at the end of every `UseEquipmentSet` call with two payload
args: `success` (1 if the set existed and we dispatched the swap, 0
if the setID was unknown) and `setID`. Note this is "we ran the
dispatch" success — not "every item ended up in its target slot."
Items that were in the bank or that couldn't complete a swap cycle
in one pass still report success=1. Listen for this if you want to
re-paint the character pane / refresh tooltips after a swap.

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
resolved unit's GUID is empty (e.g. `"target"` with nothing targeted,
empty party/raid slot).

```lua
local guid = UnitGUID("player")  -- "0x0000000000000777" (low IDs are local-realm characters)
local guid = UnitGUID("target")  -- "0xF13000059A002553" (the F130... prefix tags creatures)
local guid = UnitGUID("party1")  -- works even if party1 is on a different continent
```

**Works for OOR party / raid members.** Earlier versions of this
function returned nil for `"partyN"` / `"raidN"` when the member's
CGObject wasn't currently active in the client (e.g. on another
continent, in a different zone phase). We now read from the engine's
parallel group-roster GUID array (populated by `SMSG_GROUP_LIST`),
which is independent of unit visibility.

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

### `GetUnitSpeed(unit)`

Returns `currentSpeed, runSpeed, flightSpeed, swimSpeed` — all four
in yards/second. Modern WoW signature exactly; 1.12 doesn't have
flying, so `flightSpeed` is always `0`.

```lua
local current, run, _, swim = GetUnitSpeed("player")
-- e.g. 7.0, 7.0, 0, 4.722 for an unmounted player running normally
-- e.g. 14.0, 14.0, 0, 4.722 with a 100% mount active (run speed
--      reflects the modifier; swim ignores mount)
```

- **`currentSpeed`** — speed the engine would apply to this frame's
  movement step. `0` when stationary; otherwise the walk/run/swim/
  backward variant the unit is actually moving with. Read via the
  engine's own selector at `0x007C4C90` so movement-flag handling
  (walking, swimming, taxi paths) matches the engine exactly.
- **`runSpeed`** — raw forward-run speed from MovementInfo `+0x8C`.
  Includes mount / buff / debuff multipliers — the engine maintains
  this field as the post-modifier value, updated by
  `SMSG_FORCE_RUN_SPEED_CHANGE` and friends.
- **`flightSpeed`** — always `0` in 1.12.
- **`swimSpeed`** — raw forward-swim speed from MovementInfo `+0x94`.

All four returns are `0` if the token doesn't resolve to a live
CGUnit — empty `"target"`, out-of-range party member, etc. Matches
3.3.5's `Script_GetUnitSpeed` behavior of pushing `0.0` rather than
nil for non-visible units.

Reads `[CGUnit + 0x118]` to get the MovementInfo pointer, then
reads speed fields directly. Field offsets verified via the
movement-prediction loop (`FUN_005FC350` and the sprint-jump
calculator at `0x00511920` both reach into the same struct).

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

### `UnitStandState(unit)`

Returns the unit's standstate as an integer, matching the modern
`Enum.PlayerStandState` values:

| Value | Meaning |
|------:|---------|
| `0` | STANDING |
| `1` | SITTING |
| `2` | SITTING_CHAIR |
| `3` | SLEEPING |
| `4` | SITTING_LOW_CHAIR |
| `5` | SITTING_MEDIUM_CHAIR |
| `6` | SITTING_HIGH_CHAIR |
| `7` | DEAD |
| `8` | KNEELING |

Reads the low byte of `UNIT_BYTES_1` (descriptor `+0x210`), a
broadcast UpdateField — works for any synced unit (player, target,
party*, raid*, mouseover, etc.). Returns `0` (STANDING) for
unresolvable units (empty `party*` slot, no current target, etc.)
matching the modern behavior of returning a safe default.

```lua
UnitStandState("player")    -- 0 standing, 1 sitting, 5 chair-sit, …
UnitStandState("target")    -- works for any visible unit
UnitStandState("party1")    -- 0 if the slot is empty
```

1.12 has `IsSitOrStanding()` (local-player boolean) but no
unit-token form; `UnitStandState` fills the gap and exposes the
full enum.

### `UnitClassBase(unit)`

Returns `(classFile, classID)` — the locale-independent class
token plus the numeric classID. The token is one of
`"WARRIOR"`, `"PALADIN"`, `"HUNTER"`, `"ROGUE"`, `"PRIEST"`,
`"SHAMAN"`, `"MAGE"`, `"WARLOCK"`, `"DRUID"`; the classID matches
the integer `UnitClass`'s third return surfaces (1=Warrior,
2=Paladin, 3=Hunter, 4=Rogue, 5=Priest, 7=Shaman, 8=Mage,
9=Warlock, 11=Druid — 6 and 10 are post-vanilla).

```lua
local token, id = UnitClassBase("player")    -- "WARRIOR", 1
local token = UnitClassBase("target")        -- works for any synced unit
local color = RAID_CLASS_COLORS[UnitClassBase("party1")]
```

Modern addons use the token for class detection because vanilla's
`UnitClass(unit)` returns a localized first return (e.g.
`"Krieger"` on a German client), which is fine for display but
breaks any addon code that keys on `if class == "WARRIOR"`. The
classID second return is a vanilla extension — modern's
`UnitClassBase` returns the token only — but it saves callers
from chaining `UnitClass(unit)` just to get the integer.

Reads the class byte from `UNIT_FIELD_BYTES_0` (descriptor `+0x79`,
the same byte `UnitClass`'s general-token path reads) and looks up
the english filename at `ChrClasses.dbc::Filename` (record `+0x38`).
Both fields are broadcast / static, so remote players' class is
current regardless of distance.

Returns `(nil, nil)` for:
- `target` with no current target, empty `partyN` / `raidN` slots,
  or units whose descriptor isn't loaded yet
- creatures and other non-player units (their class byte indexes a
  different table; the class-byte lookup naturally returns nil for
  out-of-range / zero values)

Throws a Lua error on missing / non-string `unit` argument — same
shape as `UnitClass` itself.

## UnitAuras

Backport of the modern `C_UnitAuras` namespace. Returns
`AuraData`-shaped tables instead of vanilla's `UnitBuff` /
`UnitDebuff` multi-return tuples, so modern addon code that does
`local d = C_UnitAuras.GetAuraDataByIndex(unit, 1); if d.dispelName ==
"Magic" then ...` works unchanged.

Reads everything off the unit's `m_objectFields` descriptor — same
data source `UnitBuff` / `UnitDebuff` use. The descriptor has 48 aura
slots total: 32 helpful (buffs) at indices 0..31, 16 harmful
(debuffs) at indices 32..47. Functions in this namespace take a
1-based Lua index that translates onto whichever range the filter
selects.

### `AuraData` table shape

| Field | Type | Source / value on vanilla |
|---|---|---|
| `name` | string | localized spell name from `Spell.dbc` |
| `icon` | string | icon path from `SpellIcon.dbc` |
| `applications` | number | stack count (engine stores `stacks-1`, we display `+1`) |
| `spellId` | number | spell ID from the descriptor's aura array |
| `dispelName` | string | `"Magic"` / `"Curse"` / `"Disease"` / `"Poison"` (from `SpellDispelType.dbc`), or `""` if non-dispellable |
| `isHelpful` | boolean | true for slot < 32 |
| `isHarmful` | boolean | true for slot >= 32 |
| `duration` | number | always `0` — vanilla server doesn't broadcast remaining time |
| `expirationTime` | number | always `0` (same reason) |
| `charges` / `maxCharges` | number | always `0` — vanilla has stacks, not charges |
| `timeMod` | number | always `1` — vanilla has no haste-affected auras |
| `isStealable`, `isBossAura`, `isFromPlayerOrPlayerPet`, `isNameplateOnly`, `nameplateShowAll`, `nameplateShowPersonal`, `canApplyAura`, `shouldConsolidate`, `isRaid` | boolean | always `false` — modern UI concepts vanilla doesn't have |
| `sourceUnit`, `auraInstanceID`, `points` | (absent) | omitted from the table — Lua read yields nil, matching modern semantics for "field doesn't apply" |

### Filter parsing

The optional `filter` string is a pipe-separated set of upper-case
tokens, matching modern syntax (`"HELPFUL"`, `"HARMFUL"`,
`"HELPFUL|PLAYER"`, etc.). Only `HELPFUL` (default) and `HARMFUL`
are honored on vanilla; other tokens (`PLAYER` / `RAID` /
`CANCELABLE` / `INCLUDE_NAME_PLATE_ONLY`) are accepted but no-op —
they'd need source-GUID tracking or systems vanilla doesn't have.

### `C_UnitAuras.GetAuraDataByIndex(unit, index [, filter])`

Returns the `AuraData` table for the `index`-th aura (1-based)
matching `filter`, or `nil` if the unit has fewer than `index`
matching auras. Empty / non-visible descriptor slots are skipped
during iteration, so consecutive indices return consecutive *active*
auras the same way `UnitBuff` does.

```lua
local d = C_UnitAuras.GetAuraDataByIndex("player", 1, "HELPFUL")
if d then
    print(d.name, d.spellId, d.dispelName, d.applications)
end
```

### `C_UnitAuras.GetBuffDataByIndex(unit, index)` / `GetDebuffDataByIndex(unit, index)`

Convenience wrappers locking the filter to `HELPFUL` or `HARMFUL`
respectively. Equivalent to `GetAuraDataByIndex(unit, index,
"HELPFUL")` / `"HARMFUL"`.

### `C_UnitAuras.GetUnitAuraBySpellID(unit, spellID [, filter])`

Linear-searches `unit`'s aura array for the first populated slot
whose `spellId` matches and returns its `AuraData`, or `nil` if not
found. Without a filter, searches both ranges (helpful first, then
harmful).

```lua
-- Is the player blessed with Wisdom right now?
local d = C_UnitAuras.GetUnitAuraBySpellID("player", 25290)
if d then print("yes, with", d.applications, "stacks") end
```

### `C_UnitAuras.GetPlayerAuraBySpellID(spellID)`

Shorthand for `GetUnitAuraBySpellID("player", spellID)` — the most
common consumer pattern (WeakAuras-style aura tracking).

### `C_UnitAuras.GetUnitAuras(unit [, filter])`

Returns a numerically-indexed array of `AuraData` tables for every
populated aura on `unit`. Without a filter the array is helpful auras
followed by harmful; with a filter it's restricted to one range.
Empty array (`{}`) if the unit has no auras or doesn't resolve.

```lua
for _, aura in ipairs(C_UnitAuras.GetUnitAuras("target")) do
    print(aura.spellId, aura.dispelName, aura.isHarmful)
end
```

### `C_UnitAuras.GetAuraDispelTypeColor(dispelName)`

Returns a `{r, g, b, a}` table for the given dispel-type name,
matching modern FrameXML's `DebuffTypeColor` values:

| dispelName | r | g | b |
|---|---|---|---|
| `"Magic"` | 0.20 | 0.60 | 1.00 |
| `"Curse"` | 0.60 | 0.00 | 1.00 |
| `"Disease"` | 0.60 | 0.40 | 0.00 |
| `"Poison"` | 0.00 | 0.60 | 0.00 |
| `"Enrage"` | 1.00 | 0.55 | 0.00 |
| (anything else, including `""`) | 0.80 | 0 | 0 |

Returns a `ColorMixin` instance the same way modern does — the C
function `pcall`s Lua's `CreateColor(r, g, b, a)` (defined in
`!!!ClassicAPI/Util/Color.lua`) and returns whatever table it
builds. So the returned value carries the mixin methods
(`GetRGB`, `GenerateHexColorMarkup`, etc.) in addition to the
`.r/.g/.b/.a` fields. Falls back to a plain `{r,g,b,a}` table if
`CreateColor` isn't loaded yet (shouldn't happen — `!!!ClassicAPI`
loads first thanks to the triple-`!` prefix).

## NameCache

GUID-keyed cache of player names and classes. The engine itself
maintains an in-memory `NameCache` at `0x00C0E228`, populated by
`SMSG_NAME_QUERY_RESPONSE` — but vanilla doesn't expose it to Lua,
and it doesn't survive `/reload`. This module surfaces it as
`GetPlayerInfoByGUID`, adds an opt-in **persistent** layer that
survives `/reload` and full client restarts, and (separately
toggleable) sweeps the engine's visible-object list to populate
entries the SMSG path would never reach on its own.

Two toggles, each independent:

- **`C_PlayerCache.SetEnabled`** — turn on the on-disk cache.
  Off by default. Without this, the module is read-only against
  the engine's transient in-memory cache.
- **`C_PlayerCache.SetScanEnabled`** — turn on the visible-object
  sweep. Off by default. Requires the on-disk cache to be enabled
  to have any effect.

Three population sources feed the cache when fully enabled:

1. **`SMSG_NAME_QUERY_RESPONSE` hook** (always active when the
   on-disk cache is on). Every name-query response the engine
   processes is mirrored to disk. Covers chat, group/raid sync,
   guild updates, visible-object resolution — anything the engine
   itself issues a name query for.
2. **`C_PlayerCache.RememberPlayer`** (always available, no-op
   when the cache is off). Lets addons feed in sources the engine
   never sees: `/who` results, etc.
3. **Visible-object sweep** (active when both toggles are on).
   Walks the engine's live visible-object list every ~10 seconds
   on the `Frame::RegisterEvent` hook, resolving each player and
   feeding name + class. Picks up nearby players whether or not
   the engine ever queried them.

Storage:

- `WTF\Account\<account>\ClassicAPI.txt` — account-level settings
  (`PersistentNameCacheEnabled`, `NameCacheScanEnabled`).
- `WTF\Account\<account>\<realm>\ClassicAPI_NameCache.txt` —
  per-realm cache. Tab-separated text, ~30 bytes/entry,
  hand-editable. Shared across all characters on the same
  account+realm (a 50-character bank alt's chat scraping doesn't
  double the storage cost).

### `GetPlayerInfoByGUID(guid)`

Returns
`localizedClass, englishClass, localizedRace, englishRace, sex, name, realm`
for a player GUID the engine has cached, or `nil` on a cache miss.

```lua
local _, class, _, race, sex, name = GetPlayerInfoByGUID(UnitGUID("target"))
-- e.g. "WARRIOR", "NightElf", 2, "Sylphir"

GetPlayerInfoByGUID("0x0000000000000777")  -- same shape for a literal GUID
```

`guid` is the `"0xHHHHHHHHLLLLLLLL"` string returned by `UnitGUID`.
Bare 8-hex `"0xLLLLLLLL"` (hi-dword zero) is also accepted.

Returns:

| 1 `localizedClass` | `"Warrior"` / `"Krieger"` etc. — from `ChrClasses.dbc` indexed by locale. |
| 2 `englishClass`   | `"WARRIOR"` (uppercase tag, same value `UnitClass` returns as 2nd return on modern clients) — `ChrClasses.dbc` filename field. |
| 3 `localizedRace`  | `"Human"` / `"Mensch"` / `"Humain"` etc. — from `ChrRaces.dbc` indexed by locale. |
| 4 `englishRace`    | `"Human"`, `"Orc"`, `"Dwarf"`, `"NightElf"`, `"Scourge"` (vanilla's filename for what addons call Undead), `"Tauren"`, `"Gnome"`, `"Troll"` — from `ChrRaces.dbc` filename field. |
| 5 `sex`            | `2` = male, `3` = female. Matches `UnitSex` convention (the cache stores `0`/`1`; we add `2`). |
| 6 `name`           | Character name. |
| 7 `realm`          | Realm name. Single-realm in vanilla, so usually the local realm. |

**Cache coverage**: the engine populates entries from
`SMSG_NAME_QUERY_RESPONSE`. Anything the client has already seen
populates the cache: chat (whispers, says, party chat), raid/group
events, guild updates, visible objects (target, party, raid in
zone). Names of offline friends never seen in chat are *not*
cached — `GetPlayerInfoByGUID` returns `nil` for them until the
client does something that triggers a name query. This module
deliberately does not trigger queries from a passive getter; a
future `C_PlayerInfo.RequestLoadPlayerByID` would do that
explicitly and fire a load-result event.

**Persistent fallback**: when
[`C_PlayerCache.SetEnabled(true)`](#c_playercachesetenabledenabled)
has been opted into, a per-realm on-disk cache extends coverage
across sessions. On engine cache miss, `GetPlayerInfoByGUID` falls
back to the persistent cache and returns `name`, `class`, `race`,
and `sex` from storage (`realm` comes back as `""` since vanilla
is single-realm and we don't carry per-player realm names).
Returns `nil` only when both the engine and persistent caches miss.

**Implementation**: calls the engine's get-or-fetch primitive at
`0x0055F080` with a NULL callback (pure cache read). The cache
instance lives at `0x00C0E228`; entry layout (name, realm, race,
sex, class) was reverse-engineered from the
`SMSG_NAME_QUERY_RESPONSE` write path at `0x0055F310`.

### `C_PlayerCache.GetPlayerInfoByName(name)`

Returns
`localizedClass, englishClass, localizedRace, englishRace, sex, name, realm, guid`
for a player in the persistent NameCache, or `nil` if the name
isn't cached. Companion to `GetPlayerInfoByGUID` for the case where
an addon has a player's name but not their GUID — e.g., when
`GetCurrentChatGUID()` returns nil for a chat event the engine
didn't tag with the sender GUID (system notifications, some channel
messages, etc.).

```lua
local _, class, _, race, _, _, _, guid = C_PlayerCache.GetPlayerInfoByName("Gedwyr")
-- "MAGE", "Human", "0x000000003B9ADAA7"

C_PlayerCache.GetPlayerInfoByName("NeverSeen")  -- nil (not cached)
```

Match is **case-sensitive, exact** — vanilla server-stored names
are case-stable (`"Gedwyr"` won't match `"gedwyr"`).

Returns (vs. `GetPlayerInfoByGUID`):

| Slot | Value | Notes |
|------|-------|-------|
| 1–7  | same as `GetPlayerInfoByGUID` | see above |
| 8    | `guid` | `"0xHHHHHHHHLLLLLLLL"` — the cached player's full GUID, so name-based callers can chain into GUID-keyed APIs. Absent from `GetPlayerInfoByGUID` since the caller already has it. |

Only reads from the **persistent NameCache** — does *not* hit the
engine's in-memory NAME_QUERY cache (that's keyed by GUID and has
no name index). So this requires `C_PlayerCache.SetEnabled(true)`
to have been opted into; entries get there via the engine's
NAME_QUERY response hook, the visible-object scan, and
`C_PlayerCache.RememberPlayer`.

**Implementation**: O(1) lookup via a `name → guid` index map
maintained in lockstep with the GUID-keyed entry map. Eviction
keeps the index consistent: when a name's character is deleted and
the name is recycled to a different GUID, the prior entry is
removed from both maps.

### `C_PlayerCache.RememberPlayer(guid, name, classToken)`

Adds a `(guid → name, classID, raceID, sex)` entry to the persistent
name cache. For the engine-driven coverage (chat / group / guild /
visible objects), no addon-side feeding is needed — those flow into
the cache automatically through the engine's
`SMSG_NAME_QUERY_RESPONSE` write path. This call exists for the
*other* sources libunitscan-style addons harvest from but the engine
NameCache doesn't see directly: `/who` results, mouseover, target
snapshots, etc.

```
C_PlayerCache.RememberPlayer(guid, name, classToken [, raceToken [, sex]])
```

Returns `true` on success, `false` if the persistent cache isn't
enabled or the required args are malformed.

- `guid` — `"0xHHHHHHHHLLLLLLLL"` string (same format `UnitGUID`
  returns). 8-hex `"0xLLLLLLLL"` is also accepted (hi-dword zero).
- `name` — 1–12 ASCII chars (vanilla character-name range). Tabs,
  newlines, and high bytes are stripped.
- `classToken` — uppercase token like `"WARRIOR"`, `"MAGE"`. Looked
  up against `ChrClasses.dbc` filename field, case-insensitive.
  Passing an unknown token keeps the entry's prior class (so a
  name-only sighting doesn't erase good class data).
- `raceToken` *(optional)* — uppercase token like `"NIGHTELF"`,
  `"SCOURGE"` (vanilla's filename for Undead). Same resolution as
  classToken, against `ChrRaces.dbc`. Omitted/unknown → leaves
  prior race alone.
- `sex` *(optional)* — `0` (male) or `1` (female), matching the
  wire-format convention the engine cache stores. Modern WoW's
  `UnitSex` uses `2`/`3`; pass `UnitSex - 2` if you're forwarding
  that value. Omitted/`0` → leaves prior sex alone (so this call
  can't be used to flip a stored value back to male; the
  `SMSG_NAME_QUERY_RESPONSE` hook handles direct assignment).

```lua
-- Harvest /who results into the persistent cache
local function OnWhoUpdate()
    for i = 1, GetNumWhoResults() do
        local name, _, _, _, class, _, _, _, _, _, _, sex = GetWhoInfo(i)
        -- class/race come back localized from GetWhoInfo; the
        -- locale-token tables in pfUI/AceLocale handle the
        -- inverse mapping. RememberPlayer expects engine tokens
        -- ("WARRIOR" etc.).
    end
end
```

Each non-zero field is updated on the existing entry; zeros are
treated as "caller doesn't know" and preserve prior real data. A
`name`-only update (classToken passed but unknown) refreshes the
name without erasing class.

The "deleted character recreated with same name, different class"
collision case that name-keyed caches suffer from doesn't apply
here: GUIDs are permanent for the life of a vanilla character, so
the new character has a different GUID and gets a different cache
entry.

### `C_PlayerInfo.GUIDIsPlayer(guid)` / `GUIDIsCreature` / `GUIDIsPet` / `GUIDIsGameObject`

Type checks on the raw 1.12 GUID format. Vanilla GUIDs encode the
entity type in the high 16 bits of the qword — players have
`0x0000` (low dword = player ID), creatures `0xF130xxxx`, pets
`0xF140xxxx`, game objects (herbs / chests / etc.) `0xF110xxxx`,
items `0x4000xxxx`. Each function returns `true` only for its
specific prefix; the `"0x0000000000000000"` sentinel returns
`false` from all four.

```lua
if C_PlayerInfo.GUIDIsPlayer(UnitGUID("target")) then
    -- target is a player
end

-- Combat-log row triage: was that a player kill or a mob kill?
local function OnCombatLogEvent(_, _, eventType, srcGUID, ...)
    if eventType == "PARTY_KILL" then
        if C_PlayerInfo.GUIDIsPlayer(srcGUID) then ... end
    end
end
```

`GUIDIsPlayer` matches modern WoW's signature exactly; the other
three are companions for the most common type-distinction needs.
For other types (corpse, dynamic object, transport, item) the
underlying classifier exists internally — let us know if you need
one exposed.

Accepts either the 16-digit `"0xHHHHHHHHLLLLLLLL"` form or the
8-digit `"0xLLLLLLLL"` shortcut (high dword implicitly zero).
Malformed input returns `false` rather than raising — matching
modern's tolerance for stale GUIDs from addon-side caches.

### `C_CreatureInfo.GetCreatureID(guid)`

Extracts the creature template / NPC ID from a unit GUID. Vanilla
1.12 packs the entry ID directly into bits 24-47 of the 64-bit
GUID for the types that carry one; this function does the shift
and mask so addons don't have to.

```lua
C_CreatureInfo.GetCreatureID(UnitGUID("target"))   -- 1842 for Hogger
C_CreatureInfo.GetCreatureID(UnitGUID("pet"))      -- pet's creature template ID
C_CreatureInfo.GetCreatureID(UnitGUID("player"))   -- nil (no entry on players)
```

Accepts creature GUIDs (`0xF130xxxx…`) and pet GUIDs
(`0xF140xxxx…`). Returns `nil` for:
- non-string input or malformed GUIDs
- player GUIDs — the low 32 bits hold a player ID, not a template
- game-object / dynamic-object / corpse / item GUIDs — modern's
  `C_CreatureInfo` doesn't surface entry IDs for these even though
  the bits are in the same range; addons that need them can shift
  the raw GUID themselves (`(guid >> 24) & 0xFFFFFF`)
- entry IDs of 0 — the engine never assigns 0; treated as "no info"

The standard 16-digit (`"0xHHHHHHHHLLLLLLLL"`) and 8-digit
(`"0xLLLLLLLL"`) GUID-string forms are both accepted, same as
`C_PlayerInfo.GUIDIs*` above.

### `C_PlayerCache.SetEnabled(enabled)`

Opts into (or out of) the persistent name cache. `enabled` is a
boolean (numeric `0`/`1` also accepted). Persists to
`WTF\Account\<account>\ClassicAPI.txt` as
`PersistentNameCacheEnabled=1`/`0`, so the choice survives
client restarts.

When enabled:

- Every `SMSG_NAME_QUERY_RESPONSE` the engine processes is also
  written to `WTF\Account\<account>\<realm>\ClassicAPI_NameCache.txt`.
- Lua-side `C_PlayerCache.RememberPlayer` calls become effective
  (they're no-ops when the cache is disabled).
- `GetPlayerInfoByGUID` gains the cross-session fallback path
  documented above.
- [`SetScanEnabled`](#c_playercachesetscanenabledenabled)
  becomes effective if also turned on.

When disabled, the on-disk file is left in place (re-enabling later
restores the prior contents); future writes are simply suppressed.

```lua
C_PlayerCache.SetEnabled(true)
```

Returns nothing.

### `C_PlayerCache.IsEnabled()`

Returns the current state of the persistent name cache as a
boolean. `false` until
[`SetEnabled`](#c_playercachesetenabledenabled)
has been called (or its prior call survived in the on-disk settings
file).

```lua
if C_PlayerCache.IsEnabled() then
    -- persistent fallback is active for GetPlayerInfoByGUID
end
```

### `C_PlayerCache.SetScanEnabled(enabled)`

Opts into the **visible-object sweep**: an opportunistic walk of
the engine's live visible-object list that feeds every player in
render range (whose object is currently loaded by the client) into
the persistent name cache. Throttled to once per ~10 seconds, on
the existing `Frame::RegisterEvent` hook — no per-frame overhead,
no extra hooks installed.

`enabled` is a boolean (numeric `0`/`1` also accepted). Persists to
`WTF\Account\<account>\ClassicAPI.txt` as
`NameCacheScanEnabled=1`/`0`.

```lua
C_PlayerCache.SetEnabled(true)        -- prerequisite
C_PlayerCache.SetScanEnabled(true)
-- Now: standing in Stormwind for a minute pre-populates the cache
-- with every visible player's name and class.
```

**Independent of the cache toggle.** Turning this on alone has no
effect — the sweep relies on `Remember()` which silently no-ops
when the on-disk cache is disabled. Turning the cache off (without
also turning the scan off) preserves the scan setting for the next
time you re-enable the cache.

**Players only.** NPCs and pets aren't cached — their GUIDs are
ephemeral and can get reused across sessions, so caching them is
worse than useless.

**Implementation**: walks `ClntObjMgrEnumVisibleObjects`
(`0x00468380`) — the same engine iterator
[VanillaMinimapTracking](https://github.com/Brues/VanillaMinimapTracking)
uses for blip rendering — filtering each GUID through
`ClntObjMgrObjectPtr` (`0x00468460`) with `TYPEMASK_PLAYER`. Name
comes from the CGObject vftable's `GetName` slot; class is the
byte at `[m_objectFields + 0x79]` (UNIT_FIELD_BYTES_0 byte 1, same
field `Script_UnitClass` reads).

### `C_PlayerCache.IsScanEnabled()`

Returns the current visible-object scan setting as a boolean.
Independent of the cache toggle — this only reflects the scan-
specific setting, not whether the scan is *effectively* running
(which also requires the cache to be on).

```lua
if C_PlayerCache.IsScanEnabled()
    and C_PlayerCache.IsEnabled() then
    -- visible-object sweeps are running every ~10s
end
```

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

### `IsAssistingRitual()`

Returns `true` if the local player is currently clicking a warlock
summoning portal (the channel-on-GameObject state with no castbar
UI, where movement breaks the channel), `false` otherwise.

This is distinct from spell channeling: the function fires for
*participants* who clicked the portal, not the warlock who cast
Ritual of Summoning. Vanilla has no Lua surface for this state —
the engine's `SPELLCAST_CHANNEL_*` events don't fire and
`CastingInfo()` returns nothing — so addons that want to react to
the player being committed to a ritual (e.g. suppress autorun
toggles, hide nameplate clicks, warn before movement) have no other
way to detect it.

Detection uses two state slots:

- `UNIT_CHANNEL_SPELL` (descriptor `+0x228`) — the warlock's channel
  spell ID (698) is mirrored onto every clicker for the duration
  of the ritual.
- A CGPlayer-local pointer at `+0xB4` — the engine's "current spell
  cast target GameObject" slot, set to the portal GO while the
  clicker is engaged.

Either signal alone is ambiguous (the warlock channeling matches
the first; mining and other GO-targeted casts match the second);
the conjunction is portal-clicker-specific.

The engine fires `SPELLCAST_CHANNEL_START` / `SPELLCAST_CHANNEL_STOP`
on the portal click, even though there's no castbar — Blizzard's
vanilla castbar UI filters out spell ID 698 (Ritual of Summoning),
but the events themselves fire normally. Combine them with this
function for ritual-specific triggers:

```lua
local frame = CreateFrame("Frame")
frame:RegisterEvent("SPELLCAST_CHANNEL_START")
frame:RegisterEvent("SPELLCAST_CHANNEL_STOP")
frame:SetScript("OnEvent", function()
    if event == "SPELLCAST_CHANNEL_START" and IsAssistingRitual() then
        -- player just committed to a ritual portal
    elseif event == "SPELLCAST_CHANNEL_STOP" then
        -- channel ended; was it a ritual?
    end
end)
```

Local-player only — the `+0xB4` pointer lives on the CGPlayer
object, not in the broadcast descriptor, so the function can't
report state for `target` / `party*` / etc. Returns `false` when
called before the player object is initialized (login screen).

## CharacterList

### `GetSavedCharacterOrder(realm)` / `SetSavedCharacterOrder(realm, order)` — GlueXML only

> **GlueXML scope.** These globals are registered on the **glue**
> Lua state (the engine that runs login / realm-select /
> character-select), not the in-world FrameScript engine. In-world
> addons can't see them — `/script GetSavedCharacterOrder("X")`
> from inside the world will error on a nil global. They exist
> specifically to support GlueXML patches that reorder the
> character-select list and need to persist that order across
> sessions; 1.12 glue ships no general-purpose persistence API
> (just `GetSavedAccountName`/`SetSavedAccountName`, which is
> saturated by the autologin system).

- `GetSavedCharacterOrder(realm) -> string` — returns the saved
  order string for `realm`, or `""` if none. Never returns `nil`.
- `SetSavedCharacterOrder(realm, order)` — writes the order
  string. Calling with `order == ""` clears the realm's entry.

Both args are opaque UTF-8 strings. The caller picks the encoding
for `order`; the DLL stores it verbatim. The expected pattern
(matching the anniversary-client reorder UI) is a pipe-delimited
list of character names: `"Thrall|Jaina|Sylvanas"`.

Persistence: `WTF\Account\<account>\ClassicAPI.txt`, one
`CharacterOrder.<realm>=<order>` line per realm. Account scope is
implicit via the file path (so switching autologin accounts on the
glue screen automatically isolates per-account orderings); realm
scope is explicit via the line key. The same file is shared with
the persistent name cache settings — see [`Settings::Account`
in the source](../src/settings/Account.h) — no file-level
coordination needed at the call site.

```lua
-- GlueXML, e.g. CharacterSelect.lua: persist the user's drag-reordered list
local realm = GetServerName() or ""
local names = {}
for i = 1, GetNumCharacters() do
    table.insert(names, (GetCharacterInfo(translationTable[i])))
end
SetSavedCharacterOrder(realm, table.concat(names, "|"))

-- On the next CHARACTER_LIST_UPDATE: read the saved order back
local saved = GetSavedCharacterOrder(realm)
if saved ~= "" then
    for name in string.gfind(saved, "([^|]+)") do
        -- reconcile against the current GetCharacterInfo names
    end
end
```

> **How it works.** ClassicAPI hooks `FUN_0046ABB0` — the engine's
> master glue-init function, which runs once per glue boot (initial
> launch and every world→glue return on logout). After the engine
> registers its own 109 glue functions, our hook adds these two on
> the same Lua state via the engine's own
> `FrameScript_RegisterFunction`. The glue Lua state is torn down
> and recreated on every world transition; we re-register each
> cycle automatically.

## Chat

### `GetCurrentChatGUID()`

Returns the sender's GUID for whichever `CHAT_MSG_*` event is
currently being dispatched, or `nil` if called outside a chat
event or for a synthetic chat with no real sender (e.g.
`CHAT_MSG_SYSTEM`).

Format matches `UnitGUID`: `"0xHHHHHHHHLLLLLLLL"` (16 hex digits,
hi dword first).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("CHAT_MSG_CHANNEL")
f:RegisterEvent("CHAT_MSG_SAY")
f:RegisterEvent("CHAT_MSG_WHISPER")
f:SetScript("OnEvent", function()
    local guid = GetCurrentChatGUID()
    if guid then
        local _, class, _, race = GetPlayerInfoByGUID(guid)
        DEFAULT_CHAT_FRAME:AddMessage(string.format(
            "%s: %s [%s %s]", arg2, arg1, class or "?", race or "?"))
    end
end)
```

Vanilla 1.12's `CHAT_MSG_*` events don't include the sender GUID
in their payload — that was added in 3.0+ as `arg12`. Addons that
need to identify chatters reliably (rather than by sender name,
which is locale-fragile and ambiguous across realms) currently
strcmp against an addon-maintained name cache. This function lets
them skip that work.

**Implementation**: hooks the engine's chat dispatcher at
`FUN_CHAT_DISPATCH` (`0x0049A870`), which receives the sender
GUID as args 11/12 of a 12-arg `__fastcall`. The hook stashes the
GUID into a static global before forwarding to the original
function; the engine then fires the appropriate `CHAT_MSG_*`
event synchronously inside that window, so `GetCurrentChatGUID()`
called from the addon's OnEvent sees the right GUID. The global
is restored to its prior value (not cleared) on hook return, so
nested chat dispatch (e.g. an addon calling `SendChatMessage`
from its handler) doesn't lose the outer context's GUID.

The function returns `nil` rather than `"0x0000000000000000"`
for synthetic chat (system messages, login banners, etc.) where
the GUID args are zero — matches the idiomatic
`if GetCurrentChatGUID() then` check.

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

Returns an `Enum.AddOnSecurityStatus` integer (not a string — modern
shape):

| Value | `Enum.AddOnSecurityStatus.*` | When |
|------:|------------------------------|------|
| `0`   | `Secure`                     | Blizzard-signed addons (`Blizzard_*`). |
| `1`   | `Insecure`                   | Every user addon loaded from `Interface/AddOns/`. The default for any registered addon not in the secure or banned override sets. |
| `2`   | `Banned`                     | Entries the engine has explicitly disqualified — set by server addon-banlist responses, rare in practice. |
| `3`   | `NotAvailable`               | No addon by that name / index exists. |

```lua
C_AddOns.GetAddOnSecurity("Blizzard_TalentUI") -- 0 (Secure)
C_AddOns.GetAddOnSecurity("DebugTools")        -- 1 (Insecure)
C_AddOns.GetAddOnSecurity("Nonexistent")       -- 3 (NotAvailable)

if status == Enum.AddOnSecurityStatus.Secure then ...
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

## CVar

### `C_CVar.GetCVarBool(cvar)`

Returns the cvar's value coerced to a boolean, or `nil` if no cvar
by that name is registered. The `C_CVar` namespace was added in
10.x; vanilla 1.12 only has `GetCVar` which returns a string.

```lua
C_CVar.GetCVarBool("gxMaximize")    -- true if the window is set fullscreen
C_CVar.GetCVarBool("CombatDamage")  -- false if floating damage text is off
C_CVar.GetCVarBool("doesNotExist")  -- nil — distinguishable from false
```

Coercion rules (only applied when the cvar exists):

| Cvar string value | Result |
|---|---|
| `"1"` or any non-zero numeric (`"42"`, `"-3"`, …) | `true` |
| `"true"` (case-insensitive) | `true` |
| `"0"` or empty | `false` |
| Non-numeric, non-`"true"` (`"on"`, `"yes"`) | `false` |
| Cvar name not registered | `nil` |

Implementation reads the cvar registry directly (engine's
`FUN_FIND_CVAR` at `0x0063DEC0`), bypassing `GetCVar`'s Lua-error
path for unknown cvars. The `nil` return lets callers distinguish
"the cvar exists and is falsy" from "no such cvar."

#### Glue-state availability

`C_CVar.GetCVarBool` is also registered on the glue Lua state, and
the engine's stock `GetCVar` / `SetCVar` / `RegisterCVar` /
`GetCVarDefault` — which vanilla 1.12 only exposes in-game — are
mirrored onto glue too. GlueXML patches at the login / realm /
char-select screens can now read and write CVars directly. CVar
storage is process-global, so writes on either state are
immediately visible on the other (a `SetCVar("foo", "1")` from a
GlueXML script will read back as `"1"` on the in-world side, and
vice versa).

## Table

### `table.wipe(t)`

Removes every key from `t`, leaving it empty but preserving its
internal hash and array capacity. Returns `t` for chaining.

```lua
local cache = {}
-- ... fill it up ...
table.wipe(cache)              -- cache is now {}
local also_t = table.wipe(t2)  -- also_t === t2 after wipe
```

Port of Blizzard's 3.3.5 implementation at VA `0x00852180` — same
pattern, just using 1.12's Lua 5.0 entry points instead of 5.1's.
Both Lua versions' `lua_next` walks the hash node array linearly,
so the canonical `lua_next` + `rawset(k, nil)` "during-iteration
removal" pattern works in practice even though it's technically
undefined per the Lua reference manual.

Errors on non-table input.

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

### `C_Timer.After(seconds, callback)`

Schedules `callback` to fire once after `seconds`. Returns nothing.
Backports modern WoW's `C_Timer.After` as an engine binding (retail
12.0.5 ships this as a `Script_*` C function, not Lua — we mirror
that shape so addons relying on the modern semantics get
identical behavior).

```lua
C_Timer.After(0.5, function() print("half a second later") end)
C_Timer.After(0, function() print("next frame") end)
```

Errors in the callback are caught (via `lua_pcall`) and silently
swallowed — same as modern's behavior. One broken timer doesn't
poison other timers on the same tick.

### `C_Timer.NewTimer(seconds, callback)`

Like `After`, but returns a cancel-handle table:

```lua
local handle = C_Timer.NewTimer(10, function() print("never if I cancel") end)
handle:Cancel()           -- prevents the callback from firing
print(handle:IsCancelled()) -- true
```

`Cancel()` is idempotent; calling it on a timer that already fired
or was previously cancelled is a no-op. `IsCancelled()` returns
`true` after `Cancel()`, after natural expiry, or for an unknown
timer ID. Both methods take no arguments other than `self`.

### `C_Timer.NewTicker(seconds, callback, [iterations])`

Recurring variant. `callback` fires every `seconds` until cancelled
or until `iterations` fires have happened. `iterations` is optional;
omitted or non-numeric means forever.

```lua
local h = C_Timer.NewTicker(1, function() print("each second") end)
-- ...later
h:Cancel()

-- bounded:
C_Timer.NewTicker(0.25, function() ... end, 8)  -- fires 8 times, every 250ms
```

Minimum period is clamped to `0.0001` to prevent tight loops.
Returns the same kind of cancel handle as `NewTimer`. The handle
auto-cancels itself when the iteration count exhausts.

Implementation notes (apply to all three functions):

- One unified timer queue walked once per frame from
  `FUN_WORLD_TICK`'s subscription. Same hook the
  `BAG_UPDATE_DELAYED` flusher uses, so there's no per-feature hook
  overhead.
- Resolution is one frame (~16ms at 60fps). A timer scheduled for
  `0.001` seconds will fire on the next tick, same as one
  scheduled for `0`.
- The timer queue uses a monotonic clock (`QueryPerformanceCounter`),
  so timers don't drift if the system clock changes mid-session.
- Callbacks are pinned in the Lua registry under per-timer string
  keys so they survive GC between scheduling and firing.

### `C_DateAndTime` overview

Backport of the modern `C_DateAndTime` namespace — calendar-style
date math built on top of `GetServerTime()`. All seven functions
exchange `CalendarTime` tables with these fields (matching
Blizzard's `TimeDocumentation.lua`):

| Field | Range | Notes |
|-------|-------|-------|
| `year` | full year, e.g. 2026 | |
| `month` | 1..12 | luaIndex (1-based) |
| `monthDay` | 1..31 | luaIndex (1-based) |
| `weekday` | 1..7 | luaIndex; 1 = Sunday |
| `hour` | 0..23 | |
| `minute` | 0..59 | |

> **Daily reset semantics.** `GetSecondsUntilDailyReset` treats reset
> as midnight in server wall-clock time. This is exactly what
> `GetServerTime() % 86400 == 0` gives you — the engine's gametime
> components are converted to an epoch by treating them as UTC, so
> day boundaries in epoch math align with server-clock midnight.
>
> **Weekly reset not implemented.** Vanilla has no server-broadcast
> weekly reset schedule, and Turtle WoW realm schedules vary, so
> `C_DateAndTime.GetSecondsUntilWeeklyReset` would have to hardcode a
> weekday/hour that's wrong on some realms. Addons that need it can
> compute their own value from the daily reset + a CVAR.

### `C_DateAndTime.GetCurrentCalendarTime()`

Returns the server clock as a CalendarTime table, or nothing before
login. Equivalent to `GetCalendarTimeFromEpoch(GetServerTime())`.

```lua
local t = C_DateAndTime.GetCurrentCalendarTime()
-- t = { year=2026, month=5, monthDay=11, weekday=2, hour=14, minute=30 }
```

### `C_DateAndTime.GetCalendarTimeFromEpoch(epoch)`

Converts a Unix epoch (seconds since 1970-01-01 UTC) to a
CalendarTime table. Pure `gmtime`; no engine state touched.

### `C_DateAndTime.AdjustTimeByDays(date, days)` / `AdjustTimeByMinutes(date, minutes)`

Returns a new CalendarTime offset from the input by the given number
of days (or minutes). Negative deltas walk backwards. Out-of-range
inputs (`month=13`, `monthDay=32`) normalize via the underlying
`_mkgmtime` / `gmtime` round-trip.

```lua
local today = C_DateAndTime.GetCurrentCalendarTime()
local tomorrow = C_DateAndTime.AdjustTimeByDays(today, 1)
local yesterday = C_DateAndTime.AdjustTimeByDays(today, -1)
```

### `C_DateAndTime.CompareCalendarTime(lhs, rhs)`

Returns `-1` if `lhs < rhs`, `0` if equal, `1` if `lhs > rhs`.
Compares by epoch conversion so denormalized inputs sort
consistently.

### `C_DateAndTime.GetServerTimeLocal()`

Returns the server's wall clock re-interpreted as a Unix epoch in
the **player's** local timezone. Useful when you want to feed a
server-clock value into Lua's `date(format, epoch)` and have the
formatted string show the server's apparent hour/minute. If the
server reports 14:30 and the player is in UTC-5, this returns the
epoch that corresponds to 14:30 in UTC-5 (which is 19:30 UTC).

### `C_DateAndTime.GetSecondsUntilDailyReset()`

Returns seconds until the next server midnight (00:00:00 in server
wall-clock time). Returns 0 if called before login.

> See the overview's "Daily reset semantics" note — this uses server
> midnight, not UTC midnight (though they happen to coincide for
> Turtle WoW since the server reports UTC-aligned components).

## UIColor

### `C_UIColor.GetColors()`

Returns an array of rows, each shaped
`{ baseTag = "FOO_COLOR", color = ColorMixin }`, mirroring the modern
function of the same name. The `color` field is a real `ColorMixin`
instance (carries `GetRGB`, `GenerateHexColorMarkup`, etc.) — the DLL
calls back into Lua's `CreateColor(r,g,b,a)` per row to construct it,
so `ColorMixin` and `CreateColor` must already be defined when
`GetColors` is called.

The companion addon `!!!ClassicAPI/Util/Color.lua` does both: it
defines `ColorMixin`/`CreateColor` first, then loops the result the
same way `Blizzard_SharedXMLBase/Color.lua` does on modern WoW —
assigning each row as a global under its `baseTag` plus a `_CODE`
variant holding the `|c`-prefixed hex markup:

```lua
for _, dbColor in ipairs(C_UIColor.GetColors()) do
    local color = CreateColor(dbColor.color.r, dbColor.color.g, dbColor.color.b, dbColor.color.a)
    _G[dbColor.baseTag] = color
    _G[dbColor.baseTag.."_CODE"] = color:GenerateHexColorMarkup()
end
```

After that runs, addons can use the standard modern color globals
directly:

```lua
/dump ACHIEVEMENT_COLOR:GetRGB()       -- 1, 1, 0
/dump ITEM_EPIC_COLOR:GenerateHexColor()
ITEM_EPIC_COLOR_CODE .. "Legendary" .. "|r"
```

The data comes from a snapshot of `GlobalColor.dbc` taken from BC
Classic 2.5.5 (build 67323) — vanilla 1.12 has no `GlobalColor.dbc`,
so the rows are embedded in the DLL (see [`src/ui/ColorData.h`](../src/ui/ColorData.h)).
Duplicate `baseTag`s in the source DBC (`PURE_RED_COLOR`,
`INVASION_*`, etc.) are deduplicated keeping the higher-ID entry,
matching what `ipairs(DBColors)` ends up assigning on modern. Colors
introduced after BC (Death Knight runes, Mythic+ medals, healing
absorbs, glyphs, objective tracker) aren't in the snapshot and so
aren't surfaced — none have a use case in 1.12 anyway.

If `CreateColor` happens not to be defined when `GetColors` is called
(e.g. another DLL or addon manages to call us before `!!!ClassicAPI`
loads), each `color` field falls back to a plain `{r,g,b,a}` table.
The loop above tolerates both shapes — `dbColor.color.r` reads the
same way either way — so consumers shouldn't notice.

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

### `BAG_UPDATE_DELAYED` event

Fires (with no payload) once per frame in which any `BAG_UPDATE`
fired. Matches modern WoW's coalescing semantic exactly — register
for `BAG_UPDATE_DELAYED` instead of `BAG_UPDATE` and rescan once
per frame regardless of how many updates fired.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("BAG_UPDATE_DELAYED")
f:SetScript("OnEvent", function() RescanMyInventory() end)
```

A trade with 6 stacks (12 `BAG_UPDATE`s, all processed in one
frame) produces exactly 1 `BAG_UPDATE_DELAYED` at the end of the
frame — matches Classic Era 1.15.x's observed behavior.

Implemented by three hooks, none in regions other DLLs touch:
- `FUN_004F91A0` / `FUN_004F9370` (bag subsystem at `0x004F9xxx`)
  — each just sets a `g_pending` flag, no fire
- `FUN_0066FD50` (engine's per-frame world-subsystem update at
  `0x0066xxxx`, deep in the rendering pipeline, only one caller in
  the entire binary) — drains the flag at the tail of the world
  tick

The world-tick hook is a tail hook (run original first, then fire
DELAYED), so the fire happens after every other per-frame world
work completes. Addon callbacks see DELAYED after their normal
event handlers.

> **Coverage limitation.** The keyring `BAG_UPDATE(-2)` path goes
> through a separate function (`FUN_004F8DB0`) that uses
> `__thiscall` with an awkward register-arg shape. Keyring updates
> currently won't trigger `BAG_UPDATE_DELAYED`. Player bag (0..4)
> and bank (5..10) updates work normally — the 95% case.

### Player input-state events

Three pairs of zero-payload events fired from a single per-frame
`Tick::WorldTick` callback that reads input state once and edge-
detects each pair.

| Event | Fires on | Source |
|-------|----------|--------|
| `PLAYER_STARTED_MOVING` / `PLAYER_STOPPED_MOVING` | WASD or autorun toggle | UI-input controller flags at `*0x00BE1148 + 0x04` (`0x10` W \| `0x20` S \| `0x40` A \| `0x80` D \| `0x1000` autorun) |
| `PLAYER_STARTED_TURNING` / `PLAYER_STOPPED_TURNING` | Mouselook bit held AND character body yaw is changing | Mouselook bit `0x01` + per-frame delta on `CGPlayer + 0x9C4` (body yaw, radians) |
| `PLAYER_STARTED_LOOKING` / `PLAYER_STOPPED_LOOKING` | Free-look bit held AND camera-relative yaw is changing | Free-look bit `0x02` + per-frame delta on `[*0x00B4B2BC + 0x65B8] + 0xF0` (camera yaw, radians) |

```lua
local f = CreateFrame("Frame")
for _, ev in ipairs({
    "PLAYER_STARTED_MOVING", "PLAYER_STOPPED_MOVING",
    "PLAYER_STARTED_LOOKING", "PLAYER_STOPPED_LOOKING",
    "PLAYER_STARTED_TURNING", "PLAYER_STOPPED_TURNING",
}) do
    f:RegisterEvent(ev)
end
f:SetScript("OnEvent", function(_, event) print(event) end)
```

**Key-state semantics for MOVING**: `STOPPED_MOVING` fires the
instant the movement keys release, even if the character is still
airborne from a jump. Matches retail (verified empirically). For
an "is the character actually displacing this frame" signal, use
`GetUnitSpeed("player")` and watch the first return.

**Latched semantics for TURNING and LOOKING**: each pair fires
exactly once per RMB / LMB hold. STARTED waits until both the
mouse-button bit is held AND the relevant yaw has actually
changed (matches retail's "camera has moved" semantics — clicking
without dragging doesn't fire). STOPPED fires when the button bit
clears (RMB / LMB release). The latch stays on through any drag-
stop-drag motion within a single hold; no spurious flapping.

The camera-relative yaw at `[camera + 0xF0]` stays at 0 during
RMB-mouselook (camera rotates *with* the character, so its offset
from the character doesn't change), so TURNING and LOOKING are
cleanly separable: RMB only fires TURNING, LMB only fires LOOKING.

Implementation: single subscriber on the shared `Tick::WorldTick`
registry (same `MinHook` on `FUN_0066FD50` shared with
`BAG_UPDATE_DELAYED`). When any of the input controller / player /
camera pointers is null (pre-login, loading screen, character
select), all states reset so the next valid tick doesn't fire
spurious STOPPED transitions.

### `GLOBAL_MOUSE_DOWN` / `GLOBAL_MOUSE_UP` events

Fires on every raw mouse-button press or release while WoW has
focus. Payload is the button identifier string — matches modern
WoW's signature exactly.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("GLOBAL_MOUSE_DOWN")
f:RegisterEvent("GLOBAL_MOUSE_UP")
f:SetScript("OnEvent", function(_, event, button)
    print(event, button)  -- "GLOBAL_MOUSE_DOWN", "LeftButton"
end)
```

| Payload | Source |
|---------|--------|
| `"LeftButton"` | `VK_LBUTTON` |
| `"RightButton"` | `VK_RBUTTON` |
| `"MiddleButton"` | `VK_MBUTTON` |
| `"Button4"` | `VK_XBUTTON1` |
| `"Button5"` | `VK_XBUTTON2` |

Distinct from per-frame `OnMouseDown` scripts — these fire even
when the click misses every UI frame (clicks in the world / on
nothing). Distinct from `PLAYER_STARTED_LOOKING` /
`PLAYER_STARTED_TURNING` — those are camera-rotation events;
these are raw input events.

**Focus-gated.** Polls Win32 `GetAsyncKeyState` per frame on the
shared `Tick::WorldTick` registry. DOWN transitions are only
honored when WoW is the foreground window — clicking in another
app while alt-tabbed doesn't fire. UP transitions always fire,
even if the user alt-tabbed mid-click, so an addon never gets
left in a "button is held" state.

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

### `Enum.AddOnSecurityStatus`

The integer enum `C_AddOns.GetAddOnSecurity` returns. Matches
Blizzard's `Enum.AddOnSecurityStatus`:

| Value | Field          | Notes |
|------:|----------------|-------|
| `0`   | `Secure`       | Blizzard-signed addons. |
| `1`   | `Insecure`     | User addons; default for any registered addon. |
| `2`   | `Banned`       | Server-disqualified entries. |
| `3`   | `NotAvailable` | No addon by that name / index. |

```lua
if C_AddOns.GetAddOnSecurity(name) == Enum.AddOnSecurityStatus.Secure then
    -- it's a Blizzard_* addon
end
```
