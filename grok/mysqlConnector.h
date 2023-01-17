#pragma once
#include <string>
#include <string.h>
#include <mysql/mysql.h>
#include <boost/noncopyable.hpp>
#include <memory>
#include <chrono>
#include <cstdint>
#include <boost/optional.hpp>
#include "locklist.h"

namespace grok::mysql
{
/*
    本来mysql的client用的是官方的cpp版本
    优点：
    1.  功能齐全
    2.  稳定性和后续维护性都更好
    缺点：
    1.  部署的时候总得再加一个库，感觉没必要

    经过官方文档里纯C的接口学习，发现自己用到的功能也不多，稍作封装就能用了
    于是再经过一些示例的编写后，打算开始封装它
    此外对于Records的操作，都建立在MYSQL_RES 内存上的
    返回的各种值都是建立在Records 生命周期之内


    TODO:
    1.  完善stmt接口
    2.  支持二进制接口
    3.  mysql C api还支持批量sql执行，未来看是否接入
*/
    
    struct MysqlConfig{
        int port;
        std::string host;
        std::string db;
        std::string user;
        std::string pwd;
        // 默认一个小时都没有使用过的需要ping一次
        std::chrono::milliseconds alivetime = std::chrono::milliseconds(1000 * 60 * 60);
    };

    struct ResultDeleter {
        void operator()(MYSQL_RES*res) {
            if(res) {
                mysql_free_result(res);
            }
        }
    };

    struct Records {
        MYSQL_RES* res = nullptr;
        // lens 是通过 mysql_fetch_lengths 计算得到的，缓存一下
        int* lens = nullptr;

        Records(MYSQL_RES *r):res(r){}
        ~Records() {if(res) {mysql_free_result(res);}}

        int GetRow();
        bool Next();
        // 这里获取field不再像官方cpp包里获取时自动缓存field:cor
        // 因为我认为col的数量一般很小，遍历速度也是够了
        MYSQL_FIELD* FindFieldInfo(int col);
        const char* FindField(int col);
        int FindField(const char* name);
        bool IsNull(int col);
        bool IsNull(const char* name);
        const char* GetString(int col);
        const char* GetString(const char* name);
        boost::optional<long double> GetDouble(int col);
        boost::optional<long double> GetDouble(const char* name);
        boost::optional<std::int32_t> GetInt32(int col);
        boost::optional<std::int32_t> GetInt32(const char* name);
        boost::optional<std::uint32_t> GetUInt32(int col);
        boost::optional<std::uint32_t> GetUInt32(const char* name);
        boost::optional<std::int64_t> GetInt64(int col);
        boost::optional<std::int64_t> GetInt64(const char* name);
        boost::optional<std::uint64_t> GetUInt64(int col);
        boost::optional<std::uint64_t> GetUInt64(const char* name);
        const char* GetBlob(int col, int& size);
        const char* GetBlob(const char*name, int& size);
    };

    class MysqlClient : boost::noncopyable {
    public:
        MysqlClient(MysqlConfig& config):m_config(config){}

        // 返回值是:mysql_affected_rows() 影响的行数
        int Query(const char* sql);
        Records QueryResult(const char* sql);

    protected:
        MYSQL* GetCtx();
        bool CheckValid();
        bool Reconnect();
        int QueryWithCtx(MYSQL* ctx, const char* sql);
    private:
        MysqlConfig m_config;
        MYSQL* m_ctx = nullptr;
        std::chrono::steady_clock::time_point m_lastop = std::chrono::steady_clock::now();
    };
    
    /*
        mysql的连接池
        一个mysqlpool只能针对同一个ip做池，池子中的connector不能有互不相同的ip和port
    */
    class MysqlPool : public LockList<MysqlClient>, boost::noncopyable{
    public:
        using SPtr = std::shared_ptr<MysqlPool>;
        static SPtr Create(int size, MysqlConfig& config);
    };
} // namespace grok
