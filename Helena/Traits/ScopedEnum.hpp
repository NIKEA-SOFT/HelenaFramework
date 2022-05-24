#ifndef HELENA_TRAITS_SCOPEDENUM_HPP
#define HELENA_TRAITS_SCOPEDENUM_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename T>
    struct IsScopedEnum : std::conjunction<std::is_enum<T>, std::negation<std::is_convertible<T, std::int32_t>>>::type {};

    template <typename T>
    concept ScopedEnum = IsScopedEnum<T>::value;
}


#endif // HELENA_TRAITS_SCOPEDENUM_HPP