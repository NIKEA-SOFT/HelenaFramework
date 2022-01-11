#ifndef HELENA_TRAITS_ANYOF_HPP
#define HELENA_TRAITS_ANYOF_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
	template <typename T, typename... Types>
	struct IsAnyOf : std::disjunction<std::conjunction<std::is_same<T, Types>, std::is_same<Types, T>>...> {};

	template <typename T, typename... Types>
	concept AnyOf = (std::same_as<T, Types> || ...);
}

#endif // HELENA_TRAITS_ANYOF_HPP