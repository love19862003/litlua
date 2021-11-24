/********************************************************************

Filename:   litapi.h

Description:litapi.h

Version:  1.0
Created:  25:11:2018   17:27
Revison:  none
Compiler: gcc vc

Author:   wufan, love19862003@163.com

Organization:
*********************************************************************/
#pragma once
#ifndef __lit_api_h__
#define __lit_api_h__
#include <new>
#include <stdint.h>
#include <string.h>
#include <lua.hpp>
#include <stdio.h>
#include <assert.h>
#include "littraits.h"
#include "litlua.h"
namespace LitSpace {



	//lua 版本信息
	inline std::string version() {
		return LUA_RELEASE;
	}

	//print错误信息 
	inline void  print_error(lua_State *L, const char* fmt, ...) {
		char text[4096];
		va_list args;
		va_start(args, fmt);
		vsnprintf(text, sizeof(text), fmt, args);
		va_end(args);

		lua_getglobal(L, "_ALERT");
		if (lua_isfunction(L, -1)) {
			lua_pushstring(L, text);
			lua_call(L, 1, 0);
		}
		else {
			printf("%s\n", text);
			lua_pop(L, 1);
		}
	}

  //
  inline void  info(lua_State *L, const char* fmt, ...){
    char text[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    printf("%s\n", text);

  }

	//打印栈内对象信息
	inline void  enum_stack(lua_State *L) {
		int top = lua_gettop(L);
		print_error(L, "%s", "----------stack----------");
		print_error(L, "Type:%d", top);
		for (int i = 1; i <= lua_gettop(L); ++i){
			switch (lua_type(L, i)){
			case LUA_TNIL:
				print_error(L, "\t%s", lua_typename(L, lua_type(L, i)));
				break;
			case LUA_TBOOLEAN:
				print_error(L, "\t%s    %s", lua_typename(L, lua_type(L, i)), lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TLIGHTUSERDATA:
				print_error(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TNUMBER:
				print_error(L, "\t%s    %f", lua_typename(L, lua_type(L, i)), lua_tonumber(L, i));
				break;
			case LUA_TSTRING:
				print_error(L, "\t%s    %s", lua_typename(L, lua_type(L, i)), lua_tostring(L, i));
				break;
			case LUA_TTABLE:
				print_error(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TFUNCTION:
				print_error(L, "\t%s()  0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TUSERDATA:
				print_error(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
				break;
			case LUA_TTHREAD:
				print_error(L, "\t%s", lua_typename(L, lua_type(L, i)));
				break;
			}
		}
		print_error(L, "%s", "-------------------------");
	}

  //打印栈内对象信息
  inline void  debug_stack(lua_State *L){
    int top = lua_gettop(L);
    info(L, "%s", "----------stack----------");
    info(L, "Type:%d", top);
    for (int i = 1; i <= lua_gettop(L); ++i){
      switch (lua_type(L, i)){
      case LUA_TNIL:
        info(L, "\t%s", lua_typename(L, lua_type(L, i)));
        break;
      case LUA_TBOOLEAN:
        info(L, "\t%s    %s", lua_typename(L, lua_type(L, i)), lua_toboolean(L, i) ? "true" : "false");
        break;
      case LUA_TLIGHTUSERDATA:
        info(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
        break;
      case LUA_TNUMBER:
        info(L, "\t%s    %f", lua_typename(L, lua_type(L, i)), lua_tonumber(L, i));
        break;
      case LUA_TSTRING:
        info(L, "\t%s    %s", lua_typename(L, lua_type(L, i)), lua_tostring(L, i));
        break;
      case LUA_TTABLE:
        info(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
        break;
      case LUA_TFUNCTION:
        info(L, "\t%s()  0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
        break;
      case LUA_TUSERDATA:
        info(L, "\t%s    0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
        break;
      case LUA_TTHREAD:
        info(L, "\t%s", lua_typename(L, lua_type(L, i)));
        break;
      }
    }
    info(L, "%s", "-------------------------");
  }
	
	 //错误调试栈信息
	inline void call_stack(lua_State* L, int n){
		lua_Debug ar;
		if (lua_getstack(L, n, &ar) == 1){
			lua_getinfo(L, "nSlu", &ar);

			const char* indent;
			if (n == 0){
				indent = "->\t";
				print_error(L, "\t<call stack>");
			}else{
				indent = "\t";
			}

			if (ar.name)
				print_error(L, "%s%s() : line %d [%s : line %d]", indent, ar.name, ar.currentline, ar.source, ar.linedefined);
			else
				print_error(L, "%sunknown : line %d [%s : line %d]", indent, ar.currentline, ar.source, ar.linedefined);

			call_stack(L, n + 1);
		}
	}

	//错误处理函数
	static int on_error(lua_State *L) {
		print_error(L, "%s", lua_tostring(L, -1));
		call_stack(L, 0);
		return 1;
	}
	
	//dofile
	inline void dofile(lua_State *L, const char *filename) {
		lua_settop(L, -1);
		lua_pushcclosure(L, on_error, 0);
		int errfunc = lua_gettop(L);

		if (luaL_loadfile(L, filename) == 0){
			lua_pcall(L, 0, 1, errfunc);
		}else{
			print_error(L, "%s", lua_tostring(L, -1));
		}

		lua_remove(L, errfunc);
		lua_pop(L, 1);
	}


	//dobuffer
	inline void  dobuffer(lua_State *L, const char* buff, size_t len) {
		lua_pushcclosure(L, on_error, 0);
		int errfunc = lua_gettop(L);

		if (luaL_loadbuffer(L, buff, len, "lua_tinker::dobuffer()") == 0){
			lua_pcall(L, 0, 1, errfunc);
		}else{
			print_error(L, "%s", lua_tostring(L, -1));
		}

		lua_remove(L, errfunc);
		lua_pop(L, 1);
	}

	//dostring
	inline void dostring(lua_State *L, const char* buff) {
		dobuffer(L, buff, strlen(buff));
	}
#if LUA_VERSION_NUM > 501
	//注册模块
	inline void openLuaLib(lua_State* L, const char* name, lua_CFunction fun) {
		luaL_requiref(L, name, fun, 1);
		lua_pop(L, 1);
	}
#endif

	//openlibs
	inline void openLuaLibs(lua_State* L) {
		luaopen_base(L);
		luaL_openlibs(L);
	}

	//元表查找反射
	static void invoke_parent(lua_State *L){
		lua_pushstring(L, "__parent");
		lua_rawget(L, -2);
		if (lua_istable(L, -1)){
			lua_pushvalue(L, 2);
			lua_rawget(L, -2);
			if (!lua_isnil(L, -1)){
				lua_remove(L, -2);
			}
			else{
				lua_remove(L, -1);
				invoke_parent(L);
				lua_remove(L, -2);
			}
		}
	}

	//获取元表
	inline int meta_get(lua_State *L){
		
		lua_getmetatable(L, 1);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);

		if (lua_isuserdata(L, -1)){
			user2type<var_base*>::invoke(L, -1)->get(L);
			lua_remove(L, -2);
		}
		else if (lua_isnil(L, -1)){
			lua_remove(L, -1);
			invoke_parent(L);
			if (lua_isuserdata(L, -1)){
				user2type<var_base*>::invoke(L, -1)->get(L);
				lua_remove(L, -2);
			}
			else if (lua_isnil(L, -1)){
				lua_pushfstring(L, "can't find '%s' class variable. (forgot registering class variable ?)", lua_tostring(L, 2));
				lua_error(L);
			}
		}

		lua_remove(L, -2);
		return 1;
	}

	//设置元表
	inline int meta_set(lua_State *L){
		lua_getmetatable(L, 1);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);

		if (lua_isuserdata(L, -1)){
			user2type<var_base*>::invoke(L, -1)->set(L);
		}else if (lua_isnil(L, -1)){
			lua_remove(L, -1);
			lua_pushvalue(L, 2);
			lua_pushvalue(L, 4);
			invoke_parent(L);
			if (lua_isuserdata(L, -1)){
				user2type<var_base*>::invoke(L, -1)->set(L);
			}
			else if (lua_isnil(L, -1)){
				lua_pushfstring(L, "can't find '%s' class variable. (forgot registering class variable ?)", lua_tostring(L, 2));
				lua_error(L);
			}
		}
		lua_settop(L, 3);
		return 0;
	}

	//元表入栈
	inline void push_meta(lua_State *L, const char* name){
		lua_getglobal(L, name);
	}
	

	//清栈,会导致table对象无效
	inline void resetStack(lua_State* L) {
		lua_settop(L, 0);
	}
}

#endif