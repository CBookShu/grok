#include "testWorkPool.h"

void TestWorkPool::OnStart()
{
	auto num = std::thread::hardware_concurrency() * 2 + 1;
	start(num);
}

void TestWorkPool::OnStop()
{
	stop();
}

std::shared_ptr<grok::ThreadEntryBase> TestWorkPool::createThreadEntry()
{
	return std::make_shared<MyEntry>();
}
