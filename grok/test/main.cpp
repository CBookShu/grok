#include <iostream>
#include <chrono>
#include <functional>
#include <ctime>
#include "../include/grok.h"

using namespace std;

class Controller : public grok::WorkStaff
{
public:
	grok::BasicEventNoMutex<> evStart;
	grok::BasicEventNoMutex<int> evClock;

	void Start() {
		m_timer = evp().loopTimer([this]() {
			OnTimerFresh();
		}, std::chrono::seconds(1), strand());

		evStart();
	}

	// 该接口由此类提供
	long long GetVipNum() {
		return std::chrono::steady_clock::now().time_since_epoch().count();
	}

protected:
	void OnTimerFresh() {
		evClock.notify(std::time(nullptr));
	}

private:
	grok::stdtimerPtr m_timer;
};

class Player
{
public:
	std::function<long long()> imGetVipNum;

	void OnStart() {
		cout << "VipNum:" << imGetVipNum() << endl;
		cout << "Player Start" << endl;
	}
	void OnClock(int c) {
		cout << "VipNum:" << imGetVipNum() << endl;
		cout << "Player Clock:" << c << endl;
	}
};

class Toy
{
public:
	std::function<long long()> imGetVipNum;

	void OnStart() {
		cout << "VipNum:" << imGetVipNum() << endl;
		cout << "Toy Start" << endl;
	}
	void OnClock(int c) {
		cout << "VipNum:" << imGetVipNum() << endl;
		cout << "Toy Clock:" << c << endl;
	}
};

int main(int argc, char** argv)
{
	using namespace grok;
	
	EventPools::Init();

	auto controller = Entity::GetEntity().assign<Controller>();

	auto player = Entity::GetEntity().assign<Player>();
	controller->evStart += delegate(player, &Player::OnStart);
	controller->evClock += delegate(player, &Player::OnClock);
	player->imGetVipNum = std::bind(&Controller::GetVipNum, controller);

	auto toy = Entity::GetEntity().assign<Toy>();
	controller->evStart += delegate(toy, &Toy::OnStart);
	controller->evClock += delegate(toy, &Toy::OnClock);
	toy->imGetVipNum = std::bind(&Controller::GetVipNum, controller);

	controller->Start();

	std::this_thread::sleep_for(std::chrono::seconds(30));
	cout << "hello world" << endl;

	EventPools::Uinit();
	return 0;
}