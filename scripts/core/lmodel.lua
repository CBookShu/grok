local modelcore = require "modelcore"
local lmodelcore = {}
local callback_cur_id = 0
local callback_map = {}
local get_callback_id = nil
local file_listen_map = {}
-- 跟cpp交互的回调函数副口
__LUA_CALLBACK_MAIN__ = nil

function lmodelcore.file_listen(model_name,file_path,cb,millsec)
    local id = get_callback_id(cb)
    local r = modelcore.modelcore_file_listen(model_name,file_path, id, millsec or 30000)
    if not r then
        callback_map[id] = nil
    end
    file_listen_map[file_path] = id
    return r
end

function lmodelcore.file_stoplisten(model_name, file_path)
    local id = file_listen_map[file_path]
    if not id then
        return false
    end
    local r = modelcore.modelcore_file_stoplisten(model_name, file_path)
    if not r then
        -- 出现c++和lua不同步的情况,应该永远不会出现
        -- 增加一个error 看看吧
        error(string.format("file[%s] listen miss", file_path))
    end

    callback_map[id] = nil
    file_listen_map[id] = nil
    return r
end

function lmodelcore.timer_start(model_name, cb, millsec)
    local id = get_callback_id(cb)
    local r = modelcore.modelcore_timer_start(model_name, id, millsec)
    if not r then
        callback_map[id] = nil
        return nil
    end
    return id
end

function lmodelcore.timer_stop(model_name,id)
    if not id then
        return
    end
    local r = modelcore.modelcore_timer_stop(model_name, id)
    if not r then
        -- cpp和lua数据不同步
        error("timer_stop miss")
    end
    callback_map[id] = nil
end


-- 回调函数的全局接口，给cpp用
get_callback_id = function (cb)
    local id = callback_cur_id
    while true do
        id = id + 1
        if not callback_map[id] then
            -- 这里一定要保存起来，否则可能会gc掉
            callback_map[id] = cb
            return id
        end
        if id == callback_cur_id then
            -- 相当于循环溢出后再转了一圈回到最初
            -- 除非有这么多回调注册，这是不可能的
            error("callback map overflow")
        end
    end
end

__LUA_CALLBACK_MAIN__ = function (cid)
    local cb = callback_map[cid]
    if cb then
        cb()
    end
end

return lmodelcore