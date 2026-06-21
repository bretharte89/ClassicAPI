-- Backport of FrameXML/TimeUtil.lua (feasible subset) to vanilla 1.12 /
-- Lua 5.0.
--
-- Ports the deprecated-but-widely-used duration formatters (SecondsToTime,
-- SecondsToClock, SecondsToTimeAbbrev) plus the unit constants/helpers. The
-- SecondsFormatterMixin (the modern replacement) is omitted — it's a heavy
-- mixin class little addon code targets on classic.
--
-- 5.0 notes: `%` -> math.mod, `s:format()` -> string.format. FrameXML
-- format-string globals get `or` fallbacks so native/localized values win
-- when present.

SECONDS_PER_MIN = 60;
SECONDS_PER_HOUR = 60 * SECONDS_PER_MIN;
SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR;
SECONDS_PER_MONTH = 30 * SECONDS_PER_DAY;
SECONDS_PER_YEAR = 12 * SECONDS_PER_MONTH;

D_DAYS = D_DAYS or "%d Days";
D_HOURS = D_HOURS or "%d Hours";
D_MINUTES = D_MINUTES or "%d Minutes";
D_SECONDS = D_SECONDS or "%d Seconds";
DAYS_ABBR = DAYS_ABBR or "%d Day(s)";
HOURS_ABBR = HOURS_ABBR or "%d Hr";
MINUTES_ABBR = MINUTES_ABBR or "%d Min";
SECONDS_ABBR = SECONDS_ABBR or "%d Sec";
DAY_ONELETTER_ABBR = DAY_ONELETTER_ABBR or "%d d";
HOUR_ONELETTER_ABBR = HOUR_ONELETTER_ABBR or "%d h";
MINUTE_ONELETTER_ABBR = MINUTE_ONELETTER_ABBR or "%d m";
SECOND_ONELETTER_ABBR = SECOND_ONELETTER_ABBR or "%d s";
TIME_UNIT_DELIMITER = TIME_UNIT_DELIMITER or " ";
HOURS_MINUTES_SECONDS = HOURS_MINUTES_SECONDS or "%d:%02d:%02d";
MINUTES_SECONDS = MINUTES_SECONDS or "%d:%02d";

function SecondsToMinutes(seconds)
    return seconds / SECONDS_PER_MIN;
end

function MinutesToSeconds(minutes)
    return minutes * SECONDS_PER_MIN;
end

function ConvertSecondsToUnits(timestamp)
    timestamp = math.max(timestamp, 0);
    local days = math.floor(timestamp / SECONDS_PER_DAY);
    timestamp = timestamp - (days * SECONDS_PER_DAY);
    local hours = math.floor(timestamp / SECONDS_PER_HOUR);
    timestamp = timestamp - (hours * SECONDS_PER_HOUR);
    local minutes = math.floor(timestamp / SECONDS_PER_MIN);
    timestamp = timestamp - (minutes * SECONDS_PER_MIN);
    local seconds = math.floor(timestamp);
    local milliseconds = timestamp - seconds;
    return {
        days = days,
        hours = hours,
        minutes = minutes,
        seconds = seconds,
        milliseconds = milliseconds,
    };
end

function SecondsToClock(seconds, displayZeroHours)
    local units = ConvertSecondsToUnits(seconds);
    if units.hours > 0 or displayZeroHours then
        return string.format(HOURS_MINUTES_SECONDS, units.hours, units.minutes, units.seconds);
    else
        return string.format(MINUTES_SECONDS, units.minutes, units.seconds);
    end
end

-- 1.12's native unit strings are bare labels ("Hr", "Min") with no `%d`,
-- unlike modern's "%d Hr"; insert the count correctly for either shape.
local function FormatUnit(fmt, count)
    if string.find(fmt, "%%d") then
        return string.format(fmt, count);
    end
    return count .. " " .. fmt;
end

function SecondsToTime(seconds, noSeconds, notAbbreviated, maxCount, roundUp)
    local time = "";
    local count = 0;
    local tempTime;
    seconds = roundUp and math.ceil(seconds) or math.floor(seconds);
    maxCount = maxCount or 2;

    -- With a single term, use a higher 1.5x threshold; with 2+, 1.0x.
    local threshold = (maxCount > 1) and 1.0 or 1.5;

    if seconds >= SECONDS_PER_DAY * threshold then
        count = count + 1;
        tempTime = (count == maxCount and roundUp) and math.ceil(seconds / SECONDS_PER_DAY)
            or math.floor(seconds / SECONDS_PER_DAY);
        time = FormatUnit(notAbbreviated and D_DAYS or DAYS_ABBR, tempTime);
        seconds = math.mod(seconds, SECONDS_PER_DAY);
    end
    if count < maxCount and seconds >= SECONDS_PER_HOUR * threshold then
        count = count + 1;
        if time ~= "" then time = time .. TIME_UNIT_DELIMITER; end
        tempTime = (count == maxCount and roundUp) and math.ceil(seconds / SECONDS_PER_HOUR)
            or math.floor(seconds / SECONDS_PER_HOUR);
        time = time .. FormatUnit(notAbbreviated and D_HOURS or HOURS_ABBR, tempTime);
        seconds = math.mod(seconds, SECONDS_PER_HOUR);
    end
    if count < maxCount and seconds >= SECONDS_PER_MIN * threshold then
        count = count + 1;
        if time ~= "" then time = time .. TIME_UNIT_DELIMITER; end
        tempTime = (count == maxCount and roundUp) and math.ceil(seconds / SECONDS_PER_MIN)
            or math.floor(seconds / SECONDS_PER_MIN);
        time = time .. FormatUnit(notAbbreviated and D_MINUTES or MINUTES_ABBR, tempTime);
        seconds = math.mod(seconds, SECONDS_PER_MIN);
    end
    if count < maxCount and seconds > 0 and not noSeconds then
        if time ~= "" then time = time .. TIME_UNIT_DELIMITER; end
        time = time .. FormatUnit(notAbbreviated and D_SECONDS or SECONDS_ABBR, seconds);
    end
    return time;
end

-- Returns (formatString, value) for the single largest applicable unit.
function SecondsToTimeAbbrev(seconds, thresholdOverride)
    local threshold = thresholdOverride or 1.5;
    if seconds >= SECONDS_PER_DAY * threshold then
        return DAY_ONELETTER_ABBR, math.ceil(seconds / SECONDS_PER_DAY);
    end
    if seconds >= SECONDS_PER_HOUR * threshold then
        return HOUR_ONELETTER_ABBR, math.ceil(seconds / SECONDS_PER_HOUR);
    end
    if seconds >= SECONDS_PER_MIN * threshold then
        return MINUTE_ONELETTER_ABBR, math.ceil(seconds / SECONDS_PER_MIN);
    end
    return SECOND_ONELETTER_ABBR, seconds;
end
