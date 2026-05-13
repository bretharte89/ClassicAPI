------------------------------------------------------------------------------
-- DebugTools.lua  (1.12 / Lua 5.0 backport of Blizzard_DebugTools 3.3.5)
--
-- Major changes from the 3.3.5 source:
--  * Lua 5.0 has no "..." varargs and no select(); script handlers take
--    explicit (this, event, arg1..arg9) parameters from XML inline scripts.
--  * RegisterAllEvents is used when available; the curated default list is
--    a fallback if the client doesn't expose it.
--  * No GameTooltip:SetFrameStack -- a basic walk over global frames is
--    used instead.
--  * No UIPanelDialogTemplate, no parentKey, no clampedToScreen, no
--    OnSizeChanged in vanilla -- frames have fixed dimensions.
--  * No debuglocals
------------------------------------------------------------------------------

EVENT_TRACE_EVENT_HEIGHT = 16;
EVENT_TRACE_MAX_ENTRIES  = 1000;
EVENT_TRACE_NUM_BUTTONS  = 28;
EVENT_TRACE_FRAME_HEIGHT = (EVENT_TRACE_NUM_BUTTONS * EVENT_TRACE_EVENT_HEIGHT) + 36;

DEBUGLOCALS_LEVEL = 4;
local _normalFontColor = { 1, .82, 0, 1 };

EVENT_TRACE_SYSTEM_TIMES = {};
EVENT_TRACE_SYSTEM_TIMES["System"]  = true;
EVENT_TRACE_SYSTEM_TIMES["Elapsed"] = true;

EVENT_TRACE_EVENT_COLORS = {};
EVENT_TRACE_EVENT_COLORS["System"]  = _normalFontColor;
EVENT_TRACE_EVENT_COLORS["Elapsed"] = { .6, .6, .6, 1 };

-- Vanilla has no Frame:IsMouseOver(); do it ourselves from cursor + bounds.
local function DebugTools_IsMouseOver(frame)
    if (not frame) then return false; end
    local visOK, visible = pcall(frame.IsVisible, frame);
    if (not visOK or not visible) then return false; end
    local sOK, scale = pcall(frame.GetEffectiveScale, frame);
    if (not sOK or not scale or scale == 0) then return false; end
    local cx, cy = GetCursorPosition();
    cx = cx / scale;
    cy = cy / scale;
    local lOK, left   = pcall(frame.GetLeft,   frame);
    local rOK, right  = pcall(frame.GetRight,  frame);
    local tOK, top    = pcall(frame.GetTop,    frame);
    local bOK, bottom = pcall(frame.GetBottom, frame);
    if (not (lOK and rOK and tOK and bOK)) then return false; end
    if (not (left and right and top and bottom)) then return false; end
    return cx >= left and cx <= right and cy >= bottom and cy <= top;
end

local _EventTraceFrame;

_framesSinceLast = 0;
_timeSinceLast   = 0;

local _timer = CreateFrame("Frame");
_timer:SetScript("OnUpdate", function()
    _framesSinceLast = _framesSinceLast + 1;
    _timeSinceLast   = _timeSinceLast + (arg1 or 0);
end);

------------------------------------------------------------------------------
-- EventTraceFrame
------------------------------------------------------------------------------

function EventTraceFrame_OnLoad(self)
    self.buttons        = {};
    self.events         = {};
    self.times          = {};
    self.rawtimes       = {};
    self.args           = { {}, {}, {}, {}, {}, {}, {}, {}, {} };
    self.ignoredEvents  = {};
    self.lastIndex      = 0;
    self.visibleButtons = EVENT_TRACE_NUM_BUTTONS;
    _EventTraceFrame    = self;

    self:EnableMouse(true);
    self:EnableMouseWheel(true);
    self:SetScript("OnMouseWheel", function() EventTraceFrame_OnMouseWheel(this, arg1); end);

    -- Pre-create the fixed list of buttons.
    for i = 1, EVENT_TRACE_NUM_BUTTONS do
        local button = CreateFrame("Button", "EventTraceFrameButton" .. i,
                                   self, "EventTraceEventTemplate");
        button:SetPoint("BOTTOMLEFT", 12, (EVENT_TRACE_EVENT_HEIGHT * (i - 1)) + 12);
        button:SetPoint("RIGHT", -28, 0);
        table.insert(self.buttons, button);
        button:Show();
    end
