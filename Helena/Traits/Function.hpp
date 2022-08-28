#ifndef HELENA_TRAITS_FUNCTIONINFO_HPP
#define HELENA_TRAITS_FUNCTIONINFO_HPP

#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/Identity.hpp>

namespace Helena::Traits
{
    template <typename Fn>
    struct Function : Function<decltype(&Fn::operator())> {};

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) const> : Identity<Ret(T::*)(Args...) const>, Arguments<Args...> {
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...)> : Identity<Ret(T::*)(Args...)>, Arguments<Args...> {
        using Return = Ret;
    };

    template <typename Ret, typename... Args>
    struct Function<Ret(*)(Args...)> : Identity<Ret(*)(Args...)>, Arguments<Args...> {
        using Return = Ret;
    };
}

#endif // HELENA_TRAITS_FUNCTIONINFO_HPP