#ifndef HELENA_TRAITS_MAP_HPP
#define HELENA_TRAITS_MAP_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename T, typename = void>
    struct IsMap : std::false_type {};

    template <typename T>
    struct IsMap<T, std::void_t<typename T::key_type, typename T::mapped_type>> : std::true_type {};

    template <typename T>
    concept Map = IsMap<T>::value;
}

#endif // HELENA_TRAITS_MAP_HPP