/******************************************************************************/
/*!
 * @file   bf_meta_function_traits.hpp
 * @author Shareef Abdoul-Raheem (http://blufedora.github.io/)
 * @brief
 *   Allows for compile time introspection on the properties of callable objects.
 * 
 *   Requires at least C++14 but is best used with C++17.
 *
 * @version 0.0.1
 * @date    2019-12-28
 *
 * @copyright Copyright (c) 2019-2020
 */
/******************************************************************************/
#ifndef BF_META_FUNCTION_TRAITS_HPP
#define BF_META_FUNCTION_TRAITS_HPP

#include <cstddef>     /* size_t                                           */
#include <functional>  /* invoke                                           */
#include <tuple>       /* tuple, tuple_element, apply                      */
#include <type_traits> /* decay_t, invoke_result_t, is_nothrow_invocable_v */

namespace bf::meta
{
  /*!
   * @brief
   *   ParameterPack allows you to hold a list of arguments
   *   you want to _apply_ to a type at a later point in time.
   * 
   * @tparam ...P
   *   The arguments types you want stored in this ParameterPack.
   */
  template<typename... P>
  struct ParameterPack
  {
    static constexpr std::size_t size = sizeof...(P);  //!< The number of template arguments this ParameterPack holds.

    /*!
     * @brief
     *   Applies the stored template arguments onto the type passed in.
     *  
     *  Example of Usage:
     * 
     *    using ParamPack = ParameterPack<A, B, C>;
     *    
     *    template <typename...> struct S {};
     *    ParamPack::apply<S> var;   [Equivalent to `S<A, B, C> var;`]
     */
    template<template<typename...> typename T>
    using apply = T<P...>;

    /*!
     * @brief
     *   Allows extending this ParameterPack with more template arguments.
     * 
     *   Example of Usage:
     * 
     *    using ParamPackBase         = ParameterPack<A, B, C>;
     *    using ParamPackWithMoreArgs = ParamPackBase::extend<D, E, F>;
     * 
     *    // ParamPackWithMoreArgs is Equivalent to 'ParameterPack<A, B, C, D, E, F>'
     * 
     * @tparam ...Args
     *   The arguments you want added to the end of this ParameterPack's own argument list.
     */
    template<typename... Args>
    using extend = ParameterPack<P..., Args...>;
  };

  /*!
   * @brief
   *   Hold the type of the arguments for a function signature.
   * 
   * @tparam ...Args
   *   The types that the function takes in.
   */
  template<typename... Args>
  using FunctionArgsTuple = std::tuple<Args...>;

  namespace detail
  {
    template<typename TFirst, typename... TRest>
    struct remove_first_tuple
    {
      using type = std::tuple<TRest...>;
    };

    template<typename TFirst, typename... TRest>
    struct remove_first_tuple<std::tuple<TFirst, TRest...>>
    {
      using type = std::tuple<TRest...>;
    };

    template<typename TFirst, typename... TRest>
    using remove_first_tuple_t = typename remove_first_tuple<TFirst, TRest...>::type;
  }  // namespace detail

  template<typename F>
  struct function_traits; /* undefined */

  template<typename R, typename... Args>
  struct function_traits<R(Args...)>
  {
    static constexpr std::size_t arity = sizeof...(Args);
    using return_type                  = R;
    using tuple_type                   = FunctionArgsTuple<Args...>;
    using parameter_pack               = ParameterPack<Args...>;

    template<std::size_t N>
    struct argument
    {
      static_assert(N < arity, "error: invalid parameter index.");
      using type = typename std::tuple_element<N, tuple_type>::type;
    };
  };

  // function pointer
  template<typename R, typename... Args>
  struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)>
  {
    static constexpr bool is_member_fn = false;
  };

  // member function pointer
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...)> : public function_traits<R(C*, Args...)>
  {
    static constexpr bool is_member_fn = true;

    //
    // 'tuple_type_raw' and 'parameter_pack_raw' are for the arguments of a member function / pointer without the class as the first argument.
    //

    using class_type         = std::decay_t<C>;
    using tuple_type_raw     = FunctionArgsTuple<Args...>;
    using parameter_pack_raw = ParameterPack<Args...>;
  };

  // const member function pointer
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const> : public function_traits<R(const C*, Args...)>
  {
    static constexpr bool is_member_fn = true;

    //
    // 'tuple_type_raw' and 'parameter_pack_raw' are for the arguments of a member function / pointer without the class as the first argument.
    //

    using class_type         = std::decay_t<C>;
    using tuple_type_raw     = FunctionArgsTuple<Args...>;
    using parameter_pack_raw = ParameterPack<Args...>;
  };

  // In C++17 and later 'noexcept'-ness is part of a function's type.
