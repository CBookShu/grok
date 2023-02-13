#pragma once
#include <string.h>
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

namespace grok::redis {
/*
    这里其实可以再仔细更进一步:
    1.  把replay全部特化出来，包括list和map对应多返回值和zset这种类型
    2.  把支持的命令都封装成函数调用，封装好大部分的字符串命令

    但是为什么没有这么做：
    1. redis的命令都是以字符串见长，封装过于冗余，暂时没有那么多时间去做
    2. 一旦封装，就需要把replay的值中转一次到stl容器中，会进行一次内存拷贝，这其实没必要
    3. 可以把这个想法留着以后再做
*/

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

        int RedisCmdAppend(const char* fmt, ...);
        int RedisCmdAppenArgv(int argc, const char **argv, const size_t *argvlen);

        ReplyUPtr RedisReplay();
        redisContext* GetCtx();

    public:
        struct GuardBad {
            RedisCon* m_c = nullptr;
            bool m_bad = true;

            void SetOk() {m_bad = false;}
            GuardBad(RedisCon* c):m_c(c){}
            ~GuardBad() {
                if (m_bad && m_c) {
                    m_c->SetBad();
                }
            }
        };
        void SetBad() {m_bad = true;}
    protected:
        bool CheckValid();
        bool Reconnect();
    private:
        redisContext* m_ctx = nullptr;
        RedisConfig m_config;
        bool m_bad{false};
        std::chrono::steady_clock::time_point m_lastop = std::chrono::steady_clock::now();
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
        一个redisconpool只能针对同一个ip做池，不允许其中有各不相同IP和PORT
    */
    class RedisConPool : public LockList<RedisCon>, boost::noncopyable{
    public:
        using SPtr = std::shared_ptr<RedisConPool>;
        static SPtr Create(int size, RedisConfig& config);
    };
}
