#pragma once
#include <memory>
#include <map>
#include <unordered_map>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <lauxlib.h>
#include <boost/any.hpp>
#include <boost/variant.hpp>

#include "grok/grok.h"
#include "utils.h"

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
};

class LuaModelManager {
public:

private:
    boost::asio::io_service m_iosvr;
};