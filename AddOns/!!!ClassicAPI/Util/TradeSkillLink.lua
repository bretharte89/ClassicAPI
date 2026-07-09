-- Trade-skill *list* links (`|Htrade:...|h`) — backport of the 2.x+ shareable
-- profession link. The link is built entirely in the DLL
-- (`GetTradeSkillListLink` / `GetCraftListLink`, see src/tradeskill/Link.cpp);
-- this file only intercepts clicks on received links and renders the linker's
-- recipe list in a scroll frame (the UI half).
--
--   link:  |cffffd000|Htrade:<skillLineID>:<curRank>:<maxRank>:<linkerName>:<base64 bits>|h[Name]|h|r
--
-- "Which recipes are known" is a bitfield over the skill line's canonical
-- recipe list (deterministic per DBC). The viewer lists the recipes the linker
-- knows (with an "X of Y" summary of the whole skill line), regardless of what
-- the reader can craft themselves.

-- Frame geometry mirrors the real TradeSkillFrame (Blizzard_TradeSkillUI):
-- 768x512, TW-TradeSkill quadrant art, 19 rows of 16px, list at (37,-98).
local FRAME_W, FRAME_H = 768, 512;
local NUM_ROWS   = 19;
local ROW_HEIGHT = 16;

-- Profession portrait icons, keyed by SkillLine.dbc id. Vanilla's DBC icon
-- field is a "Temp" placeholder for professions, so the real TradeSkillFrame
-- hardcodes these too (Blizzard_TradeSkillUI's TradeSkillIcons table) — we key
-- by id instead of name so it's locale-independent.
local PROFESSION_ICONS = {
	[171] = "Trade_Alchemy",
	[164] = "Trade_BlackSmithing",
	[185] = "INV_Misc_Food_15",         -- Cooking
	[333] = "Trade_Engraving",          -- Enchanting
	[202] = "Trade_Engineering",
	[129] = "Spell_Holy_SealOfSacrifice", -- First Aid
	[165] = "INV_Misc_ArmorKit_17",     -- Leatherworking
	[197] = "Trade_Tailoring",
	[186] = "Trade_Mining",
	[182] = "Trade_Herbalism",
	[393] = "INV_Misc_Pelt_Wolf_01",    -- Skinning
	[356] = "Trade_Fishing",
};

if TURTLE_WOW_VERSION then
	PROFESSION_ICONS[142] = "Trade_Survival"
	PROFESSION_ICONS[755] = "INV_Misc_Gem_01" -- Jewelcrafting
end

local function ProfessionIcon(skillLineID)
	local icon = PROFESSION_ICONS[skillLineID];
	return icon and ("Interface\\Icons\\" .. icon)
		or "Interface\\Icons\\INV_Misc_QuestionMark";
end

-- Exact port of the engine's trade-skill difficulty (the builder inside
-- Script_GetTradeSkillInfo, FUN_004fca20): `hi` = SkillLineAbility trivialHigh,
-- `lo` = trivialLow (defaults to hi-25 when zero), yellow = midpoint. Returns
-- the band 0..3 (orange/yellow/green/grey) for the linker's skill — identical
-- to what the default UI shows in the linker's own window.
local function DifficultyBand(skill, hi, lo)
	if not hi or hi <= 0 then
		return 3;
	end
	if not lo or lo == 0 then
		lo = (hi ~= 25) and (hi - 25) or 0;
	end
	local yellow = (hi + lo) / 2;
	if skill < lo then
		return 0;   -- orange (optimal)
	elseif skill < yellow then
		return 1;   -- yellow (medium)
	elseif skill < hi then
		return 2;   -- green (easy)
	end
	return 3;       -- grey (trivial)
end

local BAND_COLOR = {
	[0] = DIFFICULT_DIFFICULTY_COLOR,  -- orange
	[1] = FAIR_DIFFICULTY_COLOR,       -- yellow
	[2] = EASY_DIFFICULTY_COLOR,       -- green
	[3] = TRIVIAL_DIFFICULTY_COLOR,    -- grey
};

