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

local KNOWN_COLOR   = { 0.1, 1.0, 0.1 };   -- green: linker knows it
local UNKNOWN_COLOR = { 0.5, 0.5, 0.5 };   -- grey: linker doesn't
local NUM_ROWS      = 16;
local ROW_HEIGHT    = 16;

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

local function CreateFrame_TradeSkillLink()
	local f = CreateFrame("Frame", "ClassicAPITradeSkillLinkFrame", UIParent);
	f:SetWidth(320);
	f:SetHeight(NUM_ROWS * ROW_HEIGHT + 78);
	f:SetPoint("CENTER", 0, 0);
	f:SetBackdrop({
		bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
		edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
		tile = true, tileSize = 32, edgeSize = 32,
		insets = { left = 11, right = 12, top = 12, bottom = 11 },
	});
	f:SetMovable(true);
	f:EnableMouse(true);
	f:RegisterForDrag("LeftButton");
	f:SetScript("OnDragStart", function() this:StartMoving(); end);
	f:SetScript("OnDragStop", function() this:StopMovingOrSizing(); end);
	f:SetFrameStrata("DIALOG");
	f:Hide();

	local title = f:CreateFontString(nil, "OVERLAY", "GameFontNormal");
	title:SetPoint("TOP", 0, -16);
	f.title = title;

	local summary = f:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall");
	summary:SetPoint("TOP", title, "BOTTOM", 0, -4);
	f.summary = summary;

	local close = CreateFrame("Button", nil, f, "UIPanelCloseButton");
	close:SetPoint("TOPRIGHT", -8, -8);

	local scroll = CreateFrame("ScrollFrame", "ClassicAPITradeSkillLinkScroll",
		f, "FauxScrollFrameTemplate");
	scroll:SetPoint("TOPLEFT", 16, -46);
	scroll:SetPoint("BOTTOMRIGHT", -36, 16);
	scroll:SetScript("OnVerticalScroll", function()
		FauxScrollFrame_OnVerticalScroll(ROW_HEIGHT, function() f:Refresh(); end);
	end);
	f.scroll = scroll;

	f.rows = {};
	for i = 1, NUM_ROWS do
		local row = CreateFrame("Frame", nil, f);
		row:SetWidth(268);
		row:SetHeight(ROW_HEIGHT);
		if i == 1 then
			row:SetPoint("TOPLEFT", scroll, "TOPLEFT", 0, 0);
		else
			row:SetPoint("TOPLEFT", f.rows[i - 1], "BOTTOMLEFT", 0, 0);
		end

		local icon = row:CreateTexture(nil, "ARTWORK");
		icon:SetWidth(ROW_HEIGHT - 2);
		icon:SetHeight(ROW_HEIGHT - 2);
		icon:SetPoint("LEFT", 0, 0);
		row.icon = icon;

		local label = row:CreateFontString(nil, "OVERLAY", "GameFontNormalSmall");
		label:SetPoint("LEFT", icon, "RIGHT", 4, 0);
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
		local recipes = self.recipes or {};
		local total = table.getn(recipes);
		FauxScrollFrame_Update(self.scroll, total, NUM_ROWS, ROW_HEIGHT);
		local offset = FauxScrollFrame_GetOffset(self.scroll);
		for i = 1, NUM_ROWS do
			local row = self.rows[i];
			local index = offset + i;
			local entry = recipes[index];
			if entry then
				local name, _, icon = GetSpellInfo(entry.spellID);
				name = name or ("spell:" .. entry.spellID);
				row.label:SetText(name);
				row.icon:SetTexture(icon or "Interface\\Icons\\INV_Misc_QuestionMark");
				local c = entry.isKnown and KNOWN_COLOR or UNKNOWN_COLOR;
				row.label:SetTextColor(c[1], c[2], c[3]);
				row.spellID = entry.spellID;
				row:Show();
			else
				row.spellID = nil;
				row:Hide();
			end
		end
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
	-- Show only the recipes the linker actually knows, highest craft first
	-- (by the skill level it turns grey at; spellID breaks ties for stability).
	local total = table.getn(recipes);
	local knownList = {};
	for i = 1, total do
		if recipes[i].isKnown then
			table.insert(knownList, recipes[i]);
		end
	end
	table.sort(knownList, function(a, b)
		if a.trivialLevel ~= b.trivialLevel then
			return a.trivialLevel > b.trivialLevel;
		end
		return a.spellID < b.spellID;
	end);
	local known = table.getn(knownList);

	if not frame then
		frame = CreateFrame_TradeSkillLink();
	end
	local titleText;
	if who and who ~= "" then
		titleText = string.format("%s - %s (%d/%d)", who, prof, cur or 0, max or 0);
	else
		titleText = string.format("%s (%d/%d)", prof, cur or 0, max or 0);
	end
	frame.title:SetText(titleText);
	frame.summary:SetText(string.format("%d of %d recipes known", known, total));
	frame.recipes = knownList;
	-- Reset scroll to the top: the FauxScrollFrameTemplate slider is
	-- `<name>ScrollBar`; setting it to 0 fires OnVerticalScroll -> Refresh.
	local bar = getglobal("ClassicAPITradeSkillLinkScrollScrollBar");
	if bar then
		bar:SetValue(0);
	end
	frame:Show();
	frame:Refresh();
	return true;
end

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
