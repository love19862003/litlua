#include <iostream>
#include <functional>
#include <assert.h>
#include "litlua.h"

namespace LuaSpace = LitSpace; 
using namespace LitSpace;
typedef LuaSpace::table table;
typedef LuaSpace::nil nil;

void run_test() {
	lua_State* L = luaL_newstate();
	LuaSpace::openLuaLibs(L);
	LuaSpace::dofile(L, "lua_return.lua");
	{
		call<void>(L, "fun1");
		int p = call<int>(L, "fun1");
		assert(p == 1);
		typedef LuaSpace::lua_returns<int, int, std::string, float, nil, table> muti_return;
		const muti_return p1 = rcall<muti_return>(L, "fun1");
		assert(!p1._err);
		assert(std::get<0>(p1) == 1);
		assert(std::get<1>(p1) == 2);
		assert(std::get<2>(p1) == "string");
		assert(std::get<3>(p1) == 3.1f);
		nil n = std::get<4>(p1);
		assert(n.m_nil);
		table t = std::get<5>(p1);
		assert(!t.m_nil);
		t.call<void>("func");

		assert(1 == t.call<int>("func"));
		auto p2 = t.rcall<LuaSpace::lua_returns<unsigned int, unsigned int, unsigned int, unsigned int>>("func");
    assert(!p2._err);
    assert(1 == std::get<0>(p2));
		assert(2 == std::get<1>(p2));
		assert(3 == std::get<2>(p2));
		assert(4 == std::get<3>(p2));

		auto p3 = rcall<LuaSpace::lua_returns<int, int, int, float, nil, table>>(L, "fun1");
		assert(p3._err);
		assert(std::get<0>(p3) == 1);
		assert(std::get<1>(p3) == 2);
		assert(std::get<2>(p3) == 0);
		assert(std::get<3>(p3) == 3.1f);
		nil n2 = std::get<4>(p3);
		assert(n2.m_nil);
		table t2 = std::get<5>(p3);
		assert(!t2.m_nil);

		call<void>(L, "fun2");
		call<void>(L, "fun3");
		t.reset();
		t2.reset();
		resetStack(L);
	}
	lua_close(L);
	L = nullptr;
}