end

function EventTraceFrame_OnEvent(self, event, a1, a2, a3, a4, a5, a6, a7, a8, a9)
    if (self.ignoredEvents[event]) then return; end

    if (not self.ignoreElapsed and _framesSinceLast ~= 0) then
        self.ignoreElapsed = true;
        EventTraceFrame_OnEvent(self, "On Update");
        self.ignoreElapsed = nil;
    end

    local nextIndex = self.lastIndex + 1;
    if (nextIndex > EVENT_TRACE_MAX_ENTRIES) then
        local staleIndex = nextIndex - EVENT_TRACE_MAX_ENTRIES;
        self.events[staleIndex]   = nil;
        self.times[staleIndex]    = nil;
        self.rawtimes[staleIndex] = nil;
        for k, v in pairs(self.args) do
            self.args[k][staleIndex] = nil;
        end
    end

    if (string.find(event, "Begin Capture", 1, true) or
        string.find(event, "End Capture", 1, true)) then
        self.times[nextIndex] = "System";
        if (self.eventsToCapture) then
            self.events[nextIndex] = string.format("%s (%s events)", event,
                                                   tostring(self.eventsToCapture));
        else
            self.events[nextIndex] = event;
        end
    elseif (event == "On Update") then
        self.times[nextIndex] = "Elapsed";
        self.events[nextIndex] = string.format(
            string.format("%.3f sec", _timeSinceLast) .. " - %d frame(s)",
            _framesSinceLast);
        _timeSinceLast   = 0;
        _framesSinceLast = 0;
    else
        self.events[nextIndex] = event;
        local seconds = GetTime();
        local minutes = math.floor(math.floor(seconds) / 60);
        local hours   = math.floor(minutes / 60);
        seconds = seconds - 60 * minutes;
        minutes = minutes - 60 * hours;
        hours = math.mod(hours, 1000);
        self.times[nextIndex] = string.format("%.2d:%.2d:%06.3f", hours, minutes, seconds);

        local packed = { a1, a2, a3, a4, a5, a6, a7, a8, a9 };
        local numArgs = 0;
        for i = 9, 1, -1 do
            if (packed[i] ~= nil) then
                numArgs = i;
                break;
            end
        end
        for i = 1, numArgs do
            if (not self.args[i]) then self.args[i] = {}; end
            self.args[i][nextIndex] = packed[i];
        end

        if (self.eventsToCapture) then
            self.eventsToCapture = self.eventsToCapture - 1;
        end
    end

    self.rawtimes[nextIndex] = GetTime();
    self.lastIndex = nextIndex;
    EventTraceFrame_Update();
    if (self.eventsToCapture and self.eventsToCapture <= 0) then
        self.eventsToCapture = nil;
        EventTraceFrame_StopEventCapture();
    end
end

function EventTraceFrame_OnUpdate(self, elapsed)
    EventTraceFrame_Update();
end

