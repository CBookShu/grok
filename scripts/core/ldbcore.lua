local dbcore = require "dbcore"
local ldbcore = {}

-- mysql 接口
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


-- redis 接口
--[[
    ldbcore.redis_get(function(db)
        local rds = ldbcore.redis_wrapper_query(db)
        local cmds = {}
        table.insert(cmds, ldbcore.redis_make_cmd("SET ? ?", "test_key", "test_value"))
        table.insert(cmds, ldbcore.redis_make_cmd("GET ?", "test_key"))
        local res = rds:query(cmds)
        assert(res)
        assert(res[1][1] and res[2][1] and res[2][2] == "test_value")
    end)

]]
function ldbcore.redis_get(cb)
    return dbcore.dbcore_redis_get(function (db)
        if cb then
            return cb(db)
        end
    end)
end

function ldbcore.redis_make_cmd(fmt,...)
    local vec_argv = {}
    local argv = {...}
    local argc = 0
    for w in string.gmatch(fmt, "%S+") do
        if w == "?" then
            argc = argc + 1
        else
            table.insert(vec_argv, w)
        end
    end
    assert(argc == #argv)
    for i,v in ipairs(argv) do
        table.insert(vec_argv, tostring(v))
    end
    return vec_argv
end

function ldbcore.redis_wrapper_query(rds)
    local t = {}
    function t:query(cmds)
        return rds:query(cmds)
    end
    return t
end
return ldbcore