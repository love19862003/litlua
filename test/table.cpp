#include <iostream>
#include <functional>
#include "litlua.h"

namespace LuaSpace = LitSpace;
typedef LuaSpace::table table;


void run_test() {
	lua_State* L = luaL_newstate();
	LuaSpace::openLuaLibs(L);
	LuaSpace::dofile(L, "table.lua");

	table t = LuaSpace::call<table>(L, "gettable");
	std::cout << "a = " << t.get<int>("a") << std::endl;
	std::cout << "b = " << t.get<std::string>("b") << std::endl;
	std::cout << "c = " << t.get<int>("c") << std::endl;
	
	t.call<void>( "f", "call table function");
	table child = t.child();
	child.set("add child1 table", 100);
	child.add(100);

	table child2 = t.child("child table");
	child2.set("add child2 table", 100);
	child2.add(100);
	LuaSpace::call<void>(L, "print_table", t, "");
	lua_close(L);
	//L = nullptr;
}