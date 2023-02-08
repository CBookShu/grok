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
