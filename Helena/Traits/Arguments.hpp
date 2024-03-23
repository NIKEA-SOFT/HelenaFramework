#ifndef HELENA_TRAITS_ARGUMENTS_HPP
#define HELENA_TRAITS_ARGUMENTS_HPP

#include <Helena/Traits/TypeIndex.hpp>

namespace Helena::Traits
{
    template <typename... Args>
    struct Arguments {
        static constexpr auto Size = sizeof...(Args);
        static constexpr auto Orphan = !Size;
        static constexpr auto Single = (Size == 1);
        static constexpr auto Double = (Size == 2);

        template <std::size_t Index>
        requires (Index < Size)
        using Get = typename TypeIndex<Index, Args...>::Type;
    };

    template <template <typename... > typename T, typename... Args>
    struct Arguments<T<Args...>> : Arguments<Args...> {};
}

#endif // HELENA_TRAITS_ARGUMENTS_HPP
