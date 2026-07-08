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

-- Resolve each recipe's crafted-item subclass (via GetItemInfo, which also
-- warms uncached items so a later GET_ITEM_INFO_RECEIVED can fill them in).
-- Returns the sorted distinct subclass list, and whether any recipe crafts an
-- item at all (false for enchanting -> no filter shown).
local function ResolveSubclasses(list)
	local present, subs, anyItem = {}, {}, false;
	for i = 1, table.getn(list) do
		local r = list[i];
		if r.createdItem and r.createdItem > 0 then
			anyItem = true;
			if not r.subclass then
				-- GetItemInfoInstant's 3rd return is the localized subclass
				-- name (or nil when uncached); GetItemInfo warms it so a later
				-- GET_ITEM_INFO_RECEIVED can fill it in.
				local _, _, subType = C_Item.GetItemInfoInstant(r.createdItem);
				if subType and subType ~= "" then
					r.subclass = subType;
				else
					GetItemInfo(r.createdItem);
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
	local barText = bar:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall");
	barText:SetPoint("LEFT", 6, 1);
	f.bar = bar;
	f.barText = barText;

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
		local row = CreateFrame("Frame", nil, f);
		row:SetSize(290, ROW_HEIGHT);
		if i == 1 then
			row:SetPoint("TOPLEFT", scroll, "TOPLEFT", 0, 0);
		else
			row:SetPoint("TOPLEFT", f.rows[i - 1], "BOTTOMLEFT", 0, 0);
		end

		local hl = row:CreateTexture(nil, "HIGHLIGHT");
		hl:SetTexture("Interface\\Buttons\\UI-Listbox-Highlight2");
		hl:SetBlendMode("ADD");
		hl:SetAllPoints(row);
		hl:SetAlpha(0.5);

		local label = row:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall");
		label:SetPoint("LEFT", 4, 0);
		label:SetJustifyH("LEFT");
		row.label = label;

		row:EnableMouse(true);
		row:SetScript("OnEnter", function()
			if this.spellID then
				GameTooltip:SetOwner(this, "ANCHOR_RIGHT");
				GameTooltip:SetSpellByID(this.spellID);
				GameTooltip:Show();
			end
		end);
		row:SetScript("OnLeave", GameTooltip_Hide);

		f.rows[i] = row;
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
				row.label:SetText(entry.name or "");
				row.label:SetTextColor(BAND_COLOR[entry.band or 3]:GetRGB());
				row.spellID = entry.spellID;
				row:Show();
			else
				row.spellID = nil;
				row:Hide();
			end
		end
	end

	-- Rebuild the displayed list from `recipes` and the active subclass filter,
	-- reset scroll, and redraw.
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
	-- Show only the recipes the linker knows, ordered like the real window:
	-- by difficulty band (orange -> grey), then alphabetically within a band.
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
	frame.barText:SetText(string.format("%s  %d/%d", prof, cur or 0, max or 0));
	frame.summary:SetText(string.format("%d of %d recipes known", known, total));
	frame.recipes = knownList;
	frame.subclassFilter = nil;

	-- Subclass filter: derive from the recipes' crafted items. Item-producing
	-- professions get the dropdown; enchanting (no items) doesn't.
	local subs, anyItem = ResolveSubclasses(knownList);
	frame.subclasses = subs;
	if anyItem then
		UIDropDownMenu_Initialize(frame.dropdown, SubClassDropDown_Initialize);
		UIDropDownMenu_SetWidth(120, frame.dropdown);
		UIDropDownMenu_SetText(ALL_SUBCLASSES, frame.dropdown);
		frame.dropdown:Show();
	else
		frame.dropdown:Hide();
	end

	frame:Show();
	frame:ApplyFilter();
	return true;
end

-- Crafted items may not be cached when a link opens; as they arrive, re-derive
-- the subclass list and reapply the filter (debounced — GET_ITEM_INFO_RECEIVED
-- can fire once per item).
local subclassWatcher = CreateFrame("Frame");
subclassWatcher:RegisterEvent("GET_ITEM_INFO_RECEIVED");
subclassWatcher:SetScript("OnEvent", function()
	if subclassWatcher.pending or not (frame and frame:IsShown() and frame.recipes) then
		return;
	end
	subclassWatcher.pending = true;
	C_Timer.After(0.3, function()
		subclassWatcher.pending = nil;
		if not (frame and frame:IsShown() and frame.recipes) then
			return;
		end
		local subs, anyItem = ResolveSubclasses(frame.recipes);
		frame.subclasses = subs;
		if anyItem then
			UIDropDownMenu_Initialize(frame.dropdown, SubClassDropDown_Initialize);
		end
		frame:ApplyFilter();
	end);
end);

------------------------------------------------------------------------------
-- Click interception: handle `trade:` before the engine's SetHyperlink (which
-- in 1.12 only knows item:/enchant: and would reject the link).
------------------------------------------------------------------------------

local originalSetItemRef = SetItemRef;
function SetItemRef(link, text, button, chatFrame)
	if type(link) == "string" and string.sub(link, 1, 6) == "trade:" then
		-- Shift-click pastes the full link into the active chat editbox,
		-- exactly like an item link (1.12 has no IsModifiedClick); a plain
		-- click opens the viewer.
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
