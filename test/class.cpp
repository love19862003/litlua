#include <iostream>
#include <functional>
#include "litlua.h"

namespace LuaSpace = LitSpace;


void logDebug(std::string info, std::string info2, std::string info3, std::string info4) {
	std::cout << "logDebug:" << info << " | " << info2  << " | " << info3 << " | " << info4<< std::endl;
}

class CBase {
public:
	explicit CBase():_base() {
	}
	explicit CBase(std::string s) : _base(s){
	}
	virtual ~CBase() {}
	void print_base(){ 
		std::cout << "Base class print:" << _base << std::endl; 
	}
	
	void set_base(const char* v) { 
		_base = v; 
	}
protected:
	std::string _base = "Base";
};

enum EnumTest
{
	__TEST__ = 0,
	__RUN__ = 1,
};


class CA {
public:
	void print() { std::cout << "CA" << std::endl; }
};

class CTest	 : public CBase
{
public:
	explicit CTest() :CBase("22")/*, m_p(new CA)*/ {

	}
	explicit CTest(bool v):CBase("11"), _test_value(v)/*, m_p(new CA)*/{

	}

	virtual ~CTest(){
		
	}

	void print() { 
		std::cout << "test class _test_value:" << (int)_test_value << std::endl; 
	}

	void set(bool v) { _test_value = v; }

	void stable(int a, LuaSpace::table t, EnumTest en) {
		if (t.m_nil){
			std::cout << " nil table" << std::endl;
			return;
		}

		t.add(a);
		t.add("xxx");
		t.set("aa", 1);
		t.set("b", 2);
		t.set("en", en);

		_str = "call CTest stable && call table function p";
		t.call<void>("p", _str);

	}

	CTest* val() { return new  CTest("11111111111111111111111111111111111111111111111"); }


protected:
private:
	bool _test_value = true;
	std::string _str = "100";

};
void run_test() {
	
  lua_State* L = luaL_newstate();
  LuaSpace::openLuaLibs(L);
  LuaSpace::class_reg<CBase>(L, "CBase");
  LuaSpace::class_new<CBase>(L, &LuaSpace::constructor<CBase, std::string>);
  LuaSpace::class_fun<CBase>(L, "print_base", &CBase::print_base);
  LuaSpace::class_fun<CBase>(L, "set_base", &CBase::set_base);
  LuaSpace::class_reg<CTest>(L, "CTest");
  LuaSpace::class_new<CTest>(L, &LuaSpace::constructor<CTest>);
  LuaSpace::class_fun<CTest>(L, "print1", &CTest::print);
  LuaSpace::class_fun<CTest>(L, "set", &CTest::set);
  LuaSpace::class_fun<CTest>(L, "stable", &CTest::stable);
  LuaSpace::class_fun<CTest>(L, "val", &CTest::val);
  LuaSpace::class_parent<CTest, CBase>(L);
  LuaSpace::add_fun(L, "logDB", &logDebug);

  LuaSpace::set(L, "gtest", 1);
  LuaSpace::dofile(L, "class.lua");
  { 
	  CTest* c = new CTest();
	  auto p = LuaSpace::rcall<LuaSpace::lua_returns<int, int, int, int, int, int, LuaSpace::table>>(L, "test", c);
	  assert(11 == std::get<0>(p));
	  assert(1 == std::get<1>(p));
	  assert(2 == std::get<2>(p));
	  assert(3 == std::get<3>(p));
	  assert(4 == std::get<4>(p));
	  assert(5 == std::get<5>(p));
	  assert(!std::get<6>(p).m_nil);

	  LuaSpace::table t = std::get<LuaSpace::table>(p);
	  assert(!t.m_nil);
	  t.call<void>("p", "cpp call table function good");
	  LuaSpace::resetStack(L);
	  t.call<void>("p", "cpp call table function bad");  // table is nil
	  assert(t.m_nil);

	  auto p2 = LuaSpace::call<int>(L, "test", c);
	  std::cout << "cpp out: call lua function with result int && return value is " << p2 << std::endl;
	  LuaSpace::call<void>(L, "test", c);
	  std::cout << "cpp out call lua test value is void" << std::endl;
	  delete c;
	  c = nullptr;
  }
  lua_close(L);
  L = nullptr;
  
}
