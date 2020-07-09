#pragma once
#include "../grok/include/grok.h"


class TestLoop
{
public:
	grok::BasicEventNoMutex<bool&, std::string&> evCmd;

	void StartLoop(const std::string& quitCmd = "quit");
};