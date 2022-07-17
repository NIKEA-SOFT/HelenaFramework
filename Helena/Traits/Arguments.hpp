#ifndef HELENA_TRAITS_ARGUMENTS_HPP
#define HELENA_TRAITS_ARGUMENTS_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace Helena::Traits {
    template <typename... Args>
    struct Arguments {
        static constexpr auto Size = sizeof...(Args);
        static constexpr auto Orphan = !Size;
        static constexpr auto Single = (Size == 1);
        static constexpr auto Double = (Size == 2);

        template <std::size_t Index>
        using Get = std::tuple_element_t<Index, std::tuple<Args...>>;
    };
}

#endif // HELENA_TRAITS_ARGUMENTS_HPP