function EventTraceFrame_Update()
    if (not _EventTraceFrame or not _EventTraceFrame.buttons) then
        -- Slider OnValueChanged can fire during slider OnLoad, which happens
        -- before EventTraceFrame_OnLoad in 1.12 XML load order.
        return;
    end
    local scrollBar = getglobal("EventTraceFrameScroll");
    if (not scrollBar) then return; end
    local scrollBarValue = scrollBar:GetValue();
    local minValue, maxValue = scrollBar:GetMinMaxValues();

    local firstID = math.max(1, _EventTraceFrame.lastIndex - EVENT_TRACE_MAX_ENTRIES + 1);
    local lastID  = _EventTraceFrame.lastIndex or 1;

    if (firstID >= lastID) then
        scrollBar:SetMinMaxValues(firstID - 1, lastID);
    else
        scrollBar:SetMinMaxValues(firstID, lastID);
    end
    if (scrollBarValue < firstID) then
        scrollBar:SetValue(firstID);
        scrollBarValue = firstID;
    end

    if (scrollBarValue < 1) then
        scrollBarValue = 1;
    elseif (not _EventTraceFrame.selectedEvent) then
        if (scrollBarValue == maxValue) then
            scrollBar:SetValue(_EventTraceFrame.lastIndex);
        end
    end

    for i = 1, _EventTraceFrame.visibleButtons do
        local button = _EventTraceFrame.buttons[i];
        if (button) then
            local index = scrollBarValue - (i - 1);
            local event = _EventTraceFrame.events[index];
            if (event) then
                local timeString = _EventTraceFrame.times[index];
                button.index = index;
                local timeFS  = getglobal(button:GetName() .. "Time");
                local eventFS = getglobal(button:GetName() .. "Event");
                timeFS:SetText(timeString);
                eventFS:SetText(event);
                local color = EVENT_TRACE_EVENT_COLORS[event]
                              or EVENT_TRACE_EVENT_COLORS[timeString];
                if (color) then
                    timeFS:SetTextColor(unpack(color));
                    eventFS:SetTextColor(unpack(color));
                else
                    timeFS:SetTextColor(1, 1, 1, 1);
                    eventFS:SetTextColor(1, 1, 1, 1);
                end
                button:Show();
                if (_EventTraceFrame.selectedEvent) then
                    if (index == _EventTraceFrame.selectedEvent) then
                        EventTraceFrameEvent_DisplayTooltip(button);
                        button:GetHighlightTexture():SetVertexColor(.15, .25, 1, .35);
                        button:LockHighlight();
                        button.wasSelected = true;
                    elseif (button.wasSelected) then
                        button.wasSelected = nil;
                        button:GetHighlightTexture():SetVertexColor(.8, .8, 1, .15);
                        button:UnlockHighlight();
                    end
                else
                    if (button.wasSelected) then
                        button.wasSelected = nil;
                        button:GetHighlightTexture():SetVertexColor(.8, .8, 1, .15);
                        button:UnlockHighlight();
                    end
                    if (DebugTools_IsMouseOver(button)) then
                        EventTraceFrameEvent_OnEnter(button);
                    end
                end
            else
                button.index = index;
                button:Hide();
            end
        end
    end
end

function EventTraceFrame_StartEventCapture()
    if (_EventTraceFrame.started) then return; end

    _EventTraceFrame.started = true;
    _framesSinceLast = 0;
    _timeSinceLast   = 0;

    _EventTraceFrame:RegisterAllEvents();

    EventTraceFrame_OnEvent(_EventTraceFrame, "Begin Capture");
end

function EventTraceFrame_StopEventCapture()
    if (not _EventTraceFrame.started) then return; end

    _EventTraceFrame.started = false;
    _framesSinceLast = 0;
    _timeSinceLast   = 0;
    _EventTraceFrame:UnregisterAllEvents();
    EventTraceFrame_OnEvent(_EventTraceFrame, "End Capture");
end

local function _ChatPrint(msg)
    DEFAULT_CHAT_FRAME:AddMessage("|cff88ff88EventTrace:|r " .. msg);
end

function EventTraceFrame_HandleSlashCmd(msg)
    if (msg == nil) then msg = ""; end
    msg = string.lower(msg);

    -- /etrace add EVENT
    local _, _, addEvent = string.find(msg, "^add%s+(%S+)");
    if (addEvent) then
        addEvent = string.upper(addEvent);
        if (_EventTraceFrame.started) then
            _EventTraceFrame:RegisterEvent(addEvent);
        end
        _ChatPrint("added " .. addEvent);
        return;
    end

    -- /etrace remove EVENT
    local _, _, rmEvent = string.find(msg, "^remove%s+(%S+)");
    if (rmEvent) then
        rmEvent = string.upper(rmEvent);
        if (_EventTraceFrame.started) then
            _EventTraceFrame:UnregisterEvent(rmEvent);
        end
        _ChatPrint("removed " .. rmEvent);
        return;
    end

    -- /etrace ignore EVENT
    local _, _, igEvent = string.find(msg, "^ignore%s+(%S+)");
    if (igEvent) then
        igEvent = string.upper(igEvent);
        _EventTraceFrame.ignoredEvents[igEvent] = true;
        _ChatPrint("ignoring " .. igEvent);
        return;
    end

    if (msg == "start") then
        EventTraceFrame_StartEventCapture();
    elseif (msg == "stop") then
        EventTraceFrame_StopEventCapture();
    elseif (msg == "clear") then
        _EventTraceFrame.events   = {};
        _EventTraceFrame.times    = {};
        _EventTraceFrame.rawtimes = {};
        _EventTraceFrame.args     = { {}, {}, {}, {}, {}, {}, {}, {}, {} };
        _EventTraceFrame.lastIndex = 0;
        EventTraceFrame_Update();
    elseif (tonumber(msg) and tonumber(msg) > 0) then
        if (not _EventTraceFrame.started) then
            _EventTraceFrame.eventsToCapture = tonumber(msg);
            EventTraceFrame_StartEventCapture();
        end
    elseif (msg == "" or msg == "toggle") then
        if (not _EventTraceFrame:IsShown()) then
            _EventTraceFrame:Show();
            if (_EventTraceFrame.started == nil) then
                EventTraceFrame_StartEventCapture();
            end
        else
            _EventTraceFrame:Hide();
        end
    else
        _ChatPrint("usage:");
        _ChatPrint("  /etrace            -- toggle window (auto-start first time)");
        _ChatPrint("  /etrace start      -- begin event capture");
        _ChatPrint("  /etrace stop       -- end event capture");
        _ChatPrint("  /etrace <N>        -- capture next N events");
        _ChatPrint("  /etrace add <EV>   -- register additional event");
        _ChatPrint("  /etrace remove <EV>-- unregister event");
        _ChatPrint("  /etrace ignore <EV>-- hide event from display");
        _ChatPrint("  /etrace clear      -- clear log");
    end
