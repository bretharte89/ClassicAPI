
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

string.gmatch = string.gfind