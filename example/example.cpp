#include <iostream>
#include <chrono>
#include <functional>
#include <ctime>
#include <string>

using namespace std;
#include "grok/groklog.h"
#include "grok/grok.h"
#include "luaModule.h"


int main(int argc, char** argv)
{
	using namespace grok;
	auto luaMgr = LuaModelManager::get_instance();
	auto nodecenter = grok::NodeCenter::Create(luaMgr->ios, 9595);
	luaMgr->msgcenter = MsgCenter::Create();
	luaMgr->msgcenter->imMsgNextID = make_function_wrapper(nodecenter, &grok::NodeCenter::msg_msgnextidx);
	luaMgr->msgcenter->imWriteMsgPack = make_function_wrapper(nodecenter, &grok::NodeCenter::write_msgpack);
	luaMgr->msgcenter->imNodeName = make_function_wrapper(nodecenter, &grok::NodeCenter::get_name);
	
	luaMgr->init(argc, argv);
	EventPools::Init();

	boost::asio::basic_waitable_timer<std::chrono::system_clock> w(luaMgr->ios);
	w.expires_from_now(std::chrono::seconds(1));
	w.async_wait([luaMgr](boost::system::error_code ec){
		// luaMgr->stop();
	});
	luaMgr->start();

	EventPools::Uinit();
	luaMgr->uninit();
	return 0;
}