end

function EventTraceFrame_OnMouseWheel(self, delta)
    local scrollBar = getglobal("EventTraceFrameScroll");
    local minVal, maxVal = scrollBar:GetMinMaxValues();
    local currentValue = scrollBar:GetValue();

    local newValue = currentValue - (delta * 3);
    if (newValue < minVal) then newValue = minVal; end
    if (newValue > maxVal) then newValue = maxVal; end
    if (newValue ~= currentValue) then
        scrollBar:SetValue(newValue);
    end
end

local TIME_ENTRY_FORMAT     = "Time:";
local DETAILS_ENTRY_FORMAT  = "Details:";
local ARGUMENT_ENTRY_FORMAT = "arg %d:";

local function EventTrace_FormatArgValue(val)
    local t = type(val);
    if (t == "string") then
        return string.format('"%s"', val);
    elseif (t == "number") then
        return tostring(val);
    elseif (t == "boolean") then
        return string.format('|cffaaaaff%s|r', tostring(val));
    elseif (t == "table" or t == "function" or t == "userdata") then
        return string.format('|cffffaaaa%s|r', tostring(val));
    end
    return tostring(val);
end

function EventTraceFrameEvent_DisplayTooltip(eventButton)
    local index = eventButton.index;
    if (not index) then return; end

    local tooltip = EventTraceTooltip;
    tooltip:SetOwner(eventButton, "ANCHOR_NONE");
    tooltip:SetPoint("TOPLEFT", eventButton, "TOPRIGHT", 24, 2);
    local timeString = _EventTraceFrame.times[index];
    if (EVENT_TRACE_SYSTEM_TIMES[timeString]) then
        tooltip:AddLine(timeString, 1, 1, 1);
        tooltip:AddDoubleLine(TIME_ENTRY_FORMAT,
                              tostring(_EventTraceFrame.rawtimes[index]),
                              1, .82, 0, 1, 1, 1);
        tooltip:AddDoubleLine(DETAILS_ENTRY_FORMAT,
                              _EventTraceFrame.events[index],
                              1, .82, 0, 1, 1, 1);
    else
        tooltip:AddLine(_EventTraceFrame.events[index], 1, 1, 1);
        tooltip:AddDoubleLine(TIME_ENTRY_FORMAT,
                              tostring(_EventTraceFrame.rawtimes[index]),
                              1, .82, 0, 1, 1, 1);
        for k, v in ipairs(_EventTraceFrame.args) do
            if (v[index] ~= nil) then
                tooltip:AddDoubleLine(string.format(ARGUMENT_ENTRY_FORMAT, k),
                                      EventTrace_FormatArgValue(v[index]),
                                      1, .82, 0, 1, 1, 1);
            end
        end
    end
    tooltip:Show();
end

function EventTraceFrameEvent_OnEnter(self)
    if (_EventTraceFrame.selectedEvent) then return; end
    EventTraceFrameEvent_DisplayTooltip(self);
end

function EventTraceFrameEvent_OnLeave(self)
    if (not _EventTraceFrame.selectedEvent) then
        EventTraceTooltip:Hide();
    end
end

