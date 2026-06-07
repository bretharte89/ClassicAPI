-- AoE Loot Test
--
-- /aoeloot opens a picker window listing every item in every lootable
-- corpse within click range. Clicking a row hands the (guid, itemID)
-- pair to C_Loot.LootUnitItem and re-scans so the picker refreshes
-- with whatever's left.
--
-- Known visual: each scan opens / closes loot windows in rapid
-- succession, so the Blizzard LootFrame flickers during the scan. This
-- addon intentionally doesn't suppress that — the goal is to exercise
-- the C++ surface, not to ship polished UI.

local MAX_ROWS = 24
local RESCAN_DELAY = 0.3 -- seconds; lets LootUnitItem complete before re-scan

-- ---------- UI scaffolding ----------

-- Frame geometry. Height is recomputed dynamically per scan so the
-- window only takes up as much screen real estate as the result count
-- needs — see `ResizeForRows`.
local FRAME_WIDTH = 320
local HEADER_HEIGHT = 64 -- title + status + insets; minimum frame height
local ROW_HEIGHT = 22

local frame = CreateFrame("Frame", "AoELootTestFrame", UIParent)
frame:SetWidth(FRAME_WIDTH)
frame:SetHeight(HEADER_HEIGHT)
frame:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
frame:SetBackdrop({
    bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
    edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
    tile = true, tileSize = 32, edgeSize = 32,
    insets = { left = 11, right = 12, top = 12, bottom = 11 },
})
frame:SetMovable(true)
frame:EnableMouse(true)
frame:RegisterForDrag("LeftButton")
-- Vanilla 1.12 fires script handlers with the frame set as the global
-- `this`, not as a `self` arg to the function. Passing `frame.StartMoving`
-- directly (the modern shortcut) breaks because the C method has no
-- `self` to operate on. Wrap in closures so `this:method()` resolves
-- correctly inside the handler.
frame:SetScript("OnDragStart", function() this:StartMoving() end)
frame:SetScript("OnDragStop", function() this:StopMovingOrSizing() end)
frame:Hide()

local title = frame:CreateFontString(nil, "OVERLAY", "GameFontNormal")
title:SetPoint("TOP", frame, "TOP", 0, -16)
title:SetText("AoE Loot")

local status = frame:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
status:SetPoint("TOPLEFT", frame, "TOPLEFT", 18, -36)
status:SetText("")

local closeBtn = CreateFrame("Button", nil, frame, "UIPanelCloseButton")
closeBtn:SetPoint("TOPRIGHT", frame, "TOPRIGHT", -6, -6)

-- ---------- Row pool ----------

local rows = {}

local function CreateRow(parent, idx)
    local row = CreateFrame("Button", nil, parent)
    row:SetWidth(290)
    row:SetHeight(20)
    row:SetPoint("TOPLEFT", parent, "TOPLEFT", 15,
                 -54 - (idx - 1) * ROW_HEIGHT)

    local icon = row:CreateTexture(nil, "ARTWORK")
    icon:SetWidth(18); icon:SetHeight(18)
    icon:SetPoint("LEFT", row, "LEFT", 0, 0)
    row.icon = icon

    local text = row:CreateFontString(nil, "OVERLAY", "GameFontNormal")
    text:SetPoint("LEFT", icon, "RIGHT", 6, 0)
    text:SetPoint("RIGHT", row, "RIGHT", 0, 0)
    text:SetJustifyH("LEFT")
    row.text = text

    local hl = row:CreateTexture(nil, "HIGHLIGHT")
    hl:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
    hl:SetBlendMode("ADD")
    hl:SetAllPoints(row)

    -- Tooltip on hover. The engine's native `SetHyperlink` does a
    -- literal `SStrCmpI` of the input's first 5 chars against the
    -- string `"item:"` at `.rdata` VA `0x00842D60` — anything that
    -- starts with `|c...` (full link with color) or `|Hitem` (full
    -- link without color) fails the prefix check with "Unknown link
    -- type". Only the bare payload form survives.
    --
    -- We extract the payload from the engine's full link rather than
    -- composing `item:ID:0:0:0` ourselves, so per-instance fields
    -- (enchant / random-suffix / unique) survive round-trip and
    -- "Stringy Wolf Meat of the Bear" tooltips correctly instead of
    -- collapsing to the un-suffixed itemID.
    row:SetScript("OnEnter", function()
        if this.payload then
            GameTooltip:SetOwner(this, "ANCHOR_RIGHT")
            GameTooltip:SetHyperlink(this.payload)
            GameTooltip:Show()
        end
    end)
    row:SetScript("OnLeave", function() GameTooltip:Hide() end)

    row:Hide()
    return row
