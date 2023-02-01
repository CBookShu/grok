#include "luaModule.h"

LuaTable::Var *LuaTable::get_value(const Key &k)
{
    auto it = data.find(k);
    if (it == data.end()) {
        return nullptr;
    }
    return &it->second;
}

const LuaTable::Var *LuaTable::get_value(const Key &k) const
{
    auto it = data.find(k);
    if (it == data.end()) {
        return nullptr;
    }
    return &it->second;
}

LuaTable &LuaTable::set_value(const Key &k, Var &v)
{
    data[k] = v;
    return *this;
}

int LuaTable::l_get_value(lua_State *L, int kid) const
{
    Key k;
	if (!t_packkey(L, kid, k)) {
		lua_pushnil(L);
		return 1;
	}
	
	auto* v = get_value(k);
	if (!v) {
		lua_pushnil(L);
		return 1;
	}

	f_packval(L, v);

	return 1;
}

int LuaTable::l_get_value(lua_State *L) const
{
    if (data.empty()) {
		lua_pushnil(L);
		return 1;
	}
	lua_newtable(L);
	for (auto& it : data)
	{
		f_packkey(L, &it.first);
		f_packval(L, &it.second);
		lua_rawset(L, -3);
	}
	return 1;
}

int LuaTable::l_set_value(lua_State *L, int kid, int vid)
{
    int top = lua_gettop(L);

	Key k;
	if (!t_packkey(L, kid, k)) {
		lua_settop(L, top);
		lua_pushnil(L);
		return 1;
	}
	Var v;
	if (!t_packval(L, vid, v)) {
		lua_settop(L, top);
		lua_pushnil(L);
		return 1;
	}

	if (v.type() == typeid(LuaTable)) {
		auto* ptbl = boost::get<LuaTable>(&v);
		if (ptbl->data.empty()) {
			data.erase(k);
		}
		else {
			data[k] = std::move(v);
		}
	}
	else {
		data[k] = std::move(v);
	}
	lua_settop(L, top);
	lua_pushnil(L);
	return 1;
}

int LuaTable::l_set_value(lua_State *L, int vid)
{
    if (!lua_istable(L, vid)) {
		// c++ cache 一定不能是nil
		lua_pushboolean(L, 0);
		return 1;
	}
	if (lua_isnil(L, vid)) {
		data.clear();
		lua_pushboolean(L, 1);
		return 1;
	}

	Type t;
	lua_pushnil(L);
	while (lua_next(L, vid)) {
		int top = lua_gettop(L);
		Key _k;
		if (!t_packkey(L, top-1, _k)) {
			return false;
		}
		Var _v;
		if (!t_packval(L, top, _v)) {
			return false;
		}
		t[_k] = _v;
		lua_pop(L, 1);
	}
	data = std::move(t);
	lua_pushboolean(L, 1);
	return 1;
}

bool LuaTable::t_packkey(lua_State *L, int id, Key &k)
{
	if (lua_type(L, id) == LUA_TNUMBER) {
		k = lua_tonumber(L, id);
	}
	else if (lua_type(L, id) == LUA_TSTRING) {
		size_t sz = 0;
		const char *s = luaL_checklstring(L, id, &sz);
		k = std::string(s, sz);
	}
	else {
		return false;
	}
	return true;
}

bool LuaTable::t_packval(lua_State *L, int id, Var &v)
{
    int tt = lua_type(L, id);
	if (tt == LUA_TNUMBER) {
		v = lua_tonumber(L, id);
	}
	else if (tt == LUA_TSTRING) {
		size_t sz = 0;
		const char *s = luaL_checklstring(L, id, &sz);
		v = std::string(s, sz);
	}
	else if (tt == LUA_TTABLE) {
		LuaTable t;
		lua_pushnil(L);
		while (lua_next(L, id)) {
			Key _k;
			if (!t_packkey(L, -2, _k)) {
				return false;
			}
			Var _v;
			if (!t_packval(L, -1, _v)) {
				return false;
			}
			t.set_value(_k, _v);
			lua_pop(L, 2);
		}
		v = std::move(t);
	}
	else if (tt == LUA_TBOOLEAN) {
		v = lua_toboolean(L, id) == 1;
	}
	else {
		return false;
	}
	return true;
}

void LuaTable::f_packkey(lua_State *L, const Key *k)
{
    // 这里可以使用static_visitor
	if (k->type() == typeid(double)) {
		lua_pushnumber(L, *boost::get<double>(k));
	}
	else if (k->type() == typeid(std::string)) {
		auto* s = boost::get<std::string>(k);
		lua_pushlstring(L, s->c_str(), s->size());
	}
	else {
		// 逻辑正确这里不可能走到 Key的模板中没有该类型
		assert(false);
	}
}

void LuaTable::f_packval(lua_State *L, const Var *v)
{
    if (v->type() == typeid(double)) {
		lua_pushnumber(L, *boost::get<double>(v));
	}
	else if (v->type() == typeid(std::string)) {
		auto* s = boost::get<std::string>(v);
		lua_pushlstring(L, s->c_str(), s->size());
	}
	else if (v->type() == typeid(bool)) {
		bool b = *boost::get<bool>(v);
		lua_pushboolean(L, b);
	}
	else if (v->type() == typeid(LuaTable)) {
		lua_newtable(L);
		auto* t = boost::get<LuaTable>(v);
		for (auto& it:t->data)
		{
			f_packkey(L, &it.first);
			f_packval(L, &it.second);
			lua_rawset(L, -3);
		}
	}
	else {
		assert(false);
	}
}
