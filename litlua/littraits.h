/********************************************************************

Filename:   littraits.h

Description:littraits.h

Version:  1.0
Created:  25:11:2018   17:27
Revison:  none
Compiler: gcc vc

Author:   wufan, love19862003@163.com

Organization:
*********************************************************************/
#pragma once
#ifndef __lit_traits_h__
#define __lit_traits_h__

#include <new>
#include <stdint.h>
#include <string.h>
#include <string>
#include <lua.hpp>
#include <stdio.h>
#include <typeinfo>
#include <memory>
#include <assert.h>
#include <tuple>
#include <functional>

#if LUA_VERSION_NUM == 501
static bool lua_isinteger(lua_State* L, int index) {
    int32_t x = (int32_t)lua_tointeger(L, index);
    lua_Number n = lua_tonumber(L, index);
    return ((lua_Number)x == n);
}
#endif

namespace LitSpace {
	struct lua_value;
	struct table;
	struct nil;



	static int on_error(lua_State* L);
	inline void push_meta(lua_State* L, const char* name);

	template<typename T>
	inline T read(lua_State* L, int index);
	template<typename T>
	inline void push(lua_State* L, T ret);
	template<typename T>
	inline bool check(lua_State* L, int index);
	template<typename T>
	T upvalue_(lua_State* L);

	template <std::size_t... M>
	struct _indices {};

	template <std::size_t N, std::size_t... M>
	struct _indices_builder : _indices_builder<N - 1, N - 1, M...> {};

	template <std::size_t... M>
	struct _indices_builder<0, M...> {
		using type = _indices<M...>;
	};

  template<typename F>
  struct guard{
    typedef  F Fun;
    explicit guard(const Fun& f) : _fun(f){}
    ~guard(){ _fun(); }
    Fun _fun;
  };
  typedef guard<std::function<void()>> guardfun;


  //读取栈内连续数据,主要是实现读取C函数调用参数和lua返回值
  template<size_t N, size_t M, typename T>
  struct tuple_reader{
    static constexpr size_t I = M - N;
    static const T& reader(lua_State* L, T& r, int& index){
      auto& ref = std::get<I>(r);
      ref = read<typename std::remove_reference<decltype(ref)>::type>(L, index);
      tuple_reader< N - 1, M, T>::reader(L, r, ++index);
      return r;
    }
    static bool result_check_reader(lua_State* L, T& r, int& index, std::string& _error_msg){
      auto& ref = std::get<I>(r);
      bool r1 = check<typename std::remove_reference<decltype(ref)>::type>(L, index);
      if(r1){
        //ref = read<typename std::remove_reference<decltype(ref)>::type>(L, index);
		ref = read<decltype(ref)>(L, index);
	  }
	  else {
		  _error_msg.append("\nerror read index:").append(std::to_string(I)).append(" with type:").append(typeid(typename std::remove_reference<decltype(ref)>::type).name());
	  }
      bool r2 = tuple_reader< N - 1, M, T>::result_check_reader(L, r, ++index, _error_msg);
      return r2 && r1;
    }

  };

  template<size_t M, typename T>
  struct tuple_reader<1, M, T>{
    static constexpr size_t I = M - 1;
    static const T& reader(lua_State* L, T& r, int& index){
      auto& ref = std::get<I>(r);
      ref = read<typename std::remove_reference<decltype(ref)>::type>(L, index);
      return r;
    }

    static bool result_check_reader(lua_State* L, T& r, int& index, std::string& _error_msg){
      auto& ref = std::get<I>(r);
      bool r1 = check<typename std::remove_reference<decltype(ref)>::type>(L, index);
      if (r1){
		  //ref = read<typename std::remove_reference<decltype(ref)>::type>(L, index); 
		  ref = read<decltype(ref)>(L, index);
	  }
      else{ _error_msg.append("\nerror read index:").append(std::to_string(I)).append(" with type:").append(typeid(typename std::remove_reference<decltype(ref)>::type).name()); }
      return r1;
    }
  };

//   //lua传入参数列表对象
//   template<typename ... ARGS>
//   struct lua_args : public std::tuple<  ARGS ...>{
//     static constexpr size_t NSIZE = sizeof...(ARGS);
//     typedef lua_args args;
//     static args reader(lua_State* L, int& index){ args r;return tuple_reader<NSIZE, NSIZE, lua_args>::reader(L, r, index);}
//   };
// 
//   template<>
//   struct lua_args<> : public std::tuple<>{
//     static constexpr size_t NSIZE = 0;
//     typedef lua_args args;
//     static args reader(lua_State* L, int& index){ return args();}
//   };


