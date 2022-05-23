#ifndef HELENA_TRAITS_FUNCTIONINFO_HPP
#define HELENA_TRAITS_FUNCTIONINFO_HPP

#include <tuple>
#include <type_traits>
#include <cstddef>

namespace Helena::Traits {
    template <typename Fn> // primary template
    struct Function : public Function<decltype(&std::remove_reference<Fn>::type::operator())> { };

    template <typename ClassType, typename ReturnType, typename... Arguments>
    struct Function<ReturnType(ClassType::*)(Arguments...) const> : Function<ReturnType(*)(Arguments...)> { };

    template <typename ClassType, typename ReturnType, typename... Arguments>
    struct Function<ReturnType(ClassType::*)(Arguments...)> : Function<ReturnType(*)(Arguments...)> { };

    template <typename ReturnType, typename... Arguments>
    struct Function<ReturnType(*)(Arguments...)> {
        using Return = ReturnType;

        template <std::size_t Index>
        using Arg = typename std::tuple_element<Index, std::tuple<Arguments...>>::type;

        static constexpr std::size_t Args = sizeof...(Arguments);
    };
}

#endif // HELENA_TRAITS_FUNCTIONINFO_HPP