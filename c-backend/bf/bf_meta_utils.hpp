/******************************************************************************/
/*!
 * @file   bf_meta_utils.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 *   Utilities for compile time template meta programming operations.
 *
 * @version 0.0.1
 * @date    2019-12-28
 *
 * @copyright Copyright (c) 2019
 */
/******************************************************************************/
#ifndef BF_META_UTILS_HPP
#define BF_META_UTILS_HPP

#include <tuple>       /* tuple_size, get                                         */
#include <type_traits> /* index_sequence, make_index_sequence, remove_reference_t */
#include <utility>     /* forward                                                 */

namespace bf::meta
{
  //
  // NthTypeOf<N, Ts...>
  //

  template<int N, typename... Ts>
  using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;

  //
  // overloaded
  //

  template<class... Ts>
  struct overloaded : public Ts...
  {
    using Ts::operator()...;
  };
  template<class... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;

  //
  // for_each
  //

  template<typename Tuple, typename F, std::size_t... Indices>
  constexpr void for_each_impl(Tuple&& tuple, F&& f, std::index_sequence<Indices...>)
  {
    (f(std::get<Indices>(tuple)), ...);
  }

  template<typename Tuple, typename F>
  constexpr void for_each(Tuple&& tuple, F&& f)
  {
    constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
    for_each_impl(
     std::forward<Tuple>(tuple),
     std::forward<F>(f),
     std::make_index_sequence<N>{});
  }

  //
  // for_each_template
  //

  namespace detail
  {
    //
    // C++20 comes with lambda templates so we wouldn't have to do this.
    // But alas we are stuck on C++17 for now.
    //
    template<typename T, std::size_t Index>
    struct type_holder
    {
      using held_type                    = T;
      static constexpr std::size_t index = Index;

      type_holder() = default;
    };

    template<std::size_t, std::size_t Index, typename T, typename... Args>
    class for_each_template_impl
    {
     public:
      template<typename F>
      static void impl(F&& func)
      {
        func(type_holder<T, Index>());
        for_each_template_impl<sizeof...(Args), Index + 1, Args...>::impl(func);
      }
    };

    template<std::size_t Index, typename T>
    class for_each_template_impl<1, Index, T>
    {
     public:
      template<typename F>
      static void impl(F&& func)
      {
        func(type_holder<T, Index>());
      }
    };

    //
    // TODO: Add constexpr index to this version of for each....
    //

    template<std::size_t N>
    struct num
    {
      static constexpr auto value = N;
    };

    template<class F, std::size_t... Is>
    void for_constexpr_impl(F func, std::index_sequence<Is...>)
    {
      (func(detail::num<Is>{}), ...);
    }
  }  // namespace detail

#define bfForEachTemplateT(t) typename std::decay<decltype((t))>::type::held_type
#define bfForEachTemplateIndex(t) std::decay<decltype((t))>::type::index

  template<typename... Args, typename F>
  void for_each_template(F&& func)
  {
    detail::for_each_template_impl<sizeof...(Args), 0, Args...>::impl(func);
  }

  //
  // for_constexpr
  //

  // [https://nilsdeppe.com/posts/for-constexpr]
  template<std::size_t N, typename F>
  void for_constexpr(F func)
  {
    detail::for_constexpr_impl(func, std::make_index_sequence<N>());
  }
}  // namespace bf::meta

#endif /* BF_META_UTILS_HPP */

/******************************************************************************/
/*
  MIT License

  Copyright (c) 2020 Shareef Abdoul-Raheem

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
/******************************************************************************/