end

for i = 1, MAX_ROWS do
    rows[i] = CreateRow(frame, i)
end

-- ---------- Render ----------

-- Tail of a GUID string — last 4 hex chars of the low dword. Lets the
-- user tell apart two rows of "Stringy Wolf Meat x1" coming from
-- different corpses, without occupying a whole column with the full
-- 16-digit GUID.
local function ShortGuidSuffix(guid)
    return "[.." .. string.sub(guid, -4) .. "]"
end

-- Extract the `item:ID:enchant:suffix:unique` payload out of the
-- engine's full link string (`|cff...|Hitem:...|h[Name]|h|r`).
-- Vanilla `SetHyperlink` requires the input to start literally with
-- `"item:"` — so we can't pass the full link, but we also can't drop
-- the per-instance fields (random suffix is what distinguishes "of
-- the Bear" from "of the Eagle" on otherwise identical itemIDs).
-- Returns the payload, or nil if extraction fails.
local function PayloadFromLink(link)
    if not link then return nil end
    return string.match(link, "|H(item:[^|]+)|h")
end

local function ClearRows()
    for i = 1, MAX_ROWS do
        rows[i]:Hide()
        rows[i]:SetScript("OnClick", nil)
        rows[i].payload = nil
    end
end

local function PopulateRows(results)
    local idx = 0
    for _, corpse in ipairs(results) do
        for _, item in ipairs(corpse.items) do
            idx = idx + 1
            if idx > MAX_ROWS then break end
            local row = rows[idx]
            local name, _, _, _, _, _, _, _, icon = GetItemInfo(item.itemID)
            row.icon:SetTexture(icon or "Interface\\Icons\\INV_Misc_QuestionMark")
            local label = name or ("Item #" .. item.itemID)
            if item.count and item.count > 1 then
                label = label .. " x" .. item.count
            end
            label = label .. "  " .. ShortGuidSuffix(corpse.guid)
            row.text:SetText(label)
            row.payload = PayloadFromLink(item.link) -- for OnEnter tooltip
            local guid, itemID = corpse.guid, item.itemID
            row:SetScript("OnClick", function()
                C_Loot.LootUnitItem(guid, itemID)
                C_Timer.After(RESCAN_DELAY, function()
                    if frame:IsShown() then C_Loot.ScanNearbyLoot() end
                end)
            end)
            row:Show()
        end
        if idx > MAX_ROWS then break end
    end
    for i = idx + 1, MAX_ROWS do rows[i]:Hide() end
    local shown
    if idx == 0 then
        status:SetText("Nothing lootable in range.")
        shown = 0
    elseif idx > MAX_ROWS then
        status:SetText(MAX_ROWS .. "+ items (showing first " .. MAX_ROWS .. ")")
        shown = MAX_ROWS
    else
        status:SetText(idx .. " item(s) available")
        shown = idx
    end
    frame:SetHeight(HEADER_HEIGHT + shown * ROW_HEIGHT)
end

-- ---------- Scan trigger ----------

local function StartScan()
    status:SetText("Scanning...")
    ClearRows()
    frame:SetHeight(HEADER_HEIGHT) -- collapse while we wait for results
    if not C_Loot.ScanNearbyLoot() then
        status:SetText("Scan busy or not in world.")
    end
end

-- ---------- Event wiring ----------

local evt = CreateFrame("Frame")
evt:RegisterEvent("LOOT_SCAN_COMPLETED")
evt:SetScript("OnEvent", function()
    -- Race guard: a delayed scan may complete after the user closed
    -- the picker. Just drop the result rather than re-showing.
    if not frame:IsShown() then return end
    PopulateRows(C_Loot.GetLastScanResults())
end)

-- ---------- Slash command ----------

RegisterNewSlashCommand(function()
    if frame:IsShown() then
        StartScan() -- already open → just refresh
    else
        frame:Show()
        StartScan()
    end
end, "aoeloot", "aoe")
