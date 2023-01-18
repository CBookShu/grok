#include <iostream>
#include "grok/grok.h"
#include <random>
#include <cassert>


using namespace std;
using namespace grok::mysql;
std::default_random_engine ge(std::random_device{}());

static void test_datasetget(MysqlPool::SPtr pool) {
    // 准备数据库
    assert(pool->Query("DROP table IF EXISTS testtable") != -1);
    assert(pool->Query("CREATE TABLE IF NOT EXISTS `testtable` (\
        `n1` int(11) DEFAULT NULL,\
        `s2` varchar(255) DEFAULT NULL,\
        `b3` blob\
        ) ENGINE=InnoDB DEFAULT CHARSET=latin1;") != -1);

    // 添加数据
    assert(pool->Query("INSERT INTO testtable (n1,s2) VALUES(1, 'hello1')") == 1);
    assert(pool->Query("INSERT INTO testtable (n1,s2) VALUES(2, 'hello2')") == 1);

    // 获取数据
    auto result = pool->QueryResult("SELECT * FROM testtable WHERE n1 = 1");
    assert(result.FindField(0) == "n1");
    assert(result.FindField(1) == "s2");
    assert(result.GetRow() == 1);
    result.Next();
    auto n1_int = result.GetInt32("n1");
    assert(n1_int && n1_int == 1);
    auto n1_uint = result.GetUInt32("n1");
    assert(n1_uint && n1_uint == (std::uint32_t)1);
    auto n1_int64= result.GetInt64("n1");
    assert(n1_int64 && n1_int64 == (std::int64_t)1);
    auto n1_uint64 = result.GetUInt64("n1");
    assert(n1_uint64 && n1_uint64 == (std::uint64_t)1);

    boost::string_view s1 = "hello1";
    assert(result.GetString("s2") == s1);

    // 多行数据获取
    result = pool->QueryResult("SELECT * FROM testtable ORDER BY n1");
    int pos = 1;
    while(result.Next()) {
        assert(pos == result.GetInt32(0));
        char sField[16] = {0};
        sprintf(sField, "hello%d", pos++);
        assert(result.GetString(1) == sField);
    }

    // 带空的判断
    assert(pool->Query("INSERT INTO testtable (n1) VALUES(3)") == 1);
    result = pool->QueryResult("SELECT * FROM testtable WHERE n1 = 3");
    result.Next();
    assert(3 == result.GetInt32(0));
    assert(result.IsNull(1));

    // 二进制解析
    assert(pool->QueryStm("INSERT INTO testtable (n1,s2,b3) VALUES(?,?,?)", 
    4, "hello4", std::string("sdfsdf")) == 1);
    result = pool->QueryResult("SELECT * FROM testtable WHERE n1 = 4");
    if(result.Next()) {
        assert(4 == result.GetInt32(0));
        assert(result.GetString(1) == "hello4");
        assert(result.GetBlob(2) == "sdfsdf");
    }
}

int main(int argc, char** argv) {
    MysqlConfig config;
    config.host = "localhost";
    config.db = "";
    config.port = 3306;
    config.user = "cbookshu";
    config.pwd = "cs123456";
    
    // 初始化数据库
    MysqlClient client(config);
    assert(client.Query("CREATE DATABASE IF NOT EXISTS test") != -1);

    config.db = "test";

    const int count = 4;
    MysqlPool::SPtr pool = MysqlPool::Create(count, config);
    
    test_datasetget(pool);
    return 0;
}