function EventTraceFrameEvent_OnClick(self)
    if (_EventTraceFrame.selectedEvent == self.index) then
        _EventTraceFrame.selectedEvent = nil;
    else
        _EventTraceFrame.selectedEvent = self.index;
    end
    EventTraceFrame_Update();
end

------------------------------------------------------------------------------
-- ScriptErrorsFrame
------------------------------------------------------------------------------

local ERROR_FORMAT = "|cffffd200Message:|r|cffffffff %s|r\n"
                  .. "|cffffd200Time:|r|cffffffff %s|r\n"
                  .. "|cffffd200Count:|r|cffffffff %s|r\n"
                  .. "|cffffd200Stack:|r|cffffffff %s|r";

local INDEX_ORDER_FORMAT = "%d / %d";

local _ScriptErrorsFrame;
local _previousErrorHandler;

function ScriptErrorsFrame_OnLoad(self)
    local titleFS = getglobal(self:GetName() .. "Title");
    if (titleFS) then
        titleFS:SetText("Lua Error");
    end
    self:RegisterForDrag("LeftButton");
    self.seen     = {};
    self.order    = {};
    self.count    = {};
    self.messages = {};
    self.times    = {};
    _ScriptErrorsFrame = self;
end

function ScriptErrorsFrame_OnShow(self)
    ScriptErrorsFrame_Update();
end

function ScriptErrorsFrame_OnError(message, keepHidden)
    if (message == nil) then return; end
    local stack = debugstack(DEBUGLOCALS_LEVEL) or "";

    local messageStack = message .. stack;

    if (_ScriptErrorsFrame) then
        local index = _ScriptErrorsFrame.seen[messageStack];
        if (index) then
            _ScriptErrorsFrame.count[index]    = _ScriptErrorsFrame.count[index] + 1;
            _ScriptErrorsFrame.messages[index] = message;
            _ScriptErrorsFrame.times[index]    = date();
        else
            table.insert(_ScriptErrorsFrame.order, stack);
            index = table.getn(_ScriptErrorsFrame.order);
            _ScriptErrorsFrame.count[index]    = 1;
            _ScriptErrorsFrame.messages[index] = message;
            _ScriptErrorsFrame.times[index]    = date();
            _ScriptErrorsFrame.seen[messageStack] = index;
        end

        if (not _ScriptErrorsFrame:IsShown() and not keepHidden) then
            _ScriptErrorsFrame.index = index;
            _ScriptErrorsFrame:Show();
        else
            ScriptErrorsFrame_Update();
        end
    end

    -- Chain through to whatever previous handler existed (e.g. BugGrabber)
    if (_previousErrorHandler and _previousErrorHandler ~= ScriptErrorsFrame_OnError) then
        pcall(_previousErrorHandler, message);
    end
end

function ScriptErrorsFrame_Update()
    local editBox = getglobal("ScriptErrorsFrameScrollFrameText");
    local index = _ScriptErrorsFrame.index;
    if (not index or not _ScriptErrorsFrame.order[index]) then
        index = table.getn(_ScriptErrorsFrame.order);
        _ScriptErrorsFrame.index = index;
    end

    if (index == 0) then
        editBox:SetText("");
        ScriptErrorsFrame_UpdateButtons();
        return;
    end

    local text = string.format(ERROR_FORMAT,
        _ScriptErrorsFrame.messages[index],
        _ScriptErrorsFrame.times[index],
        _ScriptErrorsFrame.count[index],
        _ScriptErrorsFrame.order[index]);

    editBox:SetText(text);
    editBox:HighlightText(0);
    editBox:SetCursorPosition(0);

    local scrollFrame = getglobal("ScriptErrorsFrameScrollFrame");
    if (scrollFrame and scrollFrame.SetVerticalScroll) then
        scrollFrame:SetVerticalScroll(0);
    end

    ScriptErrorsFrame_UpdateButtons();
end

function ScriptErrorsFrame_UpdateButtons()
    local index = _ScriptErrorsFrame.index;
    local numErrors = table.getn(_ScriptErrorsFrame.order);
    local nextBtn = getglobal("ScriptErrorsFrameNext");
    local prevBtn = getglobal("ScriptErrorsFramePrevious");
    local idxLbl  = getglobal("ScriptErrorsFrameIndexLabel");

    if (index == 0 or numErrors == 0) then
        nextBtn:Disable();
        prevBtn:Disable();
    else
        if (numErrors == 1) then
            nextBtn:Disable();
            prevBtn:Disable();
        elseif (index == 1) then
            nextBtn:Enable();
            prevBtn:Disable();
        elseif (index == numErrors) then
            nextBtn:Disable();
            prevBtn:Enable();
        else
            nextBtn:Enable();
            prevBtn:Enable();
        end
    end

    idxLbl:SetText(string.format(INDEX_ORDER_FORMAT, index, numErrors));
