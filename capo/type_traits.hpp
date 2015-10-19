/*
    The Capo Library
    Code covered by the MIT License

    Author: mutouyun (http://darkc.at)
*/

#pragma once

#include <type_traits>  // std::remove_cv, std::remove_reference, std::decay, ...
#include <tuple>        // std::tuple
#include <functional>   // std::function
#include <cstddef>      // size_t

namespace capo {

////////////////////////////////////////////////////////////////

/*
    Type qualifiers & reference removing
*/

template <typename T>
struct underlying : std::remove_cv<typename std::remove_reference<T>::type> {};

/*
    Functor & closure checking
*/

struct is_functor_
{
    template <typename T> static std::true_type  check(decltype(&T::operator())*);
    template <typename T> static std::false_type check(...);
};

template <typename T>
struct is_functor : decltype(is_functor_::check<T>(nullptr)) {};

template <typename T, bool = std::is_function<typename std::remove_pointer<T>::type>::value || is_functor<T>::value>
struct is_closure_;
template <typename T> struct is_closure_<T, true>  : std::true_type  {};
template <typename T> struct is_closure_<T, false> : std::false_type {};

template <typename T>
struct is_closure : is_closure_<typename underlying<T>::type> {};

/*
    Function type traits
*/

namespace detail_function_traits_ {

template <typename F, bool = is_closure<F>::value>
struct impl_
{
    enum : size_t { arity = 0 };
    typedef void type;
    typedef void return_type;
    typedef std::tuple<> parameters;
};

// check functor

template <typename F>
struct impl_<F, true>
     : impl_<decltype(&F::operator())>
{};

// check pointer

template <typename T, bool B>
struct impl_<T*, B>
     : impl_<T>
{};

// check function pointer

template <typename R, typename... P>
struct impl_<R(*)(P...), true>
     : impl_<R(P...)>
{};

template <typename R, typename... P>
struct impl_<R(*)(P..., ...), true>
     : impl_<R(P..., ...)>
{};

// check member function pointer

#pragma push_macro("CAPO_FUNCTION_TRAITS_IMPL__")
#undef  CAPO_FUNCTION_TRAITS_IMPL__
#define CAPO_FUNCTION_TRAITS_IMPL__(...)                \
    template <typename R, typename C, typename... P>    \
    struct impl_<R(C::*)(P...) __VA_ARGS__, false>      \
         : impl_<R(P...)>                               \
    {};                                                 \
    template <typename R, typename C, typename... P>    \
    struct impl_<R(C::*)(P..., ...) __VA_ARGS__, false> \
         : impl_<R(P..., ...)>                          \
    {};

CAPO_FUNCTION_TRAITS_IMPL__()
CAPO_FUNCTION_TRAITS_IMPL__(const)
CAPO_FUNCTION_TRAITS_IMPL__(volatile)
CAPO_FUNCTION_TRAITS_IMPL__(const volatile)

#pragma pop_macro("CAPO_FUNCTION_TRAITS_IMPL__")

// check std::function

template <typename R, typename... P>
struct impl_<std::function<R(P...)>, true>
     : impl_<R(P...)>
{};

template <typename R, typename... P>
struct impl_<std::function<R(P..., ...)>, true>
     : impl_<R(P..., ...)>
{};

// check function type

template <typename R, typename... P>
struct impl_<R(P...), true>
{
    enum : size_t
    {
        arity = sizeof...(P)
    };
    typedef R type(P...);
    typedef R return_type;
    typedef std::tuple<P...> parameters;
};

template <typename R, typename... P>
struct impl_<R(P..., ...), true>
{
    enum : size_t
    {
        arity = sizeof...(P)
    };
    typedef R type(P..., ...);
    typedef R return_type;
    typedef std::tuple<P...> parameters;
};

} // namespace detail_function_traits_

template <typename F>
struct function_traits
     : detail_function_traits_::impl_<typename std::decay<F>::type>
{};

////////////////////////////////////////////////////////////////

} // namespace capo