  template<typename... _Types>
  struct lua_args;

  template<>
  struct lua_args<>{
      static constexpr size_t NSIZE = 0;
      typedef std::tuple<> Type;
      static Type reader(lua_State* L, int& index) {
          return std::tuple<>();
      }
  };

  template< typename T, typename ... ARGS>
  struct lua_args<T, ARGS...>: private lua_args< ARGS... > {
      static constexpr size_t NSIZE = sizeof...(ARGS) + 1;
      typedef std::tuple< T, ARGS ... > Type;
      static Type reader(lua_State* L, int& index) {
		  T t = read<T>(L, index);
		  return tuple_cat(std::tuple<T>(t), lua_args<ARGS ...>::reader(L, ++index));
	  }
  };




  template<typename T, typename ... ARGS>
  struct HasTable{
    static constexpr bool HAS_TABLE = HasTable<T>::HAS_TABLE || HasTable<ARGS ...>::HAS_TABLE;
  };

  template<typename T>
  struct HasTable<T>{
    static constexpr bool HAS_TABLE = std::is_same<T, table>::value;
  };

   //lua执行结果对象
   template<typename T, typename ... ARGS>
   struct lua_returns : public std::tuple<T, ARGS ...> {
       static constexpr size_t NSIZE = 1 + sizeof...(ARGS);
       static constexpr bool IS_RETURN = true;
       static constexpr bool HAS_TABLE = HasTable<T, ARGS ...>::HAS_TABLE;
       typedef lua_returns args;
       static args reader(lua_State* L, int& index) {
           args r;
           r._err = !tuple_reader<NSIZE, NSIZE, lua_returns>::result_check_reader(L, r, index, r._error_msg);
           return r;
       }
       bool _err = false;
       std::string _error_msg = "";
 
       T& get() {
           return std::get<0>(*this);
       }
 
   };
 
   template<>
   struct lua_returns<void> :public std::tuple<> {
       static constexpr size_t NSIZE = 0;
       static constexpr bool IS_RETURN = true;
       static constexpr bool HAS_TABLE = false;
       typedef lua_returns<void> args;
       static args reader(lua_State* L, int& index) {
           return lua_returns<void>();
       }
       bool _err = false;
       std::string _error_msg = "";
 
       void get() {
 
       }
   };

   //全局函数的多参数展开
	template <typename Ret, typename... ARGS, std::size_t... N>
	inline Ret litFunciton(const std::function<Ret(ARGS...)>& func, const std::tuple<ARGS...>& args, _indices<N...>) {
		return func(std::get<N>(args)...);
	}

	template <typename Ret, typename... ARGS>
	inline Ret litFunciton(const std::function<Ret(ARGS...)>& func, const std::tuple<ARGS...>& args) {
		return litFunciton(func, args, typename _indices_builder<sizeof...(ARGS)>::type());
	}

	//类成员函数的多参数展开
	template <typename Ret, typename T, typename... ARGS, std::size_t... N>
	inline Ret classLitFunciton(Ret(T::*func)(ARGS ...), T* t, const std::tuple<ARGS...>& args, _indices<N...>) {
		return (t->*func)(std::get<N>(args)...);
	}

	template <typename Ret, typename T, typename... ARGS>
	inline Ret classLitFunciton(Ret(T::*func)(ARGS ...), T* t, const std::tuple<ARGS...>& args) {
		return classLitFunciton(func, t, args, typename _indices_builder<sizeof...(ARGS)>::type());
	}