end

function ScriptErrorsFrameButton_OnClick(self)
    local id = self:GetID();
    if (id == 1) then
        _ScriptErrorsFrame.index = _ScriptErrorsFrame.index + 1;
    else
        _ScriptErrorsFrame.index = _ScriptErrorsFrame.index - 1;
    end
    ScriptErrorsFrame_Update();
end

function ScriptErrorsFrame_TitleButton_OnDragStart()
    _ScriptErrorsFrame.moving = true;
    _ScriptErrorsFrame:StartMoving();
end

function ScriptErrorsFrame_TitleButton_OnDragStop()
    _ScriptErrorsFrame.moving = nil;
    _ScriptErrorsFrame:StopMovingOrSizing();
end

function ScriptErrorsFrame_HandleSlashCmd(msg)
    if (msg == nil) then msg = ""; end
    msg = string.lower(msg);
    if (msg == "on" or msg == "install") then
        _previousErrorHandler = geterrorhandler();
        seterrorhandler(ScriptErrorsFrame_OnError);
        _ChatPrint("Lua error capture installed.");
    elseif (msg == "off" or msg == "uninstall") then
        if (_previousErrorHandler) then
            seterrorhandler(_previousErrorHandler);
        end
        _ChatPrint("Lua error capture uninstalled.");
    elseif (msg == "test") then
        ScriptErrorsFrame_OnError("test error from /luaerrors test");
    elseif (msg == "clear") then
        _ScriptErrorsFrame.seen     = {};
        _ScriptErrorsFrame.order    = {};
        _ScriptErrorsFrame.count    = {};
        _ScriptErrorsFrame.messages = {};
        _ScriptErrorsFrame.times    = {};
        _ScriptErrorsFrame.index    = 0;
        ScriptErrorsFrame_Update();
    else
        if (_ScriptErrorsFrame:IsShown()) then
            _ScriptErrorsFrame:Hide();
        else
            _ScriptErrorsFrame:Show();
        end
    end
end

------------------------------------------------------------------------------
-- FrameStackTooltip
------------------------------------------------------------------------------

function DebugTooltip_OnLoad(self)
    self:SetFrameLevel(self:GetFrameLevel() + 2);
    if (TOOLTIP_DEFAULT_COLOR and self.SetBackdropBorderColor) then
        self:SetBackdropBorderColor(TOOLTIP_DEFAULT_COLOR.r,
                                    TOOLTIP_DEFAULT_COLOR.g,
                                    TOOLTIP_DEFAULT_COLOR.b);
    end
    if (TOOLTIP_DEFAULT_BACKGROUND_COLOR and self.SetBackdropColor) then
        self:SetBackdropColor(TOOLTIP_DEFAULT_BACKGROUND_COLOR.r,
                              TOOLTIP_DEFAULT_BACKGROUND_COLOR.g,
                              TOOLTIP_DEFAULT_BACKGROUND_COLOR.b);
    end
end

local function _IsFrameLike(v)
    if (type(v) ~= "table") then return false; end
    if (type(v.IsObjectType) ~= "function") then return false; end
    if (type(v.GetFrameLevel) ~= "function") then return false; end
    if (type(v.GetLeft) ~= "function") then return false; end
    return true;
end

local _STRATA_RANK = {
    BACKGROUND = 1, LOW = 2, MEDIUM = 3, HIGH = 4,
    DIALOG = 5, FULLSCREEN = 6, FULLSCREEN_DIALOG = 7, TOOLTIP = 8,
};

