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




	EventPools::Uinit();
	luaMgr->uninit();
	return 0;
}
