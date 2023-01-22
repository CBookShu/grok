#include "redisConnector.h"

grok::redis::RedisCon::RedisCon(RedisConfig& config):m_config(config)
{
}

grok::redis::RedisCon::~RedisCon()
{
    if(m_ctx) {
        redisFree(m_ctx);
    }
}

redisContext *grok::redis::RedisCon::GetCtx()
{
    if(CheckValid()) {
        m_lastop = std::chrono::steady_clock::now();
        return m_ctx;
    }
    return nullptr;
}

int grok::redis::RedisCon::RedisCmdAppend(const char *fmt, ...)
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
    if(ret != REDIS_OK) {
        SetBad();
    }
    return ret;
}

grok::redis::ReplyUPtr grok::redis::RedisCon::RedisReplay()
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

bool grok::redis::RedisCon::CheckValid()
{
    if(m_bad) {
        return Reconnect();
    }

    if(!m_ctx || m_ctx->err != REDIS_OK) {
        return Reconnect();
    }

    // ctx ok的情况下看看是否超时了
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastop);    
    if(diff < m_config.alivetime) {
        return true;
    }
    // 超时了，我们可以pingpong一下看看
    if(redisReply* replay = (redisReply*)redisCommand(m_ctx, "PING")) {
        if(replay->type == REDIS_REPLY_STRING 
            && strcmp(replay->str, "PONG") == 0 ) {
            return true;
        }
    }

    return Reconnect();
}

bool grok::redis::RedisCon::Reconnect()
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
        RedisCmdAppend("AUTH %s", m_config.auth.c_str());
    }

    m_bad = false;
    return true;
}

grok::redis::RedisConPool::SPtr grok::redis::RedisConPool::Create(int size, RedisConfig &config)
{
    auto res = std::make_shared<grok::redis::RedisConPool>();
    for (int i = 0; i < size; ++i) {
        res->Give(new RedisCon(config));
    }
    return res;
}
