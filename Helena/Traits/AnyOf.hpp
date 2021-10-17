#ifndef HELENA_TRAITS_PAIR_HPP
#define HELENA_TRAITS_PAIR_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
	template <typename T, typename... Args>
	struct IsAnyOf : std::disjunction<std::is_same<T, Args>...> {};

	template <typename T, typename... Args>
	concept AnyOf = IsAnyOf<T, Args...>::value;
}

#endif // HELENA_TRAITS_PAIR_HPP