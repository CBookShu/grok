local lcore = require "scripts.core.lcore"
local ldbcore = require "scripts.core.ldbcore"
local ltimeutils = require "scripts.utils.ltime"
local protobuf = require "scripts.utils.protobuf"
local Config = require "scripts.lrdscacheconfig"
local lrdscahce = {}
local tag = "grok_lrdscache"
-- 默认redis dbidx
local redisdbindex = 5
local lastchecktime = nil

--[[
    redis 读写缓存mysql数据的模块
    mysql的操作要慢于redis,所以如果几乎所有的操作都对redis进行，那么可以效率最大化。
    redis作为内存服务器，保存所有数据会使得内存压力过大，有些可能永远也不访问的数据也一直放在redis中也是浪费
    该模块的目的，就是把热数据的读写都在redis中操作；冷数据会慢慢同步到mysql中。在此基础上，简化了一些Mysql的操作。
    该模块的目标数据，应该是那些以程序为主，对mysql数据的实时性要求不高，不怎么通过mysql去分析数据的情况。mysql
    纯粹作为redis硬盘的作用了

    流程:
    1.  读取数据
        get data from redis
            ok -> data op
                no -> get data from mysql

    2.  写入数据
        write data to redis
        write data key to redis_dirtytable

    3.  同步redis数据到mysql中【该操作可以单独拉一台服务器执行】
        renamenx redis_dirtytable to redis_dirtytable_sync
            set redis data expired
            get data from redis_dirtytable_sync
            sync data to mysql
            
    
    其中的写入数据和redis数据同步，是可能并行的，解决方案如下：
    1.  当我们进行同步的时候，会把当前dirtytable 重命名，新的数据再写入的时候，会再次创建新的dirtytable
        并且记录dirty，那么我们同步过后，后面还是会对该数据再同步一次，也就是可能会出现把同一份数据多次同步的情况
    2.  在同步数据的时候，一定要【先设置数据过期】，再【获取数据】。这样依赖，当我们获取数据后，再有新的数据写入
        会生成新的dirytable数据，并且把过期属性清理，那么哪怕当前的同步数据是老的，下一轮还是会再同步过去的，只是
        会出现redis和mysql的数据不一致，但是数据的有效性是没有丢失的
    3.  同步redis数据到mysql一定要保证有且仅有一个在运行
]]

function lrdscahce.create_mysql(name)
    local r = ldbcore.mysql_get(function (db)
        local mysql = ldbcore.mysql_wrapper_query(db)
        local sql = string.format([[CREATE TABLE IF NOT EXISTS %s (
            `mainkey` int(11) NOT NULL,
            `data` blob,
            PRIMARY KEY (`mainkey`)
        )]], name)
        return mysql:query(sql) ~= -1
    end)
    return r
end

function lrdscahce.get_rds_name(name, key)
    return string.format("%s_rds_%s:%s", tag, name, key)
end

function lrdscahce.get_mysql_name(name)
    return string.format("%s_mysql_%s", tag, name)
end

function lrdscahce.create_all_mysql()
    for i, name in pairs(Config.mysqlregister) do
        if not lrdscahce.create_mysql(lrdscahce.get_mysql_name(name)) then
            lcore.logerror("MySQL %s create Error", name)
            return false
        end
    end
    return true
end

function lrdscahce.get_all_keys()
    local tbl = {}
    for i, name in ipairs(Config.mysqlregister) do
        table.insert(tbl, name)
    end
    return tbl
end

function lrdscahce.get_dirtytable_name(name)
    return string.format("%s_dirytable_%s", tag, name)
end

function lrdscahce.get_dirtytabe_sync_name(name)
    return string.format("%s_dirytable_%s_sync", tag, name)
end

function lrdscahce.get_proto_name(name)
    return Config.protoregister[name]
end

function lrdscahce.rename_dirytable()
    return ldbcore.redis_get(function (db)
        local rds = ldbcore.redis_wrapper_query(db)
        local res = rds:query("SELECT ?", redisdbindex)
        if not rds:check_oneres(res) then
            return false
        end

        for i,name in ipairs(Config.mysqlregister) do
            local dirtyname = lrdscahce.get_dirtytable_name(name)
            local dirtysyncname = lrdscahce.get_dirtytabe_sync_name(name)
            rds:appendcmd("RENAMENX ? ?", dirtyname, dirtysyncname)
        end

        res = rds:query()
        if not res then
            return false
        end
        for i,v in ipairs(res) do
            if not rds:check_oneres(v) then
                return false
            end
        end
        return true
    end)