-- Parse a `trade:` hyperlink body into its fields. Returns
-- `id, cur, max, linkerName, bits` (linkerName may be nil), or nil if
-- malformed. `bits` may legitimately be empty (a profession with zero
-- recipes).
local function ParseTradeLink(link)
	-- Current format, with the linker's name: trade:id:cur:max:Name:bits
	local _, _, id, cur, max, who, bits =
		string.find(link, "^trade:(%d+):(%d+):(%d+):([^:]+):([%w%+/]*)$");
	if not id then
		-- Legacy format (no name): trade:id:cur:max:bits
		who = nil;
		_, _, id, cur, max, bits =
			string.find(link, "^trade:(%d+):(%d+):(%d+):([%w%+/]*)$");
	end
	if not id then
		return nil;
	end
	return tonumber(id), tonumber(cur), tonumber(max), who, bits or "";
end

------------------------------------------------------------------------------
-- Display frame (built lazily on first use)
------------------------------------------------------------------------------

local frame;

-- Resolve each recipe's crafted-item subclass. Returns the sorted distinct
-- subclass list, and whether any recipe crafts an item at all (false for
-- enchanting -> no filter shown). For items whose data isn't cached yet,
-- registers an Item-mixin load callback (`onLoaded`) so the list fills in as
-- the data arrives — no global GET_ITEM_INFO_RECEIVED polling.
local function ResolveSubclasses(list, onLoaded)
	local present, subs, anyItem = {}, {}, false;
	for i = 1, table.getn(list) do
		local r = list[i];
		if r.createdItem and r.createdItem > 0 then
			anyItem = true;
			if not r.subclass then
				-- GetItemInfoInstant's 3rd return is the localized subclass
				-- name (or nil when uncached).
				local _, _, subType = C_Item.GetItemInfoInstant(r.createdItem);
				if subType and subType ~= "" then
					r.subclass = subType;
				elseif onLoaded then
					-- Not cached — re-derive when the item loads.
					Item:CreateFromItemID(r.createdItem):ContinueOnItemLoad(onLoaded);
				end
			end
			if r.subclass and not present[r.subclass] then
				present[r.subclass] = true;
				table.insert(subs, r.subclass);
			end
		end
	end
	table.sort(subs);
	return subs, anyItem;
end

-- Subclass dropdown: "All Subclasses" + each distinct subclass. Buttons carry
-- their subclass in `value` so the shared click handler needs no per-iteration
-- closures (Lua 5.0 safe). The "All" button uses a sentinel because vanilla's
-- UIDropDownMenu_AddButton defaults an absent `value` to the button text —
-- which would otherwise filter by "All Subclasses" and match nothing.
local SUBCLASS_ALL = {};

local function SubClassButton_OnClick()
	local v = this.value;
	if v == SUBCLASS_ALL then
		v = nil;
	end
	frame.subclassFilter = v;
	UIDropDownMenu_SetText(v or ALL_SUBCLASSES, frame.dropdown);
	frame:ApplyFilter();
end

local function SubClassDropDown_Initialize()
	local info = {};
	info.text = ALL_SUBCLASSES;
	info.value = SUBCLASS_ALL;
	info.checked = (frame.subclassFilter == nil);
	info.func = SubClassButton_OnClick;
	UIDropDownMenu_AddButton(info);
	local subs = frame.subclasses or {};
	for i = 1, table.getn(subs) do
		local b = {};
		b.text = subs[i];
		b.value = subs[i];
		b.checked = (frame.subclassFilter == subs[i]);
		b.func = SubClassButton_OnClick;
		UIDropDownMenu_AddButton(b);
	end
end

