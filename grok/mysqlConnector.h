#pragma once
#include <string>
#include <string.h>
#include <mysql/mysql.h>
#include <boost/noncopyable.hpp>
#include <memory>
#include <vector>
#include <chrono>
#include <cstdint>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>
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

    struct Records : boost::noncopyable {
        // res一旦被构建传入，外部不要对它进行任何申明周期的控制
        // 这里之所以放在public上，纯粹是因为mysql的接口非常多，留着res方便使用
        MYSQL_RES* res = nullptr;

        Records(MYSQL_RES *r);
        ~Records();
        Records(Records&& other);
        Records& operator=(Records&& other);

        int GetRow();
        bool Next();
        // 这里获取field不再像官方cpp包里获取时自动缓存field:cor
        // 因为我认为col的数量一般很小，遍历速度也是够了
        MYSQL_FIELD* FindFieldInfo(int col);
        boost::string_view FindField(int col);
        int FindField(const char* name);
        bool IsNull(int col);
        bool IsNull(const char* name);
        boost::string_view GetString(int col);
        boost::string_view GetString(const char* name);
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
        boost::string_view GetBlob(int col);
        boost::string_view GetBlob(const char*name);
    };

    // 制作sqltext 顺便支持二进制参数
    // 例子:  auto stm =SqlTextMaker::Create("SELECT * FROM TABLE WHERE m = ? AND n = ? AND j = ?");
    // stm.BindParam(0, 1); stm.BindParam(1, "text"); stm.BindParam(con, 2, std::string(...));
    class SqlTextMaker {
    public:
        static SqlTextMaker Create(const char* fmt);
        int ParamsCount() {return params.size();}
        
        std::string GetSqlText();

        void BindParam(MYSQL* con,int pos, const char* s);
        // 二进制数据
        void BindParam(MYSQL* con, int pos, std::string& s);
        template <typename T>
        void BindParam(MYSQL* con,int pos, T t) {
            static_assert(std::is_arithmetic<T>::value);
            params[pos] = std::to_string(t);
        }
    private:
        std::vector<std::string> stmt_splice;
        std::vector<std::string> params;
    };

    struct ParamBindT {
        template <typename T>
        static void Bind(MYSQL* con, SqlTextMaker&stm, int& pos, T&& t) {
            stm.BindParam(con, pos, t);
        }

        template <typename Head, typename...Args>
        static void Bind(MYSQL* con, SqlTextMaker&stm, int& pos, Head&& t, Args&&...args) {
            stm.BindParam(con, pos++, t);
            ParamBindT::Bind(con, stm, pos, std::forward<Args>(args)...);
        }
    };
    
    class MysqlClient : boost::noncopyable {
    public:
        MysqlClient(MysqlConfig& config):m_config(config){}

        // 返回值是:mysql_affected_rows() 影响的行数
        int Query(const char* sql);
        Records QueryResult(const char* sql);
        MYSQL* GetCtx();
    protected:
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

        int Query(const char* sql);
        Records QueryResult(const char* sql);

        template <typename ...Args>
        int QueryStm(const char* sql, Args&&...args) {
            auto stm = SqlTextMaker::Create(sql);
            int pos = 0;
            auto g = GetByGuard();
            ParamBindT::Bind(g->GetCtx(), stm, pos, std::forward<Args>(args)...);
            auto sqltext = stm.GetSqlText();
            return g->Query(sqltext.c_str());
        }
        template <typename ...Args>
        Records QueryStmResult(const char* sql, Args&&...args) {
            auto stm = SqlTextMaker::Create(sql);
            int pos = 0;
            auto g = GetByGuard();
            ParamBindT::Bind(g->GetCtx(), stm, pos, std::forward<Args>(args)...);
            auto sqltext = stm.GetSqlText();
            return g->QueryResult(sqltext.c_str());
        }
    };
} // namespace grok
