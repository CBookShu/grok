#pragma once
#include <iostream>
#include <mutex>
#include <memory>
#include <vector>
#include <tuple>
#include <functional>

namespace grok
{
    template <typename...Args>
    struct AbstractDelegate
    {
        virtual AbstractDelegate* clone() const = 0;

        virtual bool equals(const AbstractDelegate& other) const = 0;

        virtual const AbstractDelegate* unwrap() const
        {
            return this;
        }

        virtual bool notify(Args...args) const = 0;

        virtual void disable() = 0;
    };

    template <typename O, int OT, typename...Args>
		// O代表Observer的类型
		// OT 代表O的传入类型, 0- O*, 1- std::shared_ptr<O>, 2- std::weak_ptr<O> 3-callback
		// Args 传入参数
		//Delegate 将函数封装成可以比较可以传递，可以扔来扔去的类
		// 没有给锁，应该由业务方保证是否线程安全
    struct Delegate {};


    //////////////////////////////////////////////////////////////////////////
    // std::weak_ptr<O>
    template <typename O, typename...Args>
    struct Delegate <O, 2, Args...> : public AbstractDelegate < Args... >
    {
        using OPtr = std::weak_ptr < O >;

        using CbType = void (O::*)(Args...);

        enum
        {
            HoldType = 2
        };

        Delegate(OPtr o, CbType cb) : m_o(o), m_cb(cb) {}
        Delegate(const Delegate& other) : m_o(other.m_o), m_cb(other.m_cb) {}


        virtual AbstractDelegate<Args...>* clone() const override
        {
            return new Delegate(*this);
        }

        virtual bool equals(const AbstractDelegate<Args...>& other) const override
        {
            return *this == other;
        }