-- (Re)build the subclass dropdown from the frame's recipe list and register
-- load callbacks (via ResolveSubclasses) for any crafted items not yet cached
-- — passing itself as the callback, so each arrival refreshes the dropdown.
-- Only rebuilds the visible list when a subclass filter is active (the "All"
-- view doesn't depend on subclasses, so we avoid resetting the scroll on every
-- item that loads).
local function RefreshSubclasses()
	if not (frame and frame:IsShown() and frame.recipes) then
		return;
	end
	local subs, anyItem = ResolveSubclasses(frame.recipes, RefreshSubclasses);
	frame.subclasses = subs;
	if anyItem then
		UIDropDownMenu_Initialize(frame.dropdown, SubClassDropDown_Initialize);
		UIDropDownMenu_SetWidth(120, frame.dropdown);
		UIDropDownMenu_SetText(frame.subclassFilter or ALL_SUBCLASSES, frame.dropdown);
		frame.dropdown:Show();
	else
		frame.dropdown:Hide();
	end
	if frame.subclassFilter then
		frame:ApplyFilter();
	end
end

local function CreateFrame_TradeSkillLink()
	local f = CreateFrame("Frame", "ClassicAPITradeSkillLinkFrame", UIParent);
	f:SetSize(FRAME_W, FRAME_H);
	f:SetPoint("CENTER", 0, 0);
	f:SetMovable(true);
	f:EnableMouse(true);
	f:RegisterForDrag("LeftButton");
	f:SetScript("OnDragStart", function() this:StartMoving(); end);
	f:SetScript("OnDragStop", function() this:StopMovingOrSizing(); end);
	f:SetFrameStrata("DIALOG");
	f:Hide();

	-- Portrait sits BEHIND the parchment so the corner ring frames it (the
	-- TopLeft quadrant art has a transparent ring cut-out).
	local portrait = f:CreateTexture(nil, "BACKGROUND");
	portrait:SetSize(60, 60);
	portrait:SetPoint("TOPLEFT", 7, -6);
	f.portrait = portrait;

	-- Authentic TradeSkillFrame parchment: four quadrant textures, on BORDER
	-- so they overlay (and frame) the portrait behind them.
	local function Quad(file, w, h, point)
		local tx = f:CreateTexture(nil, "BORDER");
		tx:SetTexture("Interface\\TradeSkillFrame\\" .. file);
		tx:SetSize(w, h);
		tx:SetPoint(point, 0, 0);
	end
	Quad("TW-TradeSkill-TopLeft",  512, 256, "TOPLEFT");
	Quad("TW-TradeSkill-TopRight", 256, 256, "TOPRIGHT");
	Quad("TW-TradeSkill-BotLeft",  512, 256, "BOTTOMLEFT");
	Quad("TW-TradeSkill-BotRight", 256, 256, "BOTTOMRIGHT");

	local title = f:CreateFontString(nil, "OVERLAY", "GameFontNormal");
	title:SetPoint("TOP", 0, -18);
	f.title = title;

	local close = CreateFrame("Button", nil, f, "UIPanelCloseButton");
	close:SetPoint("TOPRIGHT", -77, -8);

	-- Skill-rank bar — matches TradeSkillRankFrame: 596x15 at (76,-44), blue
	-- fill over the skills-bar texture with a dark-blue background.
	local bar = CreateFrame("StatusBar", nil, f);
	bar:SetSize(596, 15);
	bar:SetPoint("TOPLEFT", 76, -44);
	bar:SetStatusBarTexture("Interface\\PaperDollInfoFrame\\UI-Character-Skills-Bar");
	bar:SetStatusBarColor(0.0, 0.0, 1.0, 0.5);
	bar:SetMinMaxValues(0, 1);
	-- Background: white @ a=0.2 tinted dark blue @ 0.5 (alphas multiply, so it's
	-- faint) — matches TradeSkillRankFrameBackground exactly.
	local barBg = bar:CreateTexture(nil, "BACKGROUND");
	barBg:SetAllPoints(bar);
	barBg:SetTexture(1, 1, 1, 0.2);
	barBg:SetVertexColor(0.0, 0.0, 0.75, 0.5);
	-- Border: tooltip edge around the bar (TradeSkillRankFrameBorder).
	local border = CreateFrame("Frame", nil, bar);
	border:SetSize(604, 22);
	border:SetPoint("LEFT", -5, 1);
	border:SetBackdrop({
		edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border", edgeSize = 16,
	});
	border:SetBackdropBorderColor(0.4, 0.4, 0.4);
	-- Two FontStrings, exactly like TradeSkillRankFrame: the profession name in
	-- GameFontNormalSmall (gold) at LEFT (6,1), then the rank in
	-- GameFontHighlightSmall (white) 13px to its right.
	local barName = bar:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall");
	barName:SetPoint("LEFT", 6, 1);
	local barRank = bar:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall");
	barRank:SetJustifyH("LEFT");
	barRank:SetWidth(128);
	barRank:SetPoint("LEFT", barName, "RIGHT", 13, 0);
	f.bar = bar;
	f.barName = barName;
	f.barRank = barRank;

	local summary = f:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall");
	summary:SetPoint("TOPRIGHT", -110, -70);
	f.summary = summary;

	-- Subclass filter (item-producing professions only; hidden for enchanting,
	-- which crafts no items). Mirrors TradeSkillSubClassDropDown at (56,-64).
	local dropdown = CreateFrame("Frame", "ClassicAPITradeSkillLinkSubClass", f,
		"UIDropDownMenuTemplate");
	dropdown:SetPoint("TOPLEFT", 40, -60);
	f.dropdown = dropdown;
	f.subclassFilter = nil; -- nil = all subclasses

	local scroll = CreateFrame("ScrollFrame", "ClassicAPITradeSkillLinkScroll",
		f, "FauxScrollFrameTemplate");
	scroll:SetSize(296, 330);
	scroll:SetPoint("TOPLEFT", 37, -98);
	scroll:SetScript("OnVerticalScroll", function()
		FauxScrollFrame_OnVerticalScroll(ROW_HEIGHT, function() f:Refresh(); end)
	end)
	f.scroll = scroll;

	f.rows = {};
	for i = 1, NUM_ROWS do
		-- Inherit the real row button the CraftFrame/TradeSkillFrame use
		-- (CraftButtonTemplate -> ClassTrainerSkillButtonTemplate). The name is
		-- rendered through the template's ButtonText with Blizzard's exact font
		-- metrics, so the row spacing is identical to those windows — no
		-- hand-placed FontString to get wrong.
		local rowName = "ClassicAPITradeSkillLinkRow" .. i;
		local row = CreateFrame("Button", rowName, f,
			"ClassTrainerSkillButtonTemplate");
		row:SetWidth(290); -- height (16) comes from the template
		if i == 1 then
			row:SetPoint("TOPLEFT", scroll, "TOPLEFT", 0, 0);
		else
			row:SetPoint("TOPLEFT", f.rows[i - 1], "BOTTOMLEFT", 0, 0);
		end

		-- Flat list (no collapsible headers): drop the +/- tree icon and its
		-- icon-sized hover art.
		row:SetNormalTexture("");
		row:SetDisabledTexture("");
		row:SetHighlightTexture("");
		-- Keep difficulty colour on hover (don't swap to the white HighlightFont).
		-- if row.SetHighlightFontObject then
		-- 	row:SetHighlightFontObject(GameFontNormal);
		-- end

		-- The template anchors ButtonText past the (now hidden) tree icon;
		-- re-anchor it to the row's left edge since we show no icon.
		local txt = getglobal(rowName .. "Text");
		if txt then
			txt:ClearAllPoints();
			txt:SetPoint("LEFT", row, "LEFT", 4, 0);
		end

		-- Selection bar: Blizzard's exact selection texture (UI-Listbox-Highlight2,
		-- full alpha, default BLEND — its translucency is baked into the texture,
		-- so we don't guess an alpha), tinted per-row to the recipe's difficulty
		-- colour via SetVertexColor in Refresh.
		local sel = row:CreateTexture(nil, "BACKGROUND");
		sel:SetTexture("Interface\\Buttons\\UI-Listbox-Highlight2");
		sel:SetAllPoints(row);
		sel:Hide();
		row.select = sel;

		-- Override the template's trainer scripts with ours.
		row:SetScript("OnClick", function()
			if this.entry then
				f:SelectRecipe(this.entry);
			end
		end);

		f.rows[i] = row;
	end

	local NUM_REAGENTS = 8;

	local prodIcon = CreateFrame("Button", nil, f);
	prodIcon:SetSize(37, 37);  -- TradeSkillSkillIcon is 37x37
	prodIcon:SetPoint("TOPLEFT", 379, -113);
	prodIcon:SetNormalTexture("Interface\\Icons\\INV_Misc_QuestionMark");
	local hdrLeft = prodIcon:CreateTexture(nil, "BACKGROUND");
	hdrLeft:SetTexture("Interface\\ClassTrainerFrame\\UI-ClassTrainer-DetailHeaderLeft");
	hdrLeft:SetSize(256, 64); -- Blizzard's native fill width (+64 cap = 320 total)
	hdrLeft:SetPoint("TOPLEFT", prodIcon, "TOPLEFT", -8, 6);
	local hdrRight = prodIcon:CreateTexture(nil, "BACKGROUND");
	hdrRight:SetTexture("Interface\\ClassTrainerFrame\\UI-ClassTrainer-DetailHeaderRight");
	hdrRight:SetSize(64, 64);
	-- Adjacent to the left piece (no overlap), like Blizzard's detail header.
	hdrRight:SetPoint("TOPLEFT", hdrLeft, "TOPRIGHT", 0, 0);
	local prodCount = prodIcon:CreateFontString(nil, "ARTWORK", "NumberFontNormal");
	prodCount:SetPoint("BOTTOMRIGHT", -5, 2);
	prodIcon.count = prodCount;
	f.prodIcon = prodIcon;

	-- Name is a child of the icon (ARTWORK, like the reagents) so it draws
	-- above the plate; starts 5px right of the icon (matching the reagents'
	-- icon.right + 5) so it clears the icon that now sits on the plate.
	local prodName = prodIcon:CreateFontString(nil, "ARTWORK", "GameFontNormal");
	prodName:SetPoint("LEFT", prodIcon, "RIGHT", 5, 0);
	prodName:SetWidth(240);
	prodName:SetJustifyH("LEFT");
	prodName:SetJustifyV("MIDDLE");
	f.prodName = prodName;

	-- Spell description (enchants only — item recipes leave it empty so it
	-- collapses to zero height and the reagent label sits right under the
	-- icon). White, wrapping, like the real Craft window's CraftDescription.
	local prodDesc = f:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall");
	prodDesc:SetPoint("TOPLEFT", prodIcon, "BOTTOMLEFT", 0, -6);
	prodDesc:SetWidth(280);
	prodDesc:SetJustifyH("LEFT");
	prodDesc:SetJustifyV("TOP");
	f.prodDesc = prodDesc;

	-- Reagent label sits below the description (which is empty for items, so
	-- it lands ~7px below the 37px icon — matching TradeSkillReagentLabel).
	local reagentLabel = f:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall");
	reagentLabel:SetPoint("TOPLEFT", prodDesc, "BOTTOMLEFT", 0, -8);
	reagentLabel:SetText(SPELL_REAGENTS or "Reagents:");
	f.reagentLabel = reagentLabel;

	-- Reagent slots: two columns, four rows — Blizzard's exact geometry
	-- (TradeSkillReagentN off QuestItemTemplate): 39px icons, reagent1 at the
	-- label's BOTTOMLEFT (-5,-3), 147px horizontal pitch (button width), 43px
	-- vertical (41px button + 2px gap). Count at the icon's bottom-right, name
	-- (90px) to its right.
	f.reagents = {};
	for i = 1, NUM_REAGENTS do
		local rowN = math.floor((i - 1) / 2);
		local col = (i - 1) - rowN * 2;
		local btn = CreateFrame("Button", nil, f);
		btn:SetSize(39, 39);
		btn:SetPoint("TOPLEFT", reagentLabel, "BOTTOMLEFT",
			col * 147 - 5, -3 - rowN * 43);
		btn:SetNormalTexture("Interface\\Icons\\INV_Misc_QuestionMark");
		-- Name plate behind each reagent — QuestItemTemplate's $parentNameFrame
		-- (UI-QuestItemNameFrame, 128x64, LEFT of the icon's RIGHT at x=-10).
		local plate = btn:CreateTexture(nil, "BACKGROUND");
		plate:SetTexture("Interface\\QuestFrame\\UI-QuestItemNameFrame");
		plate:SetSize(128, 64);
		plate:SetPoint("LEFT", btn, "RIGHT", -10, 0);
		local cnt = btn:CreateFontString(nil, "ARTWORK", "NumberFontNormal");
		cnt:SetPoint("BOTTOMRIGHT", -4, 1);
		btn.count = cnt;
		-- Name anchored to the plate (LEFT + 15), exactly like the template.
		-- The 90x36 box + MIDDLE vertical justify is what keeps the name
		-- centered on the icon whether it's one line or wraps to two (the
		-- template's $parentName carries the same explicit size).
		local nm = btn:CreateFontString(nil, "ARTWORK", "GameFontHighlight");
		nm:SetPoint("LEFT", plate, "LEFT", 15, 0);
		nm:SetSize(90, 36);
		nm:SetJustifyH("LEFT");
		nm:SetJustifyV("MIDDLE");
		btn.nameText = nm;
		btn:SetScript("OnEnter", function()
			if this.itemLink then
				GameTooltip:SetOwner(this, "ANCHOR_RIGHT");
				GameTooltip:SetHyperlink(this.itemLink);
				GameTooltip:Show();
			end
		end);
		btn:SetScript("OnLeave", GameTooltip_Hide);
		btn:Hide();
		f.reagents[i] = btn;
	end

	-- Populate the detail pane for `entry` (a recipe from the list). Item
	-- name/icon/link come from the Item mixin; uncached items re-render this
	-- pane via ContinueOnItemLoad (guarded against a newer selection). On-hand
	-- counts from GetItemCount. Enchants (createdItem 0) show the spell instead.
	function f:RenderDetail(entry)
		-- Fetch an item's name/icon/link via the Item mixin. If its data isn't
		-- cached yet, re-render this pane when it loads — but only if this recipe
		-- is still the selected one (a later selection must not be clobbered).
		local function itemInfo(id)
			local item = Item:CreateFromItemID(id);
			if not item:IsItemDataCached() then
				item:ContinueOnItemLoad(function()
					if self:IsShown() and self.selectedSpellID == entry.spellID then
						self:RenderDetail(entry);
					end
				end);
			end
			return item:GetItemName(), item:GetItemIcon(), item:GetItemLink();
		end

		-- Product: item icon/name for craft recipes, spell icon/name for
		-- enchants. Name is coloured by difficulty band, like the list row and
		-- the real window's skill-name.
		local prodTex, prodText, prodLink;
		local created = entry.createdItem or 0;
		if created > 0 then
			local name, tex, link = itemInfo(created);
			prodText = name or entry.name;
			prodTex = tex;
			prodLink = link;
		else
			-- Enchant / non-item recipe: use the spell's presentation.
			prodText = entry.name;
			prodTex = C_Spell.GetSpellTexture(entry.spellID);
		end
		-- Enchants carry a spell description (white); item recipes don't, so
		-- the description collapses and the reagents move up under the icon.
		if created > 0 then
			self.prodDesc:SetText("");
		else
			self.prodDesc:SetText(C_Spell.GetSpellDescription(entry.spellID) or "");
		end
		self.prodIcon:SetNormalTexture(prodTex or
			"Interface\\Icons\\INV_Misc_QuestionMark");
		self.prodName:SetText(prodText or "");
		-- The skill name is always the default gold, like the real window
		-- (Blizzard never difficulty-colours the detail's product name).
		self.prodName:SetTextColor(NORMAL_FONT_COLOR:GetRGB());
		self.prodIcon.itemLink = prodLink;      -- item tooltip when created > 0
		self.prodIcon.spellID = entry.spellID;  -- else spell tooltip
		local made = entry.numMade or 0;
		self.prodIcon.count:SetText(made > 1 and tostring(made) or "");
		self.prodIcon:Show();

		local reagents = entry.reagents or {};
		for i = 1, NUM_REAGENTS do
			local btn = self.reagents[i];
			local r = reagents[i];
			if r then
				local name, tex, link = itemInfo(r.itemID);
				btn:SetNormalTexture(tex or
					"Interface\\Icons\\INV_Misc_QuestionMark");
				btn.itemLink = link;
				btn.nameText:SetText(name or ("item:" .. r.itemID));
				local need = r.count or 1;
				local have = C_Item.GetItemCount(r.itemID) or 0;
				if have >= need then
					btn:GetNormalTexture():SetVertexColor(1, 1, 1);
					btn.nameText:SetTextColor(HIGHLIGHT_FONT_COLOR:GetRGB());
				else
					btn:GetNormalTexture():SetVertexColor(0.5, 0.5, 0.5);
					btn.nameText:SetTextColor(GRAY_FONT_COLOR:GetRGB());
				end
				local haveText = (have >= 100) and "*" or tostring(have);
				btn.count:SetText(haveText .. " /" .. need);
				btn:Show();
			else
				btn.itemLink = nil;
				btn:Hide();
			end
		end
	end

	prodIcon:SetScript("OnEnter", function()
		GameTooltip:SetOwner(this, "ANCHOR_RIGHT");
		if this.itemLink then
			GameTooltip:SetHyperlink(this.itemLink);
		elseif this.spellID then
			GameTooltip:SetSpellByID(this.spellID);
		end
		GameTooltip:Show();
	end);
	prodIcon:SetScript("OnLeave", GameTooltip_Hide);

	function f:SelectRecipe(entry)
		self.selectedSpellID = entry.spellID;
		self:RenderDetail(entry);
		self:Refresh();
	end

	function f:ClearDetail()
		self.selectedSpellID = nil;
		self.prodIcon:Hide();
		self.prodName:SetText("");
		self.prodDesc:SetText("");
		for i = 1, NUM_REAGENTS do
			self.reagents[i]:Hide();
		end
	end

	function f:Refresh()
		local recipes = self.displayed or {};
		local total = table.getn(recipes);
		FauxScrollFrame_Update(self.scroll, total, NUM_ROWS, ROW_HEIGHT);
		local offset = FauxScrollFrame_GetOffset(self.scroll);
		for i = 1, NUM_ROWS do
			local row = self.rows[i];
			local entry = recipes[offset + i];
			if entry then
				row:SetText(entry.name or "");
				local r, g, b = BAND_COLOR[entry.band or 3]:GetRGB();
				if self.selectedSpellID and entry.spellID == self.selectedSpellID then
					-- Selected: Blizzard's highlight texture tinted to the
					-- difficulty colour (its own translucency sets the intensity),
					-- text white.
					row.select:SetVertexColor(r, g, b);
					row.select:Show();
					row:SetTextColor(1, 1, 1);
				else
					-- Unselected: no bar, text in the difficulty colour.
					row.select:Hide();
					row:SetTextColor(r, g, b);
				end
				row.spellID = entry.spellID;
				row.entry = entry;
				row:Show();
			else
				row.spellID = nil;
				row.entry = nil;
				row.select:Hide();
				row:Hide();
			end
		end
	end

	function f:ApplyFilter()
		local src = self.recipes or {};
		if not self.subclassFilter then
			self.displayed = src;
		else
			local out = {};
			for i = 1, table.getn(src) do
				if src[i].subclass == self.subclassFilter then
					table.insert(out, src[i]);
				end
			end
			self.displayed = out;
		end
		local sb = getglobal("ClassicAPITradeSkillLinkScrollScrollBar");
		if sb then
			sb:SetValue(0);
		end
		self:Refresh();
	end

	return f;
end

local function ShowTradeSkillLink(link, text)
	local id, cur, max, who, bits = ParseTradeLink(link);
	if not id then
		return false;
	end
	local recipes = C_TradeSkillUI.GetTradeSkillListRecipes(id, bits);
	if not recipes then
		return false;
	end

	local prof = text and string.match(text, "|h%[(.+)%]|h") or "Trade Skills";
	local total = table.getn(recipes);
	local knownList = {};
	for i = 1, total do
		local r = recipes[i];
		if r.isKnown then
			r.band = DifficultyBand(cur or 0, r.trivialLevel, r.greenLevel);
			r.name = GetSpellInfo(r.spellID) or ("spell:" .. r.spellID);
			table.insert(knownList, r);
		end
	end
	table.sort(knownList, function(a, b)
		if a.band ~= b.band then
			return a.band < b.band;
		end
		return a.name < b.name;
	end);
	local known = table.getn(knownList);

	if not frame then
		frame = CreateFrame_TradeSkillLink();
	end
	frame.title:SetText(who and who ~= "" and (who .. " - " .. prof) or prof);
	SetPortraitToTexture(frame.portrait, ProfessionIcon(id));
	frame.bar:SetMinMaxValues(0, (max and max > 0) and max or 1);
	frame.bar:SetValue(cur or 0);
	frame.barName:SetText(prof);
	frame.barRank:SetText(string.format("%d/%d", cur or 0, max or 0));
	frame.summary:SetText(string.format("%d of %d recipes known", known, total));
	frame.recipes = knownList;
	frame.subclassFilter = nil;

	frame:ClearDetail();
	frame:Show();
	-- Build the subclass dropdown (fills in async as uncached crafted items
	-- load, via the Item mixin) then the visible list, then auto-select row 1.
	RefreshSubclasses();
	frame:ApplyFilter();
	local first = frame.displayed and frame.displayed[1];
	if first then
		frame:SelectRecipe(first);
	end
	return true;
end

-- Uncached crafted items no longer need a global GET_ITEM_INFO_RECEIVED
-- watcher: the subclass dropdown (RefreshSubclasses) and the detail pane
-- (RenderDetail) each register per-item Item-mixin ContinueOnItemLoad
-- callbacks that fill themselves in as the data arrives.

------------------------------------------------------------------------------
-- Click interception: handle `trade:` before the engine's SetHyperlink (which
-- in 1.12 only knows item:/enchant: and would reject the link).
------------------------------------------------------------------------------

local originalSetItemRef = SetItemRef;
function SetItemRef(link, text, button, chatFrame)
	if type(link) == "string" and string.sub(link, 1, 6) == "trade:" then
		if IsShiftKeyDown() and ChatFrameEditBox and ChatFrameEditBox:IsVisible() then
			ChatFrameEditBox:Insert(text);
			return;
		end
		if ShowTradeSkillLink(link, text) then
			return;
		end
	end
	return originalSetItemRef(link, text, button, chatFrame);
end
