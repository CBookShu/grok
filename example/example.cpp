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

	luaMgr->init(argc, argv);
	auto& ios = luaMgr->thread_pool->ios();
	auto nodecenter = grok::NodeCenter::Create(ios);
	nodecenter->evMsgCome += delegate(luaMgr, &LuaModelManager::on_msg);
	luaMgr->imMsgNextID = make_function_wrapper(nodecenter, &NodeCenter::msg_msgnextidx);
	luaMgr->imNodeName = make_function_wrapper(nodecenter, &NodeCenter::get_name);
	luaMgr->imWriteMsgPack = make_function_wrapper(nodecenter, &NodeCenter::write_msgpack);

	boost::asio::basic_waitable_timer<std::chrono::system_clock> w(ios);
	w.expires_from_now(std::chrono::seconds(1));
	w.async_wait([luaMgr](boost::system::error_code ec){
		// luaMgr->stop();
	});
	// start过后所有的Model都初始化完成
	luaMgr->start();
	// model初始化完成后才开始正式监听网络消息，这样投递的消息处理model一定是已经好了
	nodecenter->get_server()->start(9595, 8);
	// 主线程消息循环
	ios.run();
	luaMgr->uninit();
	return 0;
}
