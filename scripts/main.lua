local lmsg = require "scripts.core.lmsg"
local lcore = require "scripts.core.lcore"
require "scripts.utils.functions"

lcore.logtrace("msg main enter")
function main(msgpack)
    local msg = lmsg.new(msgpack)
    local source = msg:get_source()
    lcore.logtrace("msg source:%s", source)
    local dest = msg:get_dest()
    lcore.logtrace("msg dest:%s", dest)
    local msgname = msg:get_msgname()
    lcore.logtrace("msg name:%s", msgname)
    local msgtype = msg:get_msgtype()
    lcore.logtrace("msg type:%s", msgtype)
    local sessionid = msg:get_sessionid()
    lcore.logtrace("msg sessionid:%d", sessionid)
    local pbdata = msg:get_pbdata()
    local pbdata_str = convert_pbdata_to_str(pbdata)
    lcore.logtrace("msg pbdata:%s", pbdata_str)
end