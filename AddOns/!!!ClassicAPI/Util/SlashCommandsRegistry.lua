
function RegisterNewSlashCommand(callback, command, commandAlias)
	local name = string.upper(command);
    _G["SLASH_"..name.."1"] = "/"..command;
    _G["SLASH_"..name.."2"] = "/"..commandAlias;
    SlashCmdList[name] = callback;
end

-- /focus [unit] — mirrors 4.3.4's SecureCmdList["FOCUS"]. The engine's
-- FocusUnit polyfill (src/unit/Focus.cpp) resolves any unit token —
-- "target", "party1", a GUID string, etc. — and defaults to "target"
-- when called with no argument. We can't honor macro conditionals
-- (no SecureCmdOptionParse on 1.12) but plain unit-token + no-arg
-- usage is identical to modern.
RegisterNewSlashCommand(function(msg)
    if msg == "" then
        FocusUnit();
    else
        FocusUnit(msg);
    end
end, "focus", "focus");

RegisterNewSlashCommand(ClearFocus, "clearfocus", "clearfocus");