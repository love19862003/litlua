#include <iostream>
#include <functional>
#include "litlua.h"

namespace LuaSpace = LitSpace;


void printStr(std::string info, std::string info2, std::string info3, std::string info4) {
	std::cout << "logDebug:" << info << " | " << info2  << " | " << info3 << " | " << info4<< std::endl;
}

void run_test() {
	
  lua_State* L = luaL_newstate();
  LuaSpace::openLuaLibs(L);
  LuaSpace::add_fun(L, "printStr", printStr);
  LuaSpace::dofile(L, "printStr.lua");
  lua_close(L);
  L = nullptr;
}
