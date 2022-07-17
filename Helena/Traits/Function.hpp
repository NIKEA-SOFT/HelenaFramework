#ifndef HELENA_TRAITS_FUNCTIONINFO_HPP
#define HELENA_TRAITS_FUNCTIONINFO_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace Helena::Traits {
    template <typename Fn> // primary template
    struct Function : public Function<decltype(&std::remove_reference<Fn>::type::operator())> {};

    template <typename T, typename ReturnType, typename... Args>
    struct Function<ReturnType(T::*)(Args...) const> : Function<ReturnType(*)(Args...)> {};

    template <typename T, typename ReturnType, typename... Args>
    struct Function<ReturnType(T::*)(Args...)> : Function<ReturnType(*)(Args...)> {};

    template <typename ReturnType, typename... Args>
    struct Function<ReturnType(*)(Args...)> {
        static constexpr auto Size = sizeof...(Args);
        static constexpr auto Orphan = !Size;

        template <std::size_t Index>
        using Arg = std::tuple_element_t<Index, std::tuple<Args...>>;
        using Return = ReturnType;
    };
}

#endif // HELENA_TRAITS_FUNCTIONINFO_HPP