	//类构造函数多参数展开
	template <typename T, typename... ARGS, std::size_t... N>
	inline T* constructorFun(void* p, const std::tuple<ARGS...>& args, _indices<N...>) {
		return new(p)T(std::get<N>(args)...);
	}

	template <typename T>
	inline T* constructorFun(void* p, const std::tuple<>& args) {
		return new(p)T();
	}

	template <typename T, typename... ARGS>
	inline T* constructorFun(void* p, const std::tuple<ARGS...>& args) {
		return constructorFun<T>(p, args, typename _indices_builder<sizeof...(ARGS)>::type());
	}

  inline void push_args(lua_State *L){}

  //调用参数入栈
  template<typename T>
  inline void push_args(lua_State *L, T t){
    push(L, t);
  }

  template<typename T, typename ... Args>
  inline void push_args(lua_State *L, T t, Args... args){
    push(L, t);
    push_args(L, args...);
  }

  template<typename Ret, typename ... ARGS>
  struct res_functor{
	  static constexpr bool ISVOID = std::is_void<Ret>::value;
	  static int push_result(lua_State* L, const std::function<Ret(ARGS...)>& func) {
		  if (nullptr == func) {
			  push(L, "lua error with call invalid std::function (nullptr)");
			  on_error(L);
			  return 0;
		  }
		  int index = 1;
		  push<Ret>(L, litFunciton(func, lua_args< ARGS ...>::reader(L, index)));
		  return 1;
	  }
  };

  template< typename ... ARGS>
  struct void_functor {
	  static constexpr bool ISVOID = true;
	  static int push_result(lua_State* L, const std::function<void(ARGS...)>& func) {
		  if (nullptr == func) {
			  push(L, "lua error with call invalid std::function (nullptr)");
			  on_error(L);
			  return 0;
		  }
		  int index = 1;
		  litFunciton(func, lua_args< ARGS ...>::reader(L, index));
		  return 0;
	  }
  };

  //全局函数和std::function反射
  template<typename Ret, typename ... ARGS>
  struct functor : public lua_args< ARGS ...>{
    //template<typename Ret>
    static int invoke_func(lua_State* L){
      const std::function<Ret(ARGS...)>& func = upvalue_<std::function<Ret(ARGS...)>>(L);
      return std::conditional<std::is_void<Ret>::value, void_functor<ARGS ...>, res_functor<Ret, ARGS ...> >::type::push_result(L, func);
    }


	//template<typename Ret>
	static int invoke(lua_State* L) {
		const std::function<Ret(ARGS...)>& func = upvalue_<Ret(*)(ARGS ...)>(L);
		return std::conditional<std::is_void<Ret>::value, void_functor<ARGS ...>, res_functor<Ret, ARGS ...> >::type::push_result(L, func);
	}
  };


  template<typename T,typename Ret, typename ... ARGS>
  struct res_class_functor {
	  static constexpr bool ISVOID = std::is_void<Ret>::value;
	  static int push_result(lua_State* L,  Ret(T::* func)(ARGS ...)) {
		  int index = 1;
		  T* t = read<T*>(L, index++);
		  if (nullptr == t || nullptr == func) {
			  push(L, "lua error with call invalid object(nullptr) member function");
			  on_error(L);
			  return 0;
		  }
		  push<Ret>(L, classLitFunciton(func,t, lua_args< ARGS ...>::reader(L, index)));
		  return 1;
	  }
  };

  template<typename T, typename ... ARGS>
  struct void_class_functor {
	  static constexpr bool ISVOID = true;
	  static int push_result(lua_State* L, void(T::* func)(ARGS ...)) {
		  int index = 1;
		  T* t = read<T*>(L, index++);
		  if (nullptr == t || nullptr == func) {
			  push(L, "lua error with call invalid object(nullptr) member function");
			  on_error(L);
			  return 0;
		  }
		  classLitFunciton(func,t, lua_args< ARGS ...>::reader(L, index));
		  return 0;
	  }
  };

