local Config = {

    -- 是否定时进行同步操作
    syncenable = true,

    -- 启动时是否立刻进行一次同步
    checkonstart = true,

    -- 同步后缓存存活多久
    -- 单位秒
    rediscachetimeout = 24 * 60 * 60,

    -- 每次扫描redis dirty表的数量
    redisperscancount = 500,

    -- 检查dirty表的时间间隔
    -- 单位秒
    checkdirytabletimer = 60 * 60 * 3,

    mysqlregister = {
        [1] = "test"
    },
    protoregister = {
        test = "test_model.Test_Model_Msg1"
    }
}
return Config