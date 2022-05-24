#ifndef HELENA_TRAITS_PAIR_HPP
#define HELENA_TRAITS_PAIR_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename T, typename = void>
    struct IsPair : std::false_type {};

    template <typename T>
    struct IsPair<T, std::void_t<typename T::first_type, typename T::second_type>> : std::true_type {};

	template <typename T>
	concept Pair = IsPair<T>::value;
}


#endif // HELENA_TRAITS_PAIR_HPP