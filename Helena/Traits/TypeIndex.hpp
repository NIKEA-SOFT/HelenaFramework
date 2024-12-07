#ifndef HELENA_TRAITS_TYPEINDEX_HPP
#define HELENA_TRAITS_TYPEINDEX_HPP

#include <cstddef>
#include <tuple>

namespace Helena::Traits
{
    template <typename... T>
    struct TypeList {};

    template <std::size_t Index, typename... T>
    using TypeIndex = std::tuple_element_t<Index, TypeList<T...>>;
}

#endif // HELENA_TRAITS_TYPEINDEX_HPP