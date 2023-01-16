#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <memory>
#include <atomic>
#include <vector>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <hiredis/hiredis.h>
#include "locklist.h"
#include "groklog.h"

namespace grok {

    struct RedisConfig {
        std::string url = "127.0.0.1";
        std::string auth;
        int port = 6379;
        // 默认一个小时都没有使用过的rediscon就要重新再连接一次
        std::chrono::milliseconds alivetime = std::chrono::milliseconds(1000 * 60 * 60);
    };

    struct ReplyDeleter {
        void operator()(redisReply *reply) const {
            if (reply != nullptr) {
                freeReplyObject(reply);
            }
        }
    };

    using ReplyUPtr = std::unique_ptr<redisReply, ReplyDeleter>;

    class RedisCon : boost::noncopyable {
    public:
        RedisCon() = default;
        RedisCon(RedisConfig& config);
        ~RedisCon();

        redisContext* GetCtx();

        int RedisCmdAppend(const char* fmt, ...);
        ReplyUPtr RedisReplay();

        bool CheckValid(bool resume = true);

        bool Reconnect();

        void SetBad() {m_bad = true;}
    private:
        redisContext* m_ctx = nullptr;
        RedisConfig m_config;
        bool m_bad{false};
        std::chrono::system_clock::time_point m_lastop = std::chrono::system_clock::now();
    };

    
    // replay的一些解析
    template<typename T>
    struct ReplayParse;
    template<>struct ReplayParse<std::string> {
        static boost::optional<std::string>
        Parse(redisReply* rpl) {
            switch (rpl->type)
            {
            case REDIS_REPLY_STRING:
                return boost::optional<std::string>(boost::in_place_init, rpl->str, rpl->len);
            case REDIS_REPLY_STATUS:
                return boost::optional<std::string>(boost::in_place_init, rpl->str, rpl->len);
            case REDIS_REPLY_INTEGER:
                return boost::optional<std::string>(boost::in_place_init, std::to_string(rpl->integer));
            case REDIS_REPLY_ARRAY:
                DBG("want string but array");
                break;
            case REDIS_REPLY_ERROR:
                if(rpl->str) {
                    return boost::optional<std::string>(boost::in_place_init, rpl->str, rpl->len);
                }
                break;
            case REDIS_REPLY_NIL:
                break;
            default:
                break;
            }
            return boost::none;
        }
    };

    template<>struct ReplayParse<long long> {
        static boost::optional<long long>
        Parse(redisReply* rpl) {
            switch (rpl->type)
            {
            case REDIS_REPLY_INTEGER:
                return boost::optional<long long>(boost::in_place_init, rpl->integer);
            case REDIS_REPLY_STRING:
                {
                    try {
                        long long r = std::stoll(rpl->str);
                        return boost::optional<long long>(boost::in_place_init, r);
                    } catch(...) {

                    }
                    break;
                }
            default:
                break;
            }
            return boost::none;
        }
    };

    template<>struct ReplayParse<double> {
        static boost::optional<double>
        Parse(redisReply* rpl) {
            try {
                auto d = std::stod(rpl->str);
                return boost::optional<double>(boost::in_place_init, d);
            } catch (const std::invalid_argument &) {
                return boost::none;
            } catch (const std::out_of_range &) {
                return boost::none;
            }
        }
    };

    /*
        redis的连接池
        一个redisconpool只能针对同一个ip做池，池子中的redis有互不相同的ip和port
    */
    class RedisConPool : boost::noncopyable {
    public:
        using LockListThis = LockList<RedisCon>;

        RedisConPool() = default;
        bool InitRedisCon(int size, RedisConfig& config);

        LockListThis::Guard GetRdsConGuard() {
            return m_pool.GetByGuard();
        }
    private:
        std::atomic_bool m_init{false};
        LockList<RedisCon> m_pool;
    };
}