#if __cplusplus > 201402L  // After C++14

  template<typename R, typename... Args>
  struct function_traits<R (*)(Args...) noexcept> : public function_traits<R(Args...)>
  {
    static constexpr bool is_member_fn = false;
  };

  // noexcept member function pointer
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) noexcept> : public function_traits<R(C*, Args...)>
  {
    static constexpr bool is_member_fn = true;
  };

  // noexcept const member function pointer
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const noexcept> : public function_traits<R(C*, Args...)>
  {
    static constexpr bool is_member_fn = true;
  };

#endif

  // member object pointer
  template<typename C, typename R>
  struct function_traits<R C::*> : public function_traits<R(C&)>
  {
    static constexpr bool is_member_fn = false;

    using class_type     = std::decay_t<C>;
    using tuple_type_raw = std::tuple<>;
  };

  // function objects / lambdas
  template<typename F>
  struct function_traits
  {
    static constexpr bool is_member_fn = false;

   private:
    using call_type = function_traits<decltype(&F::operator())>;

   public:
    static constexpr std::size_t arity = call_type::arity - 1;
    using return_type                  = typename call_type::return_type;
    using tuple_type                   = detail::remove_first_tuple_t<typename call_type::tuple_type>;
    using parameter_pack               = detail::remove_first_tuple_t<typename call_type::parameter_pack>;

    template<std::size_t N>
    struct argument
    {
      static_assert(N < arity, "error: invalid parameter index.");
      using type = typename call_type::template argument<N + 1>::type;
    };
  };

  // function that is passed as a r-value / universal reference.
  template<typename F>
  struct function_traits<F&&> : public function_traits<F>
  {};

  // function reference
  template<typename F>
  struct function_traits<F&> : public function_traits<F>
  {};

  // const function reference
  template<typename F>
  struct function_traits<const F&> : public function_traits<F>
  {};

#if __cplusplus > 201402L  // After C++14

  template<typename F, typename... Args>
  std::invoke_result_t<F, Args...> invoke(F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
  {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
  }

  template<typename F, typename Tuple>
  constexpr decltype(auto) apply(F&& f, Tuple&& t)
  {
    return std::apply(std::forward<F>(f), std::forward<Tuple>(t));
  }

#else

  // TODO(SR): Make shims for 'invoke' and 'apply' for pre C++17 code.

#endif

  //
  // Placement news into an object using a tuple for the parameters of the constructor.
  //

  namespace detail
  {
    template<typename T, typename Tuple, size_t... Is>
    void construct_from_tuple_(T* obj, Tuple&& tuple, std::index_sequence<Is...>)
    {
      new (obj) T(std::get<Is>(std::forward<Tuple>(tuple))...);
    }
  }  // namespace detail

  template<typename T, typename Tuple>
  void construct_from_tuple(T* obj, Tuple&& tuple)
  {
    detail::construct_from_tuple_(obj, std::forward<Tuple>(tuple), std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
  }

  //
  // https://dev.krzaq.cc/post/you-dont-need-a-stateful-deleter-in-your-unique_ptr-usually/
  // `function_caller` is useful for stateless unique_ptrs deleters.
  //
  // Use the 'bfDefineFunctionCaller' macro if you want to be able to
  // portably use this class in both C++14 and C++17 modes.
  //

#if __cplusplus > 201402L  // After C++14
  template<auto func>
  struct function_caller
  {
    template<typename... Us>
    auto operator()(Us&&... us) const
    {
      return func(std::forward<Us...>(us...));
    }
  };

#define bfDefineFunctionCaller(f) ::bf::meta::function_caller<f>

#else
  template<typename T, T* func>
  struct function_caller
  {
    template<typename... Us>
    auto operator()(Us&&... us) const
    {
      return func(std::forward<Us...>(us...));
    }
  };

#define bfDefineFunctionCaller(f) ::bf::meta::function_caller<decltype(f), f>

#endif

}  // namespace bf::meta

#endif /* BF_META_FUNCTION_TRAITS_HPP */

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
