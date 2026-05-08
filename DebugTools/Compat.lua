-- Lua 5.0 / WoW 1.12 compatibility shims for the DebugTools backport.

if not _G then
    _G = getfenv(0)
end

-- Lua 5.0 has no string.match. Approximate it with string.find.
if not string.match then
    function string.match(s, pattern, init)
        if s == nil then return nil end
        local r1, r2, c1, c2, c3, c4, c5, c6, c7, c8, c9 =
            string.find(s, pattern, init)
        if r1 == nil then return nil end
        if c1 ~= nil then
            return c1, c2, c3, c4, c5, c6, c7, c8, c9
        end
        return string.sub(s, r1, r2)
    end
end

-- Lua 5.0 spelled gmatch as gfind.
if not string.gmatch then
    string.gmatch = string.gfind
end

-- These exist in 3.x+ but not in 1.12; stub them out so calls are harmless.
if not forceinsecure then
    function forceinsecure() end
end

if not debuglocals then
    function debuglocals() return "" end
end

-- Convenience: count entries in a sequence (Lua 5.0 has no # operator).
function DebugTools_TableLen(t)
    if t == nil then return 0 end
    return table.getn(t)
end