end

function lrdscahce.sync_rds_to_mysql(name)
    local mysql_name = lrdscahce.get_mysql_name(name)
    local rds_dirtysync_table = lrdscahce.get_dirtytabe_sync_name(name)
    local sqltext = ldbcore.mysql_make_insert_or_update(mysql_name,{{mainkey="?"},{data="?"}},{{data="?"}})
    local percount = Config.redisperscancount
    local expiredtime = Config.rediscachetimeout
    local cur = 0
    local count = 0
    repeat
        local list = {}
        local r = ldbcore.mysql_get(function (db)
            local rds = ldbcore.redis_wrapper_query(db)
            local res = rds:query("SELECT ?", redisdbindex)
            if not rds:check_oneres(res) then
                return false
            end

            res = rds:query("SSCAN ? ? COUNT ?", rds_dirtysync_table, cur, percount)
            if not rds:check_oneres(res) then
                return false
            end
            
            if res[1] ~= ldbcore.redis_status.type_rpl_array then
                -- 结束了
                list = {}
                cur = 0
                return true
            end
            cur = tonumber(res[2][1]) or 0
            list = res[2][2] or {}
            return true
        end)
        if not r then
            return false
        end

        -- 这里一定要redis和mysql共同操作
        local res = nil
        r = ldbcore.redis_get(function (db)
            local rds = ldbcore.redis_wrapper_query(db)
            res = rds:query("SELECT ?", redisdbindex)
            if not rds:check_oneres(res) then
                return false
            end

            for i,key in ipairs(list) do
                -- 这里一定要EXPIRE和GET一起写！
                local redis = lrdscahce.get_rds_name(name, key)
                rds:appendcmd("EXPIRE ? ?", redis, expiredtime)
                rds:appendcmd("GET ?", redis)
            end

            res = rds:query()
            if not rds:check_oneres(res) then
                return false
            end

            for i, v in ipairs(res) do
                if not rds:check_oneres(v) then
                    return false
                end
            end
            return true
        end)

        if not r then
            return false
        end

        -- 同步给mysql
        r = ldbcore.mysql_get(function (db)
            local mysql = ldbcore.mysql_wrapper_query(db)

            -- TODO: 事务
            for i,v in ipairs(list) do
                local key = list[i]
                local data = res[i][2]
                if -1 == mysql:query(sqltext, key, data, data) then
                    -- 这里非常危险！
                    -- 一般来说其他地方的报错，数据都是没有任何影响的，最多redis会无法同步和清理
                    -- 但是这里一旦出错，不及时进行回复，那么会面临数据丢失！
                    --[[
                        1. 如果redis正常，mysql错误。那么可以再把list里key的expiredtime给清理掉
                        2. 如果redis也异常，1就无法执行了
                        那么这里有几个保证：
                        1. expiredtime如果足够大，后面会有机制再重试的。
                        2. 最好增加报警，预防expiredtime 超时都无法自动同步【即，真的是mysql异常了】的情况
                    ]]
                    lcore.logerror("sqlname:%s mainkey:%s sync error!!!!!")
                    return false
                end
            end
            return true
        end)
        if not r then
            return false
        end
    until cur == 0

    -- 删除dirtysynctable
    return ldbcore.redis_get(function (db)
        local rds = ldbcore.redis_wrapper_query(db)
        local res = rds:query("SELECT ?", redisdbindex)
        if not rds:check_oneres(res) then
            return false
        end
        res = rds:query("DEL ?", rds_dirtysync_table)
        return rds:check_oneres(res)
    end)
end

function lrdscahce.sync_all_rds_to_mysql()
    for i,name in ipairs(Config.mysqlregister) do
        if not lrdscahce.sync_rds_to_mysql(name) then
            return false
        end
    end
    return true
end

function lrdscahce.check_difftime()
    local now = ltimeutils.gettimenum()
    if lastchecktime then
        local diff = ltimeutils.time2_time1(now, lastchecktime)
        if diff < Config.checkdirytabletimer then
            return now,false
        end
    else
        if Config.checkonstart then
            return now,true 
        end
        lastchecktime = now
        return now,false
    end
    return now,true
end

function lrdscahce.set_checktime(now)
    lastchecktime = now
