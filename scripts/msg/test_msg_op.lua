local lmsg = require "scripts.core.lmsg"
local lcore = require "scripts.core.lcore"

lmsg.regsiter_msg_norsp("test_model.Test_Model_Msg1", function (msgpack,info)
    local msg = lmsg.new(msgpack)
    local dbgstr = debug_table(info)
    lcore.logtrace("this is msg:%s", msg:get_msgname())
    lcore.logtrace("msginfo:")
    lcore.logtrace("dbgstr:%s", dbgstr)
end)
