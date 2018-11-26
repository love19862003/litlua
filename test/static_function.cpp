#include <iostream>
#include <functional>
#include "litlua.h"

namespace LuaSpace = LitSpace;


void printStr(std::string info, std::string info2, std::string info3, std::string info4) {
	std::cout << "logDebug:" << info << "  " << info2  << "  " << info3 << "  " << info4<< std::endl;
}

int addFun(int a) {
	return ++a;
}

int test(int a, int b) {
	assert(b == a + 1);
	return b;
}

void run_test() {
	
  lua_State* L = luaL_newstate();
  LuaSpace::openLuaLibs(L);
  LuaSpace::add_fun(L, "printStr", &printStr);
  LuaSpace::add_fun(L, "add", &addFun);
  LuaSpace::add_fun(L, "test", &test);
  std::function<int (int, int)> fun = [](int a, int b)-> int {return a + b;};
  LuaSpace::add_fun2(L, "func", fun);
  LuaSpace::dofile(L, "static_function.lua");
  lua_close(L);
  L = nullptr;
}
