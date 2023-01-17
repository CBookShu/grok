#include <iostream>
#include "grok/grok.h"
#include <random>


using namespace std;
using namespace grok::mysql;
std::default_random_engine ge(std::random_device{}());

static void threadfunc(MysqlPool::SPtr pool) {
    auto g = pool->Get();

    std::uniform_int_distribution<int> u(0, 200);
    auto rdkey = u(ge);
    auto rdval = u(ge);

    char sqltext[4096] = {0};
    sprintf(sqltext, "INSERT INTO testtable (f1,f2) VALUES(%d, '%s%d')", rdkey, "test", rdval);
    auto n = g->Query("INSERT INTO testtable VALUES()");
    cout << "update row " << n << endl;

    sprintf(sqltext, "SELECT * FROM testtable");
    auto result = g->QueryResult(sqltext);
    while (result.Next())
    {
        if(!result.IsNull("f1")) {
            printf("%s = %s\n", result.FindField(0), result.GetString(0));
        }
        if(!result.IsNull("f2")) {
            printf("%s = %s\n", result.FindField(1), result.GetString(1));
        }
        if(!result.IsNull("f3")) {
            printf("%s = %s\n", result.FindField(2), result.GetString(2));
        }
    }
    
}

int main(int argc, char** argv) {
    MysqlConfig config;
    config.host = "localhost";
    config.db = "test";
    config.port = 3306;
    config.user = "cbookshu";
    config.pwd = "cs123456";
    
    const int count = 1;
    MysqlPool::SPtr pool = MysqlPool::Create(count, config);
    std::thread t[count];
    for(int i = 0; i < count; ++i) {
        t[i] = std::thread(threadfunc, pool);
    }
    for(int i = 0; i < count; ++i) {
        t[i].join();
    }
    return 0;
}