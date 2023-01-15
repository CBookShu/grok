#include "eventPool.h"

using namespace grok;

class ThreadEntryDefault : public ThreadEntryBase
{
public:
    typedef std::shared_ptr<ThreadEntryBase> Ptr;
    virtual void enterThread() override
    {

    }
    virtual void leaveThread() override
    {

    }
};

EventPools::Ptr EventPools::instance = std::make_shared<EventPools>();
boost::thread_specific_ptr<ThreadEntryBase> EventPools::tls_entry([](ThreadEntryBase*) {});

void EventPools::Init()
{
    auto num = std::thread::hardware_concurrency() * 2 + 1;
    instance->start(num);
}

void EventPools::Uinit()
{
    instance->stop();
}

grok::EventPools::EventPools()
{
    m_bRunning = false;
}

EventPools::~EventPools()
{
    stop();
}

void EventPools::start(int n /*= 8*/)
{
    if (m_bRunning)
    {
        return;
    }

    m_guards.clear();
    m_ios.reset();

    n = std::max<int>(1, n);
    n = std::min<int>(n, 32);

    auto self = shared_from_this();

    std::promise<void> p;
    m_ios.dispatch([&p]()
    {
        p.set_value();
    });

    for (int i = 0; i < n; ++i)
    {
        auto thread_entry = createThreadEntry();
        thread_entry->t = std::thread([this, thread_entry]()
        {
            tls_entry.reset(thread_entry.get());
            thread_entry->enterThread();
            thread_entry->work = std::make_shared<boost::asio::io_service::work>(m_ios);
            m_ios.run();
            thread_entry->leaveThread();
            tls_entry.release();
        });
        m_guards.push_back(thread_entry);
    }
    p.get_future().get();
    m_bRunning = true;
}

std::shared_ptr<ThreadEntryBase> EventPools::createThreadEntry()
{
    auto entry = std::make_shared<ThreadEntryDefault>();
    return entry;
}

ThreadEntryBase* EventPools::getThreadEntry()
{
    return tls_entry.get();
}

void EventPools::stop()
{
    if (!m_bRunning)
    {
        return;
    }

    for (auto it : m_guards)
    {
        it->work.reset();
    }

    m_ios.stop();
    for (auto it : m_guards)
    {
        it->t.join();
    }

    m_guards.clear();
    m_ios.reset();
    m_bRunning = false;
}

EventPools::Ptr EventPools::defaultEventPools()
{
    return instance;
}