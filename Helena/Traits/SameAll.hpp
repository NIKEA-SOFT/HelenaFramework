#ifndef HELENA_TRAITS_SAMEALL_HPP
#define HELENA_TRAITS_SAMEALL_HPP

#include <type_traits>

namespace Helena::Traits {
    template <typename First, typename... Other>
    inline constexpr auto SameAll = std::conjunction_v<std::is_same<First, Other>...>;
}

#endif // HELENA_TRAITS_SAMEALL_HPP