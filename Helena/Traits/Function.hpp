#ifndef HELENA_TRAITS_FUNCTIONINFO_HPP
#define HELENA_TRAITS_FUNCTIONINFO_HPP

#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/Identity.hpp>

namespace Helena::Traits
{
    template <typename Fn>
    struct Function;

    template <typename Fn>
    struct Function : Function<decltype(&Fn::operator())>{};

    template <typename Ret, typename... Args>
    struct Function<Ret(Args...)> : Identity<Ret(Args...)>, Arguments<Args...> {
        using Class = void;
        using Return = Ret;
    };

    template <typename Ret, typename... Args>
    struct Function<Ret(Args...) noexcept> : Identity<Ret(Args...) noexcept>, Arguments<Args...> {
        using Class = void;
        using Return = Ret;
    };

    template <typename Ret, typename... Args>
    struct Function<Ret(*)(Args...)> : Identity<Ret(*)(Args...)>, Arguments<Args...> {
        using Class = void;
        using Return = Ret;
    };

    template <typename Ret, typename... Args>
    struct Function<Ret(*)(Args...) noexcept> : Identity<Ret(*)(Args...) noexcept>, Arguments<Args...> {
        using Class = void;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...)> : Identity<Ret(T::*)(Args...)>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) const> : Identity<Ret(T::*)(Args...) const>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) const volatile> : Identity<Ret(T::*)(Args...) const volatile>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) const noexcept> : Identity<Ret(T::*)(Args...) const noexcept>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) const volatile noexcept> : Identity<Ret(T::*)(Args...) const volatile noexcept>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) volatile> : Identity<Ret(T::*)(Args...) volatile>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) volatile noexcept> : Identity<Ret(T::*)(Args...) volatile noexcept>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };

    template <typename Ret, typename T, typename... Args>
    struct Function<Ret(T::*)(Args...) noexcept> : Identity<Ret(T::*)(Args...) noexcept>, Arguments<Args...> {
        using Class = T;
        using Return = Ret;
    };
}

#endif // HELENA_TRAITS_FUNCTIONINFO_HPP