  //类成员函数反射
  template<typename Ret, typename T, typename ... ARGS>
  struct classfunctor : public lua_args< ARGS ...>{
	  static int invoke(lua_State* L) {
		  auto func = upvalue_<Ret(T::*)(ARGS ...)>(L);
		  return std::conditional<std::is_void<Ret>::value, void_class_functor<T, ARGS ...>, res_class_functor<T, Ret, ARGS ...> >::type::push_result(L, func);
	  }
  };

  //整数类型相关操作
  template<typename T>
  struct int_action{
    static constexpr bool value = std::is_integral<T>::value;
    static T invoke(lua_State *L, int index){
      return (T)lua_tointeger(L, index);
    }
    static bool check(lua_State *L, int index){
      return lua_isinteger(L, index) || lua_isnumber(L, index) || lua_isboolean(L, index);
    }

    static void push(lua_State *L, T val){
      lua_pushinteger(L, val);
    }


  };
  template<>
  struct int_action<bool>{
    static constexpr bool value = true;
    static bool invoke(lua_State *L, int index){
      return (bool)lua_toboolean(L, index);
    }
    static bool check(lua_State *L, int index){
      return true;
    }
    static void push(lua_State *L, bool val){
      lua_pushboolean(L, val);
    }
  };

  //浮点数类型相关操作
  template<typename T>
  struct number_action{
    static constexpr bool value = std::is_floating_point<T>::value;
    static T invoke(lua_State *L, int index){
      return (T)lua_tonumber(L, index);
    }
    static bool check(lua_State *L, int index){
      return lua_isinteger(L, index) || lua_isnumber(L, index) || lua_isboolean(L, index);
    }
    static void push(lua_State *L, T val){
      lua_pushnumber(L, val);
    }
  };

  //字符串类型相关操作
  template<typename T>
  struct str_action{
    static constexpr bool value = std::is_same<T, char*>::value || std::is_same<T, const char*>::value;
    static T invoke(lua_State *L, int index){
      return (T)lua_tostring(L, index);
    }
    static bool check(lua_State *L, int index){
      return lua_isinteger(L, index) || lua_isnumber(L, index) || lua_isboolean(L, index) || lua_isstring(L, index);
    }
    static void push(lua_State *L, T val){
      lua_pushstring(L, val);
    }
  };

  template<>
  struct str_action<std::string>{
    static constexpr bool value = true;
    static std::string invoke(lua_State *L, int index){
      size_t len = 0;
      const char* buffer = lua_tolstring(L, index, &len);
      return std::string(buffer, len);
    }
    static bool check(lua_State *L, int index){
      return lua_isinteger(L, index) || lua_isnumber(L, index) || lua_isboolean(L, index) || lua_isstring(L, index);
    }
    static void push(lua_State *L, std::string val){
      lua_pushlstring(L, val.c_str(), val.length());
    }
  };

  //判断是否是对象类型
  template<typename A>
  struct is_obj {
	  static constexpr bool value = !std::is_integral<A>::value && !std::is_floating_point<A>::value;// true;
  };

  template<> struct is_obj<char*> { static constexpr bool value = false; };
  template<> struct is_obj<const char*> { static constexpr bool value = false; };
  template<> struct is_obj<lua_value*> { static constexpr bool value = false; };
  template<> struct is_obj<table> { static constexpr bool value = false; };
  template<> struct is_obj<nil> { static constexpr bool value = false; };
  template<> struct is_obj<std::string> { static constexpr bool value = false; };

  //成员变量
  struct var_base{
    virtual ~var_base(){};
    virtual void get(lua_State *L) = 0;
    virtual void set(lua_State *L) = 0;
  };


  template<typename T, typename V>
  struct mem_var : var_base{
    V T::*_var;
    mem_var(V T::*val): _var(val){}
    void get(lua_State *L){
      T* t = read<T*>(L, 1);
      if (!check(L, t)) return;
      push<typename std::conditional<is_obj<V>::value, V&, V>::type>(L, t->*(_var)); 
    }
    void set(lua_State *L){ 
      T* t = read<T*>(L, 1);
      if(!check(L, t)) return ;
      t->*(_var) = read<V>(L, 3); 
    }

