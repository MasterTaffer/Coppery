local vars = {}


local varsmeta = {}

varsmeta.__index = function (t, f)
    f = tostring(f)
    local gv = GameVar.Get(f)
    if gv:isNull() then
        return nil
    end
    
    local t = GameVar.GetType(gv)
    if t == 0 then
        return GameVar.GetInteger(gv)
    elseif t == 1 then
        return GameVar.GetNumber(gv)
    else
        return nil
    end
end


varsmeta.__newindex = function (t, f, v)
    f = tostring(f)
    local gv = GameVar.Get(f)
    if gv:isNull() then
        print("GameVar ".. f .." doesn't exist")
        return nil
    end
    
    local t = GameVar.GetType(gv)
    if t == 0 then
        GameVar.SetInteger(gv, math.floor(v))
    elseif t == 1 then
        GameVar.SetNumber(gv, tonumber(v))
    else
        return nil
    end
end

setmetatable(vars, varsmeta)
return vars

