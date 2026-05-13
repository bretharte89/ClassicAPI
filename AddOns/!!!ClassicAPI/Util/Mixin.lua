-- Backport of 3.3.5 FrameXML/Mixin.lua to vanilla 1.12 / Lua 5.0.
--
-- Implementation differences from the 3.3.5 source:
--   - Lua 5.0 has no select(); inside a vararg function the args are
--     gathered into an implicit `arg` table (`arg.n` for the count).

function Mixin(object, ...)
    for i = 1, arg.n do
        local mixin = arg[i]
        if mixin then
            for k, v in pairs(mixin) do
                object[k] = v
            end
        end
    end
    return object
end

function CreateFromMixins(...)
    return Mixin({}, unpack(arg))
end

function CreateAndInitFromMixin(mixin, ...)
    local object = CreateFromMixins(mixin)
    object:Init(unpack(arg))
    return object
end