    bool check(lua_State *L, T* t){
      if (nullptr == t){
        push(L, "lua error with call member var with nullptr");
        on_error(L);
        return false;
      }
      return true;
    }
  };

	//萃取原始类型
 	template<typename A>
 	struct base_type { //std::remove_cvref
 		typedef  typename std::remove_cv<typename std::remove_pointer< typename std::remove_reference<A>::type >::type >::type  type;
 	};

	//绑定class类型
	template<typename A>
	struct class_type {	 
		typedef typename base_type<A>::type type;
	};	  

	//绑定class名字
	template<typename T>
	struct class_name {
		static const char* name(const char* name = NULL) {
			static char temp[256] = "";
			if (name != NULL) { strncpy(temp, name, sizeof(temp) - 1);}
			return temp;
		}
	};

	

	//void*指针转换对应类型反射
	template<typename T>
	struct void2val { 
		static T invoke(void* input) {return *(T*)input;} 
	};
	template<typename T>
	struct void2ptr { 
    static T* invoke(void* input){return (T*)input;} 
	};
	template<typename T>
	struct void2ref { 
		static T& invoke(void* input) {return *(T*)input; } 
	};


	template<typename T>
	struct void2type
	{
		static T invoke(void* ptr)
		{
			return  std::conditional<std::is_pointer<T>::value
				, void2ptr<typename base_type<T>::type>
				, typename std::conditional<std::is_reference<T>::value
				, void2ref<typename base_type<T>::type>
				, void2val<typename base_type<T>::type>
				>::type
			>::type::invoke(ptr);
		}
	};

	//c++对象的地址， ptr ref value 都绑定一个地址
	struct user{
		user(void* p) : m_p(p){}
		virtual ~user() {}
		void* m_p;
	};

	template<typename T>
	struct user2type { 
		static T invoke(lua_State *L, int index) {
			return void2type<T>::invoke(lua_touserdata(L, index)); 
		} 
	};

	//lua到枚举的反射
	template<typename T>
		struct lua2enum {
			static T invoke(lua_State *L, int index) { 
				return (T)(int)lua_tonumber(L, index);
			} 
		static bool check(lua_State *L, int index){
		  return lua_isinteger(L, index);
		}
	};

	//lua到对象的反射，当是返回值的时候，会去调用对象的check，作为参数的时候直接传递
	template<typename T>
	struct lua2object{
		static T invoke(lua_State *L, int index){
			bool err = !lua_isuserdata(L, index);
			if (err){
				auto t = base_type<T>();
				std::string note;
				note.append( "\ntype:[").append(typeid(t).name()).append("]call class type error \n1.first argument. (forgot ':' expression ?)");
				push(L, note);
				err = true;
				lua_error(L);
			}
			return void2type<T>::invoke(user2type<user*>::invoke(L, index)->m_p);
		}

		static bool check(lua_State *L, int index){
		  return lua_isuserdata(L, index);
		}
	};

	//lua转换到对应类型数据 nil table通过read的偏特化实现
	template<typename T>
	T lua2type(lua_State *L, int index){
		return std::conditional< int_action<T>::value, int_action<T>,
		  typename std::conditional< number_action<T>::value, number_action<T>,
		  typename std::conditional< str_action<T>::value, str_action<T>,
		  typename std::conditional<std::is_enum<T>::value, lua2enum<T>, lua2object<T>	>::type
		  >::type
		  >::type
		>::type::invoke(L, index);

	}

	//主要检测返回值是否合法
	//检测lua能否转换到对应类型 nil table通过read的偏特化实现
	  template<typename T>
	  bool checklua2type(lua_State *L, int index){
		return std::conditional< int_action<T>::value, int_action<T>,
		  typename std::conditional< number_action<T>::value, number_action<T>,
		  typename std::conditional< str_action<T>::value, str_action<T>,
		  typename std::conditional<std::is_enum<T>::value, lua2enum<T>, lua2object<T>	>::type
		  >::type
		  >::type
		>::type::check(L, index);
	  }

