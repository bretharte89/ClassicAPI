do
	local ColorMixin = ColorMixin
	for _, classColor in pairs(RAID_CLASS_COLORS) do
		Mixin(classColor, ColorMixin)
		classColor.a = 1
		classColor.colorStr = classColor:GenerateHexColor();
	end
end

-- Built here (not in Constants.lua) because it references the
-- PLAYER_FACTION_COLOR_* objects, which Color.lua materializes from
-- C_UIColor — and ColorUtil.lua loads after Color.lua, so they exist by
-- now. (Constants.lua loads earlier, where they were still nil.)
PLAYER_FACTION_COLORS =
{
	[0] = PLAYER_FACTION_COLOR_HORDE,
	[1] = PLAYER_FACTION_COLOR_ALLIANCE,
};

function ExtractColorValueFromHex(str, index)
	return tonumber(string.sub(str, index, index + 1), 16) / 255;
end

function CreateColorFromHexString(hexColor)
	if string.len(hexColor) == 8 then
		local a, r, g, b = ExtractColorValueFromHex(hexColor, 1), ExtractColorValueFromHex(hexColor, 3), ExtractColorValueFromHex(hexColor, 5), ExtractColorValueFromHex(hexColor, 7);
		return CreateColor(r, g, b, a);
	else
		error("CreateColorFromHexString input must be hexadecimal digits in this format: AARRGGBB.", 2);
	end
end

function CreateColorFromRGBAHexString(hexColor)
	if string.len(hexColor) == 8 then
		local r, g, b, a = ExtractColorValueFromHex(hexColor, 1), ExtractColorValueFromHex(hexColor, 3), ExtractColorValueFromHex(hexColor, 5), ExtractColorValueFromHex(hexColor, 7);
		return CreateColor(r, g, b, a);
	else
		error("CreateColorFromRGBAHexString input must be hexadecimal digits in this format: RRGGBBAA.", 2);
	end
end

function CreateColorFromRGBHexString(hexColor)
	if string.len(hexColor) == 6 then
		local r, g, b = ExtractColorValueFromHex(hexColor, 1), ExtractColorValueFromHex(hexColor, 3), ExtractColorValueFromHex(hexColor, 5);
		return CreateColor(r, g, b, 1);
	else
		error("CreateColorFromRGBHexString input must be hexadecimal digits in this format: RRGGBB.", 2);
	end
end

function CreateColorFromBytes(r, g, b, a)
	return CreateColor(r / 255, g / 255, b / 255, a / 255);
end

function AreColorsEqual(left, right)
	if left and right then
		return left:IsEqualTo(right);
	end
	return left == right;
end

function IsRGBAEqualToColor(r, g, b, a, color)
	return (color.r == r) and (color.g == g) and (color.b == b) and (color.a == a);
end

function GetClassColor(classFilename)
	local color = RAID_CLASS_COLORS[classFilename];
	if color then
		return color.r, color.g, color.b, color.colorStr;
	end
	return 1, 1, 1, "ffffffff";
end

function GetClassColorObj(classFilename)
	return RAID_CLASS_COLORS[classFilename];
end

function GetClassColoredTextForUnit(unit, text)
	local _, classFilename = UnitClass(unit);
	local color = GetClassColorObj(classFilename);
	if (color) then
		return color:WrapTextInColorCode(text);
	end
end

function GetFactionColor(factionGroupTag)
	-- Both PLAYER_FACTION_GROUP (tag -> index) and PLAYER_FACTION_COLORS
	-- (index -> color) live in our Constants.lua. Guarded defensively in
	-- case a client variant leaves one unset.
	local group = PLAYER_FACTION_GROUP[factionGroupTag];
	return PLAYER_FACTION_COLORS and PLAYER_FACTION_COLORS[group];
end

function RGBToColorCode(r, g, b)
	return string.format("|cff%.2x%.2x%.2x", r * 255, g * 255, b * 255);
end

function RGBTableToColorCode(rgbTable)
	return RGBToColorCode(rgbTable.r, rgbTable.g, rgbTable.b);
end
