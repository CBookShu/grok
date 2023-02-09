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
local s = debug_table(cache)
lcore.logtrace(s)

-- protobuf接口
local protobuf = require "scripts.utils.protobuf"
local curdir = lcore.curdir()
lcore.logtrace("cur dir:%s", curdir)
protobuf.register_file("scripts/models/test_model.pb")
local sun_info = {
    -- string name = 1;
    -- int32 age = 2;
    -- string addr = 3;
    name = "SunWuKong",
    age = 500,
    addr = "HuaGuoShan"
}
local pbdata = protobuf.encode("test_model.Test_Model_Msg1", sun_info)
local pbdata_str = convert_pbdata_to_str(pbdata)
lcore.logtrace("Sun_info pbdata_str:%s", pbdata_str)

local sun_info_1 = protobuf.decode("test_model.Test_Model_Msg1", pbdata)
local sun_info_1_des = debug_table(sun_info_1)
lcore.logtrace("sun_info_1_des:%s", sun_info_1_des)

-- cache key val接口
lcore.set_cache(self_model_name, "sun_info", sun_info)
local sun_info_cache = lcore.get_cache(self_model_name, "sun_info")
local sun_info_cache_des = debug_table(sun_info_cache or {})
lcore.logtrace("sun_info_cache_des:%s", sun_info_cache_des)

