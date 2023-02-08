local self_model_name = ...
print("this is model:", self_model_name)

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