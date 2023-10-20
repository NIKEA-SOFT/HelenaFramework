#ifndef HELENA_TRAITS_CONSTRUCTIBLE_HPP
#define HELENA_TRAITS_CONSTRUCTIBLE_HPP

#include <type_traits>
#include <concepts>
#include <utility>

namespace Helena::Traits
{
    template <typename T, typename... Args>
    concept ConstructibleFrom = std::constructible_from<T, Args...>;

    template <typename T, typename... Args>
    concept ConstructibleAggregateFrom = std::conditional_t<std::is_aggregate_v<T>, std::bool_constant<requires(Args&&... args) {
            T{std::forward<Args>(args)...};
        }>, std::bool_constant<ConstructibleFrom<T, Args...>>>::value;
}

#endif // HELENA_TRAITS_CONSTRUCTIBLE_HPP
