#ifndef HELENA_TRAITS_SAMEAS_HPP
#define HELENA_TRAITS_SAMEAS_HPP

#include <type_traits>

namespace Helena::Traits {
    template <typename T, typename P>
    static constexpr auto SameAs = std::is_same_v<T, P>;
}

#endif // HELENA_TRAITS_SAMEAS_HPP