	//class类型绑定对象
	template<typename T>
	struct val2user : user{
		template<typename ... ARGS>
		val2user(ARGS ... args) : user(new T(args...)) {}
		~val2user() { delete ((T*)m_p); }
	};


	template<typename T>
	struct ptr2user : user{
		ptr2user(T* t) : user((void*)t) {}
	};


	template<typename T>
	struct ref2user : user{
		ref2user(T& t) : user(&t) {}
	};

	//class类型绑定lua对象
	template<typename T>
	struct val2lua {
		static void invoke(lua_State *L, T& input) {
			new(lua_newuserdata(L, sizeof(val2user<T>))) val2user<T>(input); 
		}
	};

	template<typename T>
	struct ptr2lua {
		static void invoke(lua_State *L, T* input) {
			if (input) { new(lua_newuserdata(L, sizeof(ptr2user<T>))) ptr2user<T>(input);}
      else{ lua_pushnil(L); }
		} 
	};

	template<typename T>
	struct ref2lua { 
		static void invoke(lua_State *L, T& input) { 
			new(lua_newuserdata(L, sizeof(ref2user<T>))) ref2user<T>(input); 
		} 
	};

	//枚举转lua
	template<typename T>
	struct enum2lua {
		static void push(lua_State *L, T val) { 
			lua_pushnumber(L, (int)val);
		}
	};

	//class对象到lua
	template<typename T>
	struct object2lua{
		static void push(lua_State *L, T val){
			std::conditional<std::is_pointer<T>::value
				, ptr2lua<typename base_type<T>::type>
				, typename std::conditional<std::is_reference<T>::value
				, ref2lua<typename base_type<T>::type>
				, val2lua<typename base_type<T>::type>
				>::type
			>::type::invoke(L, val);

			// set metatable
			push_meta(L, class_name<typename class_type<T>::type>::name());
			lua_setmetatable(L, -2);
		}
	};

	//入栈反射
	template<typename T>
	void type2lua(lua_State *L, T val){
		//std::conditional<std::is_enum<T>::value, enum2lua<T>, object2lua<T>>::type::push(L, val);
		return std::conditional< int_action<T>::value, int_action<T>,
		  typename std::conditional< number_action<T>::value, number_action<T>,
		  typename std::conditional< str_action<T>::value, str_action<T>,
		  typename std::conditional<std::is_enum<T>::value, enum2lua<T>, object2lua<T>	>::type
		  >::type
		  >::type
		>::type::push(L, val);
	}

	//获取C函数闭包
	template<typename T>
	T upvalue_(lua_State *L){
		return user2type<T>::invoke(L, lua_upvalueindex(1));
	}

	//全局函数闭包入栈
	template<typename R, typename ... ARGS >
	void push_function(lua_State *L, R(*func)(ARGS ...)) {
		(void)func;
		lua_pushcclosure(L, functor<R, ARGS...>::invoke, 1);
	}

	//std::function闭包入栈 func的作用域和有效期回影响调用
	template<typename R, typename ... ARGS >
	void push_functor(lua_State *L, const std::function<R( ARGS...)>& func) {
		(void)func;
		lua_pushcclosure(L, functor<R, ARGS...>::invoke_func, 1);
	}
	//成员函数闭包入栈
	template<typename R, typename T, typename ... ARGS >
	void push_mfunctor(lua_State *L, R(T::*func)(ARGS ...)) {
		(void)func;
		lua_pushcclosure(L, classfunctor<R, T, ARGS...>::invoke, 1);
	}


	//构造函数入栈
	template<typename T, typename ... ARGS>
	int constructor(lua_State *L) {
		val2user<T>* p = (val2user<T>*)lua_newuserdata(L, sizeof(val2user<T>));
		int index = 2;
		constructorFun<val2user<T>>(p, lua_args<ARGS ...>::reader(L, index));
		push_meta(L, class_name<typename class_type<T>::type>::name());
		lua_setmetatable(L, -2);
		return 1;
	}

	//析构函数
	template<typename T>
	int destroyer(lua_State *L) {
		((user*)lua_touserdata(L, 1))->~user();
		return 0;
	}
}

#endif