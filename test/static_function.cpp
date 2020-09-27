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

class CTest
{
public:
  CTest(){}

  int add(){ return m_i++;}
  int value() const {return m_i;}
protected:
private:
  int m_i = 0;
};


void run_error_bind();

void run_test() {
	
  lua_State* L = luaL_newstate();
  LuaSpace::openLuaLibs(L);
  LuaSpace::add_fun(L, "printStr", &printStr);
  LuaSpace::add_fun(L, "add", &addFun);
  LuaSpace::add_fun(L, "test", &test);
  
  std::function<int (int, int)> fun = [](int a, int b)-> int {return a + b;};
  LuaSpace::add_fun2(L, "func", fun);
  
  std::function<int(int, int)> fun2 = [](int a, int b)-> int{return a + b; };
  LuaSpace::add_fun2(L, "func2", fun2);
  
  CTest* t = new CTest();
  std::function<int()> fun3 = std::bind(&CTest::add, t);
  LuaSpace::add_fun2(L, "func3", fun3);

  LuaSpace::dofile(L, "static_function.lua");
  lua_close(L);
  //lua call add() twice
  assert( 2 == t->value());
  L = nullptr;
  delete t;
  t =nullptr;

  run_error_bind();
}

void run_error_bind(){

  std::cout << "\n ............................................." << std::endl;
  std::cout << "\n ...run test2 with invailed std::function....." << std::endl;
  std::cout << "\n ............................................." << std::endl;

  lua_State* L = luaL_newstate();
  LuaSpace::openLuaLibs(L);
  LuaSpace::add_fun(L, "printStr", &printStr);
  LuaSpace::add_fun(L, "add", &addFun);
  LuaSpace::add_fun(L, "test", &test);

  //this function will call failed
  //{
    std::function<int(int, int)> fun = [](int a, int b)-> int{return a + b; };
    LuaSpace::add_fun2(L, "func", fun);

    std::function<int(int, int)> fun2 = [](int a, int b)-> int{return a + b; };
    LuaSpace::add_fun2(L, "func2", fun2);

    CTest* t = new CTest();
    std::function<int()> fun3 = std::bind(&CTest::add, t);
    LuaSpace::add_fun2(L, "func3", fun3);

    //delete t;
   // t = nullptr;
  //}

  

  LuaSpace::dofile(L, "static_function.lua");
  lua_close(L);
  L = nullptr;
  delete t;
  t = nullptr;
}
