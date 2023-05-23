#ifndef HELENA_TRAITS_UNIQUETYPES_HPP
#define HELENA_TRAITS_UNIQUETYPES_HPP

#include <type_traits>

namespace Helena::Traits
{
    template <typename T, typename... Other>
    inline constexpr auto UniqueTypes = (!std::is_same_v<T, Other> && ...) && UniqueTypes<Other...>;

    template <typename T>
    inline constexpr auto UniqueTypes<T> = true;
}

#endif // HELENA_TRAITS_UNIQUETYPES_HPP