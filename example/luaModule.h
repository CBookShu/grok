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

    // lua 实例
    lua_State* L = nullptr;
    // 模块的缓存
    grok::grwtype<LuaTable> cache;
    // 模块的名字
    std::string name;
    // 模块的队列
    grok::WorkStaff staff;
    // 文件监听
    std::unordered_map<std::string,grok::stdtimerPtr> file_listener;

    void stop();
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

struct LuaModelManager : public grok::WorkStaff {
    struct Data {
        std::unordered_map<std::string, LuaModel::SPtr> name2model;
    };

    // 主线程的消息循环
    boost::asio::io_service ios;
    // node的消息注册、处理线程池
    grok::MsgCenterSPtr msgcenter;
    // lua模块脚本
    grok::grwtype<Data> lua_models;
    // 消息分发lua脚本
    grok::LockList<lua_State, LuaStateDeleter> lua_scripts;
    // 多线程竞争的联合锁
    grok::UnionLockLocal<std::string> unionlock;
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


    void on_msg(grok::Session::Ptr s, grok::MsgPackSPtr p);

    void new_luamodel(const char* name, LuaModel::SPtr m);
    void replace_luamodel(const char* name, LuaModel::SPtr m_old, LuaModel::SPtr m_new);
    LuaModel::SPtr del_luamodel(const char* name);
    LuaModel::SPtr get_luamodel(const char* name);
};


extern "C" {
// 绑定基础接口
int luaopen_core(lua_State* L);

// 绑定命令接口
int luaopen_cmdcore(lua_State* L);
}