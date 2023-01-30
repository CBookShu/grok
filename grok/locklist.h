#pragma once
#include <list>
#include <mutex>
#include <condition_variable>
#include <cassert>

namespace grok {

    struct GrokRunable {
        virtual void run() = 0;
    };

    template <typename T>
    class LockList {
    public:
        LockList() = default;
        ~LockList() {
            // 析构函数一定是不能并行的，所以就不再加锁了！
            // 如果有并行的情况，那就是实例的生命周期控制异常，要查看怎么出现这样的情况
            // 析构函数仅保证资源不泄露
            while (!m_list.empty())
            {
                auto* t = m_list.back();
                if(t) {
                    delete t;
                }
                m_list.pop_back();
            }
        }

        T* Get() {
            std::unique_lock<std::mutex> guard(m_mutex);
            m_cv.wait(guard, [this](){
                return !m_list.empty();
            });
            T* t = m_list.front();
            m_list.pop_front();
            return t;
        }
        void Give(T*t) {
            std::unique_lock<std::mutex> guard(m_mutex);
            m_list.push_back(t);
            m_cv.notify_one();
        }
    public:
        // 简易的Guard类，如果可以尽量都用它，不用原生的Get和Give
        class Guard {
            LockList* m_ll = nullptr;
            T* m_t = nullptr;
        public:
            Guard(LockList* ll, T* t):m_ll(ll),m_t(t){}
            ~Guard() {
                if(m_t && m_ll) {
                    m_ll->Give(m_t);
                }
            }
            T* operator->() {
                return m_t;
            }
            T* Get() {
                return m_t;
            }
        };
        Guard GetByGuard() {
            return Guard(this, Get());
        }
    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::list<T*> m_list;
    };

}
