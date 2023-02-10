#include "luaModule.h"
#include <boost/filesystem.hpp>
#include "utils.h"
#include "pbc-lua.h"

static lua_State* create_lua_scripts(const char* path);
static lua_State* create_lua_model(const char* path);
static int create_lua_cmd(const char* path);

static int l_core_curdir(lua_State *L) {
	auto mgr = LuaModelManager::get_instance();
	lua_pushstring(L, mgr->work_dir.c_str());
	return 1;
}

static int l_core_set_cache(lua_State*L) {
	auto mgr = LuaModelManager::get_instance();
	auto top = lua_gettop(L);
	const char* name = luaL_checkstring(L, 1);
	auto m = mgr->get_luamodel(name);
	if(!m) {
		lua_pushboolean(L, 0);
		return 1;
	}
	{
		auto w = m->cache.writeGuard();
		if(top == 2) {
			// set value
			w->l_set_value(L, 2);
		} else if(top == 3) {
			// set key value
			w->l_set_value(L, 2, 3);
		}
	}
	lua_pushboolean(L, 1);
	return 1;
}

static int l_core_get_cache(lua_State*L) {
	auto mgr = LuaModelManager::get_instance();
	auto top = lua_gettop(L);
	const char* name = luaL_checkstring(L, 1);
	auto m = mgr->get_luamodel(name);
	if(!m) {
		lua_pushnil(L);
		return 1;
	}
	auto r = m->cache.readGuard();
	if(top == 1) {
		// get value
		return r->l_get_value(L);
	} else if(top == 2) {
		// get key value
		return r->l_get_value(L, 2);
	}
}

static int l_core_log(lua_State*L) {
	int level = luaL_checknumber(L, 1);
	const char* log = luaL_checkstring(L, 2);

	static const char* level_tags[] = {
		"TRACE","DEBUG","INFO","WARN","ERROR"
	};
	if (level < 0 || level > (sizeof(level_tags)/sizeof(const char*))) {
		DBG("[LEVEL:%d]%s", level, log);
		return 0;
	}
	DBG("[%s]%s", level_tags[level], log);
	return 0;
}

int luaopen_core(lua_State *L)
{
	luaL_Reg reg[] = {
		{ "core_curdir", l_core_curdir },
		{ "core_log", l_core_log },
		{ "core_set_cache", l_core_set_cache},
		{ "core_get_cache", l_core_get_cache},
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);
	return 1;
}

static int l_cmdcore_start(lua_State* L) {
	// name, path
	const char* name = luaL_checkstring(L, 1);
	std::string path = luaL_checkstring(L, 2);

	auto* mgr = LuaModelManager::get_instance();

	// 先检查一下name是否已经注册过lua模块了
	auto m = mgr->get_luamodel(name);
	if(m) {
		// 不能重复注册
		DBG("Repeat Register Lua Model: %s", name);
		lua_pushboolean(L, 0);
		return 1;
	}

	auto full_path = mgr->work_dir + path;
	auto* model_lua = create_lua_model(full_path.c_str());
	if(!model_lua) {
		lua_pushboolean(L, 0);
		return 1;
	}

	m = std::make_shared<LuaModel>();
	m->L = model_lua;
	m->name = name;

	// 先将创建好的模块注册起来，因为马上要执行了
	// 该模块是新的，这里的pcall一定不会并发
	mgr->new_luamodel(name, m);

	
	// 开始执行模块的真正代码，model的lua state只能在staff的strand中执行
	// 避免任何竞争
	m->staff.async<void>([m,path](){
		if(!m->L) {
			return ;
		}
		
		lua_pushstring(m->L, m->name.c_str());
		if(lua_pcall(m->L, 1, 0, 0)) {
			// 模块代码执行失败
			DBG("Model Call Error:%s,%s", path.c_str(), lua_tostring(m->L, -1));
			// 模块一旦注册，但是调用失败，模块自己不能把自己释放，这里会产生竞争
			// 应该由执行者关注结果，失败要主动调用cmd_stop来把模块关掉
		}
	});
	DBG("LuaModel start:%s", name);
	lua_pushboolean(L, 1);
	return 1;
}

static int l_cmdcore_stop(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	auto* mgr = LuaModelManager::get_instance();

	auto m = mgr->del_luamodel(name);
	if(!m) {
		lua_pushboolean(L, 1);
		return 1;
	}

	mgr->model_stop(m);
	lua_pushboolean(L, 1);
	return 1;
}

