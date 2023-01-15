#include "testWorkPool.h"

void MyEntry::enterThread() {
	// MysqlConnector::beginThread();
	// mysql_connect();
}

void MyEntry::leaveThread() {
	// mysql_close();
	// MysqlConnector::endThread();
}


void TestWorkPool::OnStart()
{
	MysqlConnector::init();
	auto num = std::thread::hardware_concurrency() * 2 + 1;
	start(num);
}

void TestWorkPool::OnStop()
{
	stop();
}

std::shared_ptr<grok::ThreadEntryBase> TestWorkPool::createThreadEntry()
{
	auto entry = std::make_shared<MyEntry>();
	// std::string url;
	// imGetStr("mysql", "addr", url);
	// std::string usr;
	// imGetStr("mysql", "usr", usr);
	// std::string pwd;
	// imGetStr("mysql", "pwd", pwd);
	// std::string db;
	// imGetStr("mysql", "db", db);

	// entry->mysql_set_connectInfo(url, usr, pwd, db);

	return entry;
}
