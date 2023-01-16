#include "redisConnector.h"

grok::RedisCon::RedisCon(RedisConfig& config):m_config(config)
{
}

grok::RedisCon::~RedisCon()
{
    if(m_ctx) {
        redisFree(m_ctx);
    }
}

redisContext *grok::RedisCon::GetCtx()
{
    if(CheckValid()) {
        m_lastop = std::chrono::system_clock::now();
        return m_ctx;
    }
    return nullptr;
}

int grok::RedisCon::RedisCmdAppend(const char *fmt, ...)
{
    va_list ap;
    int ret;
    auto* ctx = GetCtx();
    if(!ctx) {
        return REDIS_ERR;
    }
    va_start(ap,fmt);
    ret = redisvAppendCommand(ctx,fmt,ap);
    va_end(ap);
    return ret;
}

grok::ReplyUPtr grok::RedisCon::RedisReplay()
{
    auto* ctx = GetCtx();
    if(!ctx) {
        return nullptr;
    }
    void *r = nullptr;
    if (redisGetReply(ctx, &r) != REDIS_OK) {
        return nullptr;
    }
    if(ctx->err != REDIS_OK) {
        return nullptr;
    }
    return ReplyUPtr((redisReply*)r);
}

bool grok::RedisCon::CheckValid(bool resume)
{
    if(m_bad) {
        return Reconnect();
    }

    if(m_ctx && m_ctx->err == REDIS_OK) {
        return true;
    }

    if(!resume) {
        return false;
    }
    
    return Reconnect();
}

bool grok::RedisCon::Reconnect()
{
    if(m_ctx) {
        redisFree(m_ctx);
    }

    m_ctx = redisConnect(m_config.url.c_str(), m_config.port);
    if(!m_ctx) {
        DBG("rds connect error:%s,%d", m_config.url.c_str(), m_config.port);
        return false;
    }

    if(m_ctx->err != REDIS_OK) {
        // 不释放ctx下次再reconnect的时候，自会清理
        // 如果RedisCon生命周期结束，析构函数也会清理
        DBG("rds ctx err:%d", m_ctx->err);
        return false;
    }

    if(!m_config.auth.empty()) {
        // 密码校验
        RedisCmdAppend("AUTO %s", m_config.auth.c_str());
    }

    m_bad = false;
    return true;
}

bool grok::RedisConPool::InitRedisCon(int size, RedisConfig& config)
{
    bool exp = false;
    if(!m_init.compare_exchange_strong(exp, true)) {
        return true;
    }

    for (int i = 0; i < size; ++i) {
        m_pool.Give(new RedisCon(config));
    }

    return true;
}
