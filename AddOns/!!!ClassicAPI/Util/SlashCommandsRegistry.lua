
function RegisterNewSlashCommand(callback, command, commandAlias)
	local name = string.upper(command);
    _G["SLASH_"..name.."1"] = "/"..command;
    _G["SLASH_"..name.."2"] = "/"..commandAlias;
    SlashCmdList[name] = callback;
end

SlashCmdList["FOCUS"] = function(msg)
    if msg == "" then
        FocusUnit();
    else
        FocusUnit(msg);
    end
end

SlashCmdList["CLEARFOCUS"] = function(msg)
    ClearFocus()
end

SlashCmdList["EQUIP_SET"] = function(msg)
    if msg ~= "" then
        C_EquipmentSet.UseEquipmentSet(C_EquipmentSet.GetEquipmentSetID(msg))
    end
end
