#include <iostream>
#include <iostream>
#include <random>
#include <cassert>
#include "grok/grok.h"

using namespace std;
using namespace grok::redis;
std::default_random_engine ge(std::random_device{}());

void threadfunc(RedisConPool::SPtr pool) {
    auto rdscon = pool->GetByGuard();
    std::uniform_int_distribution<int> u(0, 200);
    auto rdkey = u(ge);
    auto rdval = u(ge);

    rdscon->RedisCmdAppend("SELECT 1");
    rdscon->RedisCmdAppend("SET %d %d", rdkey, rdval);
    rdscon->RedisCmdAppend("GET %d", rdkey);

    auto rpy = rdscon->RedisReplay();
    assert(strcmp("OK", rpy->str) == 0);
    rpy = rdscon->RedisReplay();
    assert(strcmp("OK", rpy->str) == 0);
    rpy = rdscon->RedisReplay();
    auto r = ReplayParse<long long>::Parse(rpy.get());
    assert(*r == rdval);
}

int main(int argc, char**argv) {
    redisContext* c;
    const char* ip = "127.0.0.1";
    int port = 6379;

    const int count = 12;

    RedisConPool::SPtr pool;
    RedisConfig config;
    pool = RedisConPool::Create(count, config);

    std::thread t[count];
    for (int i = 0; i < count; ++i) {
        t[i] = std::thread(threadfunc, pool);
    }

    for (int i = 0; i < count; ++i) {
        t[i].join();
    }
    return 0;
}