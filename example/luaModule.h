#pragma once
#include <memory>
#include <map>
#include <unordered_map>

#include <boost/any.hpp>
#include <boost/variant.hpp>

#include "grok/grok.h"
#include "utils.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// 简单映射lua table的类
// 用于保存一些可以在多个 lua_State 共同使用的缓存
struct LuaTable {
	using Key = boost::variant < double, std::string >;
	using Var = boost::variant < double, std::string, bool, LuaTable>;
	using Type = std::map < Key, Var >;

    Type data;

    Var* get_value(const Key& k);
    const Var* get_value(const Key& k) const;
    LuaTable& set_value(const Key& k, Var& v);

    int l_get_value(lua_State* L, int kid) const;
	int l_get_value(lua_State* L) const;

	int l_set_value(lua_State* L, int kid, int vid);
	int l_set_value(lua_State* L, int vid);

    // 从lua中获取值给c++
	static bool t_packkey(lua_State*L, int id, Key& k);
	static bool t_packval(lua_State*L, int id, Var& v);

	// 从c++得到值给lua
	static void f_packkey(lua_State*L, const Key* k);
	static void f_packval(lua_State*L, const Var* v);
};

struct LuaModel : public std::enable_shared_from_this<LuaModel> {
    using SPtr = std::shared_ptr<LuaModel>;

    // staff 控制 begin
    // 模块的队列
    grok::WorkStaff staff;
    // lua 实例
    lua_State* L = nullptr;
    // 模块的加载脚本
    std::string name;
    // 文件监听
    std::unordered_map<std::string,grok::stdtimerPtr> file_listeners;
    // 定时器
     std::unordered_map<double, grok::stdtimerPtr> timers;
    // staff 控制 end

    std::atomic_bool init{false};

    // 模块的缓存[由自己保证竞争安全]
    grok::grwtype<LuaTable> cache;
};

struct LuaStateDeleter {
    static void del(lua_State* t) {
        if (t) {
            lua_close(t);
        }
    }
    void operator()(lua_State *l) const {
        del(l);
    }
};

struct LuaModelManager {
    struct Data {
        std::unordered_map<std::string, LuaModel::SPtr> name2model;
    };
    ImportFunctional<void(grok::MsgPackSPtr)> imWriteMsgPack;
    ImportFunctional<std::uint32_t()> imMsgNextID; 
    ImportFunctional<std::string()> imNodeName;


    // 主线程池
    grok::EventPools::Ptr thread_pool;
    // models的 strand 主要用于执行usercmd
    grok::WorkStaff staff;
    // lua模块脚本
    grok::grwtype<Data> lua_models;
    // 消息分发lua脚本
    grok::LockList<lua_State, LuaStateDeleter> lua_scripts;
    // 多线程竞争的联合锁
    grok::UnionLockLocal<const char*> unionlock;
    // mysql pool
    grok::mysql::MysqlPool::SPtr mysqlpool;
    // redids pool
    grok::redis::RedisConPool::SPtr redispool;
    // 工作目录
    std::string work_dir;


    static LuaModelManager* get_instance();
    static void del_instance();
    
    void init(int argc, char** argv);
    void uninit();

    void start();
    void stop();

    void on_msg(grok::MsgPackSPtr p);

    // lua的回调函数用LUA_CALLBACK_FINDER来保存，简单易控
#define	LUA_CALLBACK_MAIN	    "__LUA_CALLBACK_MAIN__"
#define LUA_PROJECT_DIR         "scripts/"
#define LUA_MSG_MAIN            "main.lua"
    // lua model 操作函数
    void new_luamodel(const char* name, LuaModel::SPtr m);
    void replace_luamodel(const char* name, LuaModel::SPtr m_old, LuaModel::SPtr m_new);
    LuaModel::SPtr del_luamodel(const char* name);
    LuaModel::SPtr get_luamodel(const char* name);
    void model_stop(LuaModel::SPtr m);
    void model_stop_force(LuaModel::SPtr m);
    int invoke_lua_callback(lua_State* L, lua_Number cbid);
};


extern "C" {
// 绑定基础接口
LUAMOD_API int luaopen_core(lua_State* L);

// 绑定命令接口
LUAMOD_API int luaopen_cmdcore(lua_State* L);

// lua model 接口绑定
LUAMOD_API int luaopen_modelcore(lua_State* L);

// 绑定db接口
LUAMOD_API int luaopen_dbcore(lua_State* L);
}