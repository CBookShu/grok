local lcore = require "scripts.core.lcore"
local protobuf = require "scripts.utils.protobuf"
local lmsg = {}
lmsg.msg_type_t = {
    eMsg_none = 0,
    eMsg_request = 1,
    eMsg_response = 2,
    eMsg_notify = 3,
}
lmsg.node_center_name = "NodeCenter"

local function init_msgpack(source,dest,msgname,msgtype,sessionid,pbdata)
    return {
        source = source,
        dest = dest,
        msgname = msgname,
        msgtype = msgtype,
        sessionid = sessionid,
        pbdata = pbdata
    }
end

function lmsg.new(msgpack)
    local t = {}
    -- msgpakc begin
    function t:get_pure_msgpack()
        return msgpack
    end
    function t:get_source()
        return msgpack:get_source()
    end
    function t:get_dest()
        return msgpack:get_dest()
    end
    function t:get_msgname()
        return msgpack:get_msgname()
    end
    function t:get_sessionid()
        return msgpack:get_sessionid()
    end
    function t:get_pbdata()
        return msgpack:get_pbdata()
    end
    function t:get_msgtype()
        return msgpack:get_msgtype()
    end
    function t:send_response(pbname,msg)
        assert(self:get_msgtype() == lmsg.msg_type_t.eMsg_request)
        local pbdata = protobuf.encode(pbname,msg)
        local pack = init_msgpack(self:get_source(), self:get_dest(), pbname, lmsg.msg_type_t.eMsg_response, self:get_sessionid(), pbdata)
        return lcore.sendmsgpack(pack)
    end
    -- msgpakc begin

    -- global begin
    function t:self_node_name()
        return lcore.nodename()
    end
    function t:msg_nextid()
        return lcore.msgnextid()
    end
    function t:send_notify(dest,pbname,msg)
        local pbdata = protobuf.encode(pbname, msg)
        local pack = init_msgpack(
            self:self_node_name(),dest,pbname,lmsg.msg_type_t.eMsg_notify,0,pbdata)
        return lcore.sendmsgpack(pack)
    end
    function t:send_request(dest,pbname,msg)
        local pbdata = protobuf.encode(pbname, msg)
        local pack = init_msgpack(
            self:self_node_name(),dest,pbname,lmsg.msg_type_t.eMsg_request,self:msg_nextid(),pbdata)
        return lcore.sendmsgpack(pack)
    end
    -- global begin
    return t
end

return lmsg