function clone(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local newObject = {}
        lookup_table[object] = newObject
        for key, value in pairs(object) do
            newObject[_copy(key)] = _copy(value)
        end
        return setmetatable(newObject, getmetatable(object))
    end
    return _copy(object)
end

function debug_table(tbl,n)
    n = n or 0
    local onet = "    "
    local tag = string.rep(onet, n)
    local res = {}
    for i,v in pairs(tbl) do
        local kn = tostring(i)
        local t = type(v)
        local s = ""
        if t == "string" then
            s = tag..string.format("[%s]=[%s]", kn, v)
        elseif t == "number" then
            s = tag..string.format("[%s]=[%d]", kn, v)
        elseif t == "table" then
            local r = debug_table(v, n+1)
            s = tag..string.format("[%s]=[%s]", kn, r)
        else
            s = tag..string.format("[%s]=[type:%s]", kn, t)
        end
        table.insert(res, s)
    end
    local result = table.concat(res, ",\n")
    res = {}
    table.insert(res, "{\n")
    table.insert(res, result)
    table.insert(res, "\n")
    table.insert(res, tag)
    table.insert(res, "}")
    return table.concat(res, "")
end

function string.split(input, delimiter)
    input = tostring(input)
    delimiter = tostring(delimiter)
    if (delimiter=='') then return false end
    local pos,arr = 0, {}
    -- for each divider found
    for st,sp in function() return string.find(input, delimiter, pos, true) end do
        table.insert(arr, string.sub(input, pos, st - 1))
        pos = sp + 1
    end
    table.insert(arr, string.sub(input, pos))
    return arr
end

function convert_pbdata_to_str(pbdata)
    if not pbdata then
        return ""
    end
    local s = {}
    for i = 1, #pbdata do
        local c = string.byte(pbdata, i,i)
        table.insert(s, tostring(c))
    end
    return table.concat(s, ",")
end

function convert_str_to_pbdata(str)
    local  s = string.split(str or {}, ",")
    return string.char(unpack(s))
end
