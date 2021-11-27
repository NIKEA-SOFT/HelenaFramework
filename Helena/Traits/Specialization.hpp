#ifndef HELENA_TRAITS_SPECIALIZATION_HPP
#define HELENA_TRAITS_SPECIALIZATION_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename, template <typename...> typename>
    struct SpecializationOf : std::false_type {};

    template <template <typename...> typename Primary, typename... Args>
    struct SpecializationOf<Primary<Args...>, Primary> : std::true_type {};

    template <template <typename...> typename Primary, typename... Args>
    concept Specialization = SpecializationOf<Primary<Args...>, Primary>::value;
}


#endif // HELENA_TRAITS_SPECIALIZATION_HPP