end

-- name: lrdscacheconfig 中mysqlregister的name
-- key: 即更细致的该表中某个个体的主键
function lrdscahce.get_cache(name,key)
    local rdsname = lrdscahce.get_rds_name(name, key)
    local r, data = ldbcore.redis_get(function (db)
        local rds = ldbcore.redis_wrapper_query(db)
        local res = rds:query("SELECT ?", redisdbindex)
        if not rds:check_oneres(res) then
            return false, nil
        end

        res = rds:query("GET ?", rdsname)
        if not rds:check_oneres(res) then
            return false, nil 
        end

        -- 只有字符串是我们想要的
        if res[1] ~= ldbcore.redis_status.type_rpl_str then
            return true,nil
        end

        return true,res[2]
    end)

    if not r then
        -- 网络错误
        return r
    end

    if data then
        -- 返回数据
        local proto = lrdscahce.get_proto_name(name)
        if proto then
            return true,protobuf.decode(proto, data)
        end
        return true,data 
    end

    -- redis中无数据，尝试从mysql直接获取
    r,data = ldbcore.mysql_get(function (db)
        local mysql = ldbcore.mysql_wrapper_query(db)
        local mysql_name = lrdscahce.get_mysql_name(name)
        local sqltext = ldbcore.mysql_make_select(mysql_name, {}, {
            mainkey = key
        })
        local pbdata = nil
        local res = mysql:query(sqltext, function (res)
            local lres = ldbcore.mysql_wrapper_res(res)
            if lres:next() then
                 pbdata = lres:get_blob("data")
            end
            return true
        end)
        if not res then
            return false,nil
        end
        return true,pbdata
    end)

    if not r then
        -- 网络错误
        return false
    end

    if not data then
        -- 就是没数据！
        return true,nil
    end

    -- 这时候有数据了，为了下次方便查询，直接给redis缓存一下
    r = ldbcore.redis_get(function (db)
        local rds = ldbcore.redis_wrapper_query(db)
        local res = rds:query("SELECT ?", redisdbindex)
        if not rds:check_oneres(res) then
            return false
        end

        local expiretime = Config.rediscachetimeout
        rds:appendcmd("SET ? ?", rdsname, data)
        rds:appendcmd("EXPIRE ? ?", rdsname, expiretime)
        res = rds:query()
        if not res then
            return false
        end
        -- 这里一共两个操作,有下面2种情况
        -- 1 set操作成功 expire操作失败，并且该数据后面再无操作，则会导致redis永久保存一个临时数据
        -- 2 expire成功,set失败？ 这只有错误的并行才会导致。忽略
        for i,v in ipairs(res) do
            if not rds:check_oneres(v) then
                return false
            end
        end
        return true
    end)

    -- 这里就不再判断r的成功失败了！因为哪怕redis缓存失败，但是数据已然拿到，可以进行业务处理了
    -- 可是现实情况往往是redis异常，后面肯定有什么不妥的。。。

    -- 返回数据
    local proto = lrdscahce.get_proto_name(name)
    if proto then
        return true,protobuf.decode(proto, data)
    end
    return true,data
end

-- name: lrdscacheconfig 中mysqlregister的name
-- key: 即更细致的该表中某个个体的主键
-- data: 具体的数据，即table
function lrdscahce.set_cache(name,key,data)
    return ldbcore.redis_get(function (db)
        local rds = ldbcore.redis_wrapper_query(db)
        local res = rds:query("SELECT ?", redisdbindex)
        if not rds:check_oneres(res) then
            return false
        end

        local rdsname = lrdscahce.get_rds_name(name,key)
        local rdsdirty = lrdscahce.get_dirtytable_name(name)
        local proto = lrdscahce.get_proto_name(name)
        if proto then
            data = protobuf.encode(proto, data)
        end
        rds:appendcmd("SET ? ?", rdsname, data)
        rds:appendcmd("SADD ? ?", rdsdirty, key)
        res = rds:query()
        if not res then
            return false
        end

        -- 这里一共两个操作，有下面两种情况
        -- 1 set操作成功 sadd操作失败，并且该数据后面再无操作，则会导致redis永久保存一个临时数据
        -- 2 sadd成功,set失败，只有代码错误才会发生
        for i,v in ipairs(res) do
            if not rds:check_oneres(v) then
                return false
            end
        end
        return true
    end)
end

return lrdscahce