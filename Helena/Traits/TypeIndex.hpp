#ifndef HELENA_TRAITS_TYPEINDEX_HPP
#define HELENA_TRAITS_TYPEINDEX_HPP

#include <type_traits>

namespace Helena::Traits
{
    template <std::size_t, typename...>
    struct TypeIndex;

    template <typename T, typename... Other>
    struct TypeIndex<0, T, Other...> {
        using Type = T;
        static constexpr std::size_t Index{};
    };

    template <std::size_t Idx, typename T, typename... Other>
    requires (Idx < sizeof...(Other) + 1)
    struct TypeIndex<Idx, T, Other...> {
        using Type = typename TypeIndex<Idx - 1, Other...>::Type;
        static constexpr std::size_t Index = Idx;
    };
}

#endif // HELENA_TRAITS_TYPEINDEX_HPP