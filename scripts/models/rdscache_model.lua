local self_model_name = ...
print("this is model:", self_model_name)
require "scripts.utils.functions"
local lrdscache = require "scripts.core.lrdscache"
local lmodel = require "scripts.core.lmodel"
local lcore = require "scripts.core.lcore"


assert(lrdscache.create_all_mysql())

if not lrdscache.check_sync_enable() then
    return
end

lcore.loginfo("start lrdscahce check")

local function sync_check()
    local now,r = lrdscache.check_difftime()
    if not r then
        return
    end
    
    lcore.loginfo("check begin")
    if not lrdscache.rename_dirytable() then
        lcore.logerror("rename dirtytable error")
        -- 这里直接返回，下一轮定时器轮询过来，会再次尝试的
        return
    end

    lcore.loginfo("sync redis to mysql begin")
    if not lrdscache.sync_all_rds_to_mysql() then
        lcore.logerror("sync rds to mysql error")
        -- 这里直接返回，下一轮定时器轮询过来，会再次尝试的
        return
    end

    -- 这里设置时间戳，避免下一轮定时器继续同步
    lrdscache.set_checktime(now)
    lcore.loginfo("check end")
end

sync_check()
lmodel.timer_start(self_model_name, function ()
    sync_check()
end, 6000)