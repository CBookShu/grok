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

bool grok::RedisCon::CheckValid(bool resume)
{
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

    
}
