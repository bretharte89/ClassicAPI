# ClassicAPI — Lua Reference

Per-function reference for the calls ClassicAPI adds to the WoW 1.12.1 Lua
environment. See the [project README](../README.md) for installation and
build instructions.

## Contents

- [Account](#account)
  - [`SaveAccount(name, password)` / `DeleteAccount(name)` / `GetSavedAccounts()` / `LoginWithSavedAccount(name)` — GlueXML only](#saveaccountname-password--deleteaccountname--getsavedaccounts--loginwithsavedaccountname--gluexml-only)

- [Action](#action)
  - [`GetActionInfo(slot)`](#getactioninfoslot)

- [AddOns](#addons)
  - [`C_AddOns.GetAddOnName(indexOrName)`](#c_addonsgetaddonnameindexorname)
  - [`C_AddOns.GetAddOnTitle(indexOrName)`](#c_addonsgetaddontitleindexorname)
  - [`C_AddOns.GetAddOnNotes(indexOrName)`](#c_addonsgetaddonnotesindexorname)
  - [`C_AddOns.IsAddOnLoadable(indexOrName)`](#c_addonsisaddonloadableindexorname)
  - [`C_AddOns.IsAddOnLoaded(indexOrName)`](#c_addonsisaddonloadedindexorname)
  - [`C_AddOns.GetAddOnSecurity(indexOrName)`](#c_addonsgetaddonsecurityindexorname)
  - [`C_AddOns.DoesAddOnExist(indexOrName)`](#c_addonsdoesaddonexistindexorname)

- [CharacterList](#characterlist)
  - [`GetSavedCharacterOrder(realm)` / `SetSavedCharacterOrder(realm, order)` — GlueXML only](#getsavedcharacterorderrealm--setsavedcharacterorderrealm-order--gluexml-only)

- [Chat](#chat)
  - [`GetCurrentChatGUID()`](#getcurrentchatguid)

- [Class](#class)
  - [`FillLocalizedClassList(table [, isFemale])`](#filllocalizedclasslisttable-isfemale)

- [Combat](#combat)
  - [`InCombatLockdown()`](#incombatlockdown)

- [Console](#console)
  - [`ExportInterfaceFiles art|code` (console command)](#exportinterfacefiles-artcode-console-command)
  - [`ExportDBCFiles` (console command)](#exportdbcfiles-console-command)

- [Container](#container)
  - [`C_Container.GetContainerItemID(bagIndex, slotIndex)`](#c_containergetcontaineritemidbagindex-slotindex)
  - [`GetItemCooldown(itemInfo)` / `C_Container.GetItemCooldown(itemID)`](#getitemcooldowniteminfo--c_containergetitemcooldownitemid)
  - [`C_Container.GetContainerItemDurability(containerIndex, slotIndex)`](#c_containergetcontaineritemdurabilitycontainerindex-slotindex)
  - [`C_Container.GetContainerItemRepairCost(containerIndex, slotIndex)`](#c_containergetcontaineritemrepaircostcontainerindex-slotindex)
  - [`C_Container.GetContainerItemCharges(containerIndex, slotIndex)`](#c_containergetcontaineritemchargescontainerindex-slotindex)
  - [`C_Container.GetContainerNumFreeSlots(bagID)`](#c_containergetcontainernumfreeslotsbagid)
  - [`C_Container.IsContainerItemOpenable(containerIndex, slotIndex)`](#c_containeriscontaineritemopenablecontainerindex-slotindex)
  - [`C_Container.PlayerHasHearthstone()`](#c_containerplayerhashearthstone)
  - [`C_Container.UseHearthstone()`](#c_containerusehearthstone)
  - [`C_Container.SwapItems(srcBag, srcSlot, dstBag, dstSlot)`](#c_containerswapitemssrcbag-srcslot-dstbag-dstslot)
  - [`C_Container.MoveItem(srcBag, srcSlot, dstBag, dstSlot, count)`](#c_containermoveitemsrcbag-srcslot-dstbag-dstslot-count)

- [Coroutine](#coroutine)
  - [`coroutine.create(fn)`](#coroutinecreatefn)
  - [`coroutine.resume(co, ...)`](#coroutineresumeco-)
  - [`coroutine.yield(...)`](#coroutineyield)
  - [`coroutine.status(co)`](#coroutinestatusco)
  - [`coroutine.wrap(fn)`](#coroutinewrapfn)
  - [Async pattern (RunAsync + C_Timer.After)](#async-pattern-runasync--c_timerafter)

- [Creature](#creature)
  - [`C_CreatureInfo.GetCreatureID(guid)`](#c_creatureinfogetcreatureidguid)
  - [`C_CreatureInfo.GetCreatureInfoByID(creatureID)`](#c_creatureinfogetcreatureinfobyidcreatureid)
  - [`C_CreatureInfo.RequestLoadCreatureByID(creatureID)`](#c_creatureinforequestloadcreaturebyidcreatureid)

- [CVar](#cvar)
  - [`C_CVar.GetCVarBool(cvar)`](#c_cvargetcvarboolcvar)

- [Cursor](#cursor)
  - [`GetCursorInfo()`](#getcursorinfo)

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

- [Events](#events)
  - [`C_EventUtils.IsEventValid(eventName)`](#c_eventutilsiseventvalideventname)
  - [`BAG_UPDATE_DELAYED` event](#bag_update_delayed-event)
  - [`PLAYER_EQUIPMENT_CHANGED` event](#player_equipment_changed-event)
  - [`UPDATE_INVENTORY_DURABILITY` event](#update_inventory_durability-event)
  - [`HEARTHSTONE_BOUND` event](#hearthstone_bound-event)
  - [Player input-state events (`PLAYER_STARTED_MOVING` / `LOOKING` / `TURNING` + `STOPPED_*`)](#player-input-state-events)
  - [`GLOBAL_MOUSE_DOWN` / `GLOBAL_MOUSE_UP` events](#global_mouse_down--global_mouse_up-events)
  - [`EQUIPMENT_SETS_CHANGED` event](#equipment_sets_changed-event)
  - [`EQUIPMENT_SWAP_PENDING` event](#equipment_swap_pending-event)
  - [`EQUIPMENT_SWAP_FINISHED` event](#equipment_swap_finished-event)
  - [`FACTION_STANDING_CHANGED` event](#faction_standing_changed-event)
  - [`LOOT_SCAN_COMPLETED` event](#loot_scan_completed-event)
  - [`MODIFIER_STATE_CHANGED` event](#modifier_state_changed-event)
  - [`NAME_PLATE_CREATED` / `NAME_PLATE_UNIT_ADDED` / `NAME_PLATE_UNIT_REMOVED` events](#name_plate_created--name_plate_unit_added--name_plate_unit_removed-events)
  - [`PLAYER_FOCUS_CHANGED` event](#player_focus_changed-event)
  - [`QUEST_ACCEPTED` event](#quest_accepted-event)
  - [`QUEST_REMOVED` event](#quest_removed-event)
  - [`QUEST_TURNED_IN` event](#quest_turned_in-event)
  - [`UNIT_FACTION` event (fire-coverage fix)](#unit_faction-event-fire-coverage-fix)
  - [`UPDATE_SHAPESHIFT_FORM` event](#update_shapeshift_form-event)

- [Expansion](#expansion)
  - [`GetClassicExpansionLevel()`](#getclassicexpansionlevel)
  - [`ClassicExpansionAtLeast(expansionLevel)`](#classicexpansionatleastexpansionlevel)
  - [`ClassicExpansionAtMost(expansionLevel)`](#classicexpansionatmostexpansionlevel)

- [Faction](#faction)
  - [`GetFactionIDByIndex(factionIndex)`](#getfactionidbyindexfactionindex)
  - [`GetFactionInfoByID(factionID)`](#getfactioninfobyidfactionid)
  - [`GetFactionParentID(factionID)`](#getfactionparentidfactionid)
  - [`C_Reputation.GetFactionStandings()`](#c_reputationgetfactionstandings)
  - [`C_Reputation.GetWatchedFactionData()`](#c_reputationgetwatchedfactiondata)
  - [`C_Reputation.GetFactionDataByIndex(factionSortIndex)`](#c_reputationgetfactiondatabyindexfactionsortindex)
  - [`C_Reputation.SetWatchedFactionByID(factionID)`](#c_reputationsetwatchedfactionbyidfactionid)
  - [`C_Reputation.GetLastStandingChange()`](#c_reputationgetlaststandingchange)

- [Focus](#focus)
  - [`FocusUnit(unit)`](#focusunitunit)
  - [`ClearFocus()`](#clearfocus)
  - [Unit token (`focus` / `focustarget`)](#unit-token-focus--focustarget)
  - [Bindings (`FOCUSTARGET` / `TARGETFOCUS`)](#bindings-focustarget--targetfocus)

- [FriendList](#friendlist)
  - [`C_FriendList.SendWhoQueryByName(name)`](#c_friendlistsendwhoquerybynamename)
  - [`C_FriendList.IsWhoQueryPending()`](#c_friendlistiswhoquerypending)

- [GameObject](#gameobject)
  - [`C_GameObjectInfo.GetGameObjectInfoByID(gameObjectID)`](#c_gameobjectinfogetgameobjectinfobyidgameobjectid)
  - [`C_GameObjectInfo.RequestLoadGameObjectByID(gameObjectID)`](#c_gameobjectinforequestloadgameobjectbyidgameobjectid)

- [GameTooltip](#gametooltip)
  - [`GameTooltip:SetSpellByID(spellID)`](#gametooltipsetspellbyidspellid)
  - [`GameTooltip:SetTalentByID(talentID)`](#gametooltipsettalentbyidtalentid)
  - [`GameTooltip:SetInventoryItemByID(itemID)`](#gametooltipsetinventoryitembyiditemid)
  - [`GameTooltip:SetHyperlinkCompareItem("itemLink" [, offset, shiftButton, comparisonTooltip])`](#gametooltipsethyperlinkcompareitemitemlink--offset-shiftbutton-comparisontooltip)
  - [`GameTooltip:IsEquippedItem()`](#gametooltipisequippeditem)
  - [`OnTooltipSetItem` script](#ontooltipsetitem-script)
  - [`GameTooltip:SetItemByGUID(itemGUID)`](#gametooltipsetitembyguiditemguid)
  - [`GameTooltip:SetEquipmentSet(name)`](#gametooltipsetequipmentsetname)
  - [`GameTooltip:GetItem()`](#gametooltipgetitem)
  - [`GameTooltip:GetSpell()`](#gametooltipgetspell)
  - [`GameTooltip:HasItem()` / `GameTooltip:HasSpell()`](#gametooltiphasitem--gametooltiphasspell)
  - [`GameTooltip:GetUnitGUID()` / `GameTooltip:HasUnit()`](#gametooltipgetunitguid--gametooltiphasunit)
  - [`GameTooltip:GetGameObject()` / `GameTooltip:HasGameObject()`](#gametooltipgetgameobject--gametooltiphasgameobject)
  - [`GameTooltip:GetOwner()`](#gametooltipgetowner)

- [Globals](#globals)
  - [`CLASSIC_API_VERSION`](#classic_api_version)
  - [`LE_EXPANSION_*`](#le_expansion_)
  - [`LE_ITEM_QUALITY_*`](#le_item_quality_)
  - [`LE_UNIT_STAT_*`](#le_unit_stat_)
  - [`Enum.AddOnSecurityStatus`](#enumaddonsecuritystatus)
  - [`Enum.InventoryType`](#enuminventorytype)
  - [`Enum.PowerType`](#enumpowertype)

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

- [Hooks](#hooks)
  - [`hooksecurefunc(name, callback)` / `hooksecurefunc(table, name, callback)`](#hooksecurefuncname-callback--hooksecurefunctable-name-callback)

- [Input](#input)
  - [`IsLeftShiftKeyDown()` / `IsRightShiftKeyDown()`](#isleftshiftkeydown--isrightshiftkeydown)
  - [`IsLeftControlKeyDown()` / `IsRightControlKeyDown()`](#isleftcontrolkeydown--isrightcontrolkeydown)
  - [`IsLeftAltKeyDown()` / `IsRightAltKeyDown()`](#isleftaltkeydown--isrightaltkeydown)
  - [`IsModifierKeyDown()`](#ismodifierkeydown)
  - [`IsMouseButtonDown([button])`](#ismousebuttondownbutton)

- [Instance](#instance)
  - [`GetInstanceInfo()`](#getinstanceinfo)

- [Item](#item)
  - [`C_Item.DoesItemExist(itemLocation)` / `C_Item.DoesItemExistByID(item)`](#c_itemdoesitemexititemlocation--c_itemdoesitemexistbyiditem)
  - [`C_Item.EquipItemByName(itemInfo [, dstSlot])`](#c_itemequipitembynameiteminfo--dstslot)
  - [`C_Item.GetCurrentItemLevel(itemLocation)` / `C_Item.GetDetailedItemLevelInfo(item)`](#c_itemgetcurrentitemlevelitemlocation--c_itemgetdetaileditemlevelinfoitem)
  - [`C_Item.GetItemCount(itemInfo, [includeBank], [includeUses])`](#c_itemgetitemcountiteminfo-includebank-includeuses)
  - [`C_Item.GetItemData(itemLocation)` / `C_Item.GetItemDataByID(item)`](#c_itemgetitemdataitemlocation--c_itemgetitemdatabyiditem)
  - [`C_Item.GetItemFamily(item)`](#c_itemgetitemfamilyitem)
  - [`C_Item.GetItemGUID(itemLocation)`](#c_itemgetitemguiditemlocation)
  - [`C_Item.GetItemID(itemLocation)`](#c_itemgetitemiditemlocation)
  - [`C_Item.GetItemInfoInstant(item)`](#c_itemgetiteminfoinstantitem)
  - [`C_Item.GetItemInventorySlotInfo(inventorySlot)`](#c_itemgetiteminventoryslotinfoinventoryslot)
  - [`C_Item.GetItemInventorySlotKey(inventorySlot)`](#c_itemgetiteminventoryslotkeyinventoryslot)
  - [`C_Item.GetItemInventoryType(itemLocation)` / `C_Item.GetItemInventoryTypeByID(item)`](#c_itemgetiteminventorytypeitemlocation--c_itemgetiteminventorytypebyiditem)
  - [`C_Item.GetItemLink(itemLocation)`](#c_itemgetitemlinkitemlocation)
  - [`C_Item.GetItemLocation(itemGUID)`](#c_itemgetitemlocationitemguid)
  - [`C_Item.GetItemMaxStackSize(itemLocation)` / `C_Item.GetItemMaxStackSizeByID(item)`](#c_itemgetitemmaxstacksizeitemlocation--c_itemgetitemmaxstacksizebyiditem)
  - [`C_Item.GetItemName(itemLocation)` / `C_Item.GetItemNameByID(item)`](#c_itemgetitemnameitemlocation--c_itemgetitemnamebyiditem)
  - [`C_Item.GetItemQuality(itemLocation)` / `C_Item.GetItemQualityByID(item)`](#c_itemgetitemqualityitemlocation--c_itemgetitemqualitybyiditem)
  - [`C_Item.GetItemSellPrice(itemLocation)` / `C_Item.GetItemSellPriceByID(item)`](#c_itemgetitemsellpriceitemlocation--c_itemgetitemsellpricebyiditem)
  - [`C_Item.GetItemSetID(itemLocation)` / `C_Item.GetItemSetIDByID(item)`](#c_itemgetitemsetiditemlocation--c_itemgetitemsetidbyiditem)
  - [`C_Item.GetItemSetInfo(setID)`](#c_itemgetitemsetinfosetid)
  - [`C_Item.GetItemSpell(item)`](#c_itemgetitemspellitem)
  - [`C_Item.GetItemStatDelta(itemLink1, itemLink2)`](#c_itemgetitemstatdeltaitemlink1-itemlink2)
  - [`C_Item.GetItemStats(itemLink)`](#c_itemgetitemstatsitemlink)
  - [`C_Item.GetItemSubClassInfo(classID, subClassID)`](#c_itemgetitemsubclassinfoclassid-subclassid)
  - [`C_Item.GetItemUniqueness(itemLocation)` / `C_Item.GetItemUniquenessByID(item)`](#c_itemgetitemuniquenessitemlocation--c_itemgetitemuniquenessbyiditem)
  - [`C_Item.GetStackCount(itemLocation)`](#c_itemgetstackcountitemlocation)
  - [`C_Item.IsBound(itemLocation)`](#c_itemisbounditemlocation)
  - [`C_Item.IsEquippableItem(item)`](#c_itemisequippableitemitem)
  - [`C_Item.IsEquippedItem(item)`](#c_itemisequippeditemitem)
  - [`C_Item.IsItemDataCachedByID(item)` / `C_Item.IsItemDataCached(itemLocation)`](#c_itemisitemdatacachedbyiditem--c_itemisitemdatacacheditemlocation)
  - [`C_Item.IsItemGUIDInInventory(itemGUID)`](#c_itemisitemguidininventoryitemguid)
  - [`C_Item.IsItemOpenable(itemLocation)`](#c_itemisitemopenableitemlocation)
  - [`C_Item.IsLocked(itemLocation)`](#c_itemislockeditemlocation)
  - [`C_Item.LockItem(itemLocation)`](#c_itemlockitemitemlocation)
  - [`C_Item.LockItemByGUID(itemGUID)`](#c_itemlockitembyguiditemguid)
  - [`C_Item.RequestLoadItemDataByID(item)` / `C_Item.RequestLoadItemData(itemLocation)`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation)
  - [`C_Item.UnlockAllItems()`](#c_itemunlockallitems)
  - [`C_Item.UnlockItem(itemLocation)`](#c_itemunlockitemitemlocation)
  - [`C_Item.UseAtCursor(itemInfo)`](#c_itemuseatcursoriteminfo)
  - [`C_Item.UseAtUnit(itemInfo, unit)`](#c_itemuseatunititeminfo-unit)
  - [`C_Item.UseItemByName(itemInfo [, unit])`](#c_itemuseitembynameiteminfo--unit)
  - [`Get*ItemID` — companions to the engine's `Get*ItemLink` family](#getitemid--companions-to-the-engines-getitemlink-family)
  - [`GetAverageItemLevel()`](#getaverageitemlevel)
  - [`GetInventoryItemDurability(invSlot)`](#getinventoryitemdurabilityinvslot)
  - [`GetInventoryItemID(unit, slot)`](#getinventoryitemidunit-slot)
  - [`GetInventoryItemRepairCost(invSlot)`](#getinventoryitemrepaircostinvslot)
  - [`GetInventoryItemsForSlot(slot, returnTable [, transmogrify])`](#getinventoryitemsforslotslot-returntable--transmogrify)
  - [`GetItemIcon(itemID)` / `C_Item.GetItemIcon(itemLocation)` / `C_Item.GetItemIconByID(item)`](#getitemiconitemid--c_itemgetitemiconitemlocation--c_itemgetitemiconbyiditem)
  - [`OffhandHasWeapon()`](#offhandhasweapon)

- [Loot](#loot)
  - [`C_Loot.GetNearbyLootableUnits()`](#c_lootgetnearbylootableunits)
  - [`C_Loot.LootUnit(guid)`](#c_lootlootunitguid)
  - [`C_Loot.LootUnitItem(guid, itemID)`](#c_lootlootunititemguid-itemid)
  - [`C_Loot.ScanNearbyLoot()`](#c_lootscannearbyloot)
  - [`C_Loot.IsScanInProgress()`](#c_lootisscaninprogress)
  - [`C_Loot.GetLastScanResults()`](#c_lootgetlastscanresults)

- [Macros](#macros)
  - [Numeric spellIDs in `/cast` and `CastSpellByName`](#numeric-spellids-in-cast-and-castspellbyname)
  - [`CastSpellNoToggle` as a macro cast line](#castspellnotoggle-as-a-macro-cast-line)
  - [`GetMacroSpell(macroSlot)`](#getmacrospellmacroslot)
  - [`GetMacroIcons` / `GetMacroItemIcons` / `GetLooseMacroIcons` / `GetLooseMacroItemIcons`](#getmacroicons--getmacroitemicons--getloosemacroicons--getloosemacroitemicons)

- [Mail](#mail)
  - [`GetSendMailItemLink([attachmentIndex])`](#getsendmailitemlinkattachmentindex)
  - [`GetInboxItemLink(messageIndex[, attachmentIndex])`](#getinboxitemlinkmessageindex-attachmentindex)

- [Map](#map)
  - [`C_Map.GetBestMapForUnit(unitToken)`](#c_mapgetbestmapforunitunittoken)

- [MerchantFrame](#merchantframe)
  - [`C_MerchantFrame.GetItemInfo(slot)`](#c_merchantframegetiteminfoslot)
  - [`C_MerchantFrame.GetBuybackItemID(slot)`](#c_merchantframegetbuybackitemidslot)
  - [`C_MerchantFrame.GetNumJunkItems()`](#c_merchantframegetnumjunkitems)
  - [`C_MerchantFrame.SellAllJunkItems()`](#c_merchantframesellalljunkitems)
  - [`C_MerchantFrame.IsMerchantItemRefundable(slot)`](#c_merchantframeismerchantitemrefundableslot)
  - [`C_MerchantFrame.IsSellAllJunkEnabled()`](#c_merchantframeissellalljunkenabled)

- [NamePlate](#nameplate)
  - [`C_NamePlate.GetNamePlates()`](#c_nameplategetnameplates)
  - [`C_NamePlate.GetNamePlateGUIDs()`](#c_nameplategetnameplateguids)
  - [`C_NamePlate.GetNamePlateForUnit(unitToken)`](#c_nameplategetnameplateforunitunittoken)
  - [`C_NamePlate.GetNamePlateForGUID(guidString)`](#c_nameplategetnameplateforguidguidstring)
  - [Unit tokens (`nameplateN`)](#unit-tokens-nameplaten)

- [NameCache](#namecache)
  - [`GetPlayerInfoByGUID(guid)`](#getplayerinfobyguidguid)
  - [`UnitNameFromGUID(guid)`](#unitnamefromguidguid)
  - [`C_PlayerCache.GetPlayerInfoByName(name)`](#c_playercachegetplayerinfobynamename)
  - [`C_PlayerInfo.GUIDIsPlayer(guid)` / `GUIDIsCreature` / `GUIDIsPet` / `GUIDIsGameObject`](#c_playerinfoguidisplayerguid--guidiscreature--guidispet--guidisgameobject)
  - [`C_PlayerCache.RememberPlayer(guid, name, classToken)`](#c_playercacherememberplayerguid-name-classtoken)
  - [`C_PlayerCache.SetEnabled(enabled)`](#c_playercachesetenabledenabled)
  - [`C_PlayerCache.IsEnabled()`](#c_playercacheisenabled)
  - [`C_PlayerCache.SetScanEnabled(enabled)`](#c_playercachesetscanenabledenabled)
  - [`C_PlayerCache.IsScanEnabled()`](#c_playercacheisscanenabled)

- [Quest](#quest)
  - [`C_QuestLog.GetQuestIDForLogIndex(index)`](#c_questlogGetQuestIDForLogIndexindex)
  - [`C_QuestLog.RequestLoadQuestByID(questID)`](#c_questlogrequestloadquestbyidquestid)
  - [`C_QuestLog.IsOnQuest(questID)`](#c_questlogisonquestquestid)
  - [`C_QuestLog.IsUnitOnQuest(unit, questID)`](#c_questlogisunitonquestunit-questid)
  - [`C_QuestLog.GetTitleForQuestID(questID)`](#c_questloggettitleforquestidquestid)
  - [`C_QuestLog.GetQuestDetails(questID)`](#c_questloggetquestdetailsquestid)
  - [`C_QuestLog.GetNumQuestObjectives(questID)`](#c_questloggetnumquestobjectivesquestid)
  - [`C_QuestLog.IsQuestDataCachedByID(questID)`](#c_questlogisquestdatacachedbyidquestid)
  - [`GetQuestLogLeaderBoardID(objectiveIndex [, questIndex])`](#getquestlogleaderboardidobjectiveindex--questindex)

- [Screen](#screen)
  - [`GetPhysicalScreenSize()`](#getphysicalscreensize)

- [Spell](#spell)
  - [`C_Spell.DoesSpellExist(spellID)`](#c_spelldoesspellexistspellid)
  - [`C_Spell.GetSchoolString(schoolMask)`](#c_spellgetschoolstringschoolmask)
  - [`GetSpellInfo(spellID)` / `GetSpellInfo(slot, bookType)`](#getspellinfospellid--getspellinfoslot-booktype)
  - [`C_Spell.GetSpellInfo(spellID)`](#c_spellgetspellinfospellid)
  - [`C_Spell.GetSpellName(spellID)`](#c_spellgetspellnamespellid)
  - [`C_Spell.GetSpellTexture(spellID)`](#c_spellgetspelltexturespellid)
  - [`GetSpellLink(spellID)` / `GetSpellLink(slot, bookType)`](#getspelllinkspellid--getspelllinkslot-booktype)
  - [`C_Spell.GetSpellLink(spellID)`](#c_spellgetspelllinkspellid)
  - [`C_Spell.GetSpellDescription(spellID)`](#c_spellgetspelldescriptionspellid)
  - [`C_Spell.GetSpellMechanicByID(spellID)`](#c_spellgetspellmechanicbyidspellid)
  - [`C_Spell.GetSpellRadius(spellID)` / `GetSpellRadius(slot, bookType)`](#c_spellgetspellradiusspellid--getspellradiusslot-booktype)
  - [`C_Spell.GetSpellPowerCost(spellIdentifier)`](#c_spellgetspellpowercostspellidentifier)
  - [`C_Spell.GetSpellReagents(spellID)`](#c_spellgetspellreagentsspellid)
  - [`C_Spell.GetSpellSubtext(spellIdentifier)`](#c_spellgetspellsubtextspellidentifier)
  - [`IsPassiveSpell(spellID)` / `IsPassiveSpell(slot, bookType)`](#ispassivespellspellid--ispassivespellslot-booktype)
  - [`C_Spell.IsSpellPassive(spellID)`](#c_spellisspellpassivespellid)
  - [`IsPlayerSpell(spellID)`](#isplayerspellspellid)
  - [`IsSpellKnown(spellID, [isPet])`](#isspellknownspellid-ispet)
  - [`IsUsableSpell(spell)` / `IsUsableSpell(slot, bookType)`](#isusablespellspell--isusablespellslot-booktype)
  - [`C_Spell.IsSpellUsable(spellID)`](#c_spellisspellusablespellid)
  - [`C_Spell.GetSpellCooldown(spellIdentifier)`](#c_spellgetspellcooldownspellidentifier)
  - [`C_Spell.IsCurrentSpell(spellIdentifier)`](#c_spelliscurrentspellspellidentifier)
  - [`C_Spell.IsSelfBuff(spellID)`](#c_spellisselfbuffspellid)
  - [`C_Spell.SpellHasRange(spellIdentifier)` / `SpellHasRange(slot, bookType)`](#c_spellspellhasrangespellidentifier--spellhasrangeslot-booktype)
  - [`C_Spell.IsAutoAttackSpell(spellID)`](#c_spellisautoattackspellspellid)
  - [`C_Spell.IsRangedAutoAttackSpell(spellID)`](#c_spellisrangedautoattackspellspellid)
  - [`IsHarmfulSpell(spell)` / `IsHelpfulSpell(spell)`](#isharmfulspellspell--ishelpfulspellspell)
  - [`C_Spell.IsSpellHarmful(spellID)` / `C_Spell.IsSpellHelpful(spellID)`](#c_spellisspellharmfulspellid--c_spellisspellhelpfulspellid)
  - [`GetSpellSchool(spellID)`](#getspellschoolspellid)
  - [`CastSpellNoToggle(name | spellID)`](#castspellnotogglename--spellid)
  - [`C_Spell.CastAtCursor(spellID)`](#c_spellcastatcursorspellid)
  - [`C_Spell.CastAtUnit(spellID, unit)`](#c_spellcastatunitspellid-unit)
  - [`C_Spell.CancelSpellByID(spellID)` / `CancelSpellByName(name)`](#c_spellcancelspellbyidspellid--cancelspellbynamename)
  - [`C_Spell.UnitCastingInfo(unit)` / `C_Spell.CastingInfo()`](#c_spellunitcastinginfounit--c_spellcastinginfo)
  - [`C_Spell.UnitChannelInfo(unit)` / `C_Spell.ChannelInfo()`](#c_spellunitchannelinfounit--c_spellchannelinfo)

- [SpellBook](#spellbook)
  - [`FindSpellBookSlotByID(spellID)`](#findspellbookslotbyidspellid)
  - [`C_SpellBook.GetSpellLevelLearned(spellID)`](#c_spellbookgetspelllevellearnedspellid)
  - [`C_SpellBook.GetCurrentLevelSpells([level])`](#c_spellbookgetcurrentlevelspellslevel)
  - [`C_SpellBook.GetSpellSkillLine(spellID)`](#c_spellbookgetspellskilllinespellid)
  - [`C_SpellBook.IsAutoAttackSpellBookItem(slot, bookType)`](#c_spellbookisautoattackspellbookitemslot-booktype)
  - [`C_SpellBook.IsRangedAutoAttackSpellBookItem(slot, bookType)`](#c_spellbookisrangedautoattackspellbookitemslot-booktype)

- [State](#state)
  - [`IsMounted()`](#ismounted)
  - [`Dismount()`](#dismount)
  - [`IsStealthed()`](#isstealthed)
  - [`IsFalling()`](#isfalling)
  - [`IsSwimming()`](#isswimming)
  - [`IsAssistingRitual()`](#isassistingritual)
  - [`IsInGroup()`](#isingroup)
  - [`IsInRaid()`](#isinraid)
  - [`GetMirrorTimerInfo(index)` / `GetMirrorTimerProgress(label)`](#getmirrortimerinfoindex--getmirrortimerprogresslabel)
  - [`GetShapeshiftFormID()`](#getshapeshiftformid)
  - [`CancelShapeshiftForm()`](#cancelshapeshiftform)

- [Table](#table)
  - [`table.wipe(t)`](#tablewipet)

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

- [Unit](#unit)
  - [`UnitGUID(unit)`](#unitguidunit)
  - [`UnitTokenFromGUID(guid)`](#unittokenfromguidguid)
  - [`UnitSubName(unit)`](#unitsubnameunit)
  - [`GetUnitSpeed(unit)`](#getunitspeedunit)
  - [`UnitClassBase(unit)`](#unitclassbaseunit)
  - [`UnitRaceBase(unit)`](#unitracebaseunit)
  - [`UnitIsAFK(unit)`](#unitisafkunit)
  - [`UnitIsDND(unit)`](#unitisdndunit)
  - [`UnitIsFeignDeath(unit)`](#unitisfeigndeathunit)
  - [`UnitIsInMyGuild(unitOrName)`](#unitisinmyguildunitorname)
  - [`UnitIsPossessed(unit)`](#unitispossessedunit)
  - [`UnitStandState(unit)`](#unitstandstateunit)
  - [`UnitInRange(unit)`](#unitinrangeunit)
  - [`UnitPower(unit [, powerType])` / `UnitPowerMax(unit [, powerType])`](#unitpowerunit--powertype--unitpowermaxunit--powertype)
  - [`UnitPowerType(unit)`](#unitpowertypeunit)

- [UnitAuras](#unitauras)
  - [`C_UnitAuras.GetAuraDataByIndex(unit, index [, filter])`](#c_unitaurasgetauradatabyindexunit-index--filter)
  - [`C_UnitAuras.GetBuffDataByIndex(unit, index)` / `GetDebuffDataByIndex(unit, index)`](#c_unitaurasgetbuffdatabyindexunit-index--getdebuffdatabyindexunit-index)
  - [`C_UnitAuras.GetUnitAuraBySpellID(unit, spellID [, filter])`](#c_unitaurasgetunitaurabyspellidunit-spellid--filter)
  - [`C_UnitAuras.GetPlayerAuraBySpellID(spellID)`](#c_unitaurasgetplayeraurabyspellidspellid)
  - [`C_UnitAuras.GetAuraDataBySpellName(unit, spellName [, filter])`](#c_unitaurasgetauradatabyspellnameunit-spellname--filter)
  - [`C_UnitAuras.GetUnitAuras(unit [, filter])`](#c_unitaurasgetunitaurasunit--filter)
  - [`C_UnitAuras.GetAuraDispelTypeColor(dispelName)`](#c_unitaurasgetauradispeltypecolordispelname)

- [VoiceChat](#voicechat)
  - [`C_VoiceChat.GetTtsVoices()` / `C_VoiceChat.GetRemoteTtsVoices()`](#c_voicechatgetttsvoices--c_voicechatgetremotettsvoices)
  - [`C_VoiceChat.SpeakText(voiceID, text [, destination, rate, volume])`](#c_voicechatspeaktextvoiceid-text--destination-rate-volume)
  - [`C_VoiceChat.StopSpeakingText()`](#c_voicechatstopspeakingtext)
  - [`C_TTSSettings` — getters & setters](#c_ttssettings--getters--setters)
  - [TTS events](#tts-events)
  - [TTS CVars](#tts-cvars)

## Account

### `SaveAccount(name, password)` / `DeleteAccount(name)` / `GetSavedAccounts()` / `LoginWithSavedAccount(name)` — GlueXML only

Persists account credentials in Windows Credential Manager (per-user,
DPAPI-encrypted) and dispatches login from C so the plaintext password
never crosses the C↔Lua boundary on the way out. Designed for GlueXML
rewrites of `AccountLogin.lua` that want a selection list of remembered
accounts without persisting plaintext passwords to SavedVariables.

> **Only callable from the login screen** (the "glue" Lua state).
> Calling them from an in-world addon errors with "attempt to call nil"
> — they aren't registered there.

- `SaveAccount(name, password)` — encrypt and save under the current
  realmlist. Returns `true` on success, `false` if name or password is
  empty, no realmlist is set, or the OS rejects the write.
- `DeleteAccount(name)` — remove the saved entry. Returns `true` if a
  matching entry existed and was deleted.
- `GetSavedAccounts()` — returns a numeric-keyed table of entries for
  the current realmlist. Other realmlists' entries are invisible. Empty
  table if no realmlist is set. Each entry is a table:

  | Field | Type | Notes |
  |---|---|---|
  | `name` | string | Account name as saved. |
  | `lastUsed` | number | Unix epoch seconds of the last write to the vault entry. Refreshed automatically by `LoginWithSavedAccount`, so this functions as a "last used" timestamp. `0` if Windows didn't supply a timestamp (extremely rare, e.g. credentials manually injected). |

  ```lua
  for i, entry in ipairs(GetSavedAccounts()) do
      print(entry.name, date("%Y-%m-%d %H:%M", entry.lastUsed))
  end
  ```

- `LoginWithSavedAccount(name)` — decrypt internally and feed the
  credentials to the engine's login function. Returns `true` if
  credentials were found and dispatched, `false` if no such entry
  exists. **Plaintext is never returned to Lua.** Equivalent to what
  `DefaultServerLogin(name, password)` does, but with the password
  fetched from the vault rather than passed in. Also re-saves the
  credential (same value) to refresh the `lastUsed` timestamp.

#### Scoping per realmlist

Every operation is scoped by the current `realmList` CVar (the address
from `realmlist.wtf`, e.g. `"logon.turtle-wow.org"`), not the friendly
display name. The same account name on two different private servers
therefore gets two distinct vault entries:

```
ClassicAPI/WoW/logon.turtle-wow.org/MYACCT
ClassicAPI/WoW/logon.other-server.com/MYACCT
```

Switching realmlists changes the visible account list automatically;
no separate "select realm" step. If you need a refresh while still on
the login screen, call `Autologin_Load()` (or `GetSavedAccounts()`
directly) after the realmlist changes.

#### Storage

Entries live in the OS Credential Manager under the
`ClassicAPI/WoW/<realmlist>/` namespace. Users can inspect and wipe
them via Windows' `control /name Microsoft.CredentialManager` →
Generic Credentials, the same way they'd manage any other Windows-
stored credential.

#### Security caveats

Realistic about what this defends against:

- **Defeats**: casual file inspection of WTF, sharing the WTF folder,
  cloud-syncing it, another Windows user on the same machine, malware
  running as a different Windows user.
- **Does not defeat**: any process running as the current Windows user
  can decrypt these via DPAPI — it's per-user, not per-process. No
  "decrypt and return plaintext" path is exposed to Lua, so a hostile
  addon can trigger a login with a saved account but can't extract the
  password. Plaintext is also unavoidably in process memory while the
  engine sends SRP.

#### Addon example

```lua
-- GlueXML, e.g. AccountLogin.lua: save credentials on first manual
-- login, one-click login on subsequent shows.

local function OnLoginClicked()
    local name = AccountLoginAccountEdit:GetText()
    local password = AccountLoginPasswordEdit:GetText()
    if not name or name == "" then return end

    if password and password ~= "" then
        -- Fresh login or password change: persist + log in.
        SaveAccount(name, password)
        LoginWithSavedAccount(name)
    else
        -- Reuse stored credentials.
        LoginWithSavedAccount(name)
    end
end

-- Populate a selection list from the vault, sorted by recency.
local accounts = GetSavedAccounts()
table.sort(accounts, function(a, b) return a.lastUsed > b.lastUsed end)
for i = 1, table.getn(accounts) do
    local e = accounts[i]
    print(i, e.name, "last used:", date("%m/%d/%Y", e.lastUsed))
end

-- Forget an account.
DeleteAccount("OLDACCT")
```

## Action

### `GetActionInfo(slot)`

Returns the action descriptor for a 1-based action-bar slot, in the
shape `actionType, id, subType`. Returns `nil` for empty slots.

```lua
local actionType, id, subType = GetActionInfo(1)
```

| Slot contents             | Returns                            |
|---------------------------|------------------------------------|
| empty                     | `nil`                              |
| spell                     | `"spell", spellID, "spell"`        |
| macro                     | `"macro", macroSlot`               |
| item (by-itemID)          | `"item", itemID`                   |
| item (by bag-instance)    | `"item", nil`                      |

The 120-slot action table at `0x00BC6980` packs each entry as a
`uint32` where the top 4 bits are the type tag:

- **`0x0` — spell action.** Entry is the spellID. The third return is
  always `"spell"` for entries on this table; the engine helper
  hardcodes pet flag to 0 here. Pet-bar actions live in a separate
  table — use `PetHasActionBar` + the pet-action helpers for pet
  slots.
- **`0x4` — macro or bag-item.** Ambiguous tag, disambiguated by
  walking the 36-entry macro-slot map at `0x00BDCC60` and checking
  whether `entry & 0xBFFFFFFF` matches any of those macro IDs. On a
  hit, the slot is a macro and the matching index becomes the
  1-based `macroSlot` return. On a miss, the slot is a bag-item
  reference; we surface the type as `"item"` but return `nil` for
  the `id` — extracting the itemID from the bag-action hash struct
  needs more reversing than we've done.
- **`0x8` — item by itemID.** `entry & 0x7FFFFFFF` is the itemID.

This is the same discrimination [SuperWoWhook][superwowhook] uses in
its replacement of `GetActionText` (which returns `name, type, id`).
Without SuperWoWhook the engine's stock `GetActionText` only handles
items in bags; ClassicAPI's `GetActionInfo` provides the modern-WoW
shape independently of any other DLL patches.

Equivalent to the function of the same name in retail. Subtype is
always `"spell"` for spell entries (no pet differentiation on this
table) and the bag-item itemID path is currently incomplete — see
the comment in [src/action/Info.cpp](../src/action/Info.cpp).

[superwowhook]: https://github.com/balakethelock/SuperWoW

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

### `C_AddOns.IsAddOnLoaded(indexOrName)`

Returns `loadedOrLoading, loaded` — two booleans.

```lua
C_AddOns.IsAddOnLoaded("DebugTools")    -- true, true
C_AddOns.IsAddOnLoaded("garbage")       -- false, false
C_AddOns.IsAddOnLoaded(1)               -- true, true   (first addon by index)
```

Modern WoW splits the two returns to distinguish "load-in-progress"
from "fully loaded" — the difference matters for `LoadOnDemand`
addons whose load is split across multiple `LoadAddOn` callbacks.
Vanilla 1.12's addon loader (`FUN_0051F240`) is fully synchronous:
the `loaded` byte flips inside a single call, so the in-flight
state is never observable from Lua. Both returns are always the
same boolean here. We surface the two-return shape so consumer
code written against modern API doesn't need to special-case
vanilla.

Unknown addons (numeric index past `GetNumAddOns()`, or
string name not in the registry) return `false, false`.

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

## CharacterList

### `GetSavedCharacterOrder(realm)` / `SetSavedCharacterOrder(realm, order)` — GlueXML only

Persists a user-chosen ordering for the character-select list, per realm,
per account. Designed for GlueXML mods that add drag-to-reorder to the
character-select screen — but since the storage is a plain text file
you can also pre-populate it by hand.

> **Only callable from the login / character-select screens** (the
> "glue" Lua state). Calling them from an in-world addon errors with
> "attempt to call nil" — they aren't registered there.

- `GetSavedCharacterOrder(realm)` — returns the saved order for
  `realm` as a string, or `""` if none has been saved. Never `nil`.
- `SetSavedCharacterOrder(realm, order)` — writes the order string.
  Pass `""` to clear the saved entry for that realm.

The `order` value is a pipe-delimited list of character names, e.g.
`"Thrall|Jaina|Sylvanas"`. Names are matched case-sensitively against
what `GetCharacterInfo` returns.

#### Storage file

Saved to `WTF\Account\<ACCOUNT>\ClassicAPI.txt`, one line per realm:

```ini
CharacterOrder.Octo=Thrall|Jaina|Sylvanas
CharacterOrder.AnotherRealm=Foo|Bar
```

`<ACCOUNT>` is the account name the launcher logged in with
(uppercase, the same folder name WoW uses for SavedVariables). The
realm key on each line is whatever string was passed as `realm` —
typically the result of `GetServerName()`. This file is shared with
other ClassicAPI account-scoped settings, so don't be surprised to
see unrelated `Key=Value` lines in it; leave them alone.

You can pre-populate the file by hand if you don't have a GlueXML mod
that calls `SetSavedCharacterOrder`. Just create or edit
`ClassicAPI.txt` in the correct account folder before launching the
game — any GlueXML code that reads `GetSavedCharacterOrder(realm)`
will see your saved order.

#### Addon example

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

## Combat

### `InCombatLockdown()`

Always returns `false`. Combat lockdown gates secure-frame UI
manipulation in modern WoW, and the secure-frame system didn't exist
in 1.12 — there's nothing in vanilla to lock down. This function is
provided as a no-op stub purely so addons backported from later
expansions can call it without erroring on a missing global.

For "is the player actually in combat?", use vanilla's own
`UnitAffectingCombat("player")`, which the stock 1.12 engine ships.

```lua
if not InCombatLockdown() then
    -- always true in 1.12 — falls through unconditionally
end

if UnitAffectingCombat("player") then
    -- this is the real check
end
```

## Console

### `ExportInterfaceFiles art|code` (console command)

A **developer-console command** (not a Lua function) — type it into the
`~` console, which is available when the client is launched with
`-console`. It extracts Blizzard's stock UI files out of the game's MPQ
archives onto disk, mirroring the same-named command from later clients
(4.3.4). Useful for reading the FrameXML / GlueXML source you're
backporting addons against, or pulling out the default art.

- `ExportInterfaceFiles code` — writes `.lua` / `.xml` / `.toc` /
  `.xsd` files to `BlizzardInterfaceCode\…`
- `ExportInterfaceFiles art` — writes `.blp` / `.tga` files to
  `BlizzardInterfaceArt\…`

Files are written relative to the client's working directory (next to
`WoW.exe`), preserving their `Interface\…` subtree under the
destination folder — e.g. `Interface\FrameXML\FrameXML.toc` becomes
`BlizzardInterfaceCode\FrameXML\FrameXML.toc`. On completion the command
prints a `wrote N file(s)` line back to the console.

```
> ExportInterfaceFiles code
ExportInterfaceFiles: wrote 1234 code file(s) to BlizzardInterfaceCode\
```

Notes:

- It enumerates the mounted MPQs' `(listfile)`, so it surfaces whatever
  the archives actually ship — including Blizzard's own UI addons under
  `Interface\AddOns\Blizzard_*` (AuctionUI, TalentUI, TradeSkillUI,
  etc.), which are part of the stock UI source. Files present in
  multiple archives (base + patches) are written once.
- Your own loose, on-disk addons are *not* exported — they aren't in
  any archive's listfile, so they're never enumerated.
- It runs synchronously and briefly freezes the client while it walks
  `Interface\` — expected for a one-shot extraction.

### `ExportDBCFiles` (console command)

A ClassicAPI-original companion to `ExportInterfaceFiles` that dumps
every `.dbc` table the client loads to `DBFilesClient\` under the working
directory (next to `WoW.exe`). No subcommand — there's only one thing to
export.

```
> ExportDBCFiles
ExportDBCFiles: wrote 155 .dbc file(s) to DBFilesClient\
```

Unlike the MPQ `(listfile)` alone (which omits ~18 of the tables the
client actually loads — mostly cosmetic animation/sound/lookup DBCs that
Blizzard left out of the index), this unions two sources for a complete
set:

1. the **MPQ `(listfile)`** under `DBFilesClient\` — catches files that
   ship in the archives without a loader (e.g. `wowerror_strings.dbc`);
2. a **`.text` scan for the DBC path-getter pattern**
   (`mov eax, &"DBFilesClient\X.dbc"; ret`) — the authoritative list of
   what this build loads. The master DBC init (`FUN_0053f8b0`) is
   straight-line with no iterable name table, so the per-DBC getters
   *are* the registry; scanning for them is the same technique that
   built [docs/DBCs.md](DBCs.md).

Results are deduped case-insensitively. Handy for a local DBC dump to
inspect schemas/values directly — e.g. verifying a record's column
layout against a known row, the way `C_Item.GetEnchantInfo`'s effect
columns were confirmed — without a standalone MPQ extraction tool.

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

### `GetItemCooldown(itemInfo)` / `C_Container.GetItemCooldown(itemID)`

Returns `(startTime, duration, enable)` for the cooldown of the
spell triggered by the item's ON_USE effect. Direct-by-ID variant of
vanilla's `GetContainerItemCooldown(bag, slot)` — no slot reference
required.

```lua
-- Hearthstone (6948), right after using it:
/dump GetItemCooldown(6948)
-- 732120.014, 3600, 1  (startTime in GetTime() seconds, 60 min duration, ready)

-- Item with no ON_USE effect (or invalid itemID) returns all zeros.
/dump GetItemCooldown(2589)   -- Linen Cloth → 0, 0, 0
```

| Field | Notes |
|-------|-------|
| `startTime` | `GetTime()`-compatible seconds when the cooldown started. `0` if no cooldown. |
| `duration` | Cooldown length in seconds. `0` if no cooldown. |
| `enable` | `1` for "ready or counting down" (normal state); `0` for "used but cooldown hasn't started yet" (the potion-in-combat case). |

**Input shapes**:

- `GetItemCooldown(itemInfo)` — accepts itemID, `item:N` / chat-link
  hyperlink, numeric string, or item name (resolved via the shared
  `Item::Arg` helper, same chain `C_Item.GetItemCount` etc. use).
- `C_Container.GetItemCooldown(itemID)` — modern signature accepts
  number / hyperlink but **not** spell name (per Blizzard's spec
  "will not accept an itemlink or name", but link parsing falls out
  of the shared `Item::Arg::Resolve` for free, so we accept it).

Routes through `FUN_ITEM_QUERY_COOLDOWN` (`0x006E2ED0`) which finds
the item's ON_USE spell slot in its `ItemStats_C` record and queries
that spell's cooldown via the same manager player-spell cooldowns
use.

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

### `C_Container.GetContainerItemCharges(containerIndex, slotIndex)`

Per-slot equivalent of
[`C_Item.GetItemCount`](#c_itemgetitemcountitem-includebank-includecharges)'s
`includeCharges=true` mode — returns the total uses available in
*this single slot*, where `GetItemCount` totals the same value
across every matching slot.

```
uses = C_Container.GetContainerItemCharges(containerIndex, slotIndex)
```

`containerIndex` accepts the same values as
[`C_Container.GetContainerItemDurability`](#c_containergetcontaineritemdurabilitycontainerindex-slotindex)
(`0` = backpack, `1..4` = equipped bags, `-1`/`5..11` = bank, etc.).
`slotIndex` is 1-based.

Math is `stack * usesPerItem`, where `usesPerItem` is
`abs(SPELL_CHARGES[0])` for items with a negative charges field
(consume-on-use: wands, healthstones, mana gems, sapper charges)
and `1` for everything else. Worked examples:

| Slot contents | Stack | Raw charges | Returns |
|---|---:|---:|---:|
| Wand of Decay at 50 charges | 1 | -50 | **50** |
| Wand at 1 charge | 1 | -1 | **1** |
| Healthstone | 1 | -1 | **1** |
| Stack of 20 water | 20 | -1 | **20** |
| Stack of 5 mana potions | 5 | -1 | **5** |
| Hearthstone (cooldown-only) | 1 | 0 | **1** |
| Any other item in the slot | 1 | 0 | **1** |

Returns `nil` only when the slot is empty.

```lua
-- "how many charges does my wand have left?"
local charges = C_Container.GetContainerItemCharges(0, 1)
print("Wand charges remaining: " .. charges)

-- "how many drinks across the bags?" — same as GetItemCount(water, false, true)
local total = 0
for bag = 0, 4 do
    for slot = 1, GetContainerNumSlots(bag) do
        if C_Container.GetContainerItemID(bag, slot) == waterID then
            total = total + C_Container.GetContainerItemCharges(bag, slot)
        end
    end
end
```

ClassicAPI addition; modern WoW has no direct equivalent (modern
addons read charges off tooltip text). Useful when you need a
per-slot number rather than `GetItemCount`'s rollup — e.g. to
display "X charges" on the slot UI for a wand without re-walking
every other matching item.

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

### `C_Container.IsContainerItemOpenable(containerIndex, slotIndex)`

Positional-arg wrapper for [`C_Item.IsItemOpenable`](#c_itemisitemopenableitemlocation--c_itemisitemopenablebyiditem)
against a bag/slot pair. Same `(isOpenable, canOpen)` tuple — see
the linked section for full semantics. Both returns are `nil` for
empty slots or items whose data hasn't been cached yet.

```lua
-- Sweep main backpack for openables
for slot = 1, GetContainerNumSlots(0) do
    if C_Container.IsContainerItemOpenable(0, slot) then
        -- ...
    end
end
```

Same cache-warming caveat as the underlying `C_Item.IsItemOpenable` —
a freshly-seen item may report `false` for one call while the
`SMSG_ITEM_QUERY_SINGLE` round-trips. Bag-resident items are almost
always already cached.

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
`PlayerHasHearthstone` uses and hands the resulting `CGItem *` pointer
to the engine's by-pointer use primitive at `0x005D8D00` — the same
fallback path `Script_UseContainerItem` ends up at after every special
cursor-mode branch (repair vendor, spell-cast targeting, drop-on-bag)
is skipped. Bypassing the dispatcher avoids the Lua-stack roundtrip
of pushing `(bag, slot)` only for `Script_UseContainerItem` to re-parse
and re-resolve the same item.

### `C_Container.SwapItems(srcBag, srcSlot, dstBag, dstSlot)`

Atomically swaps two bag slots on the server, no cursor involved.
Returns `true` on send, `false` for bad args (missing bag, out-of-range
slot, empty source).

```lua
-- Move slot 3 of bag 1 to slot 7 of the backpack
C_Container.SwapItems(1, 3, 0, 7)

-- Swap two backpack slots
C_Container.SwapItems(0, 1, 0, 2)
```

Bag IDs match the rest of `C_Container.*`:

- `0` — backpack
- `1..4` — equipped bags
- `-1` — main bank (24 slots; **requires the bank window to be open**)
- `5..10` — equipped bank bags (**requires the bank window to be open**)

Slots are 1-based. The engine accepts both directions:

- Same container (e.g. backpack ↔ backpack, bag1 ↔ bag1, bank ↔ bank)
  sends opcode `0x10D` `CMSG_SWAP_INV_ITEM` with the two linear slot bytes.
- Cross-container (e.g. backpack ↔ bag1, bag2 ↔ bank, bank ↔ bank bag)
  sends opcode `0x10C` `CMSG_SWAP_ITEM` with `(srcBag, srcSlot, dstBag, dstSlot)`.

Destination empty becomes a move; destination occupied becomes an
atomic swap.

> **Bank slots require the bank window to be open.** The engine
> doesn't sync bank-side `CGContainer`s or bank inventory entries
> before the first bank interaction, so any bank `EncodeBagSlot`
> path returns false until then. Same constraint as
> `C_Container.GetContainerItemInfo` on bank IDs.

> **ClassicAPI-only.** Modern Classic Era has no direct swap-two-slots
> call — addons there drive the cursor with two `PickupContainerItem`
> calls in sequence. This single-call form bypasses the cursor
> entirely (same path
> [`C_EquipmentSet.UseEquipmentSet`](#c_equipmentsetuseequipmentsetsetid)
> uses for its swap chain).

Send is fire-and-forget. Server confirms via the normal
`BAG_UPDATE` / `SMSG_INVENTORY_CHANGE_FAILURE` flow; this call does not
wait for that round-trip.

### `C_Container.MoveItem(srcBag, srcSlot, dstBag, dstSlot, count)`

Atomic split-and-place: move exactly `count` items from src to dst,
no cursor involvement. Returns `true` on send, `false` on bad args
(missing bag, out-of-range slot, empty source, `count < 1`, or
`count > 255`).

```lua
-- Split 5 items off backpack slot 1 into backpack slot 2
C_Container.MoveItem(0, 1, 0, 2, 5)

-- Top up a partial stack by moving 3 items from another stack
C_Container.MoveItem(1, 4, 0, 7, 3)
```

Server semantics are all-or-nothing — vanilla has no partial-move
form:

- `dst` empty → places `count` there (clean split).
- `dst` holds the same item & `dstCount + count ≤ maxStack` → merges
  `count` into dst.
- `dst` holds a different item, or the merge would overflow → server
  rejects entirely (source untouched, `SMSG_INVENTORY_CHANGE_FAILURE`
  fires).

Use this instead of `C_Container.SwapItems` when you want to
consolidate partial stacks — `SwapItems` swaps the whole slots, which
isn't what you want when both slots hold the same item but one has a
partial stack.

Bank IDs (`-1`, `5..10`) work here too with the same bank-window
constraint as `SwapItems`.

> **vs. the modern equivalent.** Modern Classic Era has no direct
> one-call move — addons there string `SplitContainerItem(bag, slot,
> count)` + `PickupContainerItem(dstBag, dstSlot)` together to drive
> the cursor. This bundles the same two-step into one packet
> (`CMSG_SPLIT_ITEM`, opcode 0x10E), so the cursor is never touched.

Send is fire-and-forget (same as `SwapItems`).

## Coroutine

Restores Lua 5.0's stripped `coroutine.*` library. The C-level
coroutine machinery (`lua_resume`, `lua_yield`, `lua_newthread`,
and the `thread` type at tag 8) is linked into the engine, but
the script-facing library was never registered as a global table.
ClassicAPI rewires the standard five entries on `coroutine`,
matching Lua 5.0 semantics with two WoW-specific quirks called
out below.

### `coroutine.create(fn)`

Creates a new coroutine with `fn` as its body and returns the
resulting `thread`. `fn` must be a **Lua** function — passing a
C function (any engine global like `GetTime` or `UnitName`)
raises `bad argument #1 to 'create' (Lua function expected)`.

The new coroutine starts suspended; nothing runs until
`coroutine.resume` is called on it. The args passed to the first
resume become the body's arguments.

### `coroutine.resume(co, ...)`

Resumes a suspended coroutine, passing the trailing args to either
the body (first resume) or back as the return values of the
`coroutine.yield` that paused the coroutine.

Returns:

- `true, ...values` on success, where `values` are whatever the
  coroutine yielded (mid-execution) or returned (on completion).
- `false, errMsg` on error. Two error sources:
  - Coroutine isn't suspended — e.g. resuming a dead one;
    `errMsg` is a fixed string like `"cannot resume dead coroutine"`.
  - The body hit a Lua VM error; `errMsg` is the formatted error
    string with file:line prefix.

**Caveat — `error()` doesn't propagate.** WoW replaces Lua's
`error()` global with a soft handler that calls
`geterrorhandler()(msg)` and returns normally instead of
longjmp'ing. So `error("msg")` inside a coroutine body looks like
a clean return to `resume` — the message gets printed to chat as
a side effect and `resume` returns `true` with no values. Real VM
errors (calling `nil`, indexing `nil`, etc.) still propagate
through the standard `false, msg` path.

### `coroutine.yield(...)`

Suspends the current coroutine. Whatever's passed to `yield`
becomes the return values of the matching `coroutine.resume`
call. When the coroutine is next resumed, the args passed to
`resume` become the return values of `yield`.

Raises an error if called outside a coroutine
(`attempt to yield across metamethod/C-call boundary`) or from
a C function (`cannot yield a C function`).

### `coroutine.status(co)`

Returns the coroutine's state as a string:

| Value | Meaning |
|-------|---------|
| `"running"` | `co` is the currently-running coroutine (i.e. the same thread that called `status`). |
| `"suspended"` | `co` is paused — either initial (never resumed) or mid-yield. |
| `"dead"` | `co`'s body has returned or errored. |

The standard Lua `"normal"` state — a coroutine that resumed a
deeper coroutine and is now an ancestor on the call chain — is
collapsed into `"suspended"`. Detecting it cleanly requires
walking the global thread list, and the case is near-useless for
addon-level logic.

### `coroutine.wrap(fn)`

Equivalent to `coroutine.create(fn)` followed by returning a
closure that calls `coroutine.resume(co, ...)` on each invocation
— but propagates errors as raised exceptions (via `lua_error`)
instead of returning `false, msg`. Lets you use a coroutine as
if it were an ordinary function:

```lua
local gen = coroutine.wrap(function()
    for i = 1, 5 do coroutine.yield(i) end
end)
print(gen())  -- 1
print(gen())  -- 2
print(gen())  -- 3
```

Calling the wrapped function after the body has completed raises
`cannot resume dead coroutine`. The canonical "iterator that
terminates cleanly through `for ... in`" idiom is to yield an
explicit `nil` at the end so the for-loop's nil sentinel fires:

```lua
local function range(from, to)
    return coroutine.wrap(function()
        for i = from, to do coroutine.yield(i) end
        coroutine.yield(nil)
    end)
end

for n in range(1, 5) do print(n) end
```

### Async pattern (`RunAsync` + `C_Timer.After`)

Coroutines pair naturally with `C_Timer.After` for chunked
"do some work, wait a frame, do more" loops. The canonical
scheduler:

```lua
local function RunAsync(fn)
    local co = coroutine.create(fn)
    local function step()
        local ok, delay = coroutine.resume(co)
        if not ok or coroutine.status(co) == "dead" then return end
        C_Timer.After(delay or 0, step)
    end
    step()
end

RunAsync(function()
    for i = 1, 100 do
        DoExpensiveWork(i)
        coroutine.yield(0)     -- next frame
    end
    coroutine.yield(0.5)        -- pause half a second
    DoFinalWork()
end)
```

`coroutine.yield(n)` returns `n` to the scheduler, which schedules
the next `step` after that many seconds. `yield(0)` is the
"resume on the next frame" form (since `C_Timer.After(0, ...)`
fires on the next OnUpdate tick). `RunNextFrame(cb)` in
[`AddOns/!!!ClassicAPI/Util/FunctionUtil.lua`](../AddOns/!!!ClassicAPI/Util/FunctionUtil.lua)
wraps the same primitive for the non-coroutine case.

## Creature

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
(`"0xLLLLLLLL"`) GUID-string forms are both accepted, the same forms
the `C_PlayerInfo.GUIDIs*` functions take.

The creatureID this returns is exactly what
[`C_CreatureInfo.GetCreatureInfoByID`](#c_creatureinfogetcreatureinfobyidcreatureid)
takes — `GetCreatureInfoByID(GetCreatureID(UnitGUID("target")))` gives
the target's cached name/type/family/rank.

### `C_CreatureInfo.GetCreatureInfoByID(creatureID)`

Reads the client-side **creature cache** (`creaturecache.wdb`, fed by
`SMSG_CREATURE_QUERY_RESPONSE`) for an NPC/creature by ID and returns a
table — or `nil` if it isn't cached:

```lua
local info = C_CreatureInfo.GetCreatureInfoByID(61332)
-- {
--   creatureID = 61332,
--   name       = "Misthoof Stag",
--   subName    = "",     -- title/subtitle ("" if none)
--   type       = 1,      -- CreatureType: 1=Beast, 7=Humanoid, …
--   family     = 0,      -- CreatureFamily
--   rank       = 0,      -- 0=normal, 1=elite, 2=rareelite, 3=worldboss, 4=rare
--   displayID  = 10957,  -- model display ID
-- }
```

This is a synchronous **peek** — it returns data only for creatures
already cached (seen this session, or loaded from `creaturecache.wdb`
at login), and `nil` otherwise. Unlike `UnitCreatureType`/
`UnitClassification`, which only work on a *live unit token*, this
answers for any creatureID you have cached — even one not currently in
the world.

Reads through the engine's generic cache `_GetRecord` (with no callback,
so it's a pure lookup — no network query) and pulls the fields straight
off the cached data block. Field offsets verified against the binary
(rank read by the engine's classification helper at `[block+0x20]`) and
real `creaturecache.wdb` rows (Misthoof Stag→type 1, Nordrassil
Nymph→type 7 rank 1, Greathorn Hunter→family 26).

For a creature that isn't cached yet, call
[`RequestLoadCreatureByID`](#c_creatureinforequestloadcreaturebyidcreatureid)
first and read it once `CREATURE_DATA_LOAD_RESULT` fires.

### `C_CreatureInfo.RequestLoadCreatureByID(creatureID)`

Asynchronously fetches a creature that isn't in the cache yet — issues
`SMSG_CREATURE_QUERY` and fires **`CREATURE_DATA_LOAD_RESULT(creatureID,
success)`** when the response lands, at which point
[`GetCreatureInfoByID`](#c_creatureinfogetcreatureinfobyidcreatureid)
returns its data. The same shape as
[`C_Item.RequestLoadItemData`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("CREATURE_DATA_LOAD_RESULT")
f:SetScript("OnEvent", function()
    -- 1.12 passes event args as globals: arg1 = creatureID, arg2 = success
    if arg2 then
        local info = C_CreatureInfo.GetCreatureInfoByID(arg1)
        -- info.name, info.type, info.rank, …
    end
end)

C_CreatureInfo.RequestLoadCreatureByID(10184)   -- Onyxia → fires the event,
                                                --  then GetCreatureInfoByID works
```

Returns `true` if the request was accepted (`false` only on bad input
or a full pending set). If the creature is **already** cached, the
event fires synchronously with `success = true`. On a query that never
resolves it fires once with `success = false` after a timeout.

Implementation: a shared `Cache::QueryLoad` dispatcher hooks the engine's
generic cache response parser (`FUN_00556E20`) **once** and routes by
cache instance — the same hook serves the gameobject cache too. The
creature/gameobject caches use this generic parser; the item and quest
caches have their own, so there's no hook collision. Verified in-game: `RequestLoadCreatureByID(10184)` on
an uncached Onyxia fired the event and populated the cache (type 2
Dragonkin, rank 3 world boss).

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

## Cursor

### `GetCursorInfo()`

Returns a tuple describing whatever the player has picked up on the
cursor (item, money, spell, macro, merchant slot), or nothing when
the cursor is empty. Reads the same engine globals `CursorHasItem` /
`CursorHasSpell` / `GetCursorMoney` consult, so the answer always
matches what the rest of the engine sees.

Return shape per cursor state:

| Cursor holds | Returns |
|---|---|
| nothing | *nil* |
| a bag item (PickupContainerItem, mail/trade detach with GUID) | `"item"`, `itemID`, `itemLink` |
| a drag-source item (equipment slot, action bar, trade, mail) | `"item"`, `itemID` |
| money (gold-split UI, `PickupPlayerMoney`) | `"money"`, `copperAmount` |
| a spell from the spellbook | `"spell"`, `spellbookSlot`, `bookType`, `spellID` |
| a macro | `"macro"`, `macroIndex` |
| a merchant slot | `"merchant"`, `merchantIndex` |

`bookType` is `"spell"` for player spells, `"pet"` for pet spells.
`spellbookSlot` is the 1-based slot the spell lives at in the
spellbook arrays, found by walking the player/pet books. `0` if the
spellID isn't in either book (rare — passive talent-granted spells).

```lua
local kind, a, b, c = GetCursorInfo()
if kind == "item" then
    print(("cursor item: %d"):format(a))
elseif kind == "money" then
    print(("cursor money: %d copper"):format(a))
elseif kind == "spell" then
    print(("cursor spell: id=%d slot=%d (%s book)"):format(c, a, b))
elseif kind == "macro" then
    print(("cursor macro index: %d"):format(a))
end
```

#### `itemLink` availability

Returned only when the cursor holds a **bag item** (type 1) —
`PickupContainerItem` / `PickupInventoryItem` from a bag slot, mail
or trade detach. The cursor stores the item's GUID in this case, so
we resolve back to the live `CGItem *` and feed it to the engine's
link builder, which produces the full per-instance form including
enchant ID and random-suffix decoration:

```
|cff1eff00|Hitem:16791:255:0:0|h[Silkstream Cuffs]|h|r
|cff1eff00|Hitem:9877:0:767:0|h[Sorcerer Cloak of the Owl]|h|r
```

Not returned for drag-source items (equipment slot, action bar
slot, trade slot, mail attachment — types 5/6/7/9). In those cases
the cursor only stores the itemID (no GUID, no live `CGItem *`), so
the engine's link builder isn't reachable. Derive a basic link from
the itemID via `GetItemInfo` if you need one for display:

```lua
local kind, itemID, link = GetCursorInfo()
if kind == "item" and not link then
    link = (select(2, GetItemInfo(itemID))) or ("item:" .. itemID)
end
```

#### Types we don't surface

The engine has cursor types beyond what `GetCursorInfo` advertises in
modern WoW — pet action drag, trade-slot drag, mail-attach drag,
ability-bar drag from `PickupAction`, etc. These don't map cleanly to
any modern `GetCursorInfo` string, so we return `nil` for them.
Consumers needing to detect those can use the engine-native
`CursorHasItem` / `CursorHasSpell` / `CursorHasMoney` (which already
exist in vanilla) and the source-side `PickupX` calls.

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

No cap on the number of sets per character. The full list re-
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

Returns the new setID on success. Returns nothing if the name is
empty or already in use.

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

### `PLAYER_EQUIPMENT_CHANGED` event

Backport of the WotLK-era event that fires once per paperdoll slot
change — equip, unequip, or swap:

```
PLAYER_EQUIPMENT_CHANGED: equipmentSlot, hasCurrent
```

- **`equipmentSlot`** (number) — the 1-based inventory slot that
  changed (`1` = head … `19` = tabard; same numbering as
  `GetInventoryItemLink("player", slot)`).
- **`hasCurrent`** — `1` when the slot now holds an item, `nil` when
  it's now empty (write `if hasCurrent then` as in modern code —
  vanilla's event dispatcher can't push real booleans).

Vanilla only fires `UNIT_INVENTORY_CHANGED("player")`, which doesn't
say *which* slot changed; this event lets gear trackers, stat sheets,
and tooltip decorators refresh exactly one slot instead of rescanning
all 19.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("PLAYER_EQUIPMENT_CHANGED")
f:SetScript("OnEvent", function()
    print("slot", arg1, arg2 and "equipped" or "now empty")
end)
```

Fully event-driven — no polling. The DLL registers change observers
for the 19 equipment GUID fields of the player descriptor through the
engine's own field-observer system (the same mechanism the engine
uses to watch bag slots), so the event fires exactly when the
server's update packet writes a new item GUID into a slot, once per
changed slot. Swapping two items (e.g. weapon ⇄ bag) fires once per
affected equipment slot.

### `UPDATE_INVENTORY_DURABILITY` event

Backport of the modern event that fires — with no payload — when an
equipped item's durability changes: combat damage, resurrection
penalty, repairs. The standard consumer pattern works unchanged:
rescan the 19 slots with `GetInventoryItemDurability` on each fire.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("UPDATE_INVENTORY_DURABILITY")
f:SetScript("OnEvent", RefreshMyDurabilityHUD)
```

Event-driven, no polling: piggybacks on the engine's own
inventory-alerts recompute — the routine behind vanilla's
`UPDATE_INVENTORY_ALERTS` (the red/yellow "broken armor man"), which
the engine runs whenever an owned item's fields change, on equip
changes, and at enter-world. On each recompute the DLL diffs a
per-slot `{item, durability}` snapshot and fires once when anything
actually changed — a death dinging all your armor produces one event,
not nineteen. Also fires when the *set* of equipped items changes
(swapping to a differently-damaged item updates durability UI, so
consumers want the refresh anyway). Bag-carried items don't trigger
it: vanilla renders durability UI for equipped gear only.

### `HEARTHSTONE_BOUND` event

Fires (with no payload) every time the player binds their
hearthstone at an innkeeper. Polyfills modern WoW's event of the
same name — addons listen to `HEARTHSTONE_BOUND` and re-read
`GetBindLocation()` to refresh whatever bind-location UI they show.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("HEARTHSTONE_BOUND")
f:SetScript("OnEvent", function()
    local newLocation = GetBindLocation()
    -- update UI...
end)
```

Fires **every time** the player confirms a bind, including when the
new bind is at the same inn as before. The event is driven by the
bind ACTION (server's SMSG_BINDPOINTUPDATE), not by the area name
changing — matches modern semantics.

Implemented by hooking the BINDPOINTUPDATE packet handler at
`FUN_005ED3C0`. The handler runs in two distinct cases:

1. **Initial post-login sync** — the engine has just zeroed the
   "bind valid" flag during character-entry init (`FUN_005E2510`),
   and this packet repopulates it. We detect this by reading the
   flag *before* the original handler runs; if it's `0`, this is
   the sync and we suppress the event.
2. **Innkeeper rebind** — the flag is already `1`. Fire.

Character switch is handled automatically: the character-entry init
re-zeroes the flag, so each new character's first update is also
treated as sync and suppressed.

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

### `LOOT_SCAN_COMPLETED` event

Fires (no payload) once a [`C_Loot.ScanNearbyLoot()`](#c_lootscannearbyloot)
call has finished walking every reachable corpse. Handler should call
[`C_Loot.GetLastScanResults()`](#c_lootgetlastscanresults) to read the
collected `{ guid, coin, items }` array.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("LOOT_SCAN_COMPLETED")
f:SetScript("OnEvent", function()
    local results = C_Loot.GetLastScanResults()
    -- ... process results
end)
C_Loot.ScanNearbyLoot()
```

Fires exactly once per `ScanNearbyLoot()` call regardless of
outcome — even if every corpse timed out and the results table is
empty. Doesn't fire spuriously for scans triggered by other addons
(only one scan runs at a time; concurrent attempts return `false`
from `ScanNearbyLoot`). See the Loot section for the full scan flow.

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

### `NAME_PLATE_CREATED` / `NAME_PLATE_UNIT_ADDED` / `NAME_PLATE_UNIT_REMOVED` events

Fire when nameplate state actually changes. Payloads:

| Event | `arg1` | Notes |
|-------|--------|-------|
| `NAME_PLATE_CREATED` | nameplate **Frame** | Matches modern WoW. Fires once per unique `CGNamePlateFrame` pointer — same frame re-used via pool recycle does NOT refire. |
| `NAME_PLATE_UNIT_ADDED` | `"nameplateN"` **unit token** | Matches modern WoW. Pass straight to `UnitName` / `UnitGUID` / `UnitClass` / etc., or to [`GetNamePlateForUnit`](#c_nameplategetnameplateforunitunittoken) for the frame. The token is positional — see [Unit tokens](#unit-tokens-nameplaten) for ordering semantics. |
| `NAME_PLATE_UNIT_REMOVED` | `"nameplateN"` **unit token** | Same as above. Computed from the plate's slot *before* it shifts out of the ordered list, so the token still resolves to the leaving unit during the event handler. |

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("NAME_PLATE_CREATED")
f:RegisterEvent("NAME_PLATE_UNIT_ADDED")
f:RegisterEvent("NAME_PLATE_UNIT_REMOVED")
f:SetScript("OnEvent", function()
    if event == "NAME_PLATE_CREATED" then
        -- arg1 = the nameplate Frame itself
        arg1:SetAlpha(0.8)
    elseif event == "NAME_PLATE_UNIT_ADDED" then
        -- arg1 = "nameplate1" / "nameplate2" / ...
        local name = UnitName(arg1)
        local plate = C_NamePlate.GetNamePlateForUnit(arg1)
        -- ... style based on the unit ...
    end
end)
```

> **CREATED timing with nameplate addons (pfUI / TidyPlates / etc).**
> The event fires when the *engine* allocates the underlying
> `CGNamePlateFrame`. Nameplate-mod addons typically decorate the
> frame on their own per-frame update — so `arg1` at `CREATED` time
> is a bare frame with no addon-side decorations yet. For
> unit-specific work after the addon has decorated, use
> `NAME_PLATE_UNIT_ADDED` (fires next tick at the latest) or fetch
> the current frame on-demand via `GetNamePlateForUnit(arg1)`.

> **Token stability gotcha.** Like modern WoW, the `arg1` token is
> positional — `"nameplate3"` today may resolve to a different unit
> after the slot vacates and shifts. If you need a per-unit hash key
> for cross-event bookkeeping, call `UnitGUID(arg1)` and store the
> GUID instead. See [Unit tokens](#unit-tokens-nameplaten) for the
> ordering rules.

**Implementation notes**

Detected by per-frame polling, not engine hooks. Each world tick we
walk the object hash for nameplated units and diff against the
previous tick's snapshot. Modern WoW also synthesizes these via
diffing (the underlying engine has no event for "plate state
changed"). The cost is ~20-50µs/frame even in busy raids — well
below noise.

The diff approach absorbs the engine's transient hide/reshow cycle:
vanilla has ~7 code paths that briefly zero `unit + 0xE60` (z-order
rebuilds, anchor changes, flag-change re-eval) and the next frame's
show path re-allocates from the pool. Those transient zeroes never
become events because the unit appears in both the previous and
current tick's snapshot.

### `PLAYER_FOCUS_CHANGED` event

Fires whenever the player's focus changes — assignment via
[`FocusUnit`](#focusunitunit), clear via [`ClearFocus`](#clearfocus),
or a `FocusUnit(token)` call where the token resolves to no GUID
(implicit clear). No payload args; the new focus GUID is read off
[`UnitGUID("focus")`](#unitguidunit) in the handler.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("PLAYER_FOCUS_CHANGED")
f:SetScript("OnEvent", function()
    local guid = UnitGUID("focus")
    if guid then
        print("focusing", UnitName("focus"))
    else
        print("focus cleared")
    end
end)
```

**Identity-checked**: assigning the same GUID twice is a no-op
(no event refires), matching 3.3.5's `FUN_0051FF20` behavior.
Mirrors modern's documented semantics — "fired whenever the
player's focus target is changed, including when the focus target
is lost or cleared".

### `QUEST_ACCEPTED` event

Fires once per quest the player just accepted, with two payload args:
the 1-based quest log index and the questID. Matches the Cata/WotLK
signature `QUEST_ACCEPTED(questLogIndex, questID)`. Polyfills modern
WoW's event of the same name (added in 3.1.0).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("QUEST_ACCEPTED")
f:SetScript("OnEvent", function()
    -- 1.12: event payload is in `arg1`, `arg2`, ... globals
    if event == "QUEST_ACCEPTED" then
        local questLogIndex, questID = arg1, arg2
        -- ...
    end
end)
```

Fires for every path that adds a quest to the local log — NPC accept,
party-shared quest accept, auto-grant from quest items — by hooking
the single engine chokepoint (`FUN_QUEST_LOG_REBUILD` at `0x004DE510`)
that rebuilds the Lua-visible quest log from the player's
authoritative slot data after any quest state change.

**Does not fire on initial login / character entry**, even though the
same engine function runs the bulk-sync there. Suppression is
heuristic: if a single rebuild call adds more than one quest, it's
treated as a resync and skipped. Human input speed can't accept two
quests within the same engine tick, so single-add is always a real
user accept. A brand-new character's very first quest accept
(`0 → 1` entries) fires correctly.

### `QUEST_REMOVED` event

Fires once per quest leaving the local quest log, with the questID as
the single payload arg. Covers **both turn-ins and abandons** — the
two are distinguishable by whether a `QUEST_TURNED_IN` accompanies
the removal. Polyfills modern WoW's event of the same name (added in
8.0.1; the Classic-era signature is the bare questID, which is what
we match).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("QUEST_REMOVED")
f:SetScript("OnEvent", function()
    if event == "QUEST_REMOVED" then
        local questID = arg1
        -- ...
    end
end)
```

Synthesized from the same `FUN_QUEST_LOG_REBUILD` pre-/post-snapshot
diff as `QUEST_ACCEPTED` — the removal side of the delta. For
turn-ins, `QUEST_REMOVED` fires **after** `QUEST_TURNED_IN`, matching
retail ordering: the SMSG_QUESTGIVER_QUEST_COMPLETE packet doesn't
touch the log itself; the removal arrives in the follow-up quest-log
update packets, whose rebuild triggers the diff. The observed turn-in
sequence is `QUEST_TURNED_IN` → `UNIT_QUEST_LOG_CHANGED` →
`QUEST_LOG_UPDATE` (fired inside the rebuild) → `QUEST_REMOVED`.

**Does not fire on login / character-switch resyncs.** Same
suppression rule as `QUEST_ACCEPTED` (a user action removes at most
one quest per rebuild), plus one extra gate: a rebuild that removes
one quest while adding several is a cross-character resync shape, not
gameplay, and stays silent.

### `QUEST_TURNED_IN` event

Fires when the server confirms a quest turn-in
(SMSG_QUESTGIVER_QUEST_COMPLETE, opcode `0x191`). Payload
`(questID, xpReward, moneyReward)` matches modern WoW exactly —
`xpReward` is the experience awarded (0 at max level or for
non-XP-bearing quests), `moneyReward` is in copper.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("QUEST_TURNED_IN")
f:SetScript("OnEvent", function()
    if event == "QUEST_TURNED_IN" then
        local questID, xp, money = arg1, arg2, arg3
        -- ...
    end
end)
```

Polyfilled by hooking the per-opcode SMSG handler at `FUN_005DC400`
(found via the CMSG side: `Script_GetQuestReward` →
`FUN_005015B0` → `FUN_005EADC0` sends opcode `0x18E`
CMSG_QUESTGIVER_CHOOSE_REWARD; SMSG response is opcode `0x191`).
The hook saves the packet stream's read cursor (`stream+0x14`),
peeks the first 4 uint32s of the body (questID, unknown, xp,
money), restores the cursor, then calls the original — which
re-reads the same data without seeing our peek. Side-effect-free.

Does NOT fire on quest abandon, on `QUEST_FINISHED` window
transitions (which fire on Detail → Progress → Reward → Close —
not a clean turn-in signal), or on server-reject paths (bag-full,
quest invalidated, etc.). Only fires on a real successful turn-in.

> **Why not key off `QUEST_FINISHED`?** Native vanilla `QUEST_FINISHED`
> fires on every quest-window state transition, including reads,
> aborts, and player-side close. The SMSG_QUESTGIVER_QUEST_COMPLETE
> packet, by contrast, is server-authoritative — the server only
> sends it after committing the turn-in (XP / money / item awards
> done, quest removed from log). Hooking the packet handler gives
> us the same signal modern WoW's event uses.

### `UNIT_FACTION` event (fire-coverage fix)

`UNIT_FACTION` already exists in the vanilla event table — addons can
register for it — but the 1.12 engine **never fires it** on several
paths where modern WoW (3.3.5+) does. We restore the modern firing
semantics so addons that observe `UNIT_FACTION` stop missing state
changes.

Payload is unchanged from modern: `arg1` is the unit token string,
always `"player"` for these paths (you can only toggle / sync rep on
the local player).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("UNIT_FACTION")
f:SetScript("OnEvent", function()
    if event == "UNIT_FACTION" and arg1 == "player" then
        RefreshFactionUI()
    end
end)
```

**Paths that now fire `UNIT_FACTION("player")`:**

| Path | Trigger | Detection |
|------|---------|-----------|
| `FactionToggleAtWar` | Lua call to toggle AT_WAR | Slot's flag byte changed (skips PEACE_FORCED no-ops, looting-blocked attempts, standing-blocked attempts) |
| `SetFactionInactive` / `SetFactionActive` | Lua call to toggle INACTIVE | Slot's flag byte changed |
| `SMSG_SET_FACTION_STANDINGS` | Server rep-change push (kills, quest turn-ins, etc.) | Any byte in the 64-entry flag array changed — catches AT_WAR side-effect flips when standing crosses the -3000 threshold |
| `SMSG_SET_FACTION_ATWAR` | Server force-change of AT_WAR (faction reset, server-side state push) | Unconditional after byte write |
| `SMSG_INITIALIZE_FACTIONS` | Server initial faction sync at login | Unconditional after table populated |

The last one fixes the case where addons relying on `UNIT_FACTION`
(rather than `PLAYER_LOGIN` / `VARIABLES_LOADED`) miss the initial
load entirely on vanilla.

**Not yet covered:**
- Unit-faction-template changes for *non-player* units (mind control,
  charm, server-side faction scripts). Modern WoW fires
  `UNIT_FACTION` on the affected unit token; the vanilla broad helper
  isn't hooked here yet. Mostly affects nameplate / threat addons in
  rare encounter mechanics.

**Implementation notes**

`FactionToggleAtWar` / `SetFactionInactive` / `SetFactionActive`'s
inner setters (`FUN_004D5FD0`, `FUN_004D60F0`) write to the rep slot's
flag byte and send the corresponding CMSG, but never call the
engine's "fire UNIT_FACTION on this unit" dispatcher. 3.3.5 added a
`FUN_0071F8F0(player, 0)` call to both paths whose inner
`FUN_0060BF10(playerGUID, UNIT_FACTION_id)` broadcasts the event for
every unit token referencing the local player.

Since `FactionToggleAtWar` only ever runs for the player and the only
unit token resolving to the player is `"player"`, we fire
`UNIT_FACTION("player")` directly via the engine's printf-style
dispatcher (`FUN_FIRE_EVENT`) — same observable result without
re-deriving the vanilla broad helper. We resolve `UNIT_FACTION`'s
event-table slot lazily by name (`Event::Custom::LookupByName`) so we
stay correct against any DLL combination that reshuffles the table.

For the two SMSG-bound flag-flip paths (`SetFactionStandings`,
`SetFactionInactive`-but-from-server), we snapshot the 64-entry rep
slot flag-byte array before the original handler runs and compare
after — single fire if any byte changed. The bulk snapshot is 64
byte reads (cheap) and naturally handles the case where one packet
carries multiple rep updates whose threshold crossings ripple AT_WAR
state across unrelated slots.

### `UPDATE_SHAPESHIFT_FORM` event

Fires whenever the local player's shapeshift form changes — entering
or leaving cat / bear / travel / stance / shadowform / stealth /
ghost wolf / etc. No payload; call
[`GetShapeshiftFormID()`](#getshapeshiftformid) from the handler to
read the new form.

Polyfills the modern singular event. Vanilla 1.12 has only the plural
`UPDATE_SHAPESHIFT_FORMS` (fires when the *list* of available forms
changes — learning a new form, not changing into one). The singular
"current form changed" event was added in a later expansion.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("UPDATE_SHAPESHIFT_FORM")
f:SetScript("OnEvent", function()
    if event == "UPDATE_SHAPESHIFT_FORM" then
        print("now in form", GetShapeshiftFormID())
    end
end)
```

**Implementation notes**

Hooks the engine's bonus-action-bar refresh helper at `0x004E4FC0`,
which the engine calls every time the local player's `UNIT_BYTES_1`
descriptor field is broadcast and from `FUN_004908C0` at post-login
init. After the original runs, the post-hook reads byte 2 of
`UNIT_BYTES_1` (descriptor `+0x212` — the form ID) and fires
`UPDATE_SHAPESHIFT_FORM` only when the value differs from the last
seen — filtering out the spurious recomputes for other `UNIT_BYTES_1`
bytes (standstate, etc.) the engine also routes through this helper.

The cached "last form" sentinel uses `-1` for "player descriptor not
yet resolvable" so transient resolution failures during early login
don't get misread as leaving a form.

## Expansion

Helpers shipped by modern Classic Era / Cata Classic for addons that
want to gate code on which expansion the client targets. We always
answer as `LE_EXPANSION_CLASSIC` (`0`) — the DLL is built against
1.12 offsets, so there's nothing to detect. The matching number
constants (`LE_EXPANSION_*`) are in the [Globals section](#le_expansion_).

### `GetClassicExpansionLevel()`

Returns the live expansion level as a number. Always `0`
(`LE_EXPANSION_CLASSIC`) here.

```lua
if GetClassicExpansionLevel() >= LE_EXPANSION_BURNING_CRUSADE then
    -- never taken on 1.12
end
```

### `ClassicExpansionAtLeast(expansionLevel)`

Returns `true` iff `GetClassicExpansionLevel() >= expansionLevel`.
On 1.12 that reduces to `expansionLevel <= 0`, so only
`ClassicExpansionAtLeast(LE_EXPANSION_CLASSIC)` (and any negative
argument) are true; every later expansion answers `false`.

Errors if `expansionLevel` is missing or non-numeric — matches the
modern signature.

```lua
if ClassicExpansionAtLeast(LE_EXPANSION_WRATH_OF_THE_LICH_KING) then
    -- WotLK+ code path; never taken on 1.12
end
```

### `ClassicExpansionAtMost(expansionLevel)`

Returns `true` iff `GetClassicExpansionLevel() <= expansionLevel`.
On 1.12 that reduces to `expansionLevel >= 0`, so the only `false`
answer is for negative input.

Errors if `expansionLevel` is missing or non-numeric.

```lua
if ClassicExpansionAtMost(LE_EXPANSION_CLASSIC) then
    -- vanilla / Classic Era only code path
end
```

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

## Focus

Polyfills modern WoW's focus-target system: a single sticky GUID
that addons can pin to a unit and address via the `"focus"` unit
token across the entire `UnitX` API surface. Backed by a hook on
`FUN_TOKEN_TO_GUID` (shared with the [`nameplateN`](#unit-tokens-nameplaten)
tokens — see `unit/TokenExtensions.cpp`).

State is **session-only** — drops on `/reload`, `/logout`, and zone
loads that recreate Lua state. Modern Classic Era behaves the same;
addons that want persistence have to re-`FocusUnit` from `SavedVariables`
at `ADDON_LOADED`.

**Auto-clear on despawn.** When the focused unit leaves the client's
object table — out of rendering range, full despawn, dies and
decays — focus drops and [`PLAYER_FOCUS_CHANGED`](#player_focus_changed-event)
fires. The unit re-entering range does NOT auto-refocus. Matches
modern WoW's documented behavior. Implementation: per-tick
`ObjectByGUID(g_focusGUID)` probe; on null result, `Set(0)`.

### `FocusUnit(unit)`

Sets focus to the given unit. Argument is a unit token —
`"target"`, `"mouseover"`, `"party1"`, `"nameplate1"`, anything the
resolver accepts. Calling with no argument is shorthand for
`FocusUnit("target")` (matches modern's `/focus` slash command
default).

```lua
FocusUnit("target")        -- pin the current target
FocusUnit("nameplate1")    -- pin the unit behind nameplate1
FocusUnit()                -- same as FocusUnit("target")
```

Fires [`PLAYER_FOCUS_CHANGED`](#player_focus_changed-event) if the
resolved GUID differs from the current focus. No-op (no event) if
the same unit is already focused — matches the
identity-check-first behavior of 3.3.5's `FUN_0051FF20`.

Passing a token that doesn't resolve to a unit (e.g. `"target"`
with nothing targeted, or `"party5"` solo) clears focus — same as
calling `ClearFocus()`.

### `ClearFocus()`

Drops the focus. Fires `PLAYER_FOCUS_CHANGED` if there was one;
no-op otherwise.

```lua
ClearFocus()
```

### Unit token (`focus` / `focustarget`)

`"focus"` resolves to whatever GUID `FocusUnit` last stashed.
Accepted by every `UnitX` function:

```lua
local name = UnitName("focus")             -- nil if no focus
local hp   = UnitHealth("focus")
local _, class = UnitClass("focus")
```

Chains compose through the engine's standard suffix walker:
`"focustarget"`, `"focustargettarget"`, etc. — same behavior as
`"targettarget"`, mirrored instruction-for-instruction in our hook
so addons get the engine semantics they expect.

Returns `nil` cleanly when no focus is set; doesn't raise the
"Unknown unit name" error.

[`UnitTokenFromGUID`](#unittokenfromguidguid) scans `"focus"` right
after `"target"` (matching retail order), so a focused unit's GUID
reverse-resolves to `"focus"` only if it isn't already addressable
as `"player"` / `"party*"` / `"raid*"` / `"nameplate*"` / `"target"`.

### Bindings (`FOCUSTARGET` / `TARGETFOCUS`)

Two key bindings appear in the keybind UI (`Esc` → Key Bindings)
under the **Targeting Functions** group, between `PETATTACK` and the
Interface section:

| Binding | Lua action | Use |
|---------|------------|-----|
| `FOCUSTARGET` | `FocusUnit("target")` | Pin current target as focus |
| `TARGETFOCUS` | `TargetUnit("focus")` | Switch target to the focus |

No default key — assign one in the keybind UI like any other binding.
The two together give you a quick "set focus → swap to focus" loop
common in PvP / heal-tank-while-killing-add scenarios.

Implementation note: addon-side `Bindings.xml` files always render at
the bottom of the keybind list (orphaned from any `header="..."` they
declare) because the engine's binding registry is linearly indexed by
insertion order and addons load after FrameXML. To land inside the
TARGETING block, the DLL splices the two `<Binding>` entries into the
engine's `Interface\FrameXML\Bindings.xml` at file-read time via a
hook on `FUN_FILE_READ` — see [`src/bindings/Inject.cpp`](../src/bindings/Inject.cpp).

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

## GameObject

The gameobject analog of [`C_CreatureInfo`](#creature) — reads the
client-side gameobject cache (`gameobjectcache.wdb`, fed by
`SMSG_GAMEOBJECT_QUERY_RESPONSE`) by GO entry ID.

### `C_GameObjectInfo.GetGameObjectInfoByID(gameObjectID)`

Synchronous **peek** — returns a table for a cached gameobject, `nil`
otherwise:

```lua
local info = C_GameObjectInfo.GetGameObjectInfoByID(47297)
-- { gameObjectID = 47297, name = "Mesa Elevator", type = 11, displayID = 360 }
```

`type` is the `GameObjectType` (0=door, 1=button, 3=chest, 5=generic,
6=trap, 10=goober, …). Field offsets verified against the binary
(name `[block+0x08]`) and real `gameobjectcache.wdb` rows (Mesa
Elevator 47297→type 11; Windrunner 176250→type 15 displayID 7087).

### `C_GameObjectInfo.RequestLoadGameObjectByID(gameObjectID)`

Asynchronously fetches an uncached gameobject — issues
`SMSG_GAMEOBJECT_QUERY` and fires **`GAMEOBJECT_DATA_LOAD_RESULT(
gameObjectID, success)`** when it lands, after which
`GetGameObjectInfoByID` returns its data. Returns `true` if accepted;
fires synchronously on a cache hit. Same shape as
[`C_CreatureInfo.RequestLoadCreatureByID`](#c_creatureinforequestloadcreaturebyidcreatureid),
and rides the same shared `Cache::QueryLoad` dispatcher (the gameobject
cache is a sibling class of the creature cache with its own
`_GetRecord`/parser, hooked separately).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("GAMEOBJECT_DATA_LOAD_RESULT")
f:SetScript("OnEvent", function()  -- arg1 = gameObjectID, arg2 = success
    if arg2 then local info = C_GameObjectInfo.GetGameObjectInfoByID(arg1) end
end)
C_GameObjectInfo.RequestLoadGameObjectByID(7000032)
```

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

### `GameTooltip:SetItemByGUID(itemGUID)`

Renders the tooltip for a specific item *instance* identified by
GUID — same string `C_Item.GetItemGUID` returns
(`"0xHHHHHHHHLLLLLLLL"`). Distinct from `SetItemByID`: this path
goes through the live CGItem, so the tooltip includes enchant
lines, random-suffix-decorated name + bonuses, and locked/broken
state — the same depth `SetBagItem` / `SetInventoryItem` would
produce for that specific copy.

```lua
local guid = C_Item.GetItemGUID({ equipmentSlotIndex = 10 }) -- legs
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
GameTooltip:SetItemByGUID(guid)
GameTooltip:Show()
```

Resolves through the engine's own `FUN_OBJECT_RESOLVE_BY_GUID`
(same path `C_Item.GetItemLocation` uses), builds the full
`item:N:enchant:gem:gem:gem:gem:suffix:unique`-style hyperlink via
the engine's link builder, then dispatches to
`Script_GameTooltip_SetHyperlink`. The link round-trip is what
gives `GameTooltip:GetItem()` the dressed link back when called
after — same shape `GetInventoryItemLink` / `GetContainerItemLink`
emit for the same slot.

Silent no-op when the GUID is malformed, the item isn't currently
loaded in the client (e.g. items only the server knows about), or
the GUID resolves to a non-item object (creature/gameobject GUIDs
go to a different resolver and won't match).

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

### `GameTooltip:GetUnitGUID()` / `GameTooltip:HasUnit()`

`GetUnitGUID()` returns `(name, guidString)` for whichever unit the
tooltip is currently displaying, or nothing if it isn't showing a
unit. Return order mirrors modern's `GameTooltip:GetUnit()` (name
first) — so addons porting from
`local name, unit = ttip:GetUnit()` can swap to `GetUnitGUID` and
keep their existing destructuring. `name` is the unit's display name
— the same string that appears in the tooltip header, or one of the
engine's `"UNKNOWNOBJECT"` / `"Unknown Being"` fallbacks for a remote
unit whose info hasn't been queried yet. `guidString` is the
canonical `"0xHHHHHHHHLLLLLLLL"` format returned by
[`UnitGUID(unit)`](#unitguidunit).

`HasUnit()` is a boolean companion — returns `true` if the tooltip is
currently displaying a unit.

```lua
GameTooltip:SetUnit("target")
local name, guid = GameTooltip:GetUnitGUID()
-- name = "Hogger"
-- guid = "0xF130001234..." (Creature) or "0x000000...ABC123" (Player)

if GameTooltip:HasUnit() then
    -- cheap predicate, no name-resolution work
end
```

> **Why not match modern's `GetUnit()` signature?** Modern WoW's
> `GetUnit()` returns `(name, unitToken)` where `unitToken` is the
> exact `"target"` / `"focus"` / `"mouseover"` / etc. string passed
> to `SetUnit`. Vanilla 1.12 drops the token at the
> `Script_GameTooltip_SetUnit` boundary — it converts the token to a
> 64-bit GUID and discards the original string. Reconstructing a
> plausible token by walking known tokens and reverse-matching by
> GUID is possible but lossy (multiple tokens can refer to the same
> GUID — `"target"` and `"raid1"` simultaneously, for instance), so
> we expose the GUID directly instead, which is what addons actually
> need for cross-referencing with `UnitGUID`, the NameCache, etc.

### `GameTooltip:GetGameObject()` / `GameTooltip:HasGameObject()`

`GetGameObject()` returns `(name, id, guid)` for whichever gameobject
the tooltip is currently displaying, or nothing if it isn't showing
one. `name` is the cached display name (or `""` until the gameobject
cache has loaded the record). `id` is the gameobject's template /
entry — same key `gameobjectcache.wdb` uses, and what the server
sent in the spawn packet. `guid` is the canonical
`"0xHHHHHHHHLLLLLLLL"` format.

`HasGameObject()` is the boolean companion — returns `true` if the
tooltip is currently displaying a gameobject (single `uint32` read,
no resolution work).

```lua
-- Mouse over a chest, vein, signpost, etc. then:
local name, id, guid = GameTooltip:GetGameObject()
-- name = "Iron Deposit"
-- id   = 1731
-- guid = "0xF110000006C30000..."

if GameTooltip:HasGameObject() then
    -- cheap predicate
end
```

There is **no Lua-callable `SetGameObject` method** in vanilla 1.12
— gameobjects only populate the tooltip via in-world mouseover. The
engine's hover handler at `FUN_00492890` dispatches to a tooltip
populator (`0x0052AA20`) that writes the GUID into
`tooltip+0x370/+0x374`, right between the unit slot (`+0x368`) and
the item slot (`+0x380`). The shared tooltip-clear zeroes the same
slot on every subsequent `SetX`, so these methods follow the same
gating pattern as `HasUnit` / `HasSpell` / `HasItem`.

Returns nothing for: tooltip not showing a gameobject, or the
gameobject has left the engine's visibility window (object manager
evicted it). `HasGameObject()` stays true in the latter case —
the tooltip-frame slot still has the GUID, even though the live
object has been freed.

### `GameTooltip:GetOwner()`

Returns the frame that called `tooltip:SetOwner(frame, anchor)`, or
`nil` if the tooltip hasn't been owned by anyone since its last
`Clear` / `Hide`. Vanilla 1.12 ships `SetOwner` and `IsOwned` but
never added the matching reader; modern Classic Era's signature is
backported here for parity.

```lua
GameTooltip:SetOwner(SomeFrame, "ANCHOR_TOPLEFT")
local owner = GameTooltip:GetOwner()
-- owner == SomeFrame
```

Returns only the owner frame, not `(owner, anchorPoint)` like modern
Classic. The anchor string is reachable via vanilla's native
[`GameTooltip:GetAnchorType()`](https://wowwiki-archive.fandom.com/wiki/API_GameTooltip_GetAnchorType)
(slot 5 in the GameTooltip method registry).

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

### `GameTooltip:SetHyperlinkCompareItem("itemLink" [, offset, shiftButton, comparisonTooltip])`

Fills the tooltip with the item **currently equipped** in the slot the
given item would occupy — a grey `Currently Equipped` header, the
equipped item's own tooltip, then (when `shiftButton` is set) a
green/red per-stat delta breakdown of how the given item compares.
This is the method behind item-comparison ("shopping") tooltips;
`FrameXML`'s `GameTooltip_ShowCompareItem` drives it.

Call it on the tooltip that should show the comparison (typically
`ShoppingTooltip1`/`ShoppingTooltip2`, but any `GameTooltipTemplate`
frame works).

| Arg | Meaning |
|-----|---------|
| `itemLink` | The item being compared (chat link, `item:` string, or bare itemID). Optional if `comparisonTooltip` is given. |
| `offset` | 1-based slot selector for two-slot items — rings, trinkets, and one-hand weapons expose `offset` 1 and 2 (Finger1/2, Trinket1/2, MainHand/OffHand). Default 1. |
| `shiftButton` | Gates the stat-change breakdown. Deltas show by default (`true`/`1`/`nil`/omitted) and are hidden only for an explicit `false`/`0` (header + equipped item only). Both the boolean and vanilla `1`/`nil` conventions are accepted. |
| `comparisonTooltip` | Optional. When `itemLink` is omitted, the compared item is taken from whatever this tooltip is displaying (matches the retail tooltip-to-tooltip call). |

**Returns** the number of comparison slots for the item (1, or 2 for
rings/trinkets/one-hand weapons; 0 if it isn't equippable, isn't
cached, or nothing is equipped in the chosen slot) — so a caller knows
whether a second `offset` is worth querying.

The stat deltas cover the same keys as
[`C_Item.GetItemStats`](#c_itemgetitemstatsitemlink) (base stats,
resistances, weapon DPS, and on-equip-spell bonuses like crit / attack
power / spell power), colored with the client's
`INCREASE_STAT_COLOR` / `DECREASE_STAT_COLOR`. The method only
populates the tooltip; the caller shows/anchors it (as retail's
`SetHyperlinkCompareItem` does).

```lua
-- Compare a hovered bag item against what's equipped:
local link = GetContainerItemLink(bag, slot)
ShoppingTooltip1:SetOwner(GameTooltip, "ANCHOR_NONE")
local slots = ShoppingTooltip1:SetHyperlinkCompareItem(link, 1, true)
ShoppingTooltip1:Show()
if slots == 2 then  -- ring/trinket/1H: also compare the other slot
    ShoppingTooltip2:SetOwner(GameTooltip, "ANCHOR_NONE")
    ShoppingTooltip2:SetHyperlinkCompareItem(link, 2, true)
    ShoppingTooltip2:Show()
end
```

### `GameTooltip:IsEquippedItem()`

Returns `true` when the item the tooltip is **currently displaying** is
equipped in one of the player's character-pane slots (1..19). Backports
the 3.3.5 `GameTooltip:IsEquippedItem` method.

Works regardless of how the tooltip was populated — link paths
(`SetItemByID`, `SetHyperlink`) and CGItem paths (`SetInventoryItem`,
`SetBagItem`, `SetItemByGUID`, …) both resolve to the displayed item.
Returns `false` for a tooltip that isn't showing an item (e.g. a spell
tooltip) with no false positives.

```lua
GameTooltip:SetInventoryItem("player", INVSLOT_HAND)
if GameTooltip:IsEquippedItem() then ... end  -- true

GameTooltip:SetBagItem(0, 1)
if GameTooltip:IsEquippedItem() then ... end  -- false unless that item is also worn
```

### `OnTooltipSetItem` script

A real frame script — settable with the standard `SetScript` / `GetScript` /
`HookScript` — that fires whenever a tooltip's **item** is set. Backports the
modern tooltip script so addons annotate item tooltips by hooking one script
instead of wrapping every `Set*` method.

Fires for every item-setting path (`SetBagItem`, `SetInventoryItem`,
`SetHyperlink` for an `item:` link, `SetMerchantItem`, `SetAuctionItem`,
`SetItemByID`, …). Available on all GameTooltip-type frames (`GameTooltip`,
`ItemRefTooltip`, `ShoppingTooltip1/2`, `AtlasLootTooltip`, …).

The handler receives the tooltip as the **global `this`**, the 1.12
frame-script convention — *not* a `self` argument (like every built-in vanilla
script: `OnShow`, `OnEvent`, `OnTooltipCleared`, …). Modern-style
`function(self) self:… end` handlers will see `self == nil`; use `this`.

```lua
GameTooltip:HookScript("OnTooltipSetItem", function()
    local name, link, id = this:GetItem()   -- the tooltip is the global `this`
    if id then
        this:AddLine("ID: " .. id, 0.6, 0.6, 0.6)
    end
end)
```

Vanilla only shipped `OnTooltipAddMoney` / `OnTooltipCleared` /
`OnTooltipSetDefaultAnchor`; `OnTooltipSetSpell` / `OnTooltipSetUnit` are not
yet provided.

**Caveat — the event fires from inside the item-tooltip build.** Lightweight
handler work is fine: reading `this:GetItem()`, `this:AddLine(...)`, printing,
etc. But avoid *re-entrant* tooltip rebuilds from the handler — e.g.
`GameTooltip_ShowCompareItem()`, or a `Set*` call on another tooltip. Because
the handler runs mid-build, re-entering the tooltip / FrameScript machinery can
collide with other DLLs that hook the same Lua paths (nampower, SuperWoW,
weirdutils, …) and crash the client. If you need heavy work like that, defer it
to the next frame:

```lua
GameTooltip:HookScript("OnTooltipSetItem", function()
    C_Timer.After(0, function() GameTooltip_ShowCompareItem() end)
end)
```

### `GameTooltip:SetEquipmentSet(name)`

Fills the tooltip with a summary of the named equipment set: header,
total item count, and per-bucket counts (equipped / in inventory /
ignored / missing). Each missing item is listed on its own line by
name. Mirrors 4.3.4's native `GameTooltip:SetEquipmentSet` at
`0x0046E690`.

```lua
GameTooltip:SetOwner(UIParent, "ANCHOR_CURSOR")
GameTooltip:SetEquipmentSet("MyTank")
GameTooltip:Show()
```

```
MyTank                     (header, white)
14 items
12 equipped                (green)
1 in inventory
1 slots ignored            (gray)
Missing: Crown of the Endless Conqueror   (red)
```

Silent no-op if no set with that name exists. The owner anchor must
be set before the call — `SetEquipmentSet` only fills the lines, it
doesn't re-anchor the tooltip frame.

Item classification reuses `Locations::FindGUID` (the same walk
`GetItemCount` uses), so items in the bank count as "in inventory"
without requiring the bank window to be open. The 4.3.4 binary did
the same.

**Localization.** The count lines are formatted through
[`Game::Lua::PushLocalizedFormatInt`](#) — Blizzard's `ITEMS_VARIABLE_QUANTITY`,
`ITEMS_EQUIPPED`, `ITEMS_IN_INVENTORY`, `ITEM_SLOTS_IGNORED`, and
`ITEM_MISSING` FrameXML globals are tried first, with English C-string
fallbacks for servers stripped of standard GlobalStrings. The
companion `!!!ClassicAPI` addon ships English defaults for these keys
so the tooltip works on bare-bones builds where FrameXML hasn't
populated them.

**Missing-item names.** The on-disk equipment-set format
(`WTF\Account\<acct>\<realm>\<char>\ClassicAPI_EquipmentSets.txt`)
persists each slot as both a 64-bit GUID and the itemID at save
time:

```
set 1
  name=MyTank
  icon=INV_Helmet_03
  slot 1 guid=0xC00000000A1B2C3D item=51220
  slot 5 ignored
```

The GUID is the live handle for items currently in inventory; the
itemID is the type identifier used when the live `CGItem` isn't
findable (item sold, mailed, on another character, etc.) — without
it we couldn't recover the name for missing items. Sets saved
before the itemID field was added load fine but render their
missing slots with the count summary `%d missing` instead of named
lines; re-saving a set repopulates itemIDs.

## Globals

### `CLASSIC_API_VERSION`

Defined once FrameScript has booted. Addons can use this to detect
that the DLL is loaded and which version is in use. The value is
`X*10000 + Y*100 + Z` for a tag of `vX.Y.Z` passed to CMake at
configure time via `-DCLASSICAPI_TAG=vX.Y.Z`.

Untagged builds (local dev, CI without a release tag) get a
**sentinel value of `99999999`** — encoded as `v9999.99.99`, chosen
to be higher than every plausible real release so addon-side
feature gates like `CLASSIC_API_VERSION >= 10200` don't reject the
dev build.

```lua
if CLASSIC_API_VERSION and CLASSIC_API_VERSION >= 10200 then
    -- ClassicAPI v1.2.0 or newer is loaded (or untagged dev build)
end
```

If you specifically want to detect a dev build:

```lua
if CLASSIC_API_VERSION == 99999999 then
    -- running against a locally-built, untagged DLL
end
```

### `LE_EXPANSION_*`

The retail / Classic Era expansion-level enum, exposed as Lua globals
so addons backporting from later expansions don't have to gate on
`if LE_EXPANSION_CLASSIC then` (the constant being defined is itself
the version probe). Values match the modern `Enum.ExpansionLevel`
table. The matching helper functions
(`GetClassicExpansionLevel` / `ClassicExpansionAtLeast` /
`ClassicExpansionAtMost`) live in the [Expansion section](#expansion).

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

### `Enum.InventoryType`

The equip-type enum — the numeric `inventoryType` reported by
[`C_Item.GetItemInfoInstant`](#c_itemgetiteminfoinstantitem) /
[`C_Item.GetItemInventoryType`](#c_itemgetiteminventorytypeitemlocation--c_itemgetiteminventorytypebyiditem),
and the argument the `C_Item.GetItemInventorySlot*` functions take.

| Value | Field | Value | Field |
|------:|-------|------:|-------|
| 0 | `IndexNonEquipType` | 15 | `IndexRangedType` |
| 1 | `IndexHeadType` | 16 | `IndexCloakType` |
| 2 | `IndexNeckType` | 17 | `Index2HweaponType` |
| 3 | `IndexShoulderType` | 18 | `IndexBagType` |
| 4 | `IndexBodyType` | 19 | `IndexTabardType` |
| 5 | `IndexChestType` | 20 | `IndexRobeType` |
| 6 | `IndexWaistType` | 21 | `IndexWeaponmainhandType` |
| 7 | `IndexLegsType` | 22 | `IndexWeaponoffhandType` |
| 8 | `IndexFeetType` | 23 | `IndexHoldableType` |
| 9 | `IndexWristType` | 24 | `IndexAmmoType` |
| 10 | `IndexHandType` | 25 | `IndexThrownType` |
| 11 | `IndexFingerType` | 26 | `IndexRangedrightType` |
| 12 | `IndexTrinketType` | 27 | `IndexQuiverType` |
| 13 | `IndexWeaponType` | 28 | `IndexRelicType` |
| 14 | `IndexShieldType` | | |

Values `29..34` (`IndexProfessionToolType`, `IndexProfessionGearType`,
`IndexEquipablespell{Offensive,Utility,Defensive,Weapon}Type`) are
post-vanilla and included for parity — vanilla items never report them.

```lua
if C_Item.GetItemInventoryType(loc) == Enum.InventoryType.IndexHeadType then ...
```

### `Enum.PowerType`

The integer enum `UnitPowerType` returns and `UnitPower` /
`UnitPowerMax` accept. Vanilla 1.12 only defines slots 0..4 — the
WotLK additions (Runes, Runic Power) and post-WotLK extensions
aren't included. Slot 4 is `Happiness` (vanilla pet happiness),
not modern's `ComboPoints` reuse of the same number.

| Value | Field        | Notes |
|------:|--------------|-------|
| `-2`  | `HealthCost` | Sentinel for "use HEALTH instead of POWER". |
| `-1`  | `None`       | Sentinel for "unit's primary power" (= omit the arg). |
| `0`   | `Mana`       | |
| `1`   | `Rage`       | |
| `2`   | `Focus`      | |
| `3`   | `Energy`     | |
| `4`   | `Happiness`  | Pet happiness — vanilla-specific. |

```lua
local rage = UnitPower("player", Enum.PowerType.Rage)
```

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
2.0 for protected-frame manipulation. Vanilla 1.12 has no taint
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

### `IsMouseButtonDown([button])`

Returns `true` if the given mouse button is currently held, `false`
otherwise. With no argument (or `nil`), returns `true` if **any**
mouse button is held.

`button` is either a 1-based ID or a name string:

| ID  | Name |
|-----|------|
| `1` | `"LeftButton"` |
| `2` | `"RightButton"` |
| `3` | `"MiddleButton"` |
| `4` | `"Button4"` (XBUTTON1 / first side button) |
| `5` | `"Button5"` (XBUTTON2 / second side button) |

Unrecognized IDs / names return `false` (matches modern semantics:
the named button just isn't held — bad input doesn't fall through
to the any-button check).

State is maintained from the same `WH_GETMESSAGE` hook that drives
`GLOBAL_MOUSE_DOWN` / `GLOBAL_MOUSE_UP`, so press/release transitions
register at exactly the moment the corresponding event fires. Hook
scopes to WoW's message queue — clicks made while the game is
alt-tabbed out don't deliver, which means the state can stay
"down" if a button was held while focus left the window. Most uses
of `IsMouseButtonDown` are gated behind a frame that just received
mouse focus, so this rarely matters in practice.

```lua
if IsMouseButtonDown("RightButton") then
    -- right-button-held bindings
end
```

## Instance

### `GetInstanceInfo()`

Returns the same 9-value tuple modern WoW does (TBC and later), with
vanilla-degenerate values for the fields the 1.12 client doesn't
actually track:

```
name, instanceType, difficultyID, difficultyName, maxPlayers,
dynamicDifficulty, isDynamic, instanceID, instanceGroupSize
```

- `name` — localized instance/zone name from `Map.dbc`.
- `instanceType` — `"none"` (open world), `"party"` (5-man dungeon),
  `"raid"`, `"pvp"` (battleground), or `"arena"` (unused in vanilla).
- `difficultyID` — always `1`. No heroic mode pre-TBC.
- `difficultyName` — always `"Normal"`.
- `maxPlayers` — type-default cap: `5` for dungeons, `40` for raids,
  `40` for battlegrounds, `0` for open world. **See caveat below.**
- `dynamicDifficulty` — always `0` (dynamic difficulty was a Cataclysm
  addition).
- `isDynamic` — always `false`.
- `instanceID` — current map ID. Modern API calls this "instanceID" but
  it's really the `Map.dbc` row ID — both vanilla and modern WoW put
  the same value here.
- `instanceGroupSize` — mirrors `maxPlayers` (no per-group-config
  variants in vanilla).

```lua
/dump GetInstanceInfo()
-- In Stormwind:    "Kalimdor",   "none", 1, "Normal",  0, 0, false,   1,  0
-- In Deadmines:    "Deadmines",  "party",1, "Normal",  5, 0, false,  36,  5
-- In Molten Core:  "Molten Core","raid", 1, "Normal", 40, 0, false, 409, 40
```

**Caveat on `maxPlayers`.** Vanilla genuinely has no per-instance cap
data client-side — `MapDifficulty.dbc` was a TBC addition. The server
enforces caps via `SMSG_TRANSFER_ABORTED` when entry is denied, but
that information never reaches the client otherwise. So we return the
type's canonical max. **Zul'Gurub and AQ20 return `40` instead of
their true `20`; non-AV battlegrounds (WSG `10`, AB `15`) return `40`
instead of their true cap; any custom raid on a private server
(e.g. Turtle WoW) returns `40` regardless of its real cap.** Addons
that need exact caps must supply their own per-mapID table.

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

### `C_Item.GetItemData(itemLocation)` / `C_Item.GetItemDataByID(item)`

ClassicAPI-only kitchen-sink reader: returns a single table with every
field we can extract from the cached `ItemStats_C` record, so addons
that need more than a couple of fields don't have to chain a dozen
`C_Item.*` calls. Saves both stack churn and the cost of redoing the
cache lookup for every accessor.

```
data = C_Item.GetItemDataByID(item)        -- numeric id / "item:NNN" / chat link
data = C_Item.GetItemData(itemLocation)    -- {bagID,slotIndex} or {equipmentSlotIndex}
```

Returns `nil` if the input doesn't resolve to a valid itemID or the
item isn't cached yet. Does **not** warm the cache — same passive-
reader contract as
[`C_Item.GetItemInfoInstant`](#c_itemgetiteminfoinstantitem).
Callers that need to wait for cache fill should call
[`C_Item.RequestLoadItemDataByID`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation)
and listen for `ITEM_DATA_LOAD_RESULT`.

Returned table:

| Key | Type | Source / notes |
|-----|------|---------------|
| `itemID` | number | Echoed from input. |
| `name` | string | `m_name[0]` — localized display name. |
| `description` | string \| nil | `m_description`. Omitted when empty. |
| `icon` | string \| nil | `"Interface\\Icons\\<name>"`. Omitted when no icon. |
| `displayInfoID` | number | `m_displayInfoID` → `ItemDisplayInfo.dbc` row. |
| `quality` | number | 0..5 — see `LE_ITEM_QUALITY_*` enum. |
| `classID` | number | `m_class` — `ItemClass.dbc` row. |
| `subclassID` | number | `m_subClass` — `ItemSubClass.dbc` row. |
| `className` | string | Localized class name (e.g. `"Weapon"`). |
| `subclassName` | string | Localized subclass name (e.g. `"One-Handed Swords"`). |
| `inventoryType` | number | `m_inventoryType` — raw integer. |
| `equipLoc` | string | `"INVTYPE_*"` constant or `""` for non-equippable. |
| `bindType` | number | `m_bonding` — 0=none, 1=BoP, 2=BoE, 3=BoU, 4=Quest. |
| `flags` | number | Raw `m_flags` u32. |
| `isConjured` | bool | Flag bit `0x2`. |
| `isOpenable` | bool | Flag bit `0x4`. Same bit `C_Item.IsItemOpenable` reads. |
| `isLootable` | bool | Flag bit `0x10` — has loot generators (right-click loot). |
| `isWrapper` | bool | Flag bit `0x200` — gift-wrappable. |
| `maxStackSize` | number | `m_stackable`. |
| `maxCount` | number | `m_maxCount` — 0=unlimited, 1=unique, otherwise per-character cap. |
| `containerSlots` | number | `m_containerSlots` — bag slot count, 0 for non-bags. |
| `bagFamily` | number | `m_bagFamily` converted to modern bitmask (`1 << (id-1)`). |
| `buyPrice` | number | Vendor buy price in copper. |
| `sellPrice` | number | Vendor sell price in copper. |
| `itemLevel` | number | Base ilvl from `ItemSparse`. |
| `requiredLevel` | number | Minimum character level. |
| `requiredSkill` | number | `SkillLine.dbc` row, 0=none. |
| `requiredSkillRank` | number | Skill rank required. |
| `requiredSpell` | number | `Spell.dbc` row required to learn / use, 0=none. |
| `requiredHonorRank` | number | PvP honor rank required, 0=none. |
| `requiredCityRank` | number | Reserved; 0 for vanilla. |
| `requiredFaction` | number | `Faction.dbc` row, 0=none. |
| `requiredFactionRank` | number | Reputation tier (Friendly/Honored/…). |
| `allowableClass` | number | Class bitmask, `-1` = all classes. |
| `allowableRace` | number | Race bitmask, `-1` = all races. |
| `armor` | number | Armor value. |
| `block` | number | Shield block value. |
| `maxDurability` | number | 0 for items without durability. |
| `stats` | table | Sparse `{ [statType] = statValue, … }`. Empty for items without stat allocations. |
| `resistanceHoly` | number | Holy resistance. |
| `resistanceFire` | number | Fire resistance. |
| `resistanceNature` | number | Nature resistance. |
| `resistanceFrost` | number | Frost resistance. |
| `resistanceShadow` | number | Shadow resistance. |
| `resistanceArcane` | number | Arcane resistance. |
| `damageMin` | table | 5-element float array — weapon min damage per damage slot. |
| `damageMax` | table | 5-element float array. |
| `damageType` | table | 5-element integer array — damage school per slot. |
| `delay` | number | Weapon swing time in ms. |
| `ammoType` | number | Ammo subclass for ranged weapons. |
| `rangedModRange` | number | Range modifier for bows/guns. |
| `spells` | table | Sparse array of `{id, trigger, charges, cooldown, category, categoryCooldown}` records — one per non-empty spell slot (up to 5). |
| `useSpellID` | number \| nil | Convenience — the spellID of the first slot with trigger=ON_USE (0). Omitted when the item has none. Same value [`C_Item.GetItemSpell`](#c_itemgetitemspellitem) returns. |
| `lockID` | number | `Lock.dbc` row for the item's lock, 0=none. |
| `itemSet` | number | `ItemSet.dbc` row, 0=not part of a set. |
| `pageText` | number | `PageText.dbc` row for readable items. |
| `pageMaterial` | number | Book page material. |
| `languageID` | number | Language for in-game-readable books. |
| `startQuest` | number | `Quest` row started by right-clicking this item. |
| `material` | number | Material type (cloth/leather/metal etc. for hit sounds). |
| `sheath` | number | Weapon sheath style. |
| `randomProperty` | number | Random property template id. |
| `area` | number | `AreaTable.dbc` — bound area for area-locked items, 0=none. |
| `map` | number | `Map.dbc` — bound map for map-locked items, 0=none. |

```lua
local data = C_Item.GetItemDataByID(6948)  -- Hearthstone
-- data.name == "Hearthstone"
-- data.quality == 1, data.classID == 15, data.subclassID == 0
-- data.bindType == 1   (BoP)
-- data.useSpellID == 8690   (the recall spell)
-- data.maxStackSize == 1, data.maxCount == 1

local sword = C_Item.GetItemDataByID(2092)  -- Worn Shortsword
-- sword.equipLoc == "INVTYPE_WEAPONMAINHAND"
-- sword.damageMin[1] == 1.0, sword.damageMax[1] == 2.0, sword.delay == 1900
-- sword.stats == {}   (no stat allocations on the starter sword)
```

**Cache state.** The function reads the same `DBCache_ItemStats`
network cache every other `C_Item.*` reader consults. Items already in
the player's inventory are typically populated by the engine's natural
prefetch before `PLAYER_ENTERING_WORLD` returns; for other items
(quest rewards, AH items, link hovers, etc.) the data lands once a
`SMSG_ITEM_QUERY_SINGLE_RESPONSE` arrives. Use
`C_Item.RequestLoadItemDataByID` if you specifically need to trigger
the load.

**Stats encoding.** The `stats` table is keyed by the **ItemModType**
enum (vanilla's encoding):

| Key | Meaning |
|-----|---------|
| 0 | Mana |
| 1 | Health |
| 3 | Agility |
| 4 | Strength |
| 5 | Intellect |
| 6 | Spirit |
| 7 | Stamina |

Higher-numbered slots (consumable per-second-regen, defense rating,
hit/crit/dodge/parry chance, weapon/spell-power) only appear on items
the server actually carries — vanilla 1.12 mostly uses 0..7. Empty
slots (both type and value zero) are omitted, so iterating `stats`
yields only the stats actually allocated.

No equivalent in modern WoW — `GetItemInfo` returns the 14-tuple
piecemeal and `C_Item.GetItemInfo` adds a few more. ClassicAPI's
`GetItemData` is a single-call superset useful for backporting tooltip
addons and item-data caches.

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

### `C_Item.GetItemInfoInstant(item)`

Modern-style accessor for the always-available subset of item info — the
fields that depend only on classification, not on player-specific state.
Synchronous, side-effect-free: peeks the client-side item cache and
returns whatever it has without warming or queueing.

Accepts a numeric `itemID` or a string containing `"item:NNN"` (matches both
the bare `"item:1234"` shorthand and full chat links like
`"|cff...|Hitem:1234:...|h[Name]|h|r"`). Item names are not accepted —
vanilla itself has no name → ID resolver, and it's rarely the form addon code
actually has on hand.

Returns seven values:

```
itemID, itemType, itemSubType, itemEquipLoc, icon, classID, subClassID
```

The `itemID` return is always populated for any input that resolves to
a positive integer (so it's safe to use the function as an ID extractor
on a link without first warming the cache). The remaining six fields
come from the cache record when present — when the item isn't cached
yet they're all `nil`, but the call still returns a 7-tuple so the
positional shape is preserved.

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
    = C_Item.GetItemInfoInstant(6948)  -- Hearthstone, cached
-- type="Miscellaneous", subtype="Junk", equipLoc="",
-- icon="Interface\\Icons\\INV_Misc_Rune_01", classID=15, subClassID=0

local id = C_Item.GetItemInfoInstant("|cff...|Hitem:6948:0:0:0|h[Hearthstone]|h|r")
-- id == 6948 even when the cache is cold — the call parses the link
-- to extract the ID without consulting the cache.
```

The actual class/subclass values reflect 1.12.1's data, which differs from
modern WoW. For example, vanilla had no Cloth subclass under Trade Goods —
Silk Cloth lives at `(7, 0)` in this client, not the modern `(7, 5)`.

> **No auto-warmup.** Unlike `GetItemInfo` or `C_Item.GetItemNameByID`,
> `GetItemInfoInstant` does not trigger a network query on a cache
> miss. The "Instant" name is contractual — callers that need the
> cache populated should use
> [`C_Item.RequestLoadItemDataByID`](#c_itemrequestloaditemdatabyiditem--c_itemrequestloaditemdataitemlocation)
> explicitly, or just call `GetItemInfo` (which warms via the
> `Script_GetItemInfo` hook).

### `C_Item.GetItemInventorySlotInfo(inventorySlot)`

Returns the localized display name for an `Enum.InventoryType` value — e.g.
`C_Item.GetItemInventorySlotInfo(1)` → `"Head"`. Returns `nil` for the
non-equip slot (`0`) or out-of-range values.

The engine's INVTYPE table stores the equipLoc *key* (`"INVTYPE_HEAD"`); the
display name is the FrameXML global string of that key (`INVTYPE_HEAD =
"Head"`), so the result is correctly localized on non-English clients.

### `C_Item.GetItemInventorySlotKey(inventorySlot)`

Returns the equipLoc key string for an `Enum.InventoryType` value — e.g.
`C_Item.GetItemInventorySlotKey(1)` → `"INVTYPE_HEAD"`. This is the same
`INVTYPE_*` token [`C_Item.GetItemInfoInstant`](#c_itemgetiteminfoinstantitem)
returns as its `itemEquipLoc`. Returns `nil` for the non-equip slot (`0`) or
out-of-range values. Pair it with `GetItemInventorySlotInfo` to turn the key
into a localized label.

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

### `C_Item.GetItemMaxStackSize(itemLocation)` / `C_Item.GetItemMaxStackSizeByID(item)`

Returns the item type's max stack size — what you'd find as the 7th
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

### `C_Item.GetItemSetID(itemLocation)` / `C_Item.GetItemSetIDByID(item)`

Returns the `ItemSet.dbc` ID for the item if it's part of an
equipment set (e.g., `181` for any Magister's Regalia piece), or
`nil` if the item isn't part of any set / the item isn't cached /
the slot is empty.

```lua
local setID = C_Item.GetItemSetIDByID(16682)   -- Magister's Regalia: 181
local setID = C_Item.GetItemSetIDByID(6948)    -- Hearthstone: nil

-- by itemLocation (worn or in-bag)
local chestSet = C_Item.GetItemSetID({ equipmentSlotIndex = 5 })
```

This is the 16th return of modern WoW's `GetItemInfo` (the `setID`
field). Single `uint32` read at cache record `+0x1C0`
(`m_itemSet`). Same cache-warm pattern as the other `*ByID` getters
— cache misses return `nil` and fire a background load; call
`C_Item.RequestLoadItemDataByID(itemID)` if you need synchronous-
after-event resolution.

### `C_Item.GetItemSetInfo(setID)`

Returns a table describing the item set, or `nil` if `setID`
doesn't resolve to an `ItemSet.dbc` row.

```lua
local info = C_Item.GetItemSetInfo(181)
-- {
--   setID = 181,
--   name = "Magister's Regalia",
--   requiredSkill = 0,
--   requiredSkillRank = 0,
--   items = { 16685, 16683, 16686, 16684, 16687, 16689, 16688, 16682 },
--   bonuses = {
--     { spellID = 29091, threshold = 2 },   -- 2-piece bonus
--     { spellID = 27867, threshold = 6 },   -- 6-piece bonus
--     { spellID = 18679, threshold = 8 },   -- 8-piece bonus
--     { spellID = 30777, threshold = 4 },   -- 4-piece bonus
--   },
-- }
```

| Field | Type | Notes |
|-------|------|-------|
| `setID` | number | Echo of the input. |
| `name` | string | Localized set name. |
| `requiredSkill` | number | Skill line ID required to use the set; `0` if none. |
| `requiredSkillRank` | number | Required rank in that skill line; `0` if none. |
| `items` | array | itemIDs in the set. Empty slots filtered, so `#info.items` is the real item count. Order is the engine's own (matches the set's order in the DBC; not necessarily slot-sorted). |
| `bonuses` | array | `{ spellID, threshold }` tables for each non-empty set-bonus slot. `threshold` is the number of equipped set pieces required to grant `spellID`. Order is the DBC's; bonuses with the same threshold are not normalized. |

Reads `ItemSet.dbc` directly — no cache warm-up needed (DBCs load at
boot). Returns `nil` for `setID == 0`, out-of-range IDs, or rows
that don't exist.

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

### `C_Item.GetItemStatDelta(itemLink1, itemLink2)`

Returns a table of the per-stat **difference** between two items,
computed as `item2 - item1` — a positive value means the second item has
more of that stat. Only stats whose delta is non-zero appear.

```lua
-- Compare an equipped ring against one in your bags:
local delta = C_Item.GetItemStatDelta(
    C_Item.GetItemLink({equipmentSlotIndex = 11}),
    C_Item.GetItemLink({bagID = 0, slotIndex = 3}))
-- delta = { ITEM_MOD_INTELLECT_SHORT = 6, ITEM_MOD_SPIRIT_SHORT = 2 }
-- (the bag ring has +6 Int / +2 Spirit relative to the equipped one)
```

Same key set, accepted input forms, random-suffix handling, and caching
caveat as [`C_Item.GetItemStats`](#c_itemgetitemstatsitemlink) — see
there for the details. Returns `nil` if **either** item is uncached.

### `C_Item.GetItemStats(itemLink)`

Returns a table of an item's stats, keyed by the FrameXML global-string
names WoW's own `GetItemStats` uses (so `_G[key]` yields the display
label — `_G["ITEM_MOD_STRENGTH_SHORT"]` → `"Strength"`). Only stats the
item actually carries are present.

```lua
C_Item.GetItemStats("item:2244")            -- Krol Blade
-- { ITEM_MOD_STRENGTH_SHORT = 7, ITEM_MOD_STAMINA_SHORT = 5,
--   ITEM_MOD_DAMAGE_PER_SECOND_SHORT = 40.89,
--   ITEM_MOD_CRIT_MELEE_RATING = 1, ITEM_MOD_CRIT_RANGED_RATING = 1 }

C_Item.GetItemStats("item:12022:0:769")     -- "of the Owl" random suffix
-- { ITEM_MOD_INTELLECT_SHORT = 6, ITEM_MOD_SPIRIT_SHORT = 6 }
```

**Sources.** The table is assembled from three places, so it reflects
what the item actually grants — not just its stored stat slots:

1. **Base record** — the stored attributes, armor, resistances, and (for
   weapons) DPS.
2. **On-equip spells** (`SpellTrigger == ON_EQUIP`) — vanilla stores the
   "special" bonuses (crit, attack power, spell power, hit, mp5, defense,
   …) as an equip spell whose aura effect carries the value, not as a
   stat slot. Their auras are decoded into the keys below.
3. **Random suffix** — the link's third `item:id:enchant:SUFFIX:unique`
   field (`ItemRandomProperties.dbc` → `SpellItemEnchantment.dbc`): stat
   suffixes ("of the Owl") are themselves equip spells, armor suffixes
   ("of Toughness") are direct resistance enchants. A bare itemID or a
   suffix-less link contributes nothing here.

**Keys.**

| Key | Stat | Source |
|-----|------|--------|
| `ITEM_MOD_STRENGTH_SHORT` / `_AGILITY_` / `_STAMINA_` / `_INTELLECT_` / `_SPIRIT_SHORT` | Base attributes | record / `MOD_STAT` |
| `ITEM_MOD_MANA_SHORT` / `ITEM_MOD_HEALTH_SHORT` | Mana / health on equip | record / `MOD_INCREASE_ENERGY` (35) / `MOD_INCREASE_HEALTH` (34) |
| `RESISTANCE0_NAME` | Armor | record / `MOD_RESISTANCE` bit 0 |
| `RESISTANCE1_NAME` … `RESISTANCE6_NAME` | Holy / Fire / Nature / Frost / Shadow / Arcane resistance | record / `MOD_RESISTANCE` / type-4 enchant |
| `ITEM_MOD_DAMAGE_PER_SECOND_SHORT` | Weapon DPS (float) | avg damage ÷ swing time |
| `ITEM_MOD_ATTACK_POWER_SHORT` | Attack power | `MOD_ATTACK_POWER` (99) |
| `ITEM_MOD_RANGED_ATTACK_POWER_SHORT` | Ranged attack power (ranged-only items) | `MOD_RANGED_ATTACK_POWER` (124) |
| `ITEM_MOD_CRIT_MELEE_RATING` + `ITEM_MOD_CRIT_RANGED_RATING` | Crit % | weapon-crit aura (52) |
| `ITEM_MOD_HIT_MELEE_RATING` + `ITEM_MOD_HIT_RANGED_RATING` | Hit % | hit aura (54) |
| `ITEM_MOD_HIT_SPELL_RATING` | Spell hit % | spell-hit aura (55) |
| `ITEM_MOD_CRIT_SPELL_RATING` | Spell crit % | spell-crit aura (57 / 71) |
| `ITEM_MOD_SPELL_DAMAGE_DONE_SHORT` | Spell damage | `MOD_DAMAGE_DONE` (13) |
| `ITEM_MOD_SPELL_HEALING_DONE_SHORT` | Healing | `MOD_HEALING_DONE` (135) |
| `ITEM_MOD_MANA_REGENERATION` | mp5 (mana) | `MOD_POWER_REGEN` (85) |
| `ITEM_MOD_DEFENSE_SKILL_RATING` | Defense skill | `MOD_SKILL` (30, misc 95) |
| `ITEM_MOD_DODGE_RATING` | Dodge % | `MOD_DODGE_PERCENT` (49) |
| `ITEM_MOD_PARRY_RATING` | Parry % | `MOD_PARRY_PERCENT` (47) |
| `ITEM_MOD_BLOCK_RATING` | Block chance % | `MOD_BLOCK_PERCENT` (51) |
| `ITEM_MOD_BLOCK_VALUE` | Shield block value | record (`m_block`) |

> **Values are vanilla-native, not ratings.** Vanilla has no rating
> system, so the percent-based stats (crit / hit / defense) report the raw
> **percentage** under the modern `*_RATING` key — e.g. Krol Blade's
> "+1% crit" is `ITEM_MOD_CRIT_MELEE_RATING = 1`, where TBC/Era would
> show the level-scaled rating (`13`). A native percent is the only
> value that's honest and level-independent. Flat stats (attack power,
> spell damage, healing, mp5) are the item's actual magnitudes.
>
> Rating stats that didn't exist in 1.12 at all (haste, mastery,
> versatility, expertise, resilience) never appear. A generic "+N attack
> power" item carries both a melee and a ranged AP aura in vanilla; the
> generic `ITEM_MOD_ATTACK_POWER_SHORT` key subsumes it, so
> `ITEM_MOD_RANGED_ATTACK_POWER_SHORT` shows only for ranged-only items
> (scopes, ranged weapons). Rarer auras with no clean single key —
> percent attack power, per-weapon-line skill, damage-taken, on-hit /
> on-use procs — are skipped.

**Input.** Accepts a full chat hyperlink, an `item:N…` link string, or a
bare itemID (a superset of retail, which is link-only). Returns `nil` if
the item isn't cached yet — warm it via `GetItemInfo` and retry.

### `C_Item.GetItemSubClassInfo(classID, subClassID)`

Returns `subClassName, subClassUsesInvType` for an item class/subclass pair.

```lua
C_Item.GetItemSubClassInfo(2, 7)   -- "One-Handed Swords", false
C_Item.GetItemSubClassInfo(4, 1)   -- "Cloth", true
```

- `subClassName` (string) — the localized `ItemSubClass.dbc` name (verbose
  form, e.g. `"One-Handed Swords"`; falls back to the short form for
  subclasses that only populate it, e.g. `"Consumable"`).
- `subClassUsesInvType` (boolean) — true for subclasses whose items are
  labeled by inventory slot rather than by subclass name. On vanilla data
  that's exactly the armor material types (Miscellaneous, Cloth, Leather,
  Mail, Plate); false for weapons, shields, librams, idols, totems, and
  non-equipment. Read from the `ItemSubClass.dbc` flags bit `0x200`
  (verified against the 1.15 client).

Returns `nil` if the `(classID, subClassID)` pair has no row.

### `C_Item.GetItemUniqueness(itemLocation)` / `C_Item.GetItemUniquenessByID(item)`

The two functions have **different signatures** in modern (Classic
Era 1.15.x) WoW — we mirror them exactly:

`C_Item.GetItemUniqueness(itemLocation)` returns
`(limitCategory, limitMax)`:

| Field | Vanilla source |
|-------|----------------|
| `limitCategory` | Always `0` — vanilla has no `LimitCategory.dbc` (TBC addition). |
| `limitMax` | `ItemStats_C.m_maxCount` — `0` = unlimited, `1` = "Unique", higher = inventory cap. |

`C_Item.GetItemUniquenessByID(itemID)` returns
`(isUnique, limitCategoryName, limitCategoryCount, limitCategoryID)`:

| Field | Vanilla source |
|-------|----------------|
| `isUnique` | `m_maxCount > 0` — true for any unique-tagged item. |
| `limitCategoryName` | Always `nil` — no categories in vanilla. |
| `limitCategoryCount` | `m_maxCount` when `isUnique`, else `nil`. |
| `limitCategoryID` | Always `nil`. |

Both functions return nothing (zero Lua return values, matching
modern's `MayReturnNothing` annotation) if the item record isn't
cached. The by-ID variant fires a background cache fill on miss;
re-call after `GET_ITEM_INFO_RECEIVED`.

```lua
local _, max = C_Item.GetItemUniqueness({equipmentSlotIndex = 13})
-- max = 1 for typical trinkets (Unique)
local isUnique, _, count = C_Item.GetItemUniquenessByID(81013)
-- isUnique = true, count = 10  (Southern Sand Crawler Leg, "Unique (10)")
```

Modern WoW populates the category half for items like Brewfest Mug
or Heart of Azeroth ("Unique-Equipped: Eye of Azshara"). Vanilla
has no such items, so the category fields stay nil/0.

### `C_Item.GetStackCount(itemLocation)`

Returns the **current** stack count in a specific slot — distinct
from `C_Item.GetItemCount(item)`, which sums every stack of that
itemID across the player's inventory. Useful when an addon needs to
know "this specific stack has 13/20", not "I have 47 total".

```lua
local n = C_Item.GetStackCount({bagID = 0, slotIndex = 1})
-- 13 (the bag's first slot has 13 of whatever item)
```

Reads `ITEM_FIELD_STACK_COUNT` directly off the item's
`m_objectFields` — same field `GetContainerItemInfo` returns as
`itemCount`. Returns `0` for empty / unresolvable locations.

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

### `C_Item.IsItemGUIDInInventory(itemGUID)`

Returns whether the item with the given `"0x…"` GUID is currently carried
by the player — the 19 equipment slots plus the bags (backpack + the four
equipped bags). Equipped items count (verified against retail); the bank
does not.

```lua
local g = C_Item.GetItemGUID({bagID = 0, slotIndex = 1})
C_Item.IsItemGUIDInInventory(g)   -- true
C_Item.IsItemGUIDInInventory(
    C_Item.GetItemGUID({equipmentSlotIndex = 1}))   -- true (equipped counts)
```

Accepts the GUID string [`C_Item.GetItemGUID`](#c_itemgetitemguiditemlocation)
/ `UnitGUID` return; returns `false` for a malformed or zero GUID. To also
cover the bank, use `C_Item.GetItemCount(itemID, true) > 0` instead.

### `C_Item.IsItemOpenable(itemLocation)`

Returns two values: `(isOpenable, canOpen)`.

- **`isOpenable`** — `true` if the item type is right-click-openable
  (sack, clam, simple chest, quest box, lockbox, etc. — anything
  whose tooltip *could* show `<Right Click to Open>`). Intrinsic to
  the item type. From `ItemStats.Flags & 0x4`.
- **`canOpen`** — `true` if the player can right-click *this specific
  instance* and trigger the open action right now. For unlocked-by-
  default items (clams, sacks) this matches `isOpenable`. For items
  with a lock (lockboxes), it's only true after the lock has been
  removed (rogue Pick Lock, key item, etc.) — so a priest looking at
  a fresh lockbox sees `(true, false)`, while a rogue who just
  picked it sees `(true, true)`.

Both values are `nil` when the item data isn't cached yet (background
`SMSG_ITEM_QUERY_SINGLE` fired) or when the location is empty /
invalid. Lets callers distinguish "data unknown" from "definitely
not openable".

There's no `ByID` variant — the `canOpen` check depends on a specific
instance's `ITEM_FIELD_FLAGS`, which you can only have via an
itemLocation. If you only have an itemID, use
`C_Item.GetItemInfoInstant` to inspect the item type instead.

```lua
-- Loop a bag and gather every openable the player can act on now
for slot = 1, GetContainerNumSlots(0) do
    local _, canOpen = C_Container.IsContainerItemOpenable(0, slot)
    if canOpen then
        -- pickup + use; this slot is good to open
    end
end

-- Show a different hint depending on the lock state
local isOpenable, canOpen = C_Item.IsItemOpenable({bagID = 0, slotIndex = 1})
if isOpenable and not canOpen then
    print("This item is locked.")
end
```

Implementation: `isOpenable` reads `ItemStats.Flags & 0x4` from the
client-side ItemSparse cache. `canOpen` adds a check that either
`ItemStats.LockID == 0` (no lock) or the CGItem instance's
`ITEM_FIELD_FLAGS & 0x4` (UNLOCKED) bit is set. Same conditions the
engine's tooltip builder uses at `0x0052E323` to gate the
`ITEM_OPENABLE` line.

### `C_Item.IsLocked(itemLocation)`

Returns `true` if the item is in the client-side "in-transaction"
lock state — picked up onto the cursor, mail-attached, trade-attached,
mid-swap, etc. Reads the per-CGItem instance flag at `+0x314` bit 0
(not the `ITEM_FIELD_FLAGS` descriptor field — vanilla's actual lock
is on the instance, not in `m_objectFields`).

```lua
if C_Item.IsLocked({bagID = 0, slotIndex = 1}) then
    -- skip the action; slot is mid-transaction
end
```

The lock is **client-managed, server-confirmed**:

- Engine sets the bit *optimistically* in pickup/equip/attach paths,
  before the transaction packet goes to the server. UI greys
  immediately, no round-trip wait.
- Engine clears the bit when the matching `SMSG_UPDATE_OBJECT`
  confirms the transaction.

Listen for the vanilla-native `ITEM_LOCK_CHANGED` event (no payload —
the engine fires it on every set/clear and addons re-poll the items
they care about) to react without timed polling.

### `C_Item.LockItem(itemLocation)`

Set the client-side lock on a single item by location. Calls the same
engine primitive (`FUN_ITEM_LOCK_BY_GUID = 0x004953E0`) the engine
itself uses optimistically before sending pickup/equip transactions.
Fires `ITEM_LOCK_CHANGED` the same way.

```lua
C_Item.LockItem({bagID = 0, slotIndex = 1})
```

> **Setting the lock doesn't make anything real happen on the server.**
> It's a pure client-side flag — the player's UI greys the item out
> until something clears it. No transaction packet is sent, no
> cancel-on-failure mechanism kicks in. Treat this as a UI hint for
> custom workflows (batch-equip preview, drag-target highlighting,
> etc.), not real transaction state.
>
> The lock stays until you call `UnlockItem` / `UnlockAllItems`, OR
> the engine receives an SMSG_UPDATE_OBJECT for the item from the
> server, which clears it as a side effect.

### `C_Item.LockItemByGUID(itemGUID)`

Same as [`LockItem`](#c_itemlockitemitemlocation) but takes a GUID
string in the canonical `"0xHHHHHHHHLLLLLLLL"` format
[`C_Item.GetItemGUID`](#c_itemgetitemguiditemlocation) returns. The
engine resolves through the object manager, so this can mark items
the player doesn't currently have in bags/equipment — trade window
contents, mail attachments, freshly looted but not yet bagged — as
long as the CGItem is loaded.

```lua
local guid = C_Item.GetItemGUID({equipmentSlotIndex = INVSLOT_HEAD})
C_Item.LockItemByGUID(guid)
```

Same caveats as `LockItem` — purely client-side, doesn't affect any
real engine transaction state.

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

> **`success` is `1` or `nil`.** The engine's printf-style event
> dispatcher has no `%b` token, so we encode the boolean as `1` for
> success / `nil` for failure (leaning on `lua_pushstring(NULL)` →
> `lua_pushnil`). `if success then …` distinguishes them correctly —
> `nil` is falsy, `1` is truthy. Same encoding as `GET_ITEM_INFO_RECEIVED`
> and `QUEST_DATA_LOAD_RESULT`.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("ITEM_DATA_LOAD_RESULT")
f:SetScript("OnEvent", function()
    -- vanilla 1.12: event payload is in `arg1`, `arg2`, ... globals
    if event == "ITEM_DATA_LOAD_RESULT" and arg2 then
        local _, type = C_Item.GetItemInfoInstant(arg1)
        -- ...
    end
end)
C_Item.RequestLoadItemDataByID(2589)
```

> **Engine-aligned cache strategy.** `RequestLoadItemData` does NOT
> eagerly create a cache entry. Empirically (e.g. pfQuest calling it
> at addon-load / PLAYER_ENTERING_WORLD against a cleared WDB), pre-
> creating the entry races the engine's natural inventory prefetch
> and can leave the item permanently pending — the engine sees the
> entry exists and skips its own `SMSG_ITEM_QUERY_SINGLE`, while our
> early query gets dropped. Instead we track the itemID and hook the
> engine's `SMSG_ITEM_QUERY_SINGLE_RESPONSE` handler at `0x0055BDB0`:
> when the engine fills any entry, we sweep tracked items and fire
> `ITEM_DATA_LOAD_RESULT` for matches. For items the engine doesn't
> prefetch (quest rewards, AH browse targets, etc.) we escalate after
> ~60 ticks by calling `CacheFetch` ourselves — by then engine state
> has settled and our query goes through normally. Hard timeout at
> ~1200 ticks (~20–40s) fires `ITEM_DATA_LOAD_RESULT(itemID, nil)`.

### `C_Item.UnlockAllItems()`

Sweep clear of every CGItem the engine knows about — same primitive
the engine itself runs on `PLAYER_LEAVING_WORLD`. One
`ITEM_LOCK_CHANGED` fires at the end.

```lua
C_Item.UnlockAllItems()
```

Cheapest recovery if you don't know which item is stuck. Same caveat
as `UnlockItem`: this is purely client-side state — it won't cancel
any pending server-side transactions.

### `C_Item.UnlockItem(itemLocation)`

Force-clear the client-side lock on one item. Recovery primitive —
fires `ITEM_LOCK_CHANGED` so the UI refreshes.

```lua
C_Item.UnlockItem({bagID = 0, slotIndex = 1})
```

Useful when a transaction packet was sent but the server's
confirmation never arrived: the item stays visually greyed
indefinitely (vanilla's only built-in trigger for the unlock-all
sweep is logout). This call gives you an in-session escape hatch.

> **Cursor / server state isn't touched.** Unlocking doesn't tell the
> server "cancel my transaction" — it only clears the local visual
> lock. If the item is genuinely mid-flight, the server's next
> update will set the lock right back. For cursor-cancel semantics,
> pair with vanilla-native `ClearCursor()`.

### `C_Item.UseAtCursor(itemInfo)`

Uses `itemInfo` at the player's current cursor world position —
ClassicAPI's `[@cursor]` analog for ground-target on-use items
(Iron Grenade, Bombling, demolition charges, etc.). Returns `true`
when the cursor-placement leg landed (the item fires at terrain);
`false` for items that aren't ground-target (the item still fires
normally with no implicit target), unparseable input, items not in
bags, cursor over UI / off-screen, etc.

`itemInfo` accepts the same shapes as
[`C_Item.UseItemByName`](#c_itemuseitembynameiteminfo--unit) — itemID,
bare `"item:N"`, full chat link, or localized name.

```lua
C_Item.UseAtCursor(4068)            -- Iron Grenade at cursor
C_Item.UseAtCursor("Iron Grenade")
```

Implementation chains the existing item-use path
(`Item::Location::FindByArgInBags` + `FUN_ITEM_USE`) with
[`Spell::AtCursor::Resolve`](#c_spellcastatcursorspellid) — same
cursor-resolution helper `C_Spell.CastAtCursor` uses. When the item
fires a non-ground-target spell, the cursor leg no-ops and returns
`false`; the item still uses normally (any implicit target — current
selection, etc. — applies).

Cancels placement automatically when the cursor isn't on terrain —
the item-use packet is never sent, so an off-screen click doesn't
waste the grenade.

### `C_Item.UseAtUnit(itemInfo, unit)`

Unit-position analog of
[`C_Item.UseAtCursor`](#c_itemuseatcursoriteminfo): uses `itemInfo` at
`unit`'s feet rather than the cursor. ClassicAPI's `[@unit]` for
ground-target on-use items. `itemInfo` accepts the same forms as
`UseAtCursor` (itemID, bare `"item:N"`, full chat link, localized
name); `unit` is any unit token (`"player"`, `"target"`,
`"mouseover"`, `"party1"`, …).

```lua
C_Item.UseAtUnit(4068, "target")            -- Iron Grenade at the target's feet
C_Item.UseAtUnit("Iron Grenade", "player")
```

Same chain as `UseAtCursor` (`Item::Location::FindByArgInBags` +
`FUN_ITEM_USE`) but committing the placement at the unit's world
position via
[`Spell::AtCursor::CommitAtCoords`](#c_spellcastatunitspellid-unit)
instead of the cursor raycast. Returns `true` when the placement
landed at the unit; `false` for non-ground-target items (the item
still fires with any implicit target), unparseable input,
item-not-in-bags, or an unresolvable unit. The unit is resolved
before the item fires, so an absent unit fails without consuming the
item.

### `C_Item.UseItemByName(itemInfo [, unit])`

Finds the first item in the player's bags matching `itemInfo` and
uses it. Returns nothing; silently no-ops when:

- the input is `nil`, an empty string, or otherwise unparseable
- no matching item is in bags
- the engine refuses the use — cooldown, locked item, level
  requirement, etc.

`itemInfo` accepts the same shapes as
[`C_Item.EquipItemByName`](#c_itemequipitembynameiteminfo--dstslot) —
itemID number, bare `"item:N"` string, full chat link, or a localized
item name. Name matches are case-insensitive against the cached
`m_name[0]`.

The optional `unit` argument is a unit token (`"player"`, `"target"`,
`"focus"`, `"partyN"`, `"raidN"`, `"nameplateN"`, …) used as the cast
target for items that fire a spell (scrolls, traps, on-use targeted
effects). For self-use items (hearthstone, potions, food) the engine
overwrites the target with the item's own GUID before dispatch, so
passing a `unit` to those is harmless and has no effect. Unrecognized
strings are treated as "no target" rather than raising, matching the
silently-no-op contract of `itemInfo`.

```lua
C_Item.UseItemByName("Hearthstone")                       -- hearth home
C_Item.UseItemByName(6948)                                -- same thing, by ID
C_Item.UseItemByName("Major Healing Potion")
C_Item.UseItemByName("Scroll of Stamina IV", "target")    -- buff your tank
```

Mirrors 3.3.5's `Script_UseItemByName` structure: locate the item
directly, then hand the `CGItem *` to the engine's by-pointer use
primitive at `0x005D8D00`. That primitive dispatches internally based
on item type (food, potion, on-use spell, scroll, quiver, ...) so a
single call covers every item category. We skip
`Script_UseContainerItem` entirely — its branches for repair vendor,
spell-cast targeting, and drop-on-bag cursor modes don't apply to an
addon-issued call from a clean cursor.

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

### `GetAverageItemLevel()`

Returns `(avgItemLevel, avgItemLevelEquipped)` — modern WoW's
2-tuple shape. `avgItemLevelEquipped` is the arithmetic mean over
the player's currently-worn equipment slots; `avgItemLevel` is the
same metric but extended to include best-per-slot upgrades found
anywhere in the player's bags and bank.

Slots considered: `INVSLOT_HEAD..INVSLOT_TABARD` (1..19) minus
shirt (4) and tabard (19) = 17 candidate slots. Bag-walk uses the
same `invType → slotMask` mapping as `GetInventoryItemsForSlot`.

```lua
local overall, equipped = GetAverageItemLevel()
-- overall:  best-per-slot ilvl across equipped + bags + bank
-- equipped: ilvl of currently-worn items only
```

**Fixed denominator.** Empty slots count toward the denominator
(contributing 0 to the sum). Removing a piece of gear always
lowers `avgItemLevelEquipped` — matches modern's behavior
(verified against retail Wow.exe at `552.4375 × 16 = 8839`
exactly). Vanilla addons of the GearScore era used populated-only
count; this implementation deliberately differs.

**Max-of-two-divisors fairness.** A 2H weapon wielder has an
intentionally empty offhand (slot 17) — counting it as 0 in a
17-divisor sum would unfairly penalize them. We compute a second
candidate excluding slot 17 from both numerator and denominator
(16 slots), and return the max. Sword+shield characters typically
win the all-17 path; 2H wielders win the no-OH path. Same trick
the 4.3.4 client uses at `FUN_0097E0F0`'s tail.

**No double-counting.** Each bag/bank item is assigned to **one**
candidate slot via greedy fit — a trinket in your bag fills one
trinket slot (the empty/lower-best one), not both. Without this
gate, moving a trinket from equipped to bag would inflate the
"overall" count and drag the average down (because the same item
would count for both trinket slots).

**Bank is walked** even if the bank window has never been opened
this session — the GUID array at `invMgr + 0x04` is populated
from server data at login. Bypasses the bank-window gate on
`GetItemBySlot` the same way `C_Item.GetItemCount` does.

Limitations:
- Items not yet in the ItemStats cache are skipped from the
  running totals and a warmup is queued; the next call after
  `ITEM_DATA_LOAD_RESULT` lands picks them up.
- The full 4.3.4 weapon `qsort` dance — handling 2H-equipped vs.
  1H + OH-in-bag comparisons — is not replicated. Edge case in
  vanilla where you'd notice a difference is narrow.

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

### `GetInventoryItemsForSlot(slot, returnTable [, transmogrify])`

Populates `returnTable` with every item in the player's equipment
and bags that's eligible to be equipped in `slot` (1..19). Returned
table is keyed by the packed [ItemLocation
bitfield](#equipmentset) — same encoding
`C_EquipmentSet.GetItemLocations` uses — with the itemLink as the
value. Returns the same `returnTable` reference for chaining.

```lua
local t = {}
GetInventoryItemsForSlot(16, t)   -- mainhand
for loc, link in pairs(t) do
    -- loc is a number like 0x00300103; link is "|c…|Hitem:…|h[…]|h|r"
end
```

| slot | Eligible InventoryTypes |
|------|-------------------------|
| 1 HEAD, 2 NECK, …, 10 HANDS | The slot's matching InventoryType |
| 11, 12 FINGER | INVTYPE_FINGER (11) — items appear under both ring slots |
| 13, 14 TRINKET | INVTYPE_TRINKET (12) |
| 15 BACK | INVTYPE_CLOAK (16) |
| 16 MAINHAND | INVTYPE_WEAPON (13), INVTYPE_2HWEAPON (17), INVTYPE_WEAPONMAINHAND (21) |
| 17 OFFHAND | INVTYPE_WEAPON (13), INVTYPE_SHIELD (14), INVTYPE_WEAPONOFFHAND (22), INVTYPE_HOLDABLE (23) |
| 18 RANGED | INVTYPE_RANGED (15), INVTYPE_THROWN (25), INVTYPE_RANGEDRIGHT (26) |
| 5 CHEST | INVTYPE_CHEST (5), INVTYPE_ROBE (20) |
| 19 TABARD | INVTYPE_TABARD (19) |

Compatibility check is a single bitwise AND against a static
`invType → slotMask` table — same shape and values as 3.3.5's table
at `DAT_00A2D288` (used by `Script_GetInventoryItemsForSlot` via
`FUN_007082B0`), with two adjustments for vanilla:

- 2H weapons (`INVTYPE_2HWEAPON`) confined to the mainhand slot
  only. 3.3.5 allowed offhand for the titanic-grip era; vanilla has
  no such mechanic and the server would reject the equip anyway.
- `INVTYPE_WEAPONMAINHAND` / `WEAPONOFFHAND` confined to their
  literal slot rather than the looser "either hand" 3.3.5 allowed.

The `transmogrify` arg is accepted and ignored. Stock 1.12 has no
transmog system to query against, and the modern flag's effect
(broader cross-eligibility for visual swaps) reduces to the
regular equip check here. Private servers like Turtle WoW layer
their own transmog systems on top of vanilla; this function
doesn't currently plumb the flag through to any of them.

> **Bank items aren't included.** Vanilla's `GetItemBySlot` gates
> bank slots on a banker GUID that's only set while the bank window
> is open; modern WoW behaves the same way (bank shows up only when
> the bank UI is open). We don't walk bank slots — addons that want
> bank inclusion need their own bank scan with the bank UI open.

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

## Loot

Programmatic loot operations that vanilla 1.12's Lua surface doesn't
expose: enumerate nearby lootable corpses, open a loot session against
one by GUID, take a specific item out of one without going through the
visible `LootFrame`, and pre-scan every reachable corpse to build a
picker table before any cursor work. Foundation for AoE-loot,
auto-loot-by-filter, and similar addons that 1.12's stock
`LootSlot` / `LootSlotInfo` flow can't drive.

All `guid` args are hex-string GUIDs in the same form `UnitGUID`
returns (`"0xF130..."`). All `itemID` args are integers.

### `C_Loot.GetNearbyLootableUnits()`

Returns a 1-indexed array of `{ guid = "0x..." }` subtables, one per
visible unit that the server has flagged lootable AND that's within
the engine's right-click-loot interact range. Empty table pre-login,
when no lootable units are in range, or when the local player has no
loot rights on visible corpses (the server only sets the lootable
flag for clients with rights).

```lua
for _, unit in ipairs(C_Loot.GetNearbyLootableUnits()) do
    C_Loot.LootUnit(unit.guid)
end
```

The interact-range check matches what the engine uses for the
hover-to-loot cursor — slightly tighter than melee range, looser than
"on top of." Server still enforces real range on the resulting
`CMSG_LOOT`; in-client OOR requests just silently get no response.

### `C_Loot.LootUnit(guid)`

Opens a loot session against the given corpse. The loot window
appears asynchronously via the normal `LOOT_OPENED` event once
`SMSG_LOOT_RESPONSE` arrives — same flow as right-clicking the
corpse, just dispatched programmatically.

No return value. Silent no-op when:
- The object manager is NULL (glue / character-select).
- The GUID doesn't resolve to a visible unit (out of render range,
  despawned, wrong typemask).
- The local player pointer can't be resolved.

Errors only on bad Lua usage (missing arg, unparseable GUID).

```lua
-- Loot everything in range, one corpse at a time. With autoloot on,
-- the engine drains each window automatically; without it, you
-- still get the visible loot frame per corpse.
for _, unit in ipairs(C_Loot.GetNearbyLootableUnits()) do
    C_Loot.LootUnit(unit.guid)
end
```

`useDistanceCheck` is hardcoded off — the engine won't try to
auto-walk the player toward a corpse just out of range. That'd be a
surprise side effect for an AoE-loot loop.

### `C_Loot.LootUnitItem(guid, itemID)`

Takes a specific item from a specific corpse without using the
visible loot frame's slot list. Dispatches `CMSG_LOOT_ITEM` directly.

Two paths, picked automatically:

1. **Fast path** — the corpse's loot window is already open for the
   requested target. Synchronously finds the slot, sends the take
   packet, returns `true` if the slot was found / `false` if not.
2. **Async path** — opens the corpse's window first (same wire as
   `C_Loot.LootUnit`), waits for the response, then sends the take
   packet automatically. Returns `true` once the request is queued.
   No "did it succeed" event; observe `BAG_UPDATE` / `ITEM_PUSH` to
   confirm.

`false` immediately when:
- Args missing or wrong types (raises an error, not returns false).
- Another `LootUnitItem` is mid-flight (one at a time).
- The world isn't loaded.
- The target GUID doesn't resolve to a visible unit.
- (sync path only) No slot in the open window holds the itemID.

The send bypasses vanilla's BoP-confirmation dialog. Server still
enforces all real permissions (loot rights, distance, master-loot
rules); the BoP prompt is purely a client-side courtesy that callers
of a programmatic API have already opted out of.

```lua
-- Scan, then take just one specific item out of one specific corpse.
C_Loot.ScanNearbyLoot()  -- LOOT_SCAN_COMPLETED → results below
local results = C_Loot.GetLastScanResults()
local guid = results[1].guid
local itemID = results[1].items[1].itemID
C_Loot.LootUnitItem(guid, itemID)
```

### `C_Loot.ScanNearbyLoot()`

Initiates a pre-scan: opens each lootable corpse in range one at a
time, scrapes its slot contents (itemID, count, item link), closes
the window, advances. The `LOOT_OPENED` / `LOOT_CLOSED` events are
suppressed for the duration of the scan so `LootFrame` and other
listeners don't react — the scan is invisible to addons that aren't
opted in.

Returns `true` if the scan was queued, `false` if another scan is
already in progress or the world isn't loaded. Completion is signaled
via the [`LOOT_SCAN_COMPLETED`](#loot_scan_completed-event) event;
results are read via [`C_Loot.GetLastScanResults()`](#c_lootgetlastscanresults).

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("LOOT_SCAN_COMPLETED")
f:SetScript("OnEvent", function()
    for _, corpse in ipairs(C_Loot.GetLastScanResults()) do
        for _, item in ipairs(corpse.items) do
            ChatFrame1:AddMessage(item.link or ("item:"..item.itemID))
        end
    end
end)
C_Loot.ScanNearbyLoot()
```

Per-corpse step timeout is ~3-6 seconds (180 WorldTick frames) so a
single dropped server response doesn't hang the whole scan. Failed
corpses simply don't appear in the results.

### `C_Loot.IsScanInProgress()`

Returns `true` while a `ScanNearbyLoot` call is mid-flight, `false`
otherwise. Use to gate re-scan attempts so addons don't spam-call
during the engine's per-corpse round-trip.

### `C_Loot.GetLastScanResults()`

Returns the most-recent completed-scan results as a 1-indexed array:

```lua
{
    [1] = {
        guid = "0x...",    -- corpse GUID
        coin = 1234,       -- copper amount; 0 if no coin
        items = {
            [1] = {
                itemID = 2589,
                count  = 4,
                link   = "|cff...|Hitem:2589:0:0:0|h[Linen Cloth]|h|r",
                                   -- omitted if cache entry wasn't loaded
            },
            ...
        },
    },
    ...
}
```

Empty table before the first scan completes. Replaced wholesale by
each new scan — re-read after `LOOT_SCAN_COMPLETED` rather than
caching the table.

The `link` field is the engine's full hyperlink (color + payload +
display name + reset). Random-suffix items like "Stringy Wolf Meat
of the Bear" survive round-trip — the link encodes the per-instance
enchant / suffix / unique fields, not just the base itemID. For
tooltip display, extract the payload form via
`string.match(link, "|H(item:[^|]+)|h")` — vanilla's `SetHyperlink`
requires the literal `"item:"` prefix and rejects full `|cff...|Hitem...|h`
input.

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

### `GetMacroSpell(macroSlot)`

Returns `(name, rank, spellID)` for the spell a macro's first `/cast`
/ `/castsequence` / `CastSpellByName(...)` directive resolves to, or
nothing when the macro slot is empty, contains no cast directive, or
the directive's name doesn't resolve to a spell the player knows.

```lua
-- macro slot 1's body: "/cast Fireball(Rank 5)"
local name, rank, id = GetMacroSpell(1)
-- name = "Fireball", rank = "Rank 5", id = 25306

-- macro slot 2's body: "/cast Fireball"  (no explicit rank)
local name, rank, id = GetMacroSpell(2)
-- name = "Fireball", rank = "Rank 5", id = 25306  (the highest known rank)

-- macro slot 3's body: only /script lines or no cast
GetMacroSpell(3)
-- (no returns)
```

No body parsing happens at call time — the vanilla engine already
walks every macro body at create / edit / refresh and caches the
resolved spellID on the macro struct. We just read the cache and
look the name + rank up in `Spell.dbc`. Result: O(1) per call.

`CastSpellNoToggle("<name>")` macros are also recognized — the
parser hook from the [`CastSpellNoToggle`](#castspellnotoggle-as-a-macro-cast-line)
section tags them with the same spellID a `/cast` line would, so
`GetMacroSpell` sees them too.

> **Re-save existing macros once after dropping in the DLL.** The
> spellID cache is populated at edit time. Macros that existed
> before ClassicAPI loaded (or that use `CastSpellNoToggle` and were
> last edited under stock 1.12) will have a stale `0` cache —
> opening them in the Macro UI and clicking Okay re-runs the parser
> and the new behavior takes effect.

### `GetMacroIcons` / `GetMacroItemIcons` / `GetLooseMacroIcons` / `GetLooseMacroItemIcons`

Modern Classic Era's 4-function append-to-table icon enumeration
surface. Each takes a Lua table as its only argument and appends icon
basenames; returns nothing (mutation is the contract).

```lua
local spellList, itemList = {}, {}

-- Modern's canonical call order (mirrors IconDataProvider.lua):
GetLooseMacroIcons(spellList)
GetLooseMacroItemIcons(itemList)
GetMacroIcons(spellList)
GetMacroItemIcons(itemList)

-- spellList now has `Ability_*` / `Spell_*` basenames;
-- itemList now has `INV_*` basenames.
```

The split is `loose` (icons the user dropped into `Interface\Icons\`
on disk) vs `mpq` (icons baked into the game's MPQ archives), crossed
with `Spell` (basenames starting with `Ability_` / `Spell_`) vs
`Item` (basenames starting with `INV_`). Modern engines maintain four
parallel arrays in their loader; we replicate the scheme by hooking
vanilla's three scan callbacks
(`FUN_MACRO_ICON_CB_DISK` / `*_USER_MPQ` / `*_INSTALL_MPQ`) and tagging
each captured filename by source + prefix.

**Vanilla quirk worth noting**: the engine's main icon DB (what
`GetMacroIconInfo(i)` returns) is filtered down to `Ability_*` /
`Spell_*` only — `INV_*` filenames flow through the scan callbacks
(~2,500+ per session in the Octo client) but never land in the DB
because something downstream of the callbacks rejects them. We
capture before that rejection, so `GetMacroItemIcons` works even
though no engine-level `GetMacroItemIconInfo(i)` exists.

Each appended entry is the uppercase basename stripped of the
`Interface\Icons\` prefix and any `.blp`/`.tga` extension (e.g.
`"INV_SWORD_25"`, `"ABILITY_KICK"`). Callers concat the prefix
themselves before `texture:SetTexture(...)`. Output is alphabetically
sorted within each function (sorted lazily on first read).

Capture-time dedup ensures each unique basename only appears once
per source/prefix combination even though most files flow through
multiple scan callbacks. There is **no cross-dedup** between loose
and mpq, or between spell and item — matches modern engine
behavior (a file that exists both as a loose drop-in and inside an
MPQ will appear in both the loose and mpq lists).

The vanilla legacy globals `GetNumMacroIcons` / `GetMacroIconInfo`
remain available unchanged.

## Mail

Backports of modern WoW's mail-attachment link getters. Both follow
the modern signature including the optional `attachmentIndex` arg
(added with TBC's multi-attachment mail), even though vanilla 1.12
only supports one attachment per message — pass-through compatibility
for addons originally written against later API versions. Any
`attachmentIndex` other than `1` is treated as no-op.

Both functions return `(link, itemID)` — modern returns just `link`,
but `itemID` is cheap to surface at the same lookup site and saves
callers a `string.match` on the link.

### `GetSendMailItemLink([attachmentIndex])`

Returns the **fully-decorated per-instance** hyperlink for the item
currently attached to the outgoing-mail slot, plus that item's
`itemID`. Same enchant / random-suffix / unique-ID decoration as
`GetContainerItemLink` would return for the same item back in the
player's bag — the attached item still exists as a live CGItem until
the mail is sent, so the link builder has full instance state to
draw from.

Returns nothing when:
- No item is attached.
- `attachmentIndex` is anything other than `nil` or `1`.
- The attachment's CGItem can't be resolved (shouldn't normally
  happen — engine writes the GUID on attach and clears it on detach).

```lua
local link, itemID = GetSendMailItemLink()
if link then
    ChatFrame1:AddMessage("Attached: " .. link .. " (id " .. itemID .. ")")
end
```

### `GetInboxItemLink(messageIndex[, attachmentIndex])`

Returns the **basic itemID-only** hyperlink (`|cff…|Hitem:N:0:0:0|h[Name]|h|r`)
for the item attached to inbox message `messageIndex` (1-based),
plus the `itemID`.

**Why basic-link instead of per-instance:** vanilla inbox entries do
store per-instance modifiers inline (enchant at `+0x12C`, suffix
factor at `+0x134`, random property at `+0x138` — the same fields
`GameTooltip:SetInboxItem` copies onto the tooltip to render a fully-
decorated preview). We intentionally ignore them in the link to match
modern 3.3.5's behavior — its `GetInboxItemLink` calls the
itemID-only link builder (`FUN_0061E290(itemID)`) and ignores the
per-instance fields. Per-instance data only fully manifests on the
client side once the player takes the item out of the mail and the
engine spawns a real CGItem; the inbox-entry fields are display-only
hints used by the tooltip builder.

Errors on missing or non-numeric `messageIndex`. Returns nothing when:
- `messageIndex` is out of range (`< 1` or `> GetInboxNumItems()`).
- The message slot is empty / has no attached item.
- `attachmentIndex` is anything other than `nil` or `1`.

```lua
for i = 1, GetInboxNumItems() do
    local link, itemID = GetInboxItemLink(i)
    if link then
        ChatFrame1:AddMessage(i .. ": " .. link .. " (id " .. itemID .. ")")
    end
end
```

## Map

### `C_Map.GetBestMapForUnit(unitToken)`

Returns the `AreaTable.dbc` area ID the given unit is currently in,
or `nil` if the unit isn't trackable. Vanilla 1.12 has no UiMap.db2
concept, so the closest equivalent to modern WoW's "best map" is the
zone-level AreaTable ID — exactly what the engine itself tracks for
the local player (`GetRealZoneText` reads it) and what gets broadcast
for party/raid members via `SMSG_PARTY_MEMBER_STATS_FULL`.

Coverage:

- `"player"` — always works.
- `"party1".."party4"` — works even for members in other zones / on
  other continents (slot-indexed GUID table stays populated regardless
  of local CGUnit availability).
- `"raid1".."raid40"` — same; covers the full raid roster.
- `"target"`, `"mouseover"`, `"focus"` and any other token that
  resolves to the player or a group member — works.
- NPCs, other players not in your group — `nil`.

```lua
/dump C_Map.GetBestMapForUnit("player")
-- 1519                                            (Stormwind City)
/dump C_Map.GetBestMapForUnit("party3")
-- 876                                             (party member on GM Island)
/dump C_Map.GetBestMapForUnit("target")
-- nil                                             (target is an NPC)
```

**The returned ID is a vanilla `AreaTable.dbc` area ID, not a modern
UI map ID.** Stormwind City in this backport is `1519`, not retail's
`84`. Addons that hardcode modern UI map IDs need a translation table
(or, simpler: compare against IDs this same function produces).

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

## NamePlate

Modern `C_NamePlate.*` returns nameplate `Frame` objects keyed off
unit data. Vanilla 1.12 doesn't ship the API at all — but the
underlying data (per-unit nameplate pointer at `CGUnit + 0xE60`)
exists. We enumerate visible units via the local-player-anchored
object hash table, filter by `TYPEMASK_UNIT`, and return matches.

Modern's `"nameplateN"` unit-token family is also supported — see
[Unit tokens](#unit-tokens-nameplaten) below. `NAME_PLATE_UNIT_ADDED`
/ `_REMOVED` / `_CREATED` events fire via a per-tick visible-plate
diff (see [the events section](#name_plate_created--name_plate_unit_added--name_plate_unit_removed-events)).

### `C_NamePlate.GetNamePlates()`

Returns a 1-based table of nameplate `Frame` objects — one per
CGUnit that currently has an allocated **Lua-registered** nameplate.
The frames are real Lua tables with methods (`:GetName()`,
`:GetWidth()`, `:SetAlpha()`, etc.) and any addon-added decorations
on them.

```lua
local plates = C_NamePlate.GetNamePlates()
for i, plate in ipairs(plates) do
    print(i, plate:GetName(), plate:GetWidth())
end
```

Two kinds of plates can show up:

- **Addon-created plates** (pfUI, TidyPlates, NamePlateMod, etc.):
  registered with Lua via `CreateFrame`, so each has a real
  registry ref. We push `registry[plate + 0x08]`. Identity is
  stable across calls — caching is safe while the frame is alive.

- **Default vanilla plates**: created internally by the engine
  without ever calling `CreateFrame`. Their `+0x08` field holds the
  sentinel `LUA_NOREF` (`-2`), not a real registry key. We build a
  fresh wrapper table per call (`{[0] = lightuserdata(plate)}` with
  the global `__framescript_meta` metatable) so addons get the
  same method surface. The wrapper isn't cached engine-side, so
  identity isn't stable across calls — don't compare wrappers, and
  don't store them across the unit going out of range (the
  underlying frame may be freed). Call `GetNamePlates()` fresh
  each time you need plates.

### Reading region content from a default nameplate

Vanilla plates have six child regions in stable positions. Walk them
with `:GetRegions()`:

```lua
local plates = C_NamePlate.GetNamePlates()
for _, plate in ipairs(plates) do
    local regions = {plate:GetRegions()}
    local name  = regions[3]:GetText()              -- e.g. "Joseph Dalton"
    local level = tonumber(regions[4]:GetText())    -- e.g. 60
    -- regions[1], [2], [5], [6] are textures (border, healthbar,
    -- glow, raid-icon — order depends on the engine's draw order)
end
```

Lua 5.0 has no `select()`, so collect into a table via
`{plate:GetRegions()}` and index. Addon-created plates have
different region layouts — those frames inherit whatever shape the
addon built, not this one.

### `C_NamePlate.GetNamePlateForUnit(unitToken)`

Returns the nameplate Frame for a single unit (resolved via the
engine's token-to-GUID path, so out-of-range party/raid members
work too), or `nil` if the unit has no allocated nameplate.

```lua
local plate = C_NamePlate.GetNamePlateForUnit("target")
if plate then
    local regions = {plate:GetRegions()}
    print("targeting:", regions[3]:GetText())   -- e.g. "Santora"
end
```

Same registered-vs-fresh-wrapper behavior as `GetNamePlates()` —
addon-created plates return their cached wrapper, default vanilla
plates get a fresh per-call wrapper. Don't cache the result across
the unit going out of range.

### `C_NamePlate.GetNamePlateForGUID(guidString)`

Same as `GetNamePlateForUnit` but takes the `"0xHHHHHHHHHHHHHHHH"`
GUID-string form. Useful when you've stored a unit GUID across
events (e.g., converted the positional `"nameplateN"` token to a
GUID via `UnitGUID(arg1)` at `NAME_PLATE_UNIT_ADDED` time) and need
the frame later.

```lua
local platesByGuid = {}

local f = CreateFrame("Frame")
f:RegisterEvent("NAME_PLATE_UNIT_ADDED")
f:SetScript("OnEvent", function()
    -- arg1 = "nameplateN" token; convert to stable GUID for storage
    local guid = UnitGUID(arg1)
    platesByGuid[guid] = C_NamePlate.GetNamePlateForGUID(guid)
end)
```

Returns `nil` if the GUID doesn't parse, doesn't resolve to a
visible CGUnit, or the unit has no allocated nameplate.

### `C_NamePlate.GetNamePlateGUIDs()`

Returns a 1-based table of GUID strings (modern
`"0xHHHHHHHHHHHHHHHH"` format) — one per CGUnit with an allocated
nameplate, **regardless** of whether the frame has been registered
with Lua. Catches default vanilla nameplates that
[`GetNamePlates`](#c_nameplategetnameplates) can't surface as
frames.

```lua
/dump C_NamePlate.GetNamePlateGUIDs()
-- { "0xF13000C36C26FD02", "0xF130000009276912", ... }
```

Walks the local-player-anchored object hash table for `TYPEMASK_UNIT`
entries, filters by `*(unit + 0xE60) != nullptr`. The per-unit
nameplate pointer is set by `FUN_006086E0`'s "show nameplate" path
regardless of which nameplate system rendered it. Order follows
hash-bucket iteration and isn't stable across calls.

### Unit tokens (`nameplateN`)

`"nameplate1"`, `"nameplate2"`, … work as unit tokens against every
`UnitX` function: `UnitName`, `UnitGUID`, `UnitClass`, `UnitHealth`,
`UnitHealthMax`, `UnitLevel`, `UnitFaction`, `UnitReaction`,
`UnitExists`, `UnitIsPlayer`, `UnitIsEnemy`, `UnitIsDead`, etc. —
~30 functions for free.

```lua
for i = 1, 40 do
    if not UnitExists("nameplate" .. i) then break end
    print(i, UnitName("nameplate" .. i), UnitClass("nameplate" .. i))
end
```

Ordering is **creation-order** — each new plate appends to the end of
the list and stays at its index until the unit goes out of range or
the plate is removed, at which point later indices shift down.
Stable for the lifetime of a single plate; no mid-frame reordering.
Same semantics as modern WoW.

Token chains work too — `"nameplate1target"`, `"nameplate1targettarget"`,
etc. — by mirroring the engine's own `targettarget`-style suffix
walker (read `UNIT_FIELD_TARGET` off `m_objectFields`, loop). Other
suffixes (`pet`, `master`) aren't supported by the vanilla engine's
own walker either, so they don't compose.

Out-of-range indices return `nil` cleanly without raising "Unknown
unit name" — `UnitExists("nameplate99")` just returns `false`.

**Implementation note.** We hook `FUN_TOKEN_TO_GUID` (the central
token→GUID resolver) so the entire `Script_Unit*` surface gains the
new token form transparently. The hook is gated by an `SStrCmpI`
prefix check against `"nameplate"`; non-nameplate tokens
(`"player"`, `"target"`, `"partyN"`, etc.) fall straight through to
the unmodified resolver. The ordered list is maintained alongside
the existing `NAME_PLATE_UNIT_ADDED` / `_REMOVED` diff in the
per-tick nameplate walker.

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

### `UnitNameFromGUID(guid)`

Returns `name, realm` for the player identified by `guid`, or `nil`
if no player with that GUID has been encountered yet. Same lookup
chain as [`GetPlayerInfoByGUID`](#getplayerinfobyguidguid) (engine
NameCache, persistent on-disk fallback when enabled) — just narrower
return shape for callers that only need the name.

```lua
local name, realm = UnitNameFromGUID(UnitGUID("target"))
if name then
    ChatFrame1:AddMessage("Target: " .. name)
end

-- Resolving a chat link
local name, realm = UnitNameFromGUID("0x0000000000000777")
```

`realm` is always `""` in vanilla — the engine doesn't populate
per-player realm names and 1.12 has no cross-realm interaction.
We push the empty string rather than `nil` to match the convention
of the other player-info accessors; addons can gate on `realm == ""`.

Doesn't trigger a network query on miss. To populate an unknown
GUID, either let the engine receive a `SMSG_NAME_QUERY_RESPONSE`
through normal chat/target interaction, or call
[`C_PlayerCache.RememberPlayer`](#c_playercacherememberplayerguid-name-classtoken)
explicitly.

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

## Quest

### `C_QuestLog.GetQuestIDForLogIndex(index)`

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
    local questID = C_QuestLog.GetQuestIDForLogIndex(i)  -- 0 for headers
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

> **`success` is `1` or `nil`.** The engine's printf-style event
> dispatcher has no `%b` token, so we encode the boolean as `1` for
> success / `nil` for failure. `if success then ...` works as expected
> (`nil` falsy, `1` truthy). Same encoding as `ITEM_DATA_LOAD_RESULT`
> and `GET_ITEM_INFO_RECEIVED`.

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

### `C_QuestLog.IsOnQuest(questID)`

Returns `true` if `questID` is currently in the player's quest log
(either still in progress or ready to turn in — both states live in
the log). Returns `false` for invalid input (non-positive, non-number,
or simply not in the log).

```lua
if C_QuestLog.IsOnQuest(215) then
    -- player has "Jungle Secrets" accepted
end
```

Walks the same `VAR_QUEST_LOG_ENTRIES` array
[`C_QuestLog.GetQuestIDForLogIndex`](#c_questlogGetQuestIDForLogIndexindex)
reads and matches against each real entry's questID, skipping the
zone/category header rows.

Cheaper than the engine's `IsUnitOnQuest(logIndex, unit)`, which
requires the addon to walk the log to find the matching index first
and resolves a unit token even when the answer is just about the
player. Same answer in a single call.

### `C_QuestLog.IsUnitOnQuest(unit, questID)`

Returns `true` if the unit has `questID` in their quest list. Modern
arg order — `unit` first, `questID` second — and questID-keyed rather
than log-index-keyed like vanilla's `IsUnitOnQuest(logIndex, unit)`.

```lua
if C_QuestLog.IsUnitOnQuest("party1", 215) then
    -- partymate is also on Jungle Secrets
end
```

For `unit == "player"` equivalent to
[`C_QuestLog.IsOnQuest(questID)`](#c_questlogisonquestquestid). For
other tokens (party/raid members, target), the unit must be in the
engine's sync range — the data comes from `SMSG_QUESTGIVER_QUEST_DETAILS`
broadcasts, which only reach you while the other player is within
the client's sync window. Returns `false` for NPCs, units out of
range, and units that haven't synced their quest list yet.

Returns `false` (no `lua_error`) for invalid input: non-string unit,
non-number / non-positive questID, unresolvable token, or unit that's
not player-controlled (i.e., a creature). The
`UNIT_FLAG_PLAYER_CONTROLLED` gate is mandatory — without it the
`+0xE68` deref would AV on any NPC.

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

### `C_QuestLog.GetQuestDetails(questID)`

Returns a table with every field we know how to read from the engine's
quest static-info cache for `questID`, or `nil` if the data isn't
loaded. One round-trip alternative to calling
`GetTitleForQuestID` / `GetQuestLogRewardMoney` / etc. across each of
its individual accessors. Quest-helper-style addons that want
structured data per quest are the primary consumers.

Like `GetTitleForQuestID`, this is a pure cache probe — pair it with
`C_QuestLog.RequestLoadQuestByID` and listen for
`QUEST_DATA_LOAD_RESULT` if the quest isn't already cached.

#### Returned table

| Field | Type | Notes |
|-------|------|-------|
| `questID` | number | Echo of the input. |
| `title` | string | Locale-applied. |
| `level` | number | Quest level from the static record. |
| `questType` | string \| absent | Localized tag string — `"Elite"`, `"Group"`, `"PvP"`, `"Dungeon"`, `"Raid"` — sourced from `QuestInfo.dbc`. Field is omitted for plain quests with no tag. |
| `rewardMoney` | number | Reward copper. `0` if the quest *requires* money instead. |
| `requiredMoney` | number | Copper the player must hand in to complete (e.g. quartermaster contributions). `0` for quests with no money requirement. |
| `rewardMoneyAtMaxLevel` | number | Vanilla's level-60 reward bonus. Added to `rewardMoney` when the player is level 60; populated even on low-level quests. |
| `rewardSpellID` | number | `spellID` of a spell *taught* on completion (e.g. profession recipes), or `0` for no learned spell reward. |
| `srcItemID` | number | `itemID` the questgiver hands the player on accept (e.g. the sigil in *"Verdant Sigil"* — given on accept, read by the player, then turned back in). `0` = no source item. Same as the quest's `requirements[].id` for "give-then-return" quests. |
| `questFlags` | number | Raw `QUEST_FLAGS_*` bitfield — only bit `0x08` (sharable) is positively confirmed-tested by the vanilla engine; other bits are presumed-stored but unverified. |
| `isSharable` | boolean | Convenience extraction of bit `0x08` from `questFlags`. |
| `description` | string | The questgiver's narrative text. Raw — printf-style `$N` (player name), `$C` (class), `$R` (race) tokens are **not** substituted; use `string.gsub` if you need runtime values. |
| `objectives` | string | The "what you must do" summary text, same raw / un-substituted format as `description`. |
| `completionText` | string | The "now turn it in" text shown on the reward UI panel after objectives are met. Often empty for simple quests; populated for narrative-heavy ones. Modern API name: `GetQuestLogCompletionText`. |
| `poi` | table \| absent | Point-of-interest marker (`{mapID, x, y, opt}`). Set on quests with a server-supplied "go here" location; omitted when `mapID == 0`. |
| `rewardItems` | array of `{id, count}` | Items the quest gives unconditionally on turn-in. Empty when there are no fixed rewards. |
| `choiceItems` | array of `{id, count}` | "Pick one" reward items. Empty when none. |
| `requirements` | array of `{kind, id, count, text}` | Objectives. `kind` is `"monster"` (creature), `"object"` (gameobject), or `"item"` (collect / interact). `text` is the questgiver's per-objective override (e.g. `"Investigate the cave"` instead of the auto-generated `"Mor'shan Bear: 0/8"`); empty string means use the auto-format. Order: NPC/GO objectives first, then item objectives, both 1-indexed. Only non-empty slots included. |

> **Not included** — race, class, skill, time limit, and suggested-player count. Vanilla 1.12's `SMSG_QUEST_QUERY_RESPONSE` doesn't ship those fields; the server enforces them and filters quests *before* broadcasting, so the client only ever sees quests it could accept and never receives the static restriction values. Verified empirically: a race-restricted starter (`3120` Verdant Sigil — Night Elf Druid only), a dungeon quest (`914`), and a timed delivery (`3364` Scalding Mornbrew — 5-min authored timer) all have those cache slots zero-filled in memory. **Addons that need that data must source it from an external scraped database** like pfQuest's.
>
> **Field reliability:** everything currently returned is either confirmed-correct via the engine's own `Script_GetQuestLog*` accessors or empirically verified against in-game quest semantics. The `poi` field is the only remaining hypothesis — its offsets are confirmed but no test quest has yet exercised it.

XP rewards aren't exposed — vanilla 1.12 has no `GetQuestLogRewardXP`,
and per emulator-decoded packet structure, the server doesn't include
an XP field in `SMSG_QUEST_QUERY_RESPONSE`. XP is computed
server-side at turn-in from level-scaling tables.

#### Example

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("QUEST_DATA_LOAD_RESULT")
f:SetScript("OnEvent", function()
    if event == "QUEST_DATA_LOAD_RESULT" and arg2 == 1 then
        local d = C_QuestLog.GetQuestDetails(arg1)
        if d then
            print(d.title, "level", d.level, d.questType or "(no tag)")
            for _, r in ipairs(d.requirements) do
                print("  -", r.kind, r.id, "x", r.count)
            end
        end
    end
end)
C_QuestLog.RequestLoadQuestByID(1467)
```

Sample output for quest 1467 (`"Reagents for Reclaimers Inc."`):

```
Reagents for Reclaimers Inc.  level 40  (no tag)
  - item 6253 x 1
```

with `d.choiceItems = {{id=6793, count=1}, {id=6794, count=1}}`,
`d.rewardMoney = 3500`, `d.rewardMoneyAtMaxLevel = 1920`, etc.

### `C_QuestLog.GetNumQuestObjectives(questID)`

The quest's objective count — exactly the length
`GetQuestDetails(questID).requirements` would have — read straight from
the quest cache with **no Lua tables or strings materialized**. Returns
`nil` when the quest isn't cached (same contract as `GetQuestDetails`).

```lua
local n = C_QuestLog.GetNumQuestObjectives(1467)   -- 1
```

Use this instead of `GetQuestDetails` when only the count matters —
e.g. bounds-checking authored objective indices across every step of a
guide at load time. `GetQuestDetails` copies the quest's description /
objectives / completion text into Lua strings and builds ~20 table
fields per call; harmless one at a time, but a login-path loop over
hundreds of cached quests can put real pressure on vanilla's
fixed-size Lua memory pool. This accessor costs a few dozen byte reads
regardless of cache temperature.

### `C_QuestLog.IsQuestDataCachedByID(questID)`

Returns `true` if the quest's static data is currently in the
client-side quest cache, `false` otherwise. Pure cache probe — no
server query, no side effects. Pair with
`C_QuestLog.RequestLoadQuestByID` when you need to ensure the data
is loaded before checking.

Returns `false` (not error) for invalid input — non-number,
non-positive, or non-existent questIDs all yield `false`, matching
the `C_Item.IsItemDataCachedByID` shape.

```lua
if not C_QuestLog.IsQuestDataCachedByID(questID) then
    C_QuestLog.RequestLoadQuestByID(questID)
    -- listen for QUEST_DATA_LOAD_RESULT, or re-check on a timer
end
```

### `GetQuestLogLeaderBoardID(objectiveIndex [, questIndex])`

Companion to vanilla's `GetQuestLogLeaderBoard` — returns `(id, kind)`
for the same objective the existing call formats text for. `id` is
always positive (the raw `RequiredNPCOrGo` field is signed, but the
sign is folded into `kind` instead); `kind` is one of `"monster"`,
`"object"`, or `"item"`. Returns nothing for: out-of-range
`objectiveIndex`, header rows, quests whose static data hasn't been
cached yet, or event-text-only objectives that have no entry ID.

Arg shape matches the engine's actual `Script_GetQuestLogLeaderBoard`
(its usage string says `(index)` but the function also accepts the
2-arg form): with `questIndex` provided, looks up the entry at
that 1-based quest-log slot; without it, falls back to whichever
quest is currently selected via `SelectQuestLogEntry`.

```lua
-- Selected quest path (pairs with GetQuestLogLeaderBoard's selected mode)
SelectQuestLogEntry(13)
local text, kind, done = GetQuestLogLeaderBoard(1)
local id              = GetQuestLogLeaderBoardID(1)
-- text = "Young Stranglethorn Tiger slain: 4/10"
-- kind = "monster"
-- id   = 2630   (the creature template / entry)

-- Explicit-quest path
local text, kind = GetQuestLogLeaderBoard(1, 13)
local id, k2     = GetQuestLogLeaderBoardID(1, 13)
-- kind == k2 always
```

The ID is the same key `creaturecache.wdb` / `gameobjectcache.wdb` /
`itemcache.wdb` use, so cross-referencing against the existing item
or creature caches is direct — no localized-text parsing required.

Why not just expose this on `GetQuestLogLeaderBoard` itself? Modifying
the engine's existing return tuple would silently break addons that
destructure positionally (`local text, type, done = GetQuestLogLeaderBoard(...)`).
A separate function keeps the existing call wire-compatible.

> **Source**: the cached quest record (returned by `Quest::Cache::Peek`)
> carries the SMSG_QUEST_QUERY_RESPONSE objective arrays inline at
> fixed offsets:
> - `+0x149C` — `int32 RequiredNPCOrGo[4]` (positive = creature entry,
>   negative = gameobject entry)
> - `+0x14AC` — `uint32 RequiredNPCOrGoCount[4]`
> - `+0x14BC` — `uint32 RequiredItem[4]`
> - `+0x14CC` — `uint32 RequiredItemCount[4]`
>
> Iteration mirrors the engine at `0x004E0110`: walk the NPC/GO array
> first, skip zero slots, then the item array. 1-based `objectiveIndex`
> counts only non-empty slots.

## Screen

### `GetPhysicalScreenSize()`

Returns `(widthPixels, heightPixels)` — the display's physical
resolution in pixels. Modern WoW added this as a native in 7.0
(Legion); FrameXML's `PixelUtil` builds its entire pixel↔UI-unit
conversion on it (`768.0 / physicalHeight`), which is what lets addons
snap frames to whole pixels for crisp borders at fractional UI scales.

```lua
local w, h = GetPhysicalScreenSize()   -- e.g. 1920, 1080
```

Vanilla 1.12 has no such native — and no pre-parsed pixel-dimension
global. `GetScreenWidth()`/`GetScreenHeight()` return UI-space units
(derived from the UIParent aspect ratio at `[UIParent + 0x7c]`), not
pixels. The engine only ever holds the current mode as the
`gxResolution` CVar string (`"WIDTHxHEIGHT"`); both `SetScreenResolution`
(`0x0048bfd0`) and the display-init path sscanf it on demand rather
than caching dimensions anywhere. So we read the `gxResolution` CVar
directly through the engine's by-name lookup (`FUN_FIND_CVAR`, value at
`+0x20`) and parse it — the same source of truth the engine uses.

Returns `1024, 768` (→ factor 1.0, making PixelUtil a 1:1 no-op) when
the CVar is missing or unparseable. Caveat: in windowed mode
`gxResolution` reflects the configured render resolution, which may
differ from the OS window's client size — vanilla exposes no separate
window-pixel query.

The companion `PixelUtil` table (`PixelUtil.SetPoint`/`SetSize`/
`SetStatusBarValue`/…) is provided Lua-side by the ClassicAPI addon and
consumes this function.

## Spell

> **All `C_Spell.*` functions in this section accept any spell
> identifier**, not just a numeric `spellID`. The first argument can be:
>
> - **number** — spellID, used directly.
> - **`|Hspell:N|h` hyperlink** — extracted by parsing for the
>   embedded `spell:N` payload, so
>   `C_Spell.X(GetSpellLink(id))` round-trips cleanly.
> - **spell name** — case-sensitive lookup through the engine's name
>   resolver (`FUN_RESOLVE_SPELL_NAME_TO_BOOK_ID`, the same chain
>   `CastSpellByName` uses). Tolerates `(Rank N)` suffixes via the
>   engine's parser. Bounded to spells in the **player's known
>   spellbook** — not arbitrary Spell.dbc rows.
>
> Unrecognized identifiers (garbage strings, nil, tables, etc.) return
> `nil` rather than raising a Lua usage error — matches modern WoW's
> permissive `C_Spell.*` convention. Function entries below write
> `(spellID)` in the signature for brevity but the broader shape is
> accepted everywhere.
>
> The legacy globals in this section (`GetSpellInfo`, `IsPassiveSpell`,
> `IsHarmfulSpell`, `IsUsableSpell`, etc.) keep their existing
> `(slot, bookType)` overload — only the `C_Spell.*` namespace is
> broadened.

### `C_Spell.DoesSpellExist(spellID)`

Returns `true` if `spellID` resolves to a populated `Spell.dbc`
record, `false` otherwise. Cheap pre-flight check for any of the
spellID-keyed `C_Spell.*` calls — if this returns `false`, the rest
of the family will return nil/empty.

```lua
if C_Spell.DoesSpellExist(133) then
    local info = C_Spell.GetSpellInfo(133)  -- safe
end
```

### `C_Spell.GetSchoolString(schoolMask)`

Returns the localized name for a damage-school bitmask. Resolves
single-bit masks via the engine's `SPELL_SCHOOL<n>_CAP` global
strings (`SPELL_SCHOOL0_CAP` = "Physical", `SPELL_SCHOOL2_CAP` =
"Fire", etc.).

```lua
C_Spell.GetSchoolString(4)    -- "Fire"
C_Spell.GetSchoolString(32)   -- "Shadow"
```

Vanilla spells are single-school — the multi-school combos
("Frostfire", "Shadowflame", "Spellstorm", etc.) were TBC-and-later
additions. Any multi-bit mask returns `"Unknown"`, matching the
modern API's documented fallback for unnamed combinations.

> Lua 5.0 doesn't support hex literals (`0x04` is a syntax error);
> pass mask values as decimals, or via `tonumber("0x04", 16)` if you
> want to keep hex notation in source.

| Decimal | Hex | School |
|--------:|-----|--------|
| `1` | `0x01` | Physical |
| `2` | `0x02` | Holy |
| `4` | `0x04` | Fire |
| `8` | `0x08` | Nature |
| `16` | `0x10` | Frost |
| `32` | `0x20` | Shadow |
| `64` | `0x40` | Arcane |

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

### `C_Spell.GetSpellName(spellID)`

Returns the localized name of `spellID`, or `nil` if the spell ID is out
of range or has no name in the current locale. Convenience accessor for
the `name` field of [`C_Spell.GetSpellInfo`](#c_spellgetspellinfospellid)
when that's all you need — single field read, no DBC indirection beyond
the locale lookup.

```lua
local name = C_Spell.GetSpellName(133)  -- "Fireball"
```

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
for slot = 1, 1000 do
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

> **No caster scaling.** Values reflect the spell's base rank — caster
> level / spell power / talents are not applied. Modern WoW behaves the
> same way when called outside a unit context. If you need the
> "currently displayed" tooltip text with caster scaling, use
> `GameTooltip:SetSpellByID` and read line strings from there.

### `C_Spell.GetSpellMechanicByID(spellID)`

Returns the spell's mechanic as `(mechanicID, name)` — the standard WoW
`SpellMechanic` ID and its English name. Works for **any** `spellID`, not
just spells the player has learned.

Returns:
- nothing (`nil`) if the spell ID is invalid / out of range
- `(0, nil)` for a known spell that carries no mechanic
- `(mechanicID, name)` otherwise

The `name` is always the **enUS** string (locale-independent), because the
mechanic name is a stable token consumers string-match against
(`"rooted"`, `"stunned"`, `"polymorphed"`, …) — returning a localized
value would break that matching across clients. It is `nil` only when the
mechanic has no `SpellMechanic.dbc` record or no English name; the numeric
ID is still returned in that case.

```lua
C_Spell.GetSpellMechanicByID(118)  -- Polymorph        → 17, "polymorphed"
C_Spell.GetSpellMechanicByID(339)  -- Entangling Roots  → 7,  "rooted"
C_Spell.GetSpellMechanicByID(133)  -- Fireball          → 0,  nil
```

This reads `Spell.dbc`'s `Mechanic` field and resolves the name from
`SpellMechanic.dbc` directly — both DBCs are resident from boot, so the
call is synchronous with no caching or network round-trip (unlike item
data). It replaces hand-maintained `spellID → mechanic` lookup tables that
addons otherwise keep because vanilla exposes no Lua reader for the field
— e.g. crowd-control macro conditionals (`[cc:stun]`, `[cc:fear]`) that
need to know which mechanic a debuff applies.

The complete `SpellMechanic.dbc` set on 1.12 — all 27 rows (the `name`
column is exactly what this function returns):

| ID | name | ID | name | ID | name |
|----|------|----|------|----|------|
| 1 | `charmed` | 10 | `asleep` | 19 | `shielded` |
| 2 | `disoriented` | 11 | `ensnared` | 20 | `shackled` |
| 3 | `disarmed` | 12 | `stunned` | 21 | `mounted` |
| 4 | `distracted` | 13 | `frozen` | 22 | `seduced` |
| 5 | `fleeing` | 14 | `incapacitated` | 23 | `turned` |
| 6 | `clumsy` | 15 | `bleeding` | 24 | `horrified` |
| 7 | `rooted` | 16 | `healing` | 25 | `invulnerable` |
| 8 | `pacified` | 17 | `polymorphed` | 26 | `interrupted` |
| 9 | `silenced` | 18 | `banished` | 27 | `dazed` |

> **No mechanic `30`.** The table tops out at `27`. Sap and Gouge report
> `14` (incapacitated) in 1.12 — the `30` ("sapped") value used by some
> addon tables is a later-expansion addition and has no row here.

### `C_Spell.GetSpellRadius(spellID)` / `GetSpellRadius(slot, bookType)`

Returns `(baseRadius, modifiedRadius)` — the spell's AOE radius in yards,
or `nil` for a non-AOE spell (no effect carries a radius). Works for any
spell, not just ones the player has learned.

- **`baseRadius`** — the raw `SpellRadius.dbc` value; caster-independent,
  what an AOE tracker watching other units wants.
- **`modifiedRadius`** — `baseRadius` with the **local player's** talent /
  item modifiers applied (e.g. a Mage's Arctic Reach). Equal to
  `baseRadius` when the player has no matching modifier.

```lua
C_Spell.GetSpellRadius(122)    -- Frost Nova → 10, 10  (no Arctic Reach)
C_Spell.GetSpellRadius(122)    -- Frost Nova → 10, 12  (with Arctic Reach r2)
C_Spell.GetSpellRadius(133)    -- Fireball   → nil      (single-target)
GetSpellRadius(10, "spell")    -- 10th player-book slot, by spellbook position
```

Two signatures matching the dual-signature shape used elsewhere in this
backport (`GetSpellInfo`, `SpellHasRange`, …): the namespaced form takes
a modern spell *identifier* (numeric ID or name); the bare global takes
the vanilla `(slot, bookType)` spellbook position, where `bookType`
follows the `GetSpellName` convention — `"pet"` → pet book, `"spell"`
(or any non-pet / omitted value) → player book.

`baseRadius` reads `Spell.dbc`'s per-effect `EffectRadiusIndex[3]`
(`+0x160`) and resolves each into `SpellRadius.dbc`; since a spell's
effects can each carry their own radius, the **largest** base radius
across the three effects is returned.

> **No caster-level scaling.** The engine's internal radius helper also
> adds `radiusPerLevel * casterLevel`, but that needs a unit context;
> like modern `GetSpellRadius` this reports the base value. Talent/item
> modifiers (the `modifiedRadius` return) **are** applied, via the
> engine's SpellMod system — but only the **local player's**, since the
> client tracks spell mods for the player alone (there's no way to know
> another caster's talents). All DBCs / mod tables are resident from
> boot, so the call is synchronous with no caching.

### `C_Spell.GetSpellPowerCost(spellIdentifier)`

Returns an array of `SpellPowerCostInfo` tables, or `nil` if the spell
isn't found or has no resource cost. Vanilla spells have exactly one
power cost, so the array holds at most one entry.

```lua
C_Spell.GetSpellPowerCost(10187)   -- Blizzard, Mage with 3/3 Frost Channeling
-- { [1] = { type = 0, name = "MANA", cost = 1190, minCost = 1190,
--           costPercent = 0, costPerSec = 0, requiredAuraID = 0,
--           hasRequiredAura = false } }
-- base cost 1400, 15% reduction applied -> 1190
```

| Field | Meaning |
|-------|---------|
| `type` | `Enum.PowerType` — `0` mana, `1` rage, `2` focus, `3` energy, `4` happiness (the spell's `PowerType`) |
| `name` | power token (`"MANA"`, `"RAGE"`, …) |
| `cost` | **effective** cost for the local player |
| `minCost` | `== cost` (vanilla has no optional-cost component) |
| `costPercent` | the spell's `%`-of-base-resource cost, or `0` for a flat cost |
| `costPerSec` | `0` (vanilla doesn't expose a per-second channel cost in these terms) |
| `requiredAuraID` | `0` (no form/aura-conditional costs in 1.12) |
| `hasRequiredAura` | `false` |

`cost` is the value the engine actually charges — base + level scaling +
`ManaCostPercent`-of-resource + descriptor power-cost mods + the cost
SpellMod (talents like Frost Channeling). It comes from the engine's own
cost helper (`FUN_006e31b0`), so it stays in lockstep with what casting
the spell deducts.

> **Effective vs. base.** This returns the player-modified cost.
> [`C_Spell.GetSpellInfo`](#c_spellgetspellinfospellid)'s `cost` field is
> the **base** `ManaCost` (intrinsic spell data, caster-independent) —
> same base/modified split as
> [`GetSpellRadius`](#c_spellgetspellradiusspellid--getspellradiusslot-booktype).

### `C_Spell.GetSpellReagents(spellID)`

Returns the spell's reagent list as an array of
`{ itemID = N, count = M }` tables, one entry per non-empty reagent
slot. Returns an empty array `{}` for spells that consume no
reagents, or `nil` for invalid spell IDs.

```lua
local list = C_Spell.GetSpellReagents(23028)  -- Arcane Brilliance
-- list = { { itemID = 17020, count = 1 } }   -- 1× Arcane Powder

-- Crafting recipe:
local list = C_Spell.GetSpellReagents(2538)
-- Cooking-style: { { itemID = ..., count = ... }, ... }
```

Reads directly from `Spell.dbc` at the documented reagent offsets
(`+0xA8` for itemIDs, `+0xC8` for counts — verified empirically
from the engine's tooltip-builder; the CMaNGOS-style schema
documented in some emulator sources places these at `+0x110` /
`+0x130`, which is wrong for 1.12.1). Iteration stops at the first
empty slot, matching how the engine walks its own reagent loop.

### `C_Spell.GetSpellSubtext(spellIdentifier)`

Returns the localized "Rank N" / "Passive" / "Racial Passive" /
similar string that appears below the spell's name in the
spellbook. Read directly from `Spell.dbc.Rank[9]` at record `+0x204`
(same locale array shape as `Name[9]` two fields prior).

Accepts the modern `SpellIdentifier` shape — `spellID`, name, name
with rank (`"Fireball(Rank 3)"`), or `|Hspell:N|h` hyperlink. Returns
`nil` for unrecognized input or spells with no rank text.

```lua
C_Spell.GetSpellSubtext(133)        -- "Rank 1"   (lowest-rank Fireball)
C_Spell.GetSpellSubtext(25306)      -- "Rank 12"  (max-rank Fireball)
C_Spell.GetSpellSubtext("Frost Armor")  -- "Rank 1"
```

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

### `C_Spell.IsSpellPassive(spellID)`

Modern table-namespace form of [`IsPassiveSpell`](#ispassivespellspellid--ispassivespellslot-booktype).
Same return semantics; doesn't accept the legacy
`(slot, bookType)` shape.

```lua
C_Spell.IsSpellPassive(6603)   -- true (Auto Attack)
```

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

### `C_Spell.GetSpellCooldown(spellIdentifier)`

Returns a `SpellCooldownInfo` table for the given spell, or `nil` if
the identifier doesn't resolve to a `Spell.dbc` row. Modern
table-shape variant of vanilla's `GetSpellCooldown(slot, bookType)` —
accepts a spellID directly without forcing the caller to resolve a
spellbook slot first.

```lua
local info = C_Spell.GetSpellCooldown(1953)   -- Blink
-- on cooldown:   { startTime=728565.79, duration=15, isEnabled=true, modRate=1 }
-- off cooldown:  { startTime=0,         duration=0,  isEnabled=true, modRate=1 }
```

| Field | Type | Notes |
|-------|------|-------|
| `startTime` | number | Engine tick count (seconds) when the cooldown began. Same epoch as `GetTime()`, so `info.startTime + info.duration` is the absolute time the cooldown ends. `0` when no cooldown active. |
| `duration` | number | Cooldown length in seconds; `0` when no cooldown active. |
| `isEnabled` | boolean | `false` when the cooldown is "on hold" (e.g. some channeled abilities). `true` for normal cooldowns. |
| `modRate` | number | Always `1.0` — vanilla has no haste-on-cooldown mechanic. |

Modern-only fields (`activeCategory`, `timeUntilEndOfStartRecovery`,
`isOnGCD`) read as `nil` since they have no vanilla source.

Returns `nil` if the resolved spellID is `0` or doesn't have a
`Spell.dbc` row.

### `C_Spell.IsCurrentSpell(spellIdentifier)`

Returns `true` if the spell is currently being cast, queued mid-GCD,
or channeled by the player; `false` otherwise (including for
unresolvable identifiers).

```lua
C_Spell.IsCurrentSpell(11366)              -- true while casting Pyroblast
C_Spell.IsCurrentSpell("Pyroblast")        -- same, via name
C_Spell.IsCurrentSpell(GetSpellLink(7620)) -- true while channeling Fishing
```

Useful for action-bar addons that want to highlight the active /
queued button — modern action bars gate the "currently casting" glow
on this. Reads three engine slots and returns true on any match:

- `VAR_CURRENT_CAST_SPELL` (`0x00CECA88`) — cast-bar spellID. Written
  by `FUN_006E4AD0` when a new cast begins; cleared on cast end.
- `VAR_QUEUED_CAST_SPELL` (`0x00CECAA8`) — spell that was active when
  a new cast superseded it mid-GCD. Restored to current when the new
  cast ends, so checking it here covers the "queued to cast next"
  half of modern's documented semantics.
- `UNIT_FIELD_CHANNEL_SPELL` (descriptor `+0x228`) on the player —
  covers channeled abilities (Fishing, Drain Soul, Ritual of
  Summoning, etc.).

### `C_Spell.IsSelfBuff(spellID)`

Returns `true` if every active effect on the spell targets only the
caster — every effect's implicit target (A and B) must be either
`TARGET_NONE` (0) or `TARGET_SELF` (1).

```lua
C_Spell.IsSelfBuff(1006)   -- Inner Fire        → true
C_Spell.IsSelfBuff(7302)   -- Frost Armor       → true
C_Spell.IsSelfBuff(1459)   -- Arcane Intellect  → false (targets friendly)
C_Spell.IsSelfBuff(133)    -- Fireball          → false (targets enemy)
```

Walks `Spell.dbc.EffectImplicitTargetA[3]` and
`EffectImplicitTargetB[3]`. Spells with no active effects (a
degenerate case for normal content) return `false`.

### `C_Spell.SpellHasRange(spellIdentifier)` / `SpellHasRange(slot, bookType)`

Returns `true` if the spell has a non-zero min or max range — i.e.
it's a targeted ability you cast at a unit / location rather than a
self-cast (which has range `0`).

```lua
C_Spell.SpellHasRange(133)    -- Fireball       → true (40yd)
C_Spell.SpellHasRange(1006)   -- Inner Fire     → false (self buff)
SpellHasRange(10, "spell")    -- 10th spellbook slot in player book
```

Looks up `Spell.dbc.RangeIndex` (`+0x90`) → `SpellRange.dbc` row,
then tests `minRange > 0 or maxRange > 0`. Vanilla 1.12 doesn't ship
this function at all; 3.3.5+ added it as the `(spellIdentifier)`
form only. We expose both the modern namespaced form (any
`SpellIdentifier`) and a positional `SpellHasRange(slot, bookType)`
matching the dual-signature shape used elsewhere in this backport
(`GetSpellInfo`, `GetSpellLink`, etc.). The `bookType` argument
follows the same convention as `GetSpellName(slot, bookType)`:
`"spell"` (or any non-pet value) → player book, `"pet"` → pet book.

### `C_Spell.IsAutoAttackSpell(spellID)`

Returns `true` if the spell is the melee auto-attack — spell ID
`6603` ("Auto Attack"), used by every class.

```lua
C_Spell.IsAutoAttackSpell(6603)              -- true
C_Spell.IsAutoAttackSpell(133)               -- false
```

See [`C_SpellBook.IsAutoAttackSpellBookItem`](#c_spellbookisautoattackspellbookitemslot-booktype)
for the spellbook-slot variant.

### `C_Spell.IsRangedAutoAttackSpell(spellID)`

Returns `true` if the spell is one of the two ranged auto-attacks —
spell `75` (Hunter "Auto Shot") or spell `5019` ("Shoot" wand).

```lua
C_Spell.IsRangedAutoAttackSpell(75)          -- true (Auto Shot)
C_Spell.IsRangedAutoAttackSpell(5019)        -- true (Shoot wand)
C_Spell.IsRangedAutoAttackSpell(6603)        -- false (melee)
```

Tests `Spell.dbc.Attributes & 0x02` (the `SPELL_ATTR_AUTO_REPEAT`
flag) — verified empirically against in-game spell data. Auto Shot
(`0x00050012`) and Shoot wand (`0x00000012`) share bit 1; Heroic
Strike (`0x00050014`) and other ranged abilities don't. Naturally
covers any future auto-repeating spell a private server might add.

See [`C_SpellBook.IsRangedAutoAttackSpellBookItem`](#c_spellbookisrangedautoattackspellbookitemslot-booktype)
for the spellbook-slot variant.

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

### `C_Item.GetEnchantInfo(enchantID)`

Resolves an item-enchantment ID — the `enchantID` returned by
[`C_Item.GetWeaponEnchantInfo`](#c_itemgetweaponenchantinfo) for a
weapon's temporary enchant, and the same IDs item permanent enchants
use — into a table:

```lua
local info = C_Item.GetEnchantInfo(enchantID)
-- info.enchantID = <id>
-- info.name      = "Crusader"        -- localized display name
-- info.effects   = { {type=1, amount=0, arg=20007} }
-- info.spellID   = 20007             -- spell-type enchants only
```

```lua
-- A proc/equip enchant → chain its spellID into C_Spell:
local info = C_Item.GetEnchantInfo(1900)          -- "Crusader"
if info.spellID then
    print(C_Spell.GetSpellDescription(info.spellID))  -- the proc's text
end
```

`effects` is an array of the record's non-empty effect slots, each
`{ type, amount, arg }` where `type` is the standard
`ITEM_ENCHANTMENT_TYPE`:

| type | meaning | carries |
|------|---------|---------|
| 1 | combat-proc spell | `arg` = spellID |
| 2 | weapon damage | `amount` = +damage |
| 3 | equip spell / aura | `arg` = spellID |
| 4 | resistance / armor | `amount` = +value |
| 5 | stat | `arg` = stat index, `amount` = value |
| 6 | totem | — |
| 7 | use spell | `arg` = spellID |

For spell types (1/3/7) `arg` is a spellID feedable into
[`C_Spell.GetSpellInfo`](#getspellinfospellid--getspellinfoslot-booktype) /
`GetSpellDescription`; the first such id is also surfaced at top level
as `spellID` for convenience (absent for non-spell enchants like
sharpening stones).

Returns `nil` for a non-numeric / non-positive id, an out-of-range
id, or a record with no name.

Reads `SpellItemEnchantment.dbc` (records `0x00C0D7D8`, count
`0x00C0D7DC`) — the 24-column table every enchant ID indexes:
`Type[3]@+0x04`, `Amount[3]@+0x10`, `EffectArg[3]@+0x28`,
`Name[8]@+0x34` (locale-indexed). The layout was verified by parsing
the on-disk DBC against known records (Crusader 1900 → type 1, arg
20007; Sharpened +3 → type 2, amount 3). Fully resident from boot, so
it answers for any enchant ID with no caching or round-trip.

Lives in `C_Item` (not `C_Spell`) because the id originates from
`C_Item.GetWeaponEnchantInfo` and the concept is an item enchantment —
`SpellItemEnchantment` is just the DBC's internal name (enchants are
*implemented* via spell effects).

> **Not derivable: the source item.** The enchant record holds no
> back-reference to the item that applied it, and vanilla has no
> client-side reverse index (enchantID → item). Finding it would need
> an external scraped DB (pfQuest/Questie-style).

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

### `C_Spell.IsSpellHarmful(spellID)` / `C_Spell.IsSpellHelpful(spellID)`

Same classification logic as the globals above, exposed in the modern
`C_Spell` namespace. Don't accept the legacy `(slot, bookType)`
shape.

```lua
C_Spell.IsSpellHarmful(133)     -- true (Fireball)
C_Spell.IsSpellHelpful(2061)    -- true (Flash Heal)
```

Equivalent to `C_Spell.IsSpellHarmful` / `C_Spell.IsSpellHelpful`
introduced in 11.x.

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

### `C_Spell.CastAtCursor(spellID)`

Casts a ground-target spell at the player's current cursor world
position, bypassing the manual click on the AoE reticle the engine
would otherwise require. ClassicAPI's analog of modern's
`/cast [@cursor] Blizzard`. Returns `true` when the cursor-placement
leg landed; `false` for non-ground-target spells (cast still fires
normally on the current target), unknown spellIDs, cursor over UI /
off-screen, etc.

```lua
C_Spell.CastAtCursor(10)    -- Blizzard rank 1 at cursor
C_Spell.CastAtCursor(2120)  -- Flamestrike rank 1 at cursor
```

Two-stage internally:

1. Initiates the cast through the same path
   [`CastSpellByName`](docs/API.md#numeric-spellids-in-cast-and-castspellbyname)
   uses (`FUN_RESOLVE_SPELL_NAME_TO_SLOT` + the engine's cast
   dispatcher). If the spell needs a ground target, the engine
   enters placement mode and arms the AoE reticle.

2. Refreshes the cursor's screen→world raycast via
   `FUN_REFRESH_CURSOR_RAYCAST` (the same internal the engine fires
   on every mouse move when placement is active), confirms the
   cursor hit terrain (not a UI frame or a unit), and commits
   placement via `FUN_COMMIT_PLACEMENT_COORDS` with the resolved
   `(x, y, z)`. Cancels placement cleanly via `FUN_STOP_PLACEMENT`
   when the cursor isn't on terrain or when the spell wanted a
   unit-target rather than ground.

For non-placement spells (regular target casts, self-buffs, etc.)
the cast fires normally; the placement-resolve no-ops since the
engine never set the placement flag, and we return `false`.

The companion item version is
[`C_Item.UseAtCursor`](#c_itemuseatcursoriteminfo) — same chain via
the item-use path for grenades / on-use ground-target items.

### `C_Spell.CastAtUnit(spellID, unit)`

Casts a ground-target spell at `unit`'s feet, bypassing the AoE
reticle click. ClassicAPI's analog of modern's
`/cast [@unit] Flamestrike` — `CastAtUnit(spellID, "player")` drops it
at the player's position, `"target"` at the target's, and so on for
any unit token (`"mouseover"`, `"party1"`, `"raid7"`, …). Returns
`true` when the placement landed at the unit; `false` for
non-ground-target spells (cast still fires normally), unknown
spellIDs, or a unit with no resolvable position.

```lua
C_Spell.CastAtUnit(2120, "target")   -- Flamestrike at the target's feet
C_Spell.CastAtUnit(10, "player")     -- Blizzard centered on yourself
```

Same two-stage shape as
[`C_Spell.CastAtCursor`](#c_spellcastatcursorspellid), but instead of
the cursor raycast, step 2 reads the unit's world position from its
`GetPosition` virtual (the same one `CheckInteractDistance` /
`UnitInRange` use) and commits placement there via
`FUN_COMMIT_PLACEMENT_COORDS`. The unit is resolved *before* the cast
is dispatched, so an absent/invalid unit fails cleanly without
leaving the engine armed in placement mode. A genuinely unrecognized
unit-token string raises the engine's standard "Unknown unit" error,
matching `UnitHealth("garbage")`.

The companion item version is
[`C_Item.UseAtUnit`](#c_itemuseatunititeminfo-unit).

### `C_Spell.CancelSpellByID(spellID)` / `CancelSpellByName(name)`

Cancel a buff on the player by spellID or by spell name. Modern WoW's
`CancelUnitBuff` reduces to these two primitives internally; both are
player-only (vanilla server only accepts `CMSG_CANCEL_AURA` for the
local player's own auras).

```lua
C_Spell.CancelSpellByID(2378)      -- Cancel "Find Herbs"
CancelSpellByName("Cat Form")       -- Drop shapeshift
```

| Function | Arg | Behavior |
|----------|-----|----------|
| `C_Spell.CancelSpellByID(spellID)` | spellID (number) | Sends `CMSG_CANCEL_AURA` for `spellID` directly. No client-side check that the buff is actually present — server validates and silently no-ops if not. |
| `CancelSpellByName(name)` | name (string, case-insensitive) | Walks the player's buff range (`UnitBuff` slots 0..31) for the first aura whose Spell.dbc localized name matches, then cancels by its spellID. Buff-only — debuffs are skipped. |

Calls `FUN_006E7040` (`0x006E7040`) directly with the spellID. This
**bypasses** `Script_CancelPlayerBuff`'s client-side gates — the
per-buff cancelable flag at `[entry+0x0A] & 0x01` and the fallback
`AttributesEx & 0x04` check on the spell record. Trade-off: the server
is now the sole authority on what's cancelable. Non-cancelable auras
(proc-buffs, certain world buffs) still get rejected — just server-side
instead of client-side, which is the same effective behavior as
modern WoW's `C_Spell.CancelSpellByID`.

For invalid input (non-positive spellID, OOR, or a Spell.dbc empty
slot), both functions silently no-op. No `lua_error`, no event fired.

Spell name matching is case-insensitive via `_stricmp`. Rank-suffixed
input (e.g. `"Power Word: Fortitude(Rank 4)"`) is **not** parsed —
pass the plain name. Multi-rank buffs match on first-found; if you
have multiple ranks of the same buff active (unusual but possible with
e.g. paladin blessings before talent merge), the first slot wins.

### `C_Spell.UnitCastingInfo(unit)` / `C_Spell.CastingInfo()`

Returns the unit's in-progress **regular cast** (not a channel), or
`nil` if it isn't casting. `C_Spell.CastingInfo()` is
`C_Spell.UnitCastingInfo("player")` without the unit-token lookup.

> **Under `C_Spell`, not the global `UnitCastingInfo`.** Modern WoW
> exposes these as globals, but a global here would clobber addons that
> ship their own vanilla cast-tracking via the `_G.UnitCastingInfo or
> <fallback>` idiom — e.g. ShaguTweaks' `libcast` scrapes the combat
> log for **remote/enemy** casts the 1.12 engine never exposes. If we
> occupied the global, those addons would adopt our player-only version
> and lose their (superior, for that case) fallback. So we cede the
> global names and register under `C_Spell` instead. Same for
> `UnitChannelInfo`/`ChannelInfo`.

```
name, displayName, textureID, startTimeMs, endTimeMs, isTradeskill,
  castID, notInterruptible, castingSpellID, castBarID, delayTimeMs
    = C_Spell.UnitCastingInfo(unit)
```

```lua
-- mid-cast:
C_Spell.UnitCastingInfo("player")
-- "Conjure Food", "Conjure Food", "Interface\\Icons\\INV_Misc_Food_...",
--  907928268, 907931268, false, nil, false, 28612, nil, 0
```

`startTimeMs`/`endTimeMs` share `GetTime()`'s epoch (`GetTime()*1000`),
so progress is `(GetTime()*1000 - startTimeMs) / (endTimeMs - startTimeMs)`.

> **Works for any unit.** Vanilla 1.12 stores no cast times on the
> CGUnit (the cast bar is Lua-driven off `SPELLCAST_START`), so we
> source them two ways: the **local player** is self-tracked on a
> per-frame tick (`VAR_CURRENT_CAST_SPELL`, stamping `startTimeMs = now`,
> `endTimeMs = now + effective cast time`); **other units** come from a
> co-hook on `SMSG_SPELL_START` (`Spell::Cast`), the one packet carrying
> another unit's cast with a server-authoritative cast time, cached per
> caster GUID. So `C_Spell.UnitCastingInfo("target")` now backs
> enemy/target/focus/nameplate cast bars. Instant casts return `nil` (no
> cast bar). Aborted remote casts (interrupt, death, movement, cancel)
> clear the same tick — a co-hook on the engine's own
> `CGUnit_C::ClearCastingSpell` (the choke point every "unit stopped
> casting" path funnels through) evicts the cached cast the moment the
> engine stops the unit's cast visual, backstopped by co-hooks on the
> `SMSG_SPELL_FAILURE` / `SMSG_SPELL_FAILED_OTHER` packets for servers
> that broadcast them (Turtle's core doesn't — its interrupt propagation
> reaches observers through SuperWoW, which the choke-point hook catches
> too). Verified: Kick / Earth Shock / Counterspell each clear the
> victim's bar, while non-interrupting damage doesn't false-clear it.
> Best-effort for remote units, in three ways: only casts that began while
> the unit was in range are seen; the remote `startTimeMs`/`endTimeMs` are
> shifted later by roughly your latency (1.12's `SMSG_SPELL_START` carries
> only a single `castTime`, so we stamp `start = now, end = now + castTime`
> on receipt — 3.3.5+ avoids the skew because its packet carries *both*
> total and remaining cast time and back-dates `start = now − elapsed`;
> the 1.12 packet has no field to recover `elapsed` from); and remote
> **pushback is invisible** — the server sends `SMSG_SPELL_DELAYED` only
> to the affected caster, so another unit's bar can't stretch when they
> take damage. The **local player is unaffected** by all of this — its
> cast is detected client-side at the moment of cast, no packet involved,
> and its pushback is tracked.
>
> `isTradeskill` is real — the spell's `SPELL_ATTR_TRADESPELL` flag, so
> profession recipe casts (smelting, cooking/enchanting recipes, …)
> return `true` while regular spells return `false`. `endTimeMs` reflects
> the **effective** cast time (the engine's own cast-time helper: base +
> level scaling + cast-time talents like Improved Frostbolt + the
> haste/cast-speed multiplier), so it matches the in-game cast bar — e.g.
> a Mage's talented Frostbolt reads 2.5s, not the 3.0s base. Fields
> vanilla can't fill are structurally-correct placeholders:
> `castID`/`castBarID` = `nil`, `notInterruptible` = `false`,
> `delayTimeMs` = `0`.

### `C_Spell.UnitChannelInfo(unit)` / `C_Spell.ChannelInfo()`

Returns the unit's in-progress **channel**, or `nil` if it isn't
channeling. `C_Spell.ChannelInfo()` is `C_Spell.UnitChannelInfo("player")`
without the token lookup. (Under `C_Spell` rather than the global name
for the same reason as
[`C_Spell.UnitCastingInfo`](#c_spellunitcastinginfounit--c_spellcastinginfo).)

```
name, displayName, textureID, startTimeMs, endTimeMs, isTradeskill,
  notInterruptible, spellID = C_Spell.UnitChannelInfo(unit)
```

```lua
C_Spell.ChannelInfo()
-- "Blizzard", "Blizzard", "Interface\\Icons\\Spell_Frost_IceStorm",
--  907894314, 907902314, false, false, 10187
```

The channeled spellID **is** broadcast for every unit
(`UNIT_FIELD_CHANNEL_SPELL`), so `C_Spell.UnitChannelInfo("target")` works
on other units. Timing now comes from the same `SMSG_SPELL_START` co-hook
as casts: when we observed the channel begin, the remote unit returns full
**start/end times** (validated against the live `UNIT_FIELD_CHANNEL_SPELL`
so a stale cache entry never applies to a different/ended channel);
otherwise it falls back to `name`/`displayName`/`textureID`/`spellID` with
**`nil` times**. The player path is unchanged (full timing). Same
placeholder fields as `C_Spell.UnitCastingInfo`.

## SpellBook

Functions keyed on the player or pet **spellbook** (slot indices,
learn requirements, skill-tab membership). The underlying `Spell.dbc`
data is exposed through the [Spell](#spell) section; what lives here
is the spellbook-as-a-collection layer.

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

### `C_SpellBook.IsAutoAttackSpellBookItem(slot, bookType)`

Spellbook-slot variant of
[`C_Spell.IsAutoAttackSpell`](#c_spellisautoattackspellspellid). Resolves
the spellbook slot to its spellID first, then tests `== 6603` (melee
auto-attack).

```lua
C_SpellBook.IsAutoAttackSpellBookItem(1, "spell")
```

### `C_SpellBook.IsRangedAutoAttackSpellBookItem(slot, bookType)`

Spellbook-slot variant of
[`C_Spell.IsRangedAutoAttackSpell`](#c_spellisrangedautoattackspellspellid).
Resolves the slot to a spellID, returns `true` if it's `75` (Auto
Shot) or `5019` (Shoot wand).

```lua
C_SpellBook.IsRangedAutoAttackSpellBookItem(10, "spell")
-- true if slot 10 is your wand Shoot
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

### `Dismount()`

Dismisses the player's currently summoned mount. No-op when the
player isn't mounted.

```lua
if IsMounted() then Dismount() end
```

Walks the player's buff range for an aura whose `Spell.dbc` effects
include `SPELL_AURA_MOUNTED` (`78`), then sends `CMSG_CANCEL_AURA` for
that spellID via the same direct sender [`C_Spell.CancelSpellByID`](#c_spellcancelspellbyidspellid--cancelspellbynamename) uses. By the
time the server processes the cancel, `IsMounted()` will return
`false`.

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
`C_Spell.CastingInfo()` returns nothing — so addons that want to react to
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

### `IsInGroup()`

Returns `true` if the player is in a party or a raid, `false`
otherwise. Modern shortcut over stock 1.12's
`GetNumPartyMembers() > 0 or GetNumRaidMembers() > 0`.

Accepts an optional `groupType` argument (modern
`Enum.PartyCategory.Home` / `Instance`). Vanilla has no LFG / LFD
instance-group concept, so the argument is accepted and ignored.

```lua
if IsInGroup() then
    SendChatMessage("ready check", "PARTY")
end
```

### `IsInRaid()`

Returns `true` if the player is in a raid (specifically — parties
return `false`), `false` otherwise. Same `groupType` argument story
as `IsInGroup`.

```lua
local channel = IsInRaid() and "RAID" or "PARTY"
SendChatMessage("ready check", channel)
```

### `GetMirrorTimerInfo(index)` / `GetMirrorTimerProgress(label)`

Modern (3.0+) readers for the BREATH (drowning) / EXHAUSTION
(off-map) / FEIGNDEATH (hunter Feign Death) bar state. The vanilla
engine fires the `MIRROR_TIMER_START` / `_PAUSE` / `_STOP` events
with the full packet payload but doesn't cache anything internally;
ClassicAPI hooks the SMSG handler at `0x005E7990` and builds a
3-slot side cache so these accessors work.

`GetMirrorTimerInfo(index)` — `index` is 1, 2, or 3 (slot id). Returns
`(timer, value, maxValue, scale, paused, label)` for an active slot,
or nothing if that slot is empty:

| Return | Type | Meaning |
|--------|------|---------|
| `timer` | string | Engine type-name: `"EXHAUSTION"`, `"BREATH"`, or `"FEIGNDEATH"` |
| `value` | number | The snapshot value from the last server packet (ms). Not live-interpolated — see `GetMirrorTimerProgress` for that |
| `maxValue` | number | Timer's max (ms) |
| `scale` | number | Server-set rate. **Negative** = depleting (vanilla sends `-1` for breath, draining 60000 ms over 60 s of real time); **positive** = filling. Modern's docs describe the opposite convention — vanilla's wire format predates that flip |
| `paused` | boolean | `true` if the engine has frozen the timer |
| `label` | string | Localized display label, e.g. `"Breath"`. For FEIGNDEATH, the spell name |

`GetMirrorTimerProgress(label)` — `label` is the engine type-name
(`"BREATH"` etc.). Returns the **live interpolated** value in
milliseconds, computed each call from the snapshot + elapsed
real-time ticks. Returns `0` when no matching timer is active.

```lua
local f = CreateFrame("Frame")
f:RegisterEvent("MIRROR_TIMER_START")
f:SetScript("OnEvent", function()
    -- pull whichever slot the engine just populated
    for i = 1, 3 do
        local timer, value, maxv, scale, paused, label = GetMirrorTimerInfo(i)
        if timer then
            DEFAULT_CHAT_FRAME:AddMessage(format("%s: %d / %d (%s)",
                label, value, maxv, paused and "paused" or "running"))
        end
    end
end)

-- Smooth bar updates: poll progress in OnUpdate
local function OnUpdate(self)
    self:SetValue(GetMirrorTimerProgress("BREATH"))
end
```

> **Modern uses `"FATIGUE"`, vanilla uses `"EXHAUSTION"`.** The engine
> type-name string for the off-map timer is `"EXHAUSTION"` in 1.12 —
> we surface what the engine actually returns rather than translating
> to modern's name. Addons backporting `"FATIGUE"`-keyed code need to
> handle either string.

### `GetShapeshiftFormID()`

Returns the player's current shapeshift form as the integer ID from
vanilla 1.12.1's `SpellShapeshiftForm.dbc`. Returns `0` when the
player isn't shifted.

| Class | Form IDs |
|-------|----------|
| Druid | `1` Cat, `2` Tree, `3` Travel, `4` Aquatic, `5` Bear, `8` Dire Bear, `31` Moonkin |
| Warrior | `17` Battle, `18` Defensive, `19` Berserker |
| Shaman | `16` Ghost Wolf |
| Rogue | `30` Stealth |
| Priest | `28` Shadowform, `32` Spirit of Redemption |

**Turtle WoW** extends the DBC with custom rows that aren't present
on Blizzard 1.12.1:

| ID | Name | Notes |
|----|------|-------|
| `9` | Tree of Life Form | Restoration druid endgame form |
| `11` | Swift Travel Form | Mounted-speed travel form |

> **Vanilla numbering ≠ modern numbering.** Modern WoW renumbered the
> table — e.g. modern uses `17` for Travel and `35` for Tree of Life,
> where 1.12.1 uses `3` (Travel) and Turtle uses `9` (Tree of Life).
> Don't copy named constants from a modern addon and expect them to
> match.

```lua
if GetShapeshiftFormID() == 0 then
    CastSpellByName("Cat Form")
end
```

Reads byte 2 of `UNIT_BYTES_1` (descriptor `+0x212`) on the local
player — the same byte vanilla's `Script_GetShapeshiftFormInfo`
compares against each form-spell's effect-encoded form ID to answer
"is this form active". The engine updates the byte from
`SMSG_UPDATE_OBJECT` aura/form packets, so the value is live without
needing an event listener.

Vanilla 1.12 has only `GetShapeshiftFormInfo(index)` (1-based bar
index); `GetShapeshiftFormID` exposes the DBC ID directly so callers
can write `if formID == CAT_FORM` without iterating the bar.

### `CancelShapeshiftForm()`

Cancels the player's current shapeshift form, dropping them back to
caster (or whatever neutral state the class uses). No-op when the
player isn't shifted.

```lua
if GetShapeshiftFormID() ~= 0 then
    CancelShapeshiftForm()
end
```

Covers druid forms (Cat / Bear / Travel / Aquatic / Moonkin / Dire
Bear / Tree of Life), shaman Ghost Wolf, priest Shadowform / Spirit of
Redemption, and rogue Stealth. No form table hardcoding — the
implementation finds whichever buff currently provides the active form
by scanning `Spell.dbc` effects.

> **Does not cover warrior stances.** Vanilla treats warriors as
> *always* in a stance — the engine has no "no stance" state and
> right-clicking the active stance in the stance bar does nothing. The
> server rejects the cancel packet for stances, so this function is a
> no-op when called as a warrior. Switch stances with the normal stance
> spells instead.

**Implementation**: reads the current form byte (descriptor `+0x212`,
same source as [`GetShapeshiftFormID()`](#getshapeshiftformid)), walks
the player's buff slots (0..31), looks up each spell's
`Spell.dbc[spellID]` effect arrays, and on the first effect whose
`EffectApplyAuraName` is `SPELL_AURA_MOD_SHAPESHIFT` (`36`) and whose
`EffectMiscValue` matches the current form ID, sends `CMSG_CANCEL_AURA`
via the engine's direct sender at [`FUN_006E7040`](../src/Offsets.h#L428).

Mirrors 3.3.5a's `Script_CancelShapeshiftForm` inner (`FUN_00726CE0`),
which does the same effect-array scan + form-id match before issuing
its cancel packet. Vanilla just lacks the public Lua surface.

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

### `UnitTokenFromGUID(guid)`

Best-effort reverse lookup: given a GUID string in the
`"0xHHHHHHHHLLLLLLLL"` format `UnitGUID` returns, walk the engine's
known unit tokens and return the first one currently mapped to that
GUID, or `nil` if none of them point at it.

The search order matches modern retail with post-1.12 tokens
omitted (`vehicle`, `arenaN`, `arenapetN`, `bossN`, `softenemy`,
`softfriend`, `softinteract` all post-date vanilla and the engine's
resolver doesn't recognize them). `nameplateN` and `focus` are
included — we hook the resolver so the engine recognizes both token
forms. As with the other positional tokens, a returned
`"nameplate3"` result is only valid at that instant; the slot may
shift to a different unit between calls, so re-verify with
`UnitGUID(token)` before reusing.

```
player → pet → party1..4 → partypet1..4 → raid1..40
       → raidpet1..40 → nameplate1..N → target → focus
       → npc → mouseover
```

```lua
local name, guid = GameTooltip:GetUnitGUID()
local token = UnitTokenFromGUID(guid)
if token then
    -- can now feed `token` to UnitHealth / UnitClass / etc.
end
```

> **The return is inherently unstable.** Multiple tokens can map to
> the same GUID at once (your target IS your party1, your pet IS
> your raidpet5 if you're in a raid), and the mapping changes every
> time `SMSG_GROUP_LIST` fires, the player tabs target, or a party
> member loads in. Modern's API has the same warning. If you cache a
> result, re-verify with `UnitGUID(token) == originalGuid` before
> reusing it.
>
> Returns `nil` for malformed GUID strings, zero GUIDs, and GUIDs
> that don't match any currently-resolvable token — including
> ex-targets, ex-mouseover units, distant players seen in the chat
> log, etc. The engine simply doesn't address those by token.

### `UnitSubName(unit)`

Returns the NPC's subtitle / title string (the small italic text
under their name in the tooltip): `"Innkeeper"`, `"Stable Master"`,
`"<Master Engineer>"`, etc. Returns `nil` for players, unsynced
NPCs, and NPCs with no subtitle.

```lua
/dump UnitSubName("target")
-- "Innkeeper"             (Stormwind innkeep)
-- "<Auction House>"       (auctioneer)
-- nil                     (a player, or a wolf in the woods)
```

Modern WoW returns the subtitle as `UnitName`'s second return;
vanilla 1.12's `UnitName` returns only one value, leaving no
direct route to the subtitle. Addons that wanted it had to scrape
`GameTooltip` text, which is fragile and required the unit to be
hovered. `UnitSubName` reads it straight off the engine's creature
cache (`[unit + 0xB30] → [+0x10]`), no tooltip round-trip.

Coverage caveat: the creature cache row is populated when an NPC
becomes visible to the client AND the server has sent its
`CMSG_CREATURE_QUERY_RESPONSE`. The first time a fresh NPC appears
the row may be briefly NULL — usually only one or two frames.
Subsequent calls succeed once the response lands.

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

### `UnitIsDND(unit)`

Returns `true` if the unit is currently in DND mode ("Do Not Disturb",
toggled via `/dnd`). Same unit-coverage as `UnitIsAFK` — any
player-controlled unit, false for NPCs.

```lua
UnitIsDND("player")
UnitIsDND("target")
```

### `UnitIsFeignDeath(unit)`

Returns `true` if the unit is feigning death (Hunter's `Feign Death`).
Reads `UNIT_FIELD_FLAGS` bit 29 (`0x20000000`) from the broadcast
descriptor — works for any unit since UNIT_FIELD_FLAGS is broadcast
in object updates.

```lua
UnitIsFeignDeath("target")   -- true if a feigning hunter
```

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

### `UnitInRange(unit)`

Returns `(inRange, checkedRange)` — whether `unit` is within 40 yards
of the player, plus a flag indicating whether the range check could
actually be performed.

```lua
local inRange, checked = UnitInRange("party1")
if checked and not inRange then
    -- partymate is loaded but >40 yards away → skip the heal
end
```

| Return | Meaning |
|--------|---------|
| `inRange` | `true` if the unit's world position is within 40 yards of the player's. `false` if out of range OR if `checkedRange` is `false`. |
| `checkedRange` | `true` when a position-based range check was performed. `false` when the unit's position isn't available — either the token didn't resolve (empty `partyN` slot, no target, raid member in a different zone, etc.) or `unit == "player"` (see below). |

Reads the world position via the `CGObject::GetPosition` vtable
virtual (slot 5, offset `+0x14`) — same path
`CheckInteractDistance` uses. Squared-distance compare against
`40 * 40 = 1600`.

> **`UnitInRange("player")` returns `(false, false)`** by design,
> matching modern WoW's behavior. The function is meant for healing-
> frame "is *another* unit in range" checks; querying yourself is
> meaningless. We short-circuit before the position read so the
> result is unambiguous (`checkedRange=false`).

Works for any synced unit (party/raid members in your zone, target,
mouseover, etc.). For raid members in a different zone or otherwise
not in the engine's sync window, the position virtual returns null
and the function reports `(false, false)`.

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

### `UnitRaceBase(unit)`

Returns `(raceFile, raceID)` — the locale-independent race token
plus the numeric raceID. The token is one of `"Human"`, `"Orc"`,
`"Dwarf"`, `"NightElf"`, `"Scourge"`, `"Tauren"`, `"Gnome"`,
`"Troll"`; the raceID is `1..8` for those values respectively.

```lua
local token, id = UnitRaceBase("player")    -- "Human", 1
local token = UnitRaceBase("target")        -- works for any synced unit
```

Sibling to [`UnitClassBase`](#unitclassbaseunit). Same problem
(vanilla's `UnitRace(unit)` returns a localized name — `"Mensch"`,
`"Orc"`, etc.); same solution (locale-independent token straight
from the DBC). Reads byte 0 of `UNIT_FIELD_BYTES_0` (descriptor
`+0x78`) and looks up `ChrRaces.dbc::Filename` (`+0x3C`).

Returns `(nil, nil)` for unresolvable units and non-player units
(creature race bytes don't index `ChrRaces.dbc`).

### `UnitPower(unit [, powerType])` / `UnitPowerMax(unit [, powerType])`

Modern multi-power-type getters. Vanilla 1.12 only ships
`UnitMana(unit)` / `UnitManaMax(unit)` which return whichever
primary power the unit happens to have (mana for casters, energy
for rogues, rage for warriors, etc.); these add the explicit
`powerType` arg so addons can read a specific power slot without
caring what the unit's primary is.

```lua
local mana    = UnitPower("player", 0)   -- explicit mana
local energy  = UnitPower("player", 3)   -- explicit energy
local primary = UnitPower("player")      -- whatever the player's primary is
local maxMana = UnitPowerMax("target", 0)
```

`powerType` values (matches modern WoW's enum):

| value | type |
|---|---|
| `0` | MANA |
| `1` | RAGE |
| `2` | FOCUS |
| `3` | ENERGY |
| `4` | HAPPINESS |

These match the [`Enum.PowerType`](#globals) namespace published
alongside — `UnitPower("player", Enum.PowerType.Mana)` is the
idiomatic modern shape.

Omitting `powerType` (or passing `-1` / any out-of-range value)
falls back to the unit's primary power, read from
`UNIT_FIELD_BYTES_0` byte 3 (descriptor `+0x7B`) — same source the
vanilla engine uses internally. Same fallback the 3.3.5 client's
`Script_UnitPower` uses for its `type == 7` sentinel.

Returns `0` for invalid units, unresolvable tokens, or power types
outside the 0..4 vanilla range (Runes / Runic Power are WotLK
additions that don't have descriptor slots in the 1.12 unit
layout).

Display-divisor applied. Vanilla stores some power types at a
scaled value internally and divides before exposing them through
Lua — same trick `Script_UnitMana` (`0x00517670`) uses, reading
the divisor table at `0x0086F978`:

| type | divisor |
|---|---|
| `0` MANA | 1 |
| `1` RAGE | **10** (raw `0..1000` → display `0..100`) |
| `2` FOCUS | 1 |
| `3` ENERGY | 1 |
| `4` HAPPINESS | 1000 |

So a fresh warrior reads `UnitPower("player", 1)` = `0..100`, not
`0..1000`. Matches retail Classic.

Direct descriptor reads — `desc[+0x44 + type*4]` for current power,
`desc[+0x5C + type*4]` for max, divided by the table entry for the
type. No engine call, no Lua-stack roundtrip.

### `UnitPowerType(unit)`

Vanilla 1.12 ships this returning just the integer power type;
this implementation extends it to the modern 2-tuple
`(powerType, powerToken)`. Strict-superset signature — addons
that destructure only the first return are unaffected.

```lua
local pt, token = UnitPowerType("player")
-- e.g. (0, "MANA") for a paladin, (1, "RAGE") for a warrior
```

Token strings match modern WoW exactly:

| value | token |
|---|---|
| `0` | `"MANA"` |
| `1` | `"RAGE"` |
| `2` | `"FOCUS"` |
| `3` | `"ENERGY"` |
| `4` | `"HAPPINESS"` |
| `5` | `"RUNES"` |
| `6` | `"RUNIC_POWER"` |

5/6 are post-WotLK power types that can't appear on a 1.12 unit's
descriptor; they're included in the table for symmetry with the
modern enum in case a private server pushes one through.

Chains to the engine's original `Script_UnitPowerType` at
`0x00517940` to preserve its full unit-resolution flow (object-
manager lookup with pet / totem / vehicle fallbacks), then reads
the just-pushed integer and appends the token string.

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
| `duration` | number | applied duration in seconds. When the aura's cast was observed (in the `Aura::Source` cache), this is the caster-modified duration — talent/glyph extensions like Improved Shadow Word: Pain included — so it stays consistent with `expirationTime` (`remaining ≤ duration`). On a cache miss it falls back to the base `Spell.dbc → SpellDuration.dbc` value with level scaling. Returns 0 for spells flagged "no duration" (passives, paladin auras, infinite buffs) |
| `expirationTime` | number | for `unit == "player"`, read from the engine's player-buff table at `0x00BC6040` (same data `GetPlayerBuffTimeLeft` returns). For any other unit, taken from the `Aura::Source` cache (cast time + duration captured from `SMSG_SPELL_GO`; see below). `0` when neither source has it. `expirationTime - GetTime()` gives the true remaining time |
| `sourceUnit` | string | unit token of the caster (`"player"`, `"raid7"`, `"nameplate1"`, …), resolved from the `Aura::Source` cache. `nil` if the cast wasn't observed or the caster maps to no current token |
| `sourceGUID` | string | caster's `"0x…"` GUID string from the same cache. **ClassicAPI extension — not a retail `AuraData` field.** Set whenever a caster is known, including when `sourceUnit` is `nil` (caster left token range). Stable for the session, unlike the volatile nameplate token; doubles as a unit token under SuperWoW. `nil` on a cache miss |
| `charges` / `maxCharges` | number | always `0` — vanilla has stacks, not charges |
| `timeMod` | number | always `1` — vanilla has no haste-affected auras |
| `isStealable`, `isBossAura`, `isFromPlayerOrPlayerPet`, `isNameplateOnly`, `nameplateShowAll`, `nameplateShowPersonal`, `canApplyAura`, `shouldConsolidate`, `isRaid` | boolean | always `false` — modern UI concepts vanilla doesn't have |
| `auraInstanceID`, `points` | (absent) | omitted from the table — Lua read yields nil, matching modern semantics for "field doesn't apply" |

### Filter parsing

The optional `filter` string is a pipe-separated set of upper-case
tokens, matching modern syntax (`"HELPFUL"`, `"HARMFUL"`,
`"HELPFUL|PLAYER"`, etc.). Honored on vanilla:

- **`HELPFUL`** (default) / **`HARMFUL`** — pick the buff or debuff range.
  In `GetUnitAuras`, supplying neither returns both ranges.
- **`PLAYER`** — restrict to auras the local player cast, via the
  `Aura::Source` caster cache (`sourceGUID == ` player GUID). Combines with
  the range tokens (`"HARMFUL|PLAYER"` = your debuffs only). Because the
  caster is best-effort, an aura whose cast we didn't observe is treated as
  not-player-cast and excluded — so `PLAYER` can under-report auras that
  predate login.

Other tokens (`RAID` / `CANCELABLE` / `INCLUDE_NAME_PLATE_ONLY`) are
accepted but no-op — they need engine systems (raid-dispel relevance,
nameplate visibility flags) vanilla doesn't have.

### Caster & timing (`Aura::Source`)

`sourceUnit`, `sourceGUID`, and non-player `expirationTime` come from a
client-side cache that vanilla itself can't provide: the unit aura array
stores only spell IDs — never the caster, and no cast/expiration timing for
anyone but the local player. `Aura::Source` fills the gap by co-hooking
three engine functions and caching `(targetGuid, spellId) → { casterGuid,
expirationMs, durationMs }`; `Push` keys into it by the unit's GUID + the
aura's spell ID.

- **`SpellGo`** (`FUN_SPELL_GO`, `0x006E7A70`) — the cast packet, the only
  place the client sees an aura's **caster** + a server-authoritative,
  caster-modified duration. Fills `sourceUnit`/`sourceGUID` + timing for
  directly-cast auras.
- **`OnAuraAdded`** (`0x006123F0`) — fires when any aura first occupies a
  slot, including proc/triggered auras that emit no `SMSG_SPELL_GO` (Shadow
  Weaving etc.). Stamps `expirationMs = now + base duration` (no caster).
- **`OnAuraStacksChanged`** (`0x00612450`) — fires when an existing aura's
  stack count changes; re-stamps expiration so a climbing stacking debuff
  (e.g. Shadow Weaving 1→5) refreshes.

Application hooks never overwrite an entry `SpellGo` already owns, so a
directly-cast aura keeps its talented timing + caster regardless of hook
order.

Implications:

- **Best-effort.** Only auras observed *after* login carry data; auras
  already up when you logged in leave the defaults (`sourceUnit`/`sourceGUID`
  nil, non-player `expirationTime` 0).
- **Max-stack refresh is a blind spot.** When a stacking debuff is refreshed
  at *max* stacks (e.g. Shadow Weaving 5→5), no client-visible aura field
  changes (spellID, stacks, flags, level all identical), so the server sends
  no UpdateField and the engine fires no callback — there is no packet to
  hook (verified: the stack-change dispatcher `FUN_00604ea0` calls
  `OnAuraStacksChanged` unconditionally, so its silence proves nothing
  arrived). The countdown therefore runs to 0 and the entry evicts; once
  evicted, `expirationTime` falls back to 0 (unknown) while the aura is still
  shown. Climbing stacks and single-stack refreshes (re-cast → fresh
  `SpellGo`) are unaffected.
- **No nampower dependency.** We parse the engine functions ourselves;
  nampower (which hooks the same three) need not be loaded — when it is, we
  co-hook the sites.
- Entries are evicted once their timed aura elapses, so the cache stays
  bounded; infinite-duration auras persist until overwritten under load.

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

### `C_UnitAuras.GetAuraDataBySpellName(unit, spellName [, filter])`

Linear-searches `unit`'s aura array for the first populated slot
whose locale-resolved `name` matches `spellName` exactly. Returns
the `AuraData` for that slot or `nil` if not found. Case-sensitive
— matches modern semantics. Without a filter, searches both
ranges (helpful first, then harmful).

```lua
local d = C_UnitAuras.GetAuraDataBySpellName("player", "Inner Fire")
if d then print(d.applications, "stacks of Inner Fire") end

-- restrict to debuffs:
local poison = C_UnitAuras.GetAuraDataBySpellName(
    "target", "Mind-numbing Poison", "HARMFUL")
```

Uses the same `name` field the other `C_UnitAuras.*` functions
populate (locale-applied `Spell.dbc` name). If the player is on
a non-English client, pass the localized name — addons that
hard-code English names should use `GetUnitAuraBySpellID`
instead for portability.

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

## VoiceChat

Backport of modern WoW's text-to-speech surface — the `C_VoiceChat` and
`C_TTSSettings` namespaces — backed by **Windows SAPI** (the OS speech
engine). All playback is **local** (rendered on your own machine);
vanilla 1.12 has no voice-chat transport, so there is nothing "remote" to
transmit synthesized speech into.

> **Coexistence with VanillaTTS:** the same surface also ships as the
> standalone [VanillaTTS](https://github.com/brues-code/VanillaTTS) DLL,
> which adds a software-synth backend for Wine. If `VanillaTTS.dll` is
> loaded in the same client, this built-in module stands down automatically
> (registers none of the functions, events, or cvars below) and VanillaTTS
> provides them instead — the Lua-visible API is identical either way, so
> addons don't need to care which one is active.

SAPI is created lazily on first use. If SAPI is unavailable (e.g. running
the client under Wine, or a stripped Windows), every call degrades
gracefully: `GetTtsVoices` returns an empty table, `SpeakText` fires
`VOICE_CHAT_TTS_PLAYBACK_FAILED` and does nothing, and the client stays
stable — there is no crash. Install more voices under **Windows Settings →
Time & Language → Speech**, then call `C_TTSSettings.RefreshVoices()`.

### `C_VoiceChat.GetTtsVoices()` / `C_VoiceChat.GetRemoteTtsVoices()`

Return a numerically-indexed array of voice tables — one per SAPI voice
installed on the machine:

```lua
for _, v in ipairs(C_VoiceChat.GetTtsVoices()) do
    print(v.voiceID, v.name)   -- e.g.  0  "Microsoft David Desktop - English (United States)"
end
```

Each entry is `{ voiceID = <0-based index>, name = <display name> }`. The
`voiceID` is the index you pass to `SpeakText` and store via
`C_TTSSettings.SetVoiceOption`. `GetRemoteTtsVoices` is an alias returning
the same local list (kept for API parity — vanilla has no voice chat).
Calling either refreshes the cached voice list and fires
`VOICE_CHAT_TTS_VOICES_UPDATE` if it changed.

### `C_VoiceChat.SpeakText(voiceID, text [, destination, rate, volume])`

Speaks `text` aloud using the SAPI voice at index `voiceID`.

```lua
C_VoiceChat.SpeakText(
    C_TTSSettings.GetSpeechVoiceID(),         -- use the configured voice
    UnitName("target") or "no target")
```

- **`voiceID`** (number, required) — index into `GetTtsVoices()`. Note this
  is an **explicit argument** — `SpeakText` does *not* read the `ttsVoice`
  cvar itself, matching Blizzard's contract. Pass
  `C_TTSSettings.GetSpeechVoiceID()` to honor the configured default.
- **`text`** (string, required) — UTF-8; spoken as-is.
- **`destination`** (number, optional, default `1`) — `1` =
  `LOCAL_PLAYBACK`, `4` = `QUEUED_LOCAL_PLAYBACK`. Accepted for parity;
  both queue FIFO through SAPI (no app-side queue).
- **`rate`** (number, optional, default `0`) — speech rate, clamped to
  `[-10, 10]`.
- **`volume`** (number, optional, default `100`) — clamped to `[0, 100]`.

Speech is asynchronous. The call returns immediately and fires
`VOICE_CHAT_TTS_PLAYBACK_STARTED` when SAPI begins the utterance and
`VOICE_CHAT_TTS_PLAYBACK_FINISHED` when it completes (or
`..._PLAYBACK_FAILED` if the voice/engine is unavailable). Raises a Lua
error if `voiceID` isn't a number or `text` isn't a string.

### `C_VoiceChat.StopSpeakingText()`

Stops and purges everything currently playing or queued.

### `C_TTSSettings` — getters & setters

The settings store (separate from `SpeakText`, which takes its parameters
explicitly). All three values are backed by CVars (see below), so they
**persist across sessions** and are clamped to valid ranges on write.

| Function | Returns / effect |
|---|---|
| `C_TTSSettings.GetSpeechRate()` | configured rate, `[-10, 10]` (default `0`) |
| `C_TTSSettings.GetSpeechVolume()` | configured volume, `[0, 100]` (default `100`) |
| `C_TTSSettings.GetSpeechVoiceID()` | configured voice index (default `0`) |
| `C_TTSSettings.GetVoiceOptionName()` | display name of the configured voice, or `""` |
| `C_TTSSettings.SetSpeechRate(rate)` | set rate (clamped) |
| `C_TTSSettings.SetSpeechVolume(volume)` | set volume (clamped) |
| `C_TTSSettings.SetVoiceOption(voiceID)` | set the voice by index (clamped to installed count) |
| `C_TTSSettings.SetVoiceOptionByName(name)` | set the voice by display name (case-insensitive exact match) |
| `C_TTSSettings.SetDefaultSettings()` | reset to voice `1` (if present, else `0`), rate `0`, volume `100` |
| `C_TTSSettings.RefreshVoices()` | re-enumerate SAPI voices; fires `VOICE_CHAT_TTS_VOICES_UPDATE` if the list changed |

```lua
C_TTSSettings.SetVoiceOptionByName("Microsoft Zira Desktop - English (United States)")
C_TTSSettings.SetSpeechVolume(150)              -- clamps to 100
print(C_TTSSettings.GetVoiceOptionName())       -- "Microsoft Zira Desktop - ..."
```

### TTS events

Registered like any engine event (`frame:RegisterEvent("VOICE_CHAT_TTS_PLAYBACK_STARTED")`).

| Event | Args |
|---|---|
| `VOICE_CHAT_TTS_PLAYBACK_STARTED` | `numConsumers` (number), `utteranceID` (number), `durationMS` (number, always `0`), `destination` (number) |
| `VOICE_CHAT_TTS_PLAYBACK_FINISHED` | `numConsumers` (number), `utteranceID` (number), `destination` (number) |
| `VOICE_CHAT_TTS_PLAYBACK_FAILED` | `status` (string — e.g. `"EngineAllocationFailed"`, `"InternalError"`), `utteranceID` (number), `destination` (number) |
| `VOICE_CHAT_TTS_VOICES_UPDATE` | *(none)* — the installed-voice list changed |
| `VOICE_CHAT_TTS_SPEAK_TEXT_UPDATE` | reserved for parity; not currently fired |

`utteranceID` correlates the START/FINISHED/FAILED events for a given
`SpeakText` call. `PLAYBACK_FINISHED` fires only when playback actually
completes, so it's the signal to start the next utterance in a queue.

### TTS CVars

The `C_TTSSettings` values are stored as engine CVars, persisted to
`WTF\Config.wtf`. You can also read/write them with the standard
`GetCVar`/`SetCVar` (or `/console set`); writes are clamped by the same
validation:

| CVar | Default | Range | Backing setting |
|---|---|---|---|
| `ttsVoice` | `0` | `[0, voiceCount-1]` | `GetSpeechVoiceID` / `SetVoiceOption` |
| `ttsSpeed` | `0` | `[-10, 10]` | `GetSpeechRate` / `SetSpeechRate` |
| `ttsVolume` | `100` | `[0, 100]` | `GetSpeechVolume` / `SetSpeechVolume` |
