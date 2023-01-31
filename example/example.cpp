#include <iostream>
#include <chrono>
#include <functional>
#include <ctime>
#include <string>

using namespace std;
#include "grok/groklog.h"
#include "grok/grok.h"

int main(int argc, char** argv)
{
	using namespace grok;
	EventPools::Init();

	EventPools::Uinit();
	return 0;
}
