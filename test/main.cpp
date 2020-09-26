#include <iostream>
#include <functional>

void run_test();

namespace testSpace{
  template <std::size_t... M>
  struct _indices{};

  template <std::size_t N, std::size_t... M>
  struct _indices_builder : _indices_builder<N - 1, N - 1, M...>{};

  template <std::size_t... M>
  struct _indices_builder<0, M...>{
    using type = _indices<M...>;
  };
  template <typename Ret, typename... ARGS, std::size_t... N>
  inline Ret testTraitsTuple(const std::function<Ret(ARGS...)>& func, const std::tuple<ARGS...>& args, _indices<N...>){
    return func(std::get<N>(args)...);
  }

//   template <typename Ret, typename... ARGS>
//   inline Ret testTraitsTuple(const std::function<Ret(ARGS...)>& func, const std::tuple<ARGS...>& args){
//     return testTraitsTuple(func, args, typename _indices_builder<sizeof...(ARGS)>::type());
//   }

   template <typename Ret, typename... ARGS, std::size_t... N>
   inline Ret testTraitsTuple(Ret(*func)(ARGS ...), const std::tuple<ARGS...>& args, _indices<N...>){
     return func(std::get<N>(args)...);
   }
 
//    template <typename Ret, typename... ARGS>
//    inline Ret testTraitsTuple(Ret(*func)(ARGS ...), const std::tuple<ARGS...>& args){
//      return testTraitsTuple(func, args, typename _indices_builder<sizeof...(ARGS)>::type());
//    }

   template <typename Ret, typename... ARGS>
   inline Ret testTraits(Ret(*func)(ARGS ...), ARGS ... args){
     return testTraitsTuple(func,  std::make_tuple(args ... ), typename _indices_builder<sizeof...(ARGS)>::type());
   }

   template <typename Ret, typename... ARGS>
   inline Ret testTraits(const std::function<Ret(ARGS...)>& func, ARGS ... args){
     return testTraitsTuple(func, std::make_tuple(args ...), typename _indices_builder<sizeof...(ARGS)>::type());
   }



}

int printTest( int a, int b, int c, bool d){
  std::cout << "a:" << a << " b:" << b << " c:" << c << " d:" << (d ? "true" : "false") << std::endl;
  return d ? (a + b + c) : (a - b - c);
}


int main(int argc, char* argv[]) {
  run_test();
  //testSpace::testTraits(&printTest, 1, 2, 3, false);
  return 0;
}