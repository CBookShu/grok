#pragma once
#include "../grok/include/grok.h"

struct MyEntry : public grok::ThreadEntryBase
{
public:
	virtual void enterThread() {

	}
	virtual void leaveThread() {

	}
};

class TestWorkPool : public grok::WorkPool
{
public:
	void OnStart();
	void OnStop();

protected:
	virtual std::shared_ptr<grok::ThreadEntryBase> createThreadEntry() override;
};

