#ifndef HELENA_CONCURRENCY_INTERNAL_IPP
#define HELENA_CONCURRENCY_INTERNAL_IPP

#include <Helena/Concurrency/Internal.hpp>

#include <utility>

namespace Helena::Concurrency::Internal {

    template <typename T, typename>
    [[nodiscard]] inline constexpr T log2(const T value) noexcept {
        return ((value < 2) ? 0 : 1 + log2(value / 2));
    }

    template <typename T, typename>
    [[nodiscard]] inline constexpr T increment(const T value) noexcept {
        return value + 1u;
    }

    template <typename T, typename>
    [[nodiscard]] inline constexpr T decrement(const T value) noexcept {
        return value - 1u;
    }

    template <typename T, typename>
    [[nodiscard]] inline constexpr T or_equal(const T value, const T align) noexcept {
        return (value | value >> align);
    }

    template <typename T, typename... Args, typename>
    [[nodiscard]] inline constexpr T or_equal(const T value, const T align, Args&&... rest) noexcept {
        return or_equal(or_equal(value, align), std::forward<Args>(rest)...);
    }

    template <typename T, typename>
    [[nodiscard]] inline constexpr T round_up_to_power_of_2(const T value) noexcept {
        return increment(or_equal(decrement(value), 1u, 2u, 4u, 8u, 16u));
    }
}

#endif // HELENA_CONCURRENCY_INTERNAL_IPP
