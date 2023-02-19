local self_model_name = ...
print("this is model:", self_model_name)

require "scripts.utils.functions"
local lcore = require "scripts.core.lcore"
local lmodel = require "scripts.core.lmodel"

-- 文件监听
lmodel.file_listen(self_model_name, "test.txt", function ()
    lcore.logtrace("test.txt modify")
end,1000)

-- 定时器
local timer1 = lmodel.timer_start(self_model_name, function ()
    lcore.logtrace("this is timer1")
end, 5000)

local timer2 = nil
timer2 = lmodel.timer_start(self_model_name, function ()
    lcore.logtrace("stop timer and file listen")
    lmodel.timer_stop(self_model_name, timer1)
    lmodel.timer_stop(self_model_name, timer2)
    lmodel.file_stoplisten(self_model_name, "test.txt")
end, 10000)

-- cache val接口
local test_cache = {
    username = "test",
    age = 1,
    status = "sleep"
}
lcore.set_cache(self_model_name, test_cache)
local cache = lcore.get_cache(self_model_name)
assert(cache)

-- protobuf接口
local protobuf = require "scripts.utils.protobuf"
local curdir = lcore.curdir()
lcore.logtrace("cur dir:%s", curdir)
protobuf.register_file("scripts/pb/test_model.pb")
local sun_info = {
    -- string name = 1;
    -- int32 age = 2;
    -- string addr = 3;
    name = "SunWuKong",
    age = 500,
    addr = "HuaGuoShan"
}
local pbdata = protobuf.encode("test_model.Test_Model_Msg1", sun_info)
assert(pbdata)

local sun_info_1 = protobuf.decode("test_model.Test_Model_Msg1", pbdata)
assert(sun_info_1)

-- cache key val接口
lcore.set_cache(self_model_name, "sun_info", sun_info)
local sun_info_cache = lcore.get_cache(self_model_name, "sun_info")
assert(sun_info_cache)


-- dbcore 测试用例
local ldbcore = require "scripts.core.ldbcore"
local r = ldbcore.mysql_get(function (db)
    local t = ldbcore.mysql_wrapper_query(db)
    -- t:query("SELECT NOW() AS T", function (r)
    --     local res = ldbcore.mysql_wrapper_res(r)
    --     assert(res:next())
    --     local time = res:get_string("T")
    --     print("now is:", time)
    -- end)

    assert(t:query("DROP table IF EXISTS testtable") ~= -1)
    assert(t:query([[CREATE TABLE IF NOT EXISTS `testtable` (
    `n1` int(11) DEFAULT NULL,
    `s2` varchar(255) DEFAULT NULL,
    `b3` blob
    ) ENGINE=InnoDB DEFAULT CHARSET=latin1;]]) ~= 1)
    assert(t:query("INSERT INTO testtable (n1,s2) VALUES(1, 'hello1')") == 1)
    assert(t:query("INSERT INTO testtable (n1,s2) VALUES(?, ?)", nil, 2, "hello2") == 1)

    assert(t:query("INSERT INTO testtable (n1,s2) VALUES(?, ?)", nil, 3, pbdata) == 1)

    t:query("SELECT * FROM testtable", function (r)
        local res = ldbcore.mysql_wrapper_res(r)
        for i = 1, 2 do
            assert(res:next())
            local n1 = res:get_num("n1")
            local s2 = res:get_string("s2")
            assert(n1 == i)
            assert(s2 == string.format("hello%d", i))
        end
    end)

    return true
end)
assert(r)

local function dbg(t)
    local dbg_s = debug_table(t)
    lcore.logtrace(dbg_s)
end

local rds_rpl_t = ldbcore.redis_status
r = ldbcore.redis_get(function (db)
    local rds = ldbcore.redis_wrapper_query(db)
    local res = rds:query("SELECT ?", 1)
    assert(rds:check_res(res))
    assert(res[1][1] == rds_rpl_t.type_rpl_status)
    assert(res[1][2] == "OK")

    rds:appendcmd("SET ? ?", "test_key", "test_value")
    rds:appendcmd("GET ?", "test_key")
    res = rds:query()
    assert(res)
    assert(res[1][1] == 5 and res[1][2] == "OK")
    assert(res[2][1] == 1 and res[2][2] == "test_value")

    local fmt = string.format

    -- cmds 单个结果
    res = rds:query("DEL hash_key")
    assert(rds:check_res(res))
    local count = 100
    for i = 1, count do
        rds:appendcmd("HSET hash_key ? ?", fmt("f%d", i), fmt("v%d", i))
    end
    res = rds:query()
    assert(rds:check_res(res))
    for i = 1, count do
        assert(rds:check_res(res))
    end
    for i = 1, count do
        rds:appendcmd("HGET hash_key ?", fmt("f%d",i))
    end
    res = rds:query()
    assert(rds:check_res(res))
    for i = 1, count do
        assert(res[i][1] == rds_rpl_t.type_rpl_str and res[i][2] == fmt("v%d",i))
    end

    -- 多个array结果
    local ar = {}
    table.insert(ar, "HMGET hash_key")
    for i = 1, count do
        table.insert(ar, fmt("f%d",i))
    end
    local cmd = table.concat(ar, " ")
    res = rds:query(cmd)
    assert(rds:check_res(res))
    assert(#res[1][2] == count)
    for i = 1, count do
        assert(res[1][2][i] == fmt("v%d",i))
    end
    return true
end)
assert(r)

-- union lock 测试用例
r = lcore.unionlock({"hello", "world"}, function ()
    print("lock hello and world")
    return true, "unionlock api pass"
end)
assert(r)

-- msgcore 测试
local msgnextid = lcore.msgnextid()
assert(type(msgnextid) == "number")
lcore.logtrace("msgnextid:%d",msgnextid)
local nodename = lcore.nodename()
assert(type(nodename) == "string")
lcore.logtrace("nodename:%s", nodename)

local lmsg = require "scripts.core.lmsg"
local msg = lmsg.new()
-- 监听了 "test_model.Test_Model_Msg1" 的msg可以收到该数据消息
msg:send_notify(msg:self_node_name(),"test_model.Test_Model_Msg1", sun_info)
-- TODO request and response

-- redis cache
local lrdscache = require "scripts.core.lrdscache"
local task_name = "test"
lcore.logtrace("redis cache before")
local test_get_table = {1000, 1001}
for i,v in ipairs(test_get_table) do
    local userid = v

    local lockkey = task_name..tostring(userid)
    lcore.unionlock({lockkey}, function ()
        -- 获取玩家数据
        local zhu_info = {
            -- string name = 1;
            -- int32 age = 2;
            -- string addr = 3;
            name = "ZhuBaJie",
            age = userid,
            addr = "GaoLaoZhuang"
        }
    
        -- 没有数据，就初始化一份给过去
        local err,data = lrdscache.set_cache("test", userid, zhu_info)
        assert(err)
    
        err,data = lrdscache.get_cache("test", userid)
        assert(err)
        assert(data)
        -- lcore.logtrace("set after")
        -- dbg(data)
    end)
    
end

for i,v in ipairs(test_get_table) do
    local userid = v

    local lockkey = task_name..tostring(userid)
    lcore.unionlock({lockkey}, function ()
        local ok,data= lrdscache.get_cache("test", userid)
        assert(ok)
        lcore.logtrace("userid:%d", userid)
        lcore.logtrace("data:%s", debug_table(data))
    end)
    
end
lcore.logtrace("redis cache after")