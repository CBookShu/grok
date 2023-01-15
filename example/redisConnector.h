#pragma once
#include <iostream>
#include <hiredis/hiredis.h>
#include <string>
#include <chrono>

namespace grok {

    struct RedisConfig {
        std::string m_url = "localhost";
        std::string m_auth;
        int m_port = 6379;
    };

    // TODO: 完成send,recv接口
    class RedisCon {
    public:
        RedisCon() = default;
        RedisCon(RedisConfig& config);
        ~RedisCon();

        bool CheckValid(bool resume = true);
        bool Reconnect();

        auto GetLastOP() {
            return m_lastop;
        }
    private:
        redisContext* m_ctx = nullptr;
        RedisConfig m_config;
        std::chrono::system_clock::time_point m_lastop = std::chrono::system_clock::now();
    };

    // TODO: 完成RedisConPool
    
}
