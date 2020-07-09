#include "testloop.h"
#include <string>

void TestLoop::StartLoop(const std::string& quitCmd)
{
	std::string cmd;
	while (std::getline(std::cin, cmd)) {
		if (cmd == quitCmd) {
			break;
		}
		bool ok = true;
		evCmd(ok, cmd);
	}
}
