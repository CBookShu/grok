local dbcore = require "dbcore"
local ldbcore = {}

--[[
    举例:
    ldbcore.mysql_get(function(db)
        local t = ldbcore.mysql_wrapper_query(db)
        t:query("SELECT NOW() AS t", function(r)
            local res = ldbcore.mysql_wrapper_res(r)
            assert(res:next())
            assert(res:is_null("t"))
            print(res:get_string("t"))
        end)
        
        assert(t:query("INSERT INTO testtable (n1,s2) VALUES(1, 'hello1')") == 1)

        assert(t:query("INSERT INTO testtable (n1,s2) VALUES(?, ?)", nil, 2, 'hello2') == 1)

        t:query("SELECT * FROM testtable WHERE n1 = ?", function(r)
            local res = ldbcore.mysql_wrapper_res(r)
            assert(res:next())
            assert(res:get_num("n1") == 1)
            assert(res:get_string("s2") == "hello1")
        end, 1)
    end)
]]
function ldbcore.mysql_get(cb)
    return dbcore.dbcore_mysql_get(function (db)
        if cb then
            return cb(db)
        end
    end)
end

-- 辅助使用，便于Lua代码提示
function ldbcore.mysql_wrapper_query(db)
    local t = {}
    function t:query(sql,callback,...)
        return db:query({
            query = sql,
            callback = callback,
            argv = {...}
        })
    end
    return t
end

-- 辅助使用，便于Lua代码提示
function ldbcore.mysql_wrapper_res(r)
    local res = {}
    function res:next()
        return r:next()
    end
    function res:is_null(field)
        return r:is_null(field)
    end
    function res:get_string(field)
        return r:get_string(field)
    end
    function res:get_num(field)
        local s = r:get_string(field)
        return tonumber(s)
    end
    function res:get_blob(field)
        return r:get_blob(field)
    end
    return res
end

return ldbcore