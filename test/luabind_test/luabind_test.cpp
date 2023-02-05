#include <iostream>
#include <string.h>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

static int l_test(lua_State* L) {
    printf("hello test\r\n");
    return 0;
}

static int luaopen_test(lua_State* L) {
    luaL_Reg reg[] = {
		{ "test", l_test },
		{ NULL, NULL },
	};

	luaL_newlib(L, reg);
	return 1;
}
}

using namespace std;
const char* test_lua = \
R"(
    function main(a,b)
        print("this is main func")
        print(a)
        print(b)
    end
)";

int main(int argc, char**argv) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "test", luaopen_test, 0);

    if (luaL_loadbuffer(L, test_lua, strlen(test_lua), nullptr)) {
        cout << "error :" << lua_tostring(L, -1) << endl;
        goto finish;
    }

    if(lua_pcall(L, 0, 0, 0)) {
        cout << "error :" << lua_tostring(L, -1) << endl;
        goto finish;
    }

    lua_getglobal(L, "main");
    lua_pushstring(L, "hello");
    lua_pushstring(L, "world");
    if(lua_pcall(L, 2, 0, 0)) {
        cout << "error :" << lua_tostring(L, -1) << endl;
        goto finish;
    }
finish:
    lua_close(L);
    return 0;
}