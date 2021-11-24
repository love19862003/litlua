/********************************************************************

Filename:   litlua.h

Description:litlua.h

Version:  1.0
Created:  25:11:2018   17:27
Revison:  none
Compiler: gcc vc

Author:   wufan, love19862003@163.com

Organization:
*********************************************************************/
#pragma once
#ifndef __litlua_h__
#define __litlua_h__
#include <new>
#include <stdint.h>
#include <string.h>
#include <lua.hpp>
#include <stdio.h>
#include <typeinfo>
#include <memory>
#include <assert.h>
#include <tuple>
#include <functional>
#include "littraits.h"
#include "littype.h"
#include "litapi.h"

namespace LitSpace {

  //静态函数绑定
	template<typename F>
	void add_fun(lua_State* L, const char* name, F func){
		lua_pushlightuserdata(L, (void*)func);
		push_function(L, func);
		lua_setglobal(L, name);
	}

  //std::function bind
	template<typename F>
	void add_functor(lua_State* L, const char* name,  const F& func) {
		lua_pushlightuserdata(L, (void*)&func);
		push_functor(L, func);
		lua_setglobal(L, name);
	}

  //全局变量
	template<typename T>
	void set(lua_State* L, const char* name, T object){
		push(L, object);
		lua_setglobal(L, name);
	}
 //获取全局变量
	template<typename T>
	T get(lua_State* L, const char* name){
		lua_getglobal(L, name);
		return pop<T>(L);
	}


//多返回值调用,返回lua_returns<type1, type2, type3,...> 类型为std::tuple	
  template<typename lua_returns, typename ... ARGS>
  lua_returns rcall(lua_State* L, const char* name, ARGS ... args){
    static_assert(lua_returns::IS_RETURN);
    if (!lua_returns::HAS_TABLE){
      const int index = lua_gettop(L);
      guardfun g([index, L](){
       	    lua_settop(L, index);
      });
    }
    lua_pushcclosure(L, on_error, 0);
    int nresult = lua_returns::NSIZE;
    int errfunc = lua_gettop(L);
    lua_getglobal(L, name);
    if (lua_isfunction(L, -1)){
      push_args(L, args ...);
      lua_pcall(L, sizeof...(ARGS), nresult, errfunc);
    } else{
      print_error(L, "litlua call() attempt to call global `%s' (not a function)", name);
    }
    int readindex = 0 - nresult;
    return lua_returns::reader(L, readindex);
  }

 //单返回值调用
	template<typename R, typename ... ARGS>
	R call(lua_State* L, const char* name, ARGS ... args){
		lua_returns<R> r = rcall<lua_returns<R>>(L, name, args...);
		return r.get();
	}

 //类注册
	template<typename T>
	void class_reg(lua_State* L, const char* name){
		class_name<T>::name(name);
		lua_newtable(L);

		lua_pushstring(L, "__name");
		lua_pushstring(L, name);
		lua_rawset(L, -3);

		lua_pushstring(L, "__index");
		lua_pushcclosure(L, meta_get, 0);
		lua_rawset(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcclosure(L, meta_set, 0);
		lua_rawset(L, -3);

		lua_pushstring(L, "__gc");
		lua_pushcclosure(L, destroyer<T>, 0);
		lua_rawset(L, -3);

		lua_setglobal(L, name);
	}
//类继承关系
	template<typename T, typename P>
	void class_parent(lua_State* L){
		static_assert(std::has_virtual_destructor<T>::value );
		static_assert(std::has_virtual_destructor<P>::value );
		static_assert(std::is_convertible<typename std::add_pointer<T>::type, typename std::add_pointer<P>::type>::value);
		push_meta(L, class_name<T>::name());
		if (lua_istable(L, -1)){
			lua_pushstring(L, "__parent");
			push_meta(L, class_name<P>::name());
			lua_rawset(L, -3);
		}
		lua_pop(L, 1);
	}
 //类构造函数
	template<typename T, typename F>
	void class_new(lua_State* L, F func){
		push_meta(L, class_name<T>::name());
		if (lua_istable(L, -1)){
			lua_newtable(L);
			lua_pushstring(L, "__call");
			lua_pushcclosure(L, func, 0);
			lua_rawset(L, -3);
			lua_setmetatable(L, -2);
		}
		lua_pop(L, 1);
	}
//类成员函数
	template<typename T, typename F>
	void class_fun(lua_State* L, const char* name, F func){
		push_meta(L, class_name<T>::name());
		if (lua_istable(L, -1)){
			lua_pushstring(L, name);
			new(lua_newuserdata(L, sizeof(F))) F(func);
			push_mfunctor(L, func);
			lua_rawset(L, -3);
		}
		lua_pop(L, 1);
	}
 //类成员变量
	template<typename T, typename BASE, typename VAR>
	void class_mem(lua_State* L, const char* name, VAR BASE::*val){
		push_meta(L, class_name<T>::name());
		if (lua_istable(L, -1)){
			lua_pushstring(L, name);
			new(lua_newuserdata(L, sizeof(mem_var<BASE, VAR>))) mem_var<BASE, VAR>(val);
			lua_rawset(L, -3);
		}
		lua_pop(L, 1);
	}

}

#endif