local lmsg = require "scripts.core.lmsg"
local lcore = require "scripts.core.lcore"

lmsg.regsiter_msg_norsp("test_model.Test_Model_Msg1", function (msgpack,info)
    local msg = lmsg.new(msgpack)
    assert(info)
end)