static int l_cmdcore_restart(lua_State* L) {
	// name, path
	const char* name = luaL_checkstring(L, 1);
	std::string path = luaL_checkstring(L, 2);

	auto* mgr = LuaModelManager::get_instance();
	auto m = mgr->get_luamodel(name);
	if (!m) {
		// 不能重复注册
		DBG("No Old Lua Model: %s", name);
		lua_pushboolean(L, 0);
		return 1;
	}

	auto full_path = mgr->work_dir + path;
	auto* model_lua = create_lua_model(path.c_str());
	if (!model_lua) {
		lua_pushboolean(L, 0);
		return 1;
	}

	auto new_m = std::make_shared<LuaModel>();
	m->L = model_lua;
	m->name = name;

	// 先将创建好的模块注册起来，因为马上要执行了
	mgr->replace_luamodel(name, m, new_m);

	// 开始执行模块的真正代码，model的lua state只能在staff的strand中执行
	// 避免任何竞争
	m->staff.async<void>([m,path](){
		if(!m->L) {
			return ;
		}
		
		lua_pushstring(m->L, m->name.c_str());
		if(lua_pcall(m->L, 1, 0, 0)) {
			// 模块代码执行失败
			DBG("Model Call Error:%s,%s", path.c_str(), lua_tostring(m->L, -1));
			// 模块一旦注册，但是调用失败，模块自己不能把自己释放，这里会产生竞争
			// 应该由执行者关注结果，失败要主动调用cmd_stop来把模块关掉
		}
	});

	DBG("LuaModel start:%s", name);
	lua_pushboolean(L, 1);
	return 1;
}

