#pragma once
#include "grok/grok.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

struct LuaWithVersion {
    lua_State* L = nullptr;
    int version = 0;
    ~LuaWithVersion() {
        if (L) {
            lua_close(L);
        }
    }
};

struct LuaEntry : public grok::ThreadEntryBase {
    virtual void enterThread() {};
    virtual void leaveThread() {};

    LuaWithVersion lv;
};

class LuaThreadPool : public grok::EventPools {
public:
    using SPtr = std::shared_ptr<LuaThreadPool>;

    template <typename F>
    void post_msgop(F f) {
        auto self = shared_from_this();
        ios().dispatch([self,this,f](){
            auto* e = getThreadEntryByType<LuaEntry>();
            f(e);
        });
    }
protected:
    virtual std::shared_ptr<grok::ThreadEntryBase> 
        createThreadEntry();
};