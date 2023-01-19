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

grok::mysql::Records::Records(MYSQL_RES *r):res(r) {}

grok::mysql::Records::~Records() {
    if(res) {
        mysql_free_result(res);
    }
}

grok::mysql::Records::Records(Records &&other) {
    std::swap(res, other.res);
}

grok::mysql::Records &grok::mysql::Records::operator=(Records &&other)
{
    std::swap(res, other.res);
}

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

boost::string_view grok::mysql::Records::FindField(int col)
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

boost::string_view grok::mysql::Records::GetString(int col)
{
    auto n = mysql_num_fields(res);
    if(col >= 0 && col < n) {
        return res->current_row[col];
    }
    return nullptr;
}

boost::string_view grok::mysql::Records::GetString(const char *name)
{
    return GetString(FindField(name));
}

template <typename T>
struct ConverStringToOptionalData {
    template<typename F>
    static boost::optional<T> Convert(grok::mysql::Records* res,int col, F f) {
            try {
                auto s = res->GetString(col);
                if(s.empty()) {
                    return boost::optional<T>();
                }
                char *es;
                T d = f(s.data());
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
        std::int32_t d = std::strtoll(s, &es, 10);
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
        std::uint32_t d = std::strtoull(s, &es, 10);
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
        std::int64_t d = std::strtoll(s, &es, 10);
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
        std::uint64_t d = std::strtoull(s, &es, 10);
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

boost::string_view grok::mysql::Records::GetBlob(int col)
{
    auto n = mysql_num_fields(res);
    if(col >= 0 && col < n) {
        return boost::string_view(res->current_row[col], res->lengths[col]) ;
    }
    return nullptr;
}

boost::string_view grok::mysql::Records::GetBlob(const char *name)
{
    return GetBlob(FindField(name));
}

grok::mysql::SqlTextMaker grok::mysql::SqlTextMaker::Create(const char *fmt)
{
    SqlTextMaker stm;
    int paramcount = 0;
    const char *pos1 = fmt;
    const char *pos2 = fmt;
    const char *pend = fmt + strlen(fmt);
    while(pos2 != pend) {
        pos2++;
        if(*pos1 == '?') {
            pos1++;
            continue;
        }
        if(*pos2 == '?') {
            paramcount++;
            stm.stmt_splice.push_back(std::string(pos1, pos2 - pos1));
            pos1 = pos2;
        }
    }
    if(*pos1 == '?') {pos1++;}
    if(*pos2 == '?') {pos2++;}
    if(pos2 != pos1) {
        stm.stmt_splice.push_back(std::string(pos1, pos2 - pos1));
    }
    stm.params.resize(paramcount);
    return stm;
}

std::string grok::mysql::SqlTextMaker::GetSqlText()
{
    std::string sqltext;
    for (int i = 0; i < stmt_splice.size(); ++i) {
        sqltext.append(stmt_splice[i]);
        if(params.size() > i) {
            sqltext.append(params[i]);
        }
    }
    return sqltext;
}

void grok::mysql::SqlTextMaker::BindParam(MYSQL* con,int pos, const char *s)
{
    params[pos] = s;

    params[pos].append("'");
    stmt_splice[pos].append("'");
}

void grok::mysql::SqlTextMaker::BindParam(MYSQL *con, int pos, std::string &s) {
    auto len = s.size() * 2 + 1;
    params[pos].resize(len);
    auto newlen = mysql_real_escape_string(con, (char*)params[pos].data(), s.data(), s.size());
    params[pos].resize(newlen);
    
    params[pos].append("'");
    stmt_splice[pos].append("'");
}

MYSQL *grok::mysql::MysqlClient::GetCtx()
{
    if(CheckValid()) {
        m_lastop = std::chrono::steady_clock::now();
        return m_ctx;
    }
    return nullptr;
}

int grok::mysql::MysqlClient::Query(const char *sql, int size)
{
    auto* ctx = GetCtx();
    assert(ctx != nullptr);
    size = size ? size : strlen(sql);
    return QueryWithCtx(ctx, sql, size);
}

grok::mysql::Records grok::mysql::MysqlClient::QueryResult(const char *sql, int size)
{
    auto ctx = GetCtx();
    assert(ctx != nullptr);
    size = size ? size : strlen(sql);
    QueryWithCtx(ctx, sql, size);
     
    MYSQL_RES* res = mysql_store_result(ctx);
    Records record(res);
    return record;
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

int grok::mysql::MysqlClient::QueryWithCtx(MYSQL *ctx, const char *sql, int size)
{
    // mysql 有些操作是可以重试的！【至少看源码是这么操作的】
    // 但是我们就重试一次即可
    // TODO: 复现并测试重连场景
    bool hasretry = false;
    do
    {
        auto err = mysql_real_query(ctx, sql, size);
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
            return -1;
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

int grok::mysql::MysqlPool::Query(const char *sql)
{
    return GetByGuard()->Query(sql);
}

grok::mysql::Records grok::mysql::MysqlPool::QueryResult(const char *sql)
{
    return GetByGuard()->QueryResult(sql);
}