static int l_cmdcore_list(lua_State* L) {
	lua_newtable(L);

	auto* mgr = LuaModelManager::get_instance();
	auto r = mgr->lua_models.readGuard();
	int idx = 1;
	for(auto& it:r->name2model) {
		lua_pushnumber(L, idx++);
		lua_pushstring(L, it.first.c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

// cmd 绑定接口
int luaopen_cmdcore(lua_State *L)
{
	luaL_Reg reg[] = {
		{ "cmdcore_start", l_cmdcore_start },
		{ "cmdcore_stop", l_cmdcore_stop },
		{ "cmdcore_restart", l_cmdcore_restart },
		{ "cmdcore_list", l_cmdcore_list },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);
	return 1;
}


static int l_modelcore_file_listen(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	const char* filepath = luaL_checkstring(L, 2);
	auto callbackid = luaL_checknumber(L, 3);
	auto millsec = luaL_checknumber(L, 4);

	auto* mgr = LuaModelManager::get_instance();
	auto m = mgr->get_luamodel(name);
	if(!m) {
		DBG("ModelName Error:%s", name);
		lua_pushboolean(m->L, 0);
		return 1;
	}

	if(L != m->L) {
		DBG("不能跨模块操作定时器")
		lua_pushboolean(m->L, 0);
		return 1;
	}

	auto it = m->file_listeners.find(filepath);
	if(it != m->file_listeners.end()) {
		DBG("文件重复监听:%s", filepath);
		lua_pushboolean(m->L, 0);
		return 1;
	}

	auto timer = listen_file_modify(m->staff, filepath, [m,callbackid](const char* path){
		if(LuaModelManager::get_instance()->invoke_lua_callback(m->L, callbackid)) {
			DBG("Callback Error:%lf,%s,%s", callbackid, lua_tostring(m->L, -1), path);
		}
	},millsec);
	m->file_listeners[filepath] = timer;

	lua_pushboolean(m->L, 1);
	return 1;
}

static int l_modelcore_file_stoplisten(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	const char* filepath = luaL_checkstring(L, 2);

	auto* mgr = LuaModelManager::get_instance();
	auto m = mgr->get_luamodel(name);
	if(!m) {
		DBG("ModelName Error:%s", name);
		lua_pushboolean(m->L, 0);
		return 1;
	}

	if(L != m->L) {
		DBG("不能跨模块操作定时器")
		lua_pushboolean(m->L, 0);
		return 1;
	}

	m->file_listeners.erase(filepath);
	lua_pushboolean(L, 1);
	return 1;
}

static int l_modelcore_timer_start(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	auto callbackid = luaL_checknumber(L, 2);
	size_t millisec = luaL_checknumber(L, 3);

	auto* mgr = LuaModelManager::get_instance();
	auto m = mgr->get_luamodel(name);
	if(!m) {
		DBG("ModelName Error:%s", name);
		lua_pushboolean(m->L, 0);
		return 1;
	}

	if(L != m->L) {
		DBG("不能跨模块操作定时器")
		lua_pushboolean(m->L, 0);
		return 1;
	}

	auto timer = m->staff.evp().loopTimer([m,callbackid](){
		if(LuaModelManager::get_instance()->invoke_lua_callback(m->L, callbackid)) {
			DBG("Callback Error:%lf,%s", callbackid, lua_tostring(m->L, -1));
		}
	}, std::chrono::milliseconds(millisec), m->staff.strand());
	m->timers[callbackid] = timer;

	lua_pushboolean(L, 1);
	return 1;
}

static int l_modelcore_timer_stop(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	auto callbackid = luaL_checknumber(L, 2);

	auto* mgr = LuaModelManager::get_instance();
	auto m = mgr->get_luamodel(name);
	if(!m) {
		DBG("ModelName Error:%s", name);
		lua_pushboolean(m->L, 0);
		return 1;
	}

	if(L != m->L) {
		DBG("不能跨模块操作定时器")
		lua_pushboolean(m->L, 0);
		return 1;
	}

	m->timers.erase(callbackid);

	lua_pushboolean(L, 1);
	return 1;
}

int luaopen_modelcore(lua_State *L)
{
    luaL_Reg reg[] = {
		{ "modelcore_file_listen", l_modelcore_file_listen },
		{ "modelcore_file_stoplisten", l_modelcore_file_stoplisten },
		{ "modelcore_timer_start", l_modelcore_timer_start },
		{ "modelcore_timer_stop", l_modelcore_timer_stop },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);
	return 1;
}

static int l_dbcore_mysql_get(lua_State* L) {
	luaL_argcheck(L, lua_isfunction(L, 1), 1, "function need");
	auto* mgr = LuaModelManager::get_instance();
	assert(mgr);
	auto guard = mgr->mysqlpool->GetByGuard();
	grok::mysql::MysqlPool::Guard* g = &guard;
	lua_pushlightuserdata(L, g);
	int n = lua_gettop(L);
	int r = lua_pcall(L, 1, 1, 0);
	if(r) {
		DBG("call error:%s", lua_tostring(L, -1));
		lua_pushnumber(L, -1);
		return 1;
	}
	return 1;
}

static int l_dbcore_mysql_query(lua_State*L) {
	// p1: grok::mysql::MysqlPool::Guard*
	/*	p2: table {
			query = mysql_string,
			calback? = 回调查询结果,
			argv? = query中?绑定的参数
		}
	*/ 
	luaL_argcheck(L, lua_islightuserdata(L, 1), 1, "mysql guard need");
	luaL_argcheck(L, lua_istable(L, 2), 2, "param table need");
	auto* g = (grok::mysql::MysqlPool::Guard*)lua_touserdata(L, 1);
	assert(g);

	lua_getfield(L, 2, "query"); // L3 query 
	size_t sz = 0;
	const char* query = luaL_checklstring(L, -1, &sz);

	lua_getfield(L, 2, "argv"); // L4 argv
	if(lua_istable(L, -1)) {
		auto query_fmt = grok::mysql::SqlTextMaker::Create(query);
		lua_Integer len = luaL_len(L, 4);
		luaL_argcheck(L, query_fmt.ParamsCount() == len, 4, "argv param count error");

		for (lua_Integer i = 1; i <= len; ++i) {
			lua_geti(L, 4, i);	// L5 argi
			auto tt = lua_type(L, 5);
			// 暂时仅考虑两种情况即 number,string，不允许嵌套
			if (tt == LUA_TNUMBER) {
				query_fmt.BindParam(g->Get()->GetCtx(), i-1, luaL_checknumber(L, 6));
			} else if (tt == LUA_TSTRING) {
				size_t n = 0;
				const char* arg_str = luaL_checklstring(L, 6, &n);
				query_fmt.BindParam(g->Get()->GetCtx(), i-1, arg_str, n);
			} else {
				luaL_error(L, "mysql query error type:%d", tt);
			}
			lua_pop(L, 1); // pop L5
		}

		lua_getfield(L, 2, "callback");// L5 callback

		auto sql = query_fmt.GetSqlText();
		if (!lua_isfunction(L, 5)) {
			// 不需要回调
			auto res = g->Get()->Query(sql.c_str(), sql.size());
			lua_pushnumber(L, res);
			return 1;
		}
		// 需要回调
		auto res = g->Get()->QueryResult(sql.c_str(), sql.size());
		grok::mysql::Records* record = &res;
		lua_pushlightuserdata(L, record); // L6 records
		int r = lua_pcall(L, 1, 1, 0);
		if (r) {
			DBG("call error:%s", lua_tostring(L, -1));
			return 1;
		}
		return 1;
	}

	lua_getfield(L, 2, "callback");// L5 callback
	if (!lua_isfunction(L, 5)) {
		// 不需要回调
		auto res = g->Get()->Query(query, sz);
		lua_pushnumber(L, res);
		return 1;
	}
	// 需要回调
	auto res = g->Get()->QueryResult(query, sz);
	grok::mysql::Records* record = &res;
	lua_pushlightuserdata(L, record); // L6 records
	int r = lua_pcall(L, 1, 1, 0);
	if (r) {
		DBG("call error:%s", lua_tostring(L, -1));
		return 1;
	}
	return 1;
}

LUAMOD_API int luaopen_dbcore(lua_State *L)
{
	    luaL_Reg reg[] = {
		{ "dbcore_mysql_get", l_dbcore_mysql_get },
		{ "dbcore_mysql_query", l_dbcore_mysql_query },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);
	return 1;
}

static lua_State* create_lua_scripts(const char* path) {
	auto*L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "protobuf.c", luaopen_protobuf_c, 0);
	luaL_requiref(L, "core", luaopen_core, 0);
	return L;
}

static lua_State* create_lua_model(const char* path) {
	auto*L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "protobuf.c", luaopen_protobuf_c, 0);
	luaL_requiref(L, "core", luaopen_core, 0);
	luaL_requiref(L, "modelcore", luaopen_modelcore, 0);

	auto r = luaL_loadfile(L, path);
	if (r) {
		DBG("Load Error:%s,%s", path, lua_tostring(L, -1));
		lua_close(L);
		return nullptr;
	}
	return L;
}

static int create_lua_cmd(const char* path) {
	auto* L = luaL_newstate();
	std::unique_ptr<lua_State, LuaStateDeleter> guard(L);
	
	luaL_openlibs(L);
	luaL_requiref(L, "protobuf.c", luaopen_protobuf_c, 0);
	luaL_requiref(L, "core", luaopen_core, 0);
	luaL_requiref(L, "cmdcore", luaopen_cmdcore, 0);

	if(luaL_loadfile(L, path)) {
		DBG("load file error:%s,%s", path, lua_tostring(L, -1));
		return 0;
	}

	if(lua_pcall(L,0,0,0)) {
		DBG("call file error:%s,%s", path, lua_tostring(L, -1));
		return 0;
	}
	return 1;
}

using namespace grok;

struct LuaWithVersion {
    lua_State* L = nullptr;
    int version = 0;
    ~LuaWithVersion() {
        if (L) {
            lua_close(L);
        }
    }
};

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
			lua_pop(L, 1);
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

static LuaModelManager* g_instance = nullptr;
LuaModelManager *LuaModelManager::get_instance()
{
	if(!g_instance) {
		g_instance = new LuaModelManager();
	}
	return g_instance;
}

void LuaModelManager::del_instance()
{
	if(g_instance) {
		delete g_instance;
	}
}

void LuaModelManager::init(int argc, char** argv)
{
	if (argc == 1) {
		// 就把执行文件的目录当成工作目录
		work_dir = boost::filesystem::path(argv[0]).parent_path().generic_string();
	} else if(argc >= 2) {
		// argv[1] 就是传入的工作目录
		work_dir = boost::filesystem::path(argv[1]).generic_string();
	}
	if(!boost::filesystem::is_directory(work_dir)) {
		DBG("err work dir:%s", work_dir.c_str());
		exit(0);
	}
	if(work_dir.back() != '/') {
		work_dir.append("/");
	}

	boost::filesystem::current_path(work_dir);

	// 脚本目录在 work_dir下的scripts中
	std::string start_script = work_dir + LUA_PROJECT_DIR + "start.lua";

	int dbcon = 4;
	mysql::MysqlConfig sqlconfig;
	sqlconfig.db = "test";
	sqlconfig.host = "localhost";
	sqlconfig.port = 3306;
	sqlconfig.user = "cbookshu";
	sqlconfig.pwd = "cs123456";
	mysqlpool = mysql::MysqlPool::Create(dbcon, sqlconfig);
	// if(!mysqlpool->GetByGuard()->GetCtx()) {
	// 	DBG("mysql con error");
	// 	exit(1);
	// }

	redis::RedisConfig rdsconfig;
	rdsconfig.url = "localhost";
	rdsconfig.port = 6379;
	redispool = redis::RedisConPool::Create(dbcon, rdsconfig);
	// if(!redispool->GetByGuard()->GetCtx()) {
	// 	DBG("redis con error");
	// 	exit(1);
	// }

	// 先创建消息分发
	msgcenter = MsgCenter::Create();

	// 启动脚本，创建lua模块
	if(!create_lua_cmd(start_script.c_str())) {
		DBG("start load error");
		exit(1);
	}

	// TODO: 确认脚本path
	const char* path = "";
	for (int i = 0; i < dbcon; ++i) {
		auto* L = create_lua_scripts(path);
		lua_scripts.Give(L);
	}

	msgcenter->start(dbcon);
}

void LuaModelManager::uninit()
{
	// 停止接收消息处理
	if(msgcenter) {
		msgcenter->stop();
	}

	// 把模块删除
	std::vector<LuaModel::SPtr> models;
	{
		auto r = lua_models.readGuard();
		for(auto& it:r->name2model) {
			models.push_back(it.second);
		}
	}
	for(auto& it:models) {
		del_luamodel(it->name.c_str());
		model_stop(it);
	}

	// 停止主循环
	ios.stop();

	// 
	mysqlpool.reset();
	redispool.reset();
}

void LuaModelManager::start()
{
	boost::asio::io_service::work w(ios);
	ios.run();
}

void LuaModelManager::stop()
{
	ios.stop();
}

struct LuaMsgScriptRun : public GrokRunable {
	std::function<void()> cb;
	virtual void run() override {
		cb();
	}
};

void LuaModelManager::on_msg(Session::Ptr s, MsgPackSPtr p)
{
	auto run = new LuaMsgScriptRun();
	run->cb = [s,p](){
		auto g = LuaModelManager::get_instance()->lua_scripts.GetByGuard();
		// TODO: 指定接口进行调用
		const char* main_func = "main";
		auto* L = g.Get();
		lua_settop(L, 0);
		lua_getglobal(L, main_func);
		luaL_checktype(L, 1, LUA_TFUNCTION);
		lua_pushlightuserdata(L, s.get());
		lua_pushlightuserdata(L, p.get());
		if(lua_pcall(L, 2, 0, 0)) {
			DBG("MSG OP ERROR:%s", lua_tostring(L, -1));
			return;
		}
	};
	msgcenter->post(run);
}

void LuaModelManager::new_luamodel(const char *name, LuaModel::SPtr m)
{
	auto w = lua_models.writeGuard();
	w->name2model[name] = m;
}

void LuaModelManager::replace_luamodel(const char *name, LuaModel::SPtr m_old, LuaModel::SPtr m_new)
{
	// 重点在这里，卸载老的模块，重新添加新的模块
	// 并且把老的模块的缓存给新模块，这样从锁出来之后，就可以使用这些缓存了
	// 这里很危险的使用了多个锁，要尤为注意
	auto w = lua_models.writeGuard();
	w->name2model[name] = m_new;

	// 这里要先锁老的缓存，因为它有可能在外部被使用，newm一定是不会被使用的，它还没有注册成功
	// TODO: 是否可以考虑通过复制的方式，这样所有获取缓存的时候，都是有值的
	auto wcacheold = m_old->cache.writeGuard();
	auto wcachenew = m_new->cache.writeGuard();
	wcachenew->data.swap(wcacheold->data);
}

LuaModel::SPtr LuaModelManager::del_luamodel(const char *name)
{
	LuaModel::SPtr res;
	if(res) {
		auto w = lua_models.writeGuard();
		auto it = w->name2model.find(name);
		if(it != w->name2model.end()) {
			res = it->second;
		}
		w->name2model.erase(it);
	}
	return res;
}

LuaModel::SPtr LuaModelManager::get_luamodel(const char *name)
{
    auto r = lua_models.readGuard();
	auto it = r->name2model.find(name);
	if (it == r->name2model.end()) {
		return nullptr;
	}
	return it->second;
}

void LuaModelManager::model_stop(LuaModel::SPtr m)
{
	// 保护一下自己，在lua 被删除之前自己不能被摧毁
	// lua 的操作要线程安全
	auto self = m->shared_from_this();
	m->staff.async<void>([self](){
		if (self->L) {
			lua_close(self->L);
			self->L = nullptr;
		}
	});
}

int LuaModelManager::invoke_lua_callback(lua_State *L, lua_Number cbid)
{
	if(!L) {
		return 0;
	}
	lua_getglobal(L, LUA_CALLBACK_MAIN);
	lua_pushnumber(L, cbid);
	return lua_pcall(L, 1, 0, 0);
}
