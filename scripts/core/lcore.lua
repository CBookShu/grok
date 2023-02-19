require "scripts.utils.functions"
local core = require ("core")
local lcore = {}

function lcore.curdir()
    return core.core_curdir()
end

local function trim_src(src)
    local pos = string.find(src, "scripts")
    if pos then
        src = string.sub(src, pos + 8)
    end
    return src
end

function lcore.logtrace(...)
    local di = debug.getinfo(2)
    local s = string.format(...)
    s = string.format("[%s][%d]%s",trim_src(di.short_src),di.currentline,s)
    core.core_log(0, s)
end

function lcore.logdebug(...)
    local di = debug.getinfo(2)
    local s = string.format(...)
    s = string.format("[%s][%d]%s",trim_src(di.short_src),di.currentline,s)
    core.core_log(1, s)
end

function lcore.loginfo(...)
    local di = debug.getinfo(2)
    local s = string.format(...)
    s = string.format("[%s][%d]%s",trim_src(di.short_src),di.currentline,s)
    core.core_log(2, s)
end

function lcore.logwarn(...)
    local di = debug.getinfo(2)
    local s = string.format(...)
    s = string.format("[%s][%d]%s",trim_src(di.short_src),di.currentline,s)
    core.core_log(3, s)
end

function lcore.logerror(...)
    local trace = debug.traceback("", 2)
    local s = string.format(...)
    s = string.format( "%s\r\n %s", s, trace )
    core.core_log(4, s)
end

function lcore.logtable(tbl)
    local di = debug.getinfo(2)
    local s = debug_table(tbl)
    s = string.format("[%s][%d]%s",trim_src(di.short_src),di.currentline,s)
    core.core_log(1, s)
end

-- 举例:
-- 将test模块缓存中 filed1字段设置为value1
-- lcore.set_cache("test", "filed1", value1)
-- 将test模块缓存整个替换为value1
-- lcore.set_cache("test", value1)
function lcore.set_cache(model_name, key, val)
    if val then
        return core.core_set_cache(model_name, key, val)
    else
        return core.core_set_cache(model_name, key)
    end
end

-- 举例
-- 获取test模块缓存的filed1字段
-- lcore.get_cache("test", "filed1")
-- 获取test模块整个缓存内容
-- lcore.get_cache("test")
function lcore.get_cache(model_name, key)
    if key then
        return core.core_get_cache(model_name, key)
    else
        return core.core_get_cache(model_name)
    end
end

-- 举例
-- lcore.unionlock({"task_userid1", "task_userid2"}, function() xxx end)
function lcore.unionlock(keys, cb)
    return core.core_unionlock(keys or {}, cb)
end

-- 举例 具体使用查看lmsg.lua
-- table {source=xxx,dest=xxx,msgname=xxx,msgtype=xxx,sessionid=xxx,pbdata=xxx}
-- lcore.sendmsgpack({source="", dest="",msgname="",msgtype=x,sessionid=y,pbdata=""})
function lcore.sendmsgpack(msgpack)
    return core.msgcore_sendmsgpack(msgpack)
end

-- 举例 具体使用查看lmsg.lua
function lcore.msgnextid()
    return core.msgcore_msgnextid()
end

-- 举例 具体使用查看lmsg.lua
function lcore.nodename()
    return core.msgcore_nodename()
end

return lcore