        virtual bool notify(Args...args) const override
        {
            auto sp = m_o.lock();
            if (sp)
            {
                ((*sp).*m_cb)(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        template <typename...Params>
        bool operator()(Params... args)
        {
            auto sp = m_o.lock();
            if (sp)
            {
                ((*sp).*m_cb)(std::forward<Params>(args)...);
                return true;
            }
            return false;
        }

        bool operator == (const AbstractDelegate<Args...>& other) const
        {
            const Delegate* p = dynamic_cast<const Delegate*>(other.unwrap());
            if (!p)
            {
                return false;
            }
            if (m_o._Get() != p->m_o._Get())
            {
                return false;
            }
            if (m_cb != p->m_cb)
            {
                return false;
            }
            return true;
        }

        bool operator != (const AbstractDelegate<Args...>& other) const
        {
            return !(*this == other);
        }

        virtual void disable() override
        {
            m_o.reset();
        }

        OPtr    m_o;
        CbType  m_cb;
    };

    //////////////////////////////////////////////////////////////////////////
    // O* 原生指针
    template <typename O, typename...Args>
    struct Delegate <O, 0, Args...> : public AbstractDelegate < Args... >
    {
        using OPtr = O*;

        using CbType = void (O::*)(Args...);

        enum
        {
            HoldType = 0
        };

        Delegate(OPtr o, CbType cb) : m_o(o), m_cb(cb) {}
        Delegate(const Delegate& other) : m_o(other.m_o), m_cb(other.m_cb) {}


        virtual AbstractDelegate<Args...>* clone() const override
        {
            return new Delegate(*this);
        }

        virtual bool equals(const AbstractDelegate<Args...>& other) const override
        {
            return *this == other;
        }

        virtual bool notify(Args...args) const override
        {
            if (m_o)
            {
                ((*m_o).*m_cb)(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        bool operator()(Args...args)
        {
            if (m_o)
            {
                ((*m_o).*m_cb)(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        bool operator == (const AbstractDelegate<Args...>& other) const
        {
            const Delegate* p = dynamic_cast<const Delegate*>(other.unwrap());
            if (!p)
            {
                return false;
            }
            if (m_o != p->m_o)
            {
                return false;
            }
            if (m_cb != p->m_cb)
            {
                return false;
            }
            return true;
        }

        bool operator != (const AbstractDelegate<Args...>& other) const
        {
            return !(*this == other);
        }

        virtual void disable() override
        {
            m_o = nullptr;
        }


        OPtr    m_o;
        CbType  m_cb;
    };

    //////////////////////////////////////////////////////////////////////////
    // std::share_ptr<O> 智能指针
    template <typename O, typename...Args>
    struct Delegate <O, 1, Args...> : public AbstractDelegate < Args... >
    {

        using OPtr = std::shared_ptr < O >;

        using CbType = void (O::*)(Args...);

        enum
        {
            HoldType = 1
        };

        Delegate(OPtr o, CbType cb) : m_o(o), m_cb(cb) {}
        Delegate(const Delegate& other) : m_o(other.m_o), m_cb(other.m_cb) {}


        virtual AbstractDelegate<Args...>* clone() const override
        {
            return new Delegate(*this);
        }

        virtual bool equals(const AbstractDelegate<Args...>& other) const override
        {
            return *this == other;
        }

        virtual bool notify(Args...args) const override
        {
            if (m_o)
            {
                ((*m_o).*m_cb)(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        bool operator()(Args...args)
        {
            if (m_o)
            {
                ((*m_o).*m_cb)(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        bool operator == (const AbstractDelegate<Args...>& other) const
        {
            const Delegate* p = dynamic_cast<const Delegate*>(other.unwrap());
            if (!p)
            {
                return false;
            }
            if (m_o.get() != p->m_o.get())
            {
                return false;
            }
            if (m_cb != p->m_cb)
            {
                return false;
            }
            return true;
        }

        bool operator != (const AbstractDelegate<Args...>& other) const
        {
            return !(*this == other);
        }

        virtual void disable() override
        {
            m_o.reset();
        }


        OPtr    m_o;
        CbType  m_cb;
    };

    //////////////////////////////////////////////////////////////////////////
    // callback 纯函数
    template <typename...Args>
    struct Delegate <void, 3, Args...> : public AbstractDelegate < Args... >
    {
        using CbType = void(*)(Args...);
        enum
        {
            HoldType = 3
        };

        Delegate(CbType cb) : m_cb(cb) {}
        Delegate(const Delegate& other) : m_cb(other.m_cb) {}


        virtual AbstractDelegate<Args...>* clone() const override
        {
            return new Delegate(*this);
        }

        virtual bool equals(const AbstractDelegate<Args...>& other) const override
        {
            return *this == other;
        }

        virtual bool notify(Args...args) const override
        {
            if (m_cb)
            {
                (*m_cb)(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        bool operator()(Args...args)
        {
            if (m_cb)
            {
                (*m_cb)(std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        bool operator == (const AbstractDelegate<Args...>& other) const
        {
            const Delegate* p = dynamic_cast<const Delegate*>(other.unwrap());
            if (!p)
            {
                return false;
            }
            if (m_cb != p->m_cb)
            {
                return false;
            }
            return true;
        }

        bool operator != (const AbstractDelegate<Args...>& other) const
        {
            return !(*this == other);
        }

        virtual void disable() override
        {
            m_cb = nullptr;
        }

        CbType  m_cb;
    };


    template <typename O1, typename O2, typename...Args>
    inline Delegate<O2, 2, Args...> delegate(std::weak_ptr<O1> optr, void(O2::* NotifyMethod)(Args...))
    {
        static_assert(std::is_base_of<O2, O1>::value, "Can`t match type");

        auto sptr = std::static_pointer_cast<O2>(optr.lock());
        std::weak_ptr<O2> wptr = sptr;
        return Delegate<O2, 2, Args...>(wptr, NotifyMethod);
    }

    template <typename O1, typename O2, typename...Args>
    inline Delegate<O2, 0, Args...> delegate(O1* optr, void(O2::* NotifyMethod)(Args...))
    {
        static_assert(std::is_base_of<O2, O1>::value, "Can`t match type");

        return Delegate<O2, 0, Args...>(static_cast<O2*>(optr), NotifyMethod);
    }

    template <typename O1, typename O2, typename...Args>
    inline Delegate<O2, 1, Args...> delegate(std::shared_ptr<O1> optr, void(O2::* NotifyMethod)(Args...))
    {
        static_assert(std::is_base_of<O2, O1>::value, "Can`t match type");

        auto sptr = std::static_pointer_cast<O2>(optr);
        return Delegate<O2, 1, Args...>(sptr, NotifyMethod);
    }

    template <typename...Args>
    inline Delegate<void, 3, Args...> delegate(void(*cb)(Args...))
    {
        return Delegate<void, 3, Args...>(cb);
    }


    template <typename TDelegate, typename...Args>
    class AbstractStrategy
    {
    public:
        typedef TDelegate* DelegateHandle;

        virtual void notify(Args...args) = 0;

        virtual DelegateHandle add(const TDelegate& delegate) = 0;

        virtual void remove(const TDelegate& delegate) = 0;

        virtual void remove(DelegateHandle delegatehandle) = 0;

        virtual void clear() = 0;

        virtual bool empty() const = 0;
    };

    template <typename TDelegate, typename...Args>
    class DefaultStrategy : public AbstractStrategy < TDelegate, Args... >
    {
    public:
        typedef std::shared_ptr<TDelegate>                  DelegatePtr;
        typedef TDelegate* DelegateHandle;
        typedef std::vector<DelegatePtr>                    Delegates;
        typedef typename Delegates::iterator                Iterator;

        DefaultStrategy() {}
        DefaultStrategy(const DefaultStrategy& other) : m_delegates(other.m_delegates)
        {

        }
        DefaultStrategy(DefaultStrategy&& other) : m_delegates(std::move(other.m_delegates))
        {

        }

        DefaultStrategy& operator = (DefaultStrategy&& other)
        {
            if (&other == this)
            {
                return *this;
            }
            m_delegates.swap(other.m_delegates);
            return *this;
        }

        virtual void notify(Args...args) override
        {
            for (auto it = m_delegates.begin(); it != m_delegates.end();)
            {
                if (!(*it)->notify(std::forward<Args>(args)...))
                {
                    it = m_delegates.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        virtual DelegateHandle add(const TDelegate& delegate) override
        {
            DelegatePtr pDelegate(static_cast<TDelegate*>(delegate.clone()));
            m_delegates.push_back(pDelegate);
            return pDelegate.get();
        }

        virtual void remove(const TDelegate& delegate) override
        {
            for (Iterator it = m_delegates.begin(); it != m_delegates.end(); ++it)
            {
                if (delegate.equals(**it))
                {
                    m_delegates.erase(it);
                    return;
                }
            }
        }

        virtual void remove(DelegateHandle delegatehandle)
        {
            for (Iterator it = m_delegates.begin(); it != m_delegates.end(); ++it)
            {
                if ((*it).get() == delegatehandle)
                {
                    m_delegates.erase(it);
                    return;
                }
            }
        }

        DefaultStrategy& operator = (const DefaultStrategy& s)
        {
            if (this != &s)
            {
                m_delegates = s.m_delegates;
            }
            return *this;
        }

        virtual void clear() override
        {
            m_delegates.clear();
        }

        virtual bool empty() const override
        {
            return m_delegates.empty();
        }

    protected:
        Delegates   m_delegates;
    };


    template <typename T>
    struct lock_unlock_helper
    {
    private:
        template <typename U>
        static auto _check(int) ->decltype(std::declval<U>().lock(), std::declval<U>().unlock(), std::true_type());

        template <typename U>
        static auto _check(...)->std::false_type;
    public:
        enum
        {
            value = decltype(_check<T>(0))::value
        };
    };

    template <typename TDelegate, typename TStrategy, typename TMutex, typename... Args>
    class AbstractEvent
    {
        class ScopeGuard
        {
            TMutex& lck;
        public:
            ScopeGuard(TMutex& mutex) : lck(mutex)
            {
                lck.lock();
            }
            ~ScopeGuard()
            {
                lck.unlock();
            }
        };
    private:
        AbstractEvent(const AbstractEvent& other);
        AbstractEvent& operator = (const AbstractEvent& other);
    public:
        typedef std::shared_ptr<TStrategy>                              StrategyPtr;

        typedef AbstractDelegate<TDelegate, Args...>* DelegateHandle;


        AbstractEvent() : m_enabled(true) {}

        AbstractEvent(const TStrategy& strategy) : m_strategy(strategy), m_enabled(true) {}

        ~AbstractEvent() {}

        void operator += (const TDelegate& delegate)
        {
            ScopeGuard lock(m_mutex);
            assert(isEnabled());
            m_strategy.add(delegate);
        }

        void operator -= (const TDelegate& delegate)
        {
            ScopeGuard lock(m_mutex);
            m_strategy.remove(delegate);
        }

        AbstractEvent& add(const TDelegate& delegate)
        {
            ScopeGuard lock(m_mutex);
            assert(isEnabled());
            m_strategy.add(delegate);
            return *this;
        }

        AbstractEvent& remove(const TDelegate& delegate)
        {
            ScopeGuard lock(m_mutex);
            m_strategy.remove(delegate);
            return *this;
        }

        AbstractEvent& remove(DelegateHandle delegateHandle)
        {
            ScopeGuard lock(m_mutex);
            m_strategy.remove(delegateHandle);
            return *this;
        }

        void operator()(Args...args)
        {
            notify(std::forward<Args>(args)...);
        }

        void notify(Args...args)
        {
            TStrategy strategy;
            {
                ScopeGuard lock(m_mutex);
                if (!m_enabled) {
                    return;
                }
                strategy = m_strategy;
            }
            strategy.notify(std::forward<Args>(args)...);
        }

        void enable()
        {
            ScopeGuard lock(m_mutex);
            m_enabled = true;
        }

        void disable()
        {
            ScopeGuard lock(m_mutex);
            m_enabled = false;
        }

        bool isEnabled() const
        {
            ScopeGuard lock(m_mutex);
            return m_enabled;
        }

        void clear()
        {
            ScopeGuard lock(m_mutex);
            m_strategy.clear();
        }

        bool empty() const
        {
            ScopeGuard lock(m_mutex);
            return m_strategy.empty();
        }

    protected:
        TStrategy               m_strategy;
        bool                    m_enabled;
        mutable TMutex          m_mutex;
    };

    template <typename...Args>
    class BasicEvent : public AbstractEvent <
        AbstractDelegate<Args...>,
        DefaultStrategy<AbstractDelegate<Args...>, Args...>,
        std::mutex,
        Args...
    >
    {
    public:

    };

    template <typename TMutex, typename...Args>
    class BasicEventWithMutex : public AbstractEvent <
        AbstractDelegate<Args...>,
        DefaultStrategy<AbstractDelegate<Args...>, Args...>,
        TMutex,
        Args...
    >
    {
    public:

    };

    struct NoLock
    {
        inline void lock() {}
        inline void unlock() {}
    };

    template <typename ...Args>
    using BasicEventNoMutex = BasicEventWithMutex < NoLock, Args... >;


    template <typename T>
    static std::weak_ptr<T> ConvertToWeakPtr(std::shared_ptr<T> ptr)
    {
        std::weak_ptr<T> w = ptr;
        return  w.lock();
    }

    template<typename...Args>
    class TraitFunction;

    template<typename R, typename...Args>
    class TraitFunction<R(Args...)>
    {
    public:
        using RType = R;
        using Params = std::tuple<Args...>;
    };

    template<typename Callback>
    class FunctionWrapper
    {
    public:
        using Helper = TraitFunction<Callback>;
        using RType = typename Helper::RType;

        FunctionWrapper() = default;
        FunctionWrapper(std::function<Callback> f): m_cb(f) {}

        template <typename...Args>
        RType operator()(Args&& ... args)
        {
            assert(m_cb);
            return m_cb(std::forward<Args>(args)...);
        }

        template <typename...Args>
        RType notify(Args&& ... args)
        {
            assert(m_cb);
            return m_cb(std::forward<Args>(args)...);
        }

        operator bool() const
        {
            return m_cb != nullptr;
        }
        void assign(std::function<Callback> f)
        {
            m_cb = f;
        }
        void reset()
        {
            m_cb = std::function<Callback>();//重置
        }
    private:
        std::function<Callback> m_cb;
    };

    template<typename O1, typename O2, typename R, typename...Args>
    FunctionWrapper<R(Args...)> make_function_wrapper(O1* ptr, R(O2::*NotifyMethod)(Args...))
    {
        static_assert(std::is_base_of<O2, O1>::value, "Can`t Convert");
        return FunctionWrapper<R(Args...)>([ptr, NotifyMethod](Args... args)
        {
            return ((*ptr).*NotifyMethod)(std::forward<Args>(args)...);
        });
    }

    template<typename O1, typename O2, typename R, typename...Args>
    FunctionWrapper<R(Args...)> make_function_wrapper(std::weak_ptr<O1> ptr, R(O2::* NotifyMethod)(Args...))
    {
        static_assert(std::is_base_of<O2, O1>::value, "Can`t Convert");
        return FunctionWrapper<R(Args...)>([ptr, NotifyMethod](Args... args)
        {
            auto spr = ptr.lock();
            if (spr)
            {
                return ((*spr).*NotifyMethod)(std::forward<Args>(args)...);
            }
            return R();
        });
    }
    template<typename O1, typename O2, typename R, typename...Args>
    FunctionWrapper<R(Args...)> make_function_wrapper(std::shared_ptr<O1> ptr, R(O2::* NotifyMethod)(Args...))
    {
        static_assert(std::is_base_of<O2, O1>::value, "Can`t Convert");
        return FunctionWrapper<R(Args...)>([ptr, NotifyMethod](Args... args)
        {
            auto spr = ptr;
            if (spr)
            {
                return ((*spr).*NotifyMethod)(std::forward<Args>(args)...);
            }
            return R();
        });
    }
    template<typename R, typename...Args>
    FunctionWrapper<R(Args...)> make_function_wrapper(std::function<R(Args...)> f)
    {
        return FunctionWrapper<R(Args...)>(f);
    }
    template<typename Callback>
    FunctionWrapper<Callback> make_function_wrapper(Callback* f)
    {
        return FunctionWrapper<Callback>(std::function<Callback>(f));
    }
}

template <typename...Args>
using EventWithMutex = grok::BasicEvent < Args... > ;

template <typename...Args>
using EventNoMutex = grok::BasicEventNoMutex < Args... > ;

template <typename Callback>
using ImportFunctional = grok::FunctionWrapper<Callback>;