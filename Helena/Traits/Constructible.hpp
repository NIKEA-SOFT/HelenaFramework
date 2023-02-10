#ifndef HELENA_TRAITS_CONSTRUCTIBLE_HPP
#define HELENA_TRAITS_CONSTRUCTIBLE_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename T, typename... Args>
    concept ConstructibleFrom = std::constructible_from<T, Args...>;

    template <typename T, typename... Args>
    concept ConstructibleAggregateFrom = ConstructibleFrom<T, Args...> || requires(Args&&... args) { T{std::forward<Args>(args)...}; };
}

#endif // HELENA_TRAITS_CONSTRUCTIBLE_HPP
