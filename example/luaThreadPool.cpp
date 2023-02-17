#include "luaThreadPool.h"

std::shared_ptr<grok::ThreadEntryBase> LuaThreadPool::createThreadEntry()
{
    return std::make_shared<LuaEntry>();
}
