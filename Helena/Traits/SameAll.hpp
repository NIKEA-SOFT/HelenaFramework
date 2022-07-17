#ifndef HELENA_TRAITS_SAMEALL_HPP
#define HELENA_TRAITS_SAMEALL_HPP

#include <type_traits>

namespace Helena::Traits {
    template <bool StaticAssert, typename First, typename... Other>
    struct SameAll {
        using Type = First;
        using Same = std::conjunction<std::is_same<First, Other>...>;
        static constexpr auto Result = Same::value;

        static_assert(!StaticAssert || Result,
            "N4687 26.3.7.2 [array.cons]/2: "
            "Requires: (is_same_v<T, U> && ...) is true. Otherwise the program is ill-formed.");
    };
}

#endif // HELENA_TRAITS_SAMEALL_HPP