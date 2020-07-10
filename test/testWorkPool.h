#pragma once
#include "../grok/include/grok.h"

struct MyEntry : public grok::WorkEntryWithCache
{
public:
};

class TestWorkPool : public grok::WorkPool
{
public:
	void OnStart();
	void OnStop();

protected:
	virtual std::shared_ptr<grok::ThreadEntryBase> createThreadEntry() override;
};

