#pragma once
#include <functional>
#include "../grok/grok.h"
#include "mysqlConnector.h"

struct MyEntry : public grok::WorkEntryWithCache/*, public grok::MysqlSession*/
{
public:
	virtual void enterThread() override;
	virtual void leaveThread() override;

};

class TestWorkPool : public grok::WorkPool
{
public:
	std::function<void(const char*, const char*, int&)> imGetNum;
	std::function<void(const char*, const char*, std::string&)> imGetStr;

	void OnStart();
	void OnStop();

protected:
	virtual std::shared_ptr<grok::ThreadEntryBase> createThreadEntry() override;
};