local function _CollectFramesUnderMouse(showHidden)
    local matches = {};
    for k, v in pairs(getfenv(0)) do
        if (_IsFrameLike(v)) then
            local ok, isFrame = pcall(v.IsObjectType, v, "Frame");
            if (ok and isFrame) then
                local visOK, visible = pcall(v.IsVisible, v);
                local mouseOver = DebugTools_IsMouseOver(v);
                if (mouseOver and (showHidden or (visOK and visible))) then
                    table.insert(matches, { name = k, frame = v });
                end
            end
        end
    end
    table.sort(matches, function(a, b)
        local sa = _STRATA_RANK[a.frame:GetFrameStrata()] or 0;
        local sb = _STRATA_RANK[b.frame:GetFrameStrata()] or 0;
        if (sa ~= sb) then return sa > sb; end
        local la = a.frame:GetFrameLevel() or 0;
        local lb = b.frame:GetFrameLevel() or 0;
        return la > lb;
    end);
    return matches;
end

local function _UpdateFrameStackTooltip(tooltip, showHidden)
    tooltip:ClearLines();
    tooltip:AddLine("Frame Stack", 1, 1, 1);
    local matches = _CollectFramesUnderMouse(showHidden);
    local count = table.getn(matches);
    if (count == 0) then
        tooltip:AddLine("(no frames under cursor)", .7, .7, .7);
    else
        local shown = 0;
        for i = 1, count do
            local m = matches[i];
            local strata = m.frame:GetFrameStrata() or "?";
            local level  = m.frame:GetFrameLevel() or 0;
            local label = string.format("%s |cff888888[%s:%d]|r",
                                        m.name, strata, level);
            tooltip:AddLine(label, 1, .82, 0);
            shown = shown + 1;
            if (shown >= 30) then
                tooltip:AddLine(string.format("|cffff8888...%d more|r",
                                              count - shown),
                                1, 1, 1);
                break;
            end
        end
    end
    tooltip:Show();
end

function FrameStackTooltip_Toggle(showHidden)
    local tooltip = FrameStackTooltip;
    if (tooltip:IsVisible()) then
        tooltip:Hide();
    else
        tooltip:SetOwner(UIParent, "ANCHOR_NONE");
        tooltip:SetPoint("BOTTOMRIGHT", UIParent, "BOTTOMRIGHT", -32, 96);
        tooltip.default = 1;
        tooltip.showHidden = showHidden;
        _UpdateFrameStackTooltip(tooltip, showHidden);
    end
end

FRAMESTACK_UPDATE_TIME = .1;
local _frameStackAccum = 0;

function FrameStackTooltip_OnUpdate(self, elapsed)
    _frameStackAccum = _frameStackAccum + elapsed;
    if (_frameStackAccum >= FRAMESTACK_UPDATE_TIME) then
        _frameStackAccum = 0;
        _UpdateFrameStackTooltip(self, self.showHidden);
    end
end

function FrameStackTooltip_OnShow(self)
    local parent = self:GetParent() or UIParent;
    local ps = parent:GetEffectiveScale();
    local px, py = parent:GetCenter();
    if (not px or not py) then return; end
    px, py = px * ps, py * ps;
    local x, y = GetCursorPosition();
    self:ClearAllPoints();
    if (x > px) then
        if (y > py) then
            self:SetPoint("BOTTOMLEFT", parent, "BOTTOMLEFT", 20, 20);
        else
            self:SetPoint("TOPLEFT", parent, "TOPLEFT", 20, -20);
        end
    else
        if (y > py) then
            self:SetPoint("BOTTOMRIGHT", parent, "BOTTOMRIGHT", -20, 20);
        else
            self:SetPoint("TOPRIGHT", parent, "TOPRIGHT", -20, -20);
        end
    end
end

FrameStackTooltip_OnEnter = FrameStackTooltip_OnShow;

function FrameStackTooltip_HandleSlashCmd(msg)
    if (msg == nil) then msg = ""; end
    local showHidden = (string.find(msg, "hidden") ~= nil);
    FrameStackTooltip_Toggle(showHidden);
end

------------------------------------------------------------------------------
-- Slash command registration
------------------------------------------------------------------------------

SLASH_ETRACE1 = "/etrace";
SlashCmdList["ETRACE"] = EventTraceFrame_HandleSlashCmd;

SLASH_FRAMESTACK1 = "/framestack";
SLASH_FRAMESTACK2 = "/fstack";
SlashCmdList["FRAMESTACK"] = FrameStackTooltip_HandleSlashCmd;

SLASH_LUAERRORS1 = "/luaerrors";
SLASH_LUAERRORS2 = "/scripterrors";
SlashCmdList["LUAERRORS"] = ScriptErrorsFrame_HandleSlashCmd;
