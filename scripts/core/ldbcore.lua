local dbcore = require "dbcore"
local ldbcore = {}

--[[
#define REDIS_REPLY_STRING 1
#define REDIS_REPLY_ARRAY 2
#define REDIS_REPLY_INTEGER 3
#define REDIS_REPLY_NIL 4
#define REDIS_REPLY_STATUS 5
#define REDIS_REPLY_ERROR 6
]]
ldbcore.rdis_status = {
    type_rpl_ctxerr = 0,
    type_rpl_str = 1,
    type_rpl_array = 2,
    type_rpl_int = 3,
    type_rpl_nil = 4,
    type_rpl_status = 5,
    type_rpl_err = 6,
}

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
        assert(res[1][1] == ldbcore.rdis_status.type_rpl_str and res[2][2] == "test_value")
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

function ldbcore.redis_wrapper_query(db)
    local t = {}
    local cmds = {}
    function t:appendcmd(fmt,...)
        local cmd = ldbcore.redis_make_cmd(fmt, ...)
        table.insert(cmds, cmd)
    end
    function t:query(fmt,...)
        if fmt then 
            local cmd = ldbcore.redis_make_cmd(fmt, ...)
            table.insert(cmds, cmd)
    end
        local len = #cmds
        local res = db:query(cmds)
        cmds = {}
        return res
    end
    return t
end
return ldbcore