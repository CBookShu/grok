#include "mysqlConnector.h"
#include "groklog.h"
#include <cassert>
#include <mysql/errmsg.h>
#include <exception>

namespace grok::mysql
{
    // 定义一个私有的全局变量初始化和反初始化libmysqlclient的接口
    struct LibMysqlClientHelper {
        LibMysqlClientHelper() {
            mysql_library_init(0, 0, nullptr);
        }
        ~LibMysqlClientHelper() {
            mysql_library_end();
        }
    };
    LibMysqlClientHelper G_INIT_UINIT;
} // namespace grok::mysql

int grok::mysql::Records::GetRow()
{
    return mysql_num_rows(res);
}

bool grok::mysql::Records::Next()
{
    auto r = mysql_fetch_row(res) != 0;
    if(r) {
        mysql_fetch_lengths(res);
    }
    return r;
}

MYSQL_FIELD *grok::mysql::Records::FindFieldInfo(int col)
{
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    auto n = mysql_num_fields(res);
    if(col >= 0 && col < n) {
        return &fields[col];
    }
    return nullptr;
}

const char *grok::mysql::Records::FindField(int col)
{
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    auto n = mysql_num_fields(res);
    if(col >= 0 && col < n) {
        return fields[col].name;
    }
    return "";
}

int grok::mysql::Records::FindField(const char *name)
{
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    auto n = mysql_num_fields(res);
    for(int i = 0; i < n; ++i) {
        if (strcmp(fields[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

bool grok::mysql::Records::IsNull(int col)
{
    auto n = mysql_num_fields(res);
    if(col >= 0 && col < n) {
        return res->lengths[col] == 0;
    }
    return true;
}

bool grok::mysql::Records::IsNull(const char *name)
{
    return IsNull(FindField(name));
}

const char *grok::mysql::Records::GetString(int col)
{
    auto n = mysql_num_fields(res);
    if(col >= 0 && col < n) {
        return res->current_row[col];
    }
    return nullptr;
}

const char *grok::mysql::Records::GetString(const char *name)
{
    return GetString(FindField(name));
}

template <typename T>
struct ConverStringToOptionalData {
    template<typename F>
    static boost::optional<T> Convert(grok::mysql::Records* res,int col, F f) {
            try {
                const char* s = res->GetString(col);
                if(!s) {
                    return boost::optional<T>();
                }
                char *es;
                T d = f(s);
                return boost::optional<T>(boost::in_place_init, d);
            } catch(...) {
                return boost::optional<T>();
            }
        }
};

boost::optional<long double> grok::mysql::Records::GetDouble(int col)
{
    auto f = [](const char* s){
        char* es;
        long double d = std::strtold(s, &es);
        if(es == s) {
            throw std::bad_cast();
        }
        return d;
    };
    return ConverStringToOptionalData<long double>::Convert(this, col, f);
}

boost::optional<long double> grok::mysql::Records::GetDouble(const char *name)
{
    return GetDouble(FindField(name));
}

boost::optional<std::int32_t> grok::mysql::Records::GetInt32(int col)
{
    auto f = [](const char* s){
        char* es;
        long double d = std::strtoll(s, &es, 10);
        if(es == s) {
            throw std::bad_cast();
        }
        return d;
    };
    return ConverStringToOptionalData<std::int32_t>::Convert(this, col, f);
}

boost::optional<std::int32_t> grok::mysql::Records::GetInt32(const char *name)
{
    return GetInt32(FindField(name));
}

boost::optional<std::uint32_t> grok::mysql::Records::GetUInt32(int col)
{
    auto f = [](const char* s){
        char* es;
        long double d = std::strtoull(s, &es, 10);
        if(es == s) {
            throw std::bad_cast();
        }
        return d;
    };
    return ConverStringToOptionalData<std::uint32_t>::Convert(this, col, f);
}

boost::optional<std::uint32_t> grok::mysql::Records::GetUInt32(const char *name)
{
    return GetUInt32(FindField(name));
}

boost::optional<std::int64_t> grok::mysql::Records::GetInt64(int col)
{
    auto f = [](const char* s){
        char* es;
        long double d = std::strtoll(s, &es, 10);
        if(es == s) {
            throw std::bad_cast();
        }
        return d;
    };
    return ConverStringToOptionalData<std::int64_t>::Convert(this, col, f);
}

boost::optional<std::int64_t> grok::mysql::Records::GetInt64(const char *name)
{
    return GetInt64(FindField(name));
}

boost::optional<std::uint64_t> grok::mysql::Records::GetUInt64(int col)
{
    auto f = [](const char* s){
        char* es;
        long double d = std::strtoull(s, &es, 10);
        if(es == s) {
            throw std::bad_cast();
        }
        return d;
    };
    return ConverStringToOptionalData<std::uint64_t>::Convert(this, col, f);
}

boost::optional<std::uint64_t> grok::mysql::Records::GetUInt64(const char *name)
{
    return GetUInt64(FindField(name));
}

const char *grok::mysql::Records::GetBlob(int col, int &size)
{
    auto n = mysql_num_fields(res);
    if(col >= 0 && col < n) {
        size = res->lengths[col];
        return res->current_row[col];
    }
    size = 0;
    return nullptr;
}

const char *grok::mysql::Records::GetBlob(const char *name, int &size)
{
    return GetBlob(FindField(name), size);
}

MYSQL *grok::mysql::MysqlClient::GetCtx()
{
    if(CheckValid()) {
        m_lastop = std::chrono::steady_clock::now();
        return m_ctx;
    }
    return nullptr;
}

int grok::mysql::MysqlClient::Query(const char *sql)
{
    auto* ctx = GetCtx();
    assert(ctx != nullptr);
    return QueryWithCtx(ctx, sql);
}

grok::mysql::Records grok::mysql::MysqlClient::QueryResult(const char *sql)
{
    auto ctx = GetCtx();
    assert(ctx != nullptr);
    QueryWithCtx(ctx, sql);
     
    MYSQL_RES* res = mysql_store_result(ctx);
    return Records(res);
}

bool grok::mysql::MysqlClient::CheckValid()
{
    if(!m_ctx) {
        return Reconnect();
    }

    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastop);    
    if(diff <= m_config.alivetime) {
        return true;
    }
    // 我们可以pingpong一下
    if(!mysql_ping(m_ctx)) {
        // mysql可以设置自动reconnect模式
        // 这个时候如果pingpong失败，内部会自动重连
        // 如果这时候重连仍然失败，我们就不要重试了，肯定有其他异常了
        DBG("MYSQL Reconnect Error %d:%s", mysql_errno(m_ctx), mysql_error(m_ctx));
        return false;
    }
    return true;
}

bool grok::mysql::MysqlClient::Reconnect()
{
    bool reconnect = 1;

    if(m_ctx) {
        mysql_close(m_ctx);
    }
    m_ctx = mysql_init(nullptr);
    if(!mysql_real_connect(m_ctx, 
        m_config.host.c_str(), 
        m_config.user.c_str(), 
        m_config.pwd.c_str() , 
        m_config.db.c_str(),
        m_config.port,
        nullptr, 0)) {
        DBG("MYSQL connect Error: %s,%s,%s,%s,%d", m_config.host.c_str(),m_config.user.c_str(),m_config.pwd.c_str(),m_config.db.c_str(),m_config.port);
        DBG("MYSQL connect Error: %d,%s", mysql_errno(m_ctx), mysql_error(m_ctx));
        goto final;
    }

    // 设置自动重连
    if (mysql_options(m_ctx, MYSQL_OPT_RECONNECT, &reconnect)) {
        DBG("MYSQL setreconnect Error: %d,%s", mysql_errno(m_ctx), mysql_error(m_ctx));
        goto final;
    }

    return true;
final:
    if(m_ctx) {mysql_close(m_ctx);}
    return false;
}

int grok::mysql::MysqlClient::QueryWithCtx(MYSQL *ctx, const char *sql)
{
    // mysql 有些操作是可以重试的！【至少看源码是这么操作的】
    // 但是我们就重试一次即可
    // TODO: 复现并测试重连场景
    bool hasretry = false;
    do
    {
        auto err = mysql_query(ctx, sql);
        if(err) {
            if(err == CR_SERVER_LOST && !hasretry) {
                // 这里直接用ping重连
                mysql_ping(ctx);
                hasretry = true;
                continue;
            }
            // 已经error了
            DBG("MYSQL QUERY SQLTEXT:%s", sql);
            DBG("MYSQL QUERY ERROR:%d,%s", mysql_errno(ctx), mysql_error(ctx));
            break;
        }
        // 到了这里，就说明query执行完成
    } while (0);
    return mysql_affected_rows(ctx);
}

grok::mysql::MysqlPool::SPtr grok::mysql::MysqlPool::Create(int size, MysqlConfig &config)
{
    auto res = std::make_shared<grok::mysql::MysqlPool>();
    for (int i = 0; i < size; ++i) {
        res->Give(new MysqlClient(config));
    }
    return res;

}
