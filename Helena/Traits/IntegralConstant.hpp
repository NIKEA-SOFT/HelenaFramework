#ifndef HELENA_TRAITS_INTEGRALCONSTANT_HPP
#define HELENA_TRAITS_INTEGRALCONSTANT_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename T>
    struct IsIntegralConstant : std::false_type {};

    template <typename T, T Value>
    struct IsIntegralConstant<std::integral_constant<T, Value>> : std::true_type {};

    template <typename T>
    concept IntegralConstant = IsIntegralConstant<T>::value;
}


#endif // HELENA_TRAITS_INTEGRALCONSTANT_HPP