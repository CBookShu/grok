#include <iostream>
#include <chrono>
#include <functional>
#include <ctime>
#include <string>



using namespace std;
#include "grok/groklog.h"
#include "grok/grok.h"
#include "testWorkPool.h"
#include "configTool.h"
#include "simpleLogService.h"

class Controller : public grok::WorkStaff
{
public:
	grok::BasicEventNoMutex<> evStart;
	grok::BasicEventNoMutex<> evStop;
	grok::BasicEventNoMutex<bool&, std::string&> evCmd;

	void StartLoop(const std::string& quitcmd = "quit") {
		evStart();

		std::string cmd;
		while (std::getline(std::cin, cmd)) {
			if (cmd == quitcmd) {
				break;
			}
			bool ok = true;
			evCmd(ok, cmd);
		}

		evStop();
	}

	// 给外部使用的接口
	long long GetVipNum() {
		return std::chrono::steady_clock::now().time_since_epoch().count();
	}

	void OnCmd(bool& ok, std::string& cmd) {
		if (cmd == "Controller") {
			m_timer = this->evp().loopTimer([this]() {
				OnTimerFresh();
			}, std::chrono::seconds(1), strand());

		}
		else if (cmd == "Controller1") {
			m_timer = nullptr;
		}
	}

protected:
	void OnTimerFresh() {
		// 定时器
		std::cout << "timer " << std::endl;
	}

private:
	grok::stdtimerPtr m_timer;
};

class Player
{
public:
	std::function<long long()> imGetVipNum;
	std::function<std::future<int>(const TestWorkPool::Keys&, std::function<int(MyEntry*)>)> imInsertJob;

	void OnStart() {
		cout << "VipNum:" << imGetVipNum() << endl;
		cout << "Player Start" << endl;
	}
	void OnCmd(bool& ok, std::string& cmd) {
		if (cmd == "Player") {
			TestWorkPool::Keys keys = { "Player" };
			imInsertJob(keys, [this](MyEntry* entry) {
				char sqlText[1024] = {0};
				sprintf(sqlText, "SELECT * FROM user");
				entry->mysql_excute(sqlText, [](sql::ResultSet*res){
					while(res->next()) {
						cout << "username:" << res->getString("username") << endl;
						cout << "userid:" << res->getInt("userid") << endl;
					}
					return 1;
				});

				return 1;
			}).get();
		}
	}
};

class Toy
{
public:
	std::function<long long()> imGetVipNum;
	std::function<std::future<void>(const TestWorkPool::Keys&, std::function<void(MyEntry*)>)> imInsertJob;

	void OnStart() {
		cout << "VipNum:" << imGetVipNum() << endl;
		cout << "Toy Start" << endl;
	}
	void OnCmd(bool& ok, std::string& cmd) {
		if (cmd == "Toy") {
			TestWorkPool::Keys keys = { "Toy" };
			imInsertJob(keys, [this](MyEntry* entry) {
				cout << "Toy work:" << imGetVipNum() << endl;
			}).get();
		}
	}
};

int main(int argc, char** argv)
{
	using namespace grok;
	EventPools::Init();

	auto configTool = Entity::GetEntity().assign<ConfigTool>(argv[0]);

	auto controller = Entity::GetEntity().assign<Controller>();
	controller->evCmd += delegate(controller, &Controller::OnCmd);

	auto simpleDaiyLog = Entity::GetEntity().assign<SimpleDailyLogService>(configTool->getProcessDir().c_str(), configTool->getProcessName().c_str());
	controller->evStart += delegate(simpleDaiyLog, &SimpleDailyLogService::OnStart);
	controller->evStop += delegate(simpleDaiyLog, &SimpleDailyLogService::OnStop);
	controller->evCmd += delegate(simpleDaiyLog, &SimpleDailyLogService::OnTest);

	auto workPool = Entity::GetEntity().share_assign<TestWorkPool>();
	controller->evStart += delegate(workPool, &TestWorkPool::OnStart);
	workPool->imGetStr = make_function_wrapper(configTool, &ConfigTool::readStr);
	workPool->imGetNum = make_function_wrapper(configTool, &ConfigTool::readNum);

	auto player = Entity::GetEntity().assign<Player>();
	controller->evStart += delegate(player, &Player::OnStart);
	player->imGetVipNum = std::bind(&Controller::GetVipNum, controller);
	player->imInsertJob = [workPool](const TestWorkPool::Keys&r1, std::function<int(MyEntry*)>r2) {
		return workPool->insertWorkJob(r1, r2);
	};
	controller->evCmd += delegate(player, &Player::OnCmd);

	auto toy = Entity::GetEntity().assign<Toy>();
	controller->evStart += delegate(toy, &Toy::OnStart);
	toy->imGetVipNum = std::bind(&Controller::GetVipNum, controller);
	toy->imInsertJob = [workPool](const TestWorkPool::Keys& r1, std::function<void(MyEntry*)>r2) {
		return workPool->insertWorkJob(r1, r2);
	};
	controller->evCmd += delegate(toy, &Toy::OnCmd);


	controller->StartLoop();

	EventPools::Uinit();
	return 0;
}
