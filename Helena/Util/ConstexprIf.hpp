#ifndef HELENA_UTIL_CONSTEXPRIF_HPP
#define HELENA_UTIL_CONSTEXPRIF_HPP

#include <utility>

namespace Helena::Util
{
    template <bool Condition, typename Success, typename Failure>
    decltype(auto) ConstexprIf(Success&& lhs, Failure&& rhs) noexcept {
        if constexpr (Condition) {
            return std::forward<Success>(lhs);
        } else {
            return std::forward<Failure>(rhs);
        }
    }
}

#endif // HELENA_UTIL_CONSTEXPRIF_HPP