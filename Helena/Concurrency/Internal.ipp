#ifndef HELENA_CONCURRENCY_INTERNAL_IPP
#define HELENA_CONCURRENCY_INTERNAL_IPP

#include <Helena/Concurrency/Internal.hpp>

#include <utility>

namespace Helena::Concurrency::Internal {

    template <typename T, typename>
    HF_NODISCARD constexpr T log2(const T value) HF_NOEXCEPT {
        return ((value < 2) ? 0 : 1 + GetIndex(value / 2));
    }

    template <typename T, typename>
    HF_NODISCARD constexpr T increment(const T value) HF_NOEXCEPT {
        return value + 1u;
    }

    template <typename T, typename>
    HF_NODISCARD constexpr T decrement(const T value) HF_NOEXCEPT {
        return value - 1u;
    }

    template <typename T, typename>
    HF_NODISCARD constexpr T or_equal(const T value, const T align) HF_NOEXCEPT {
        return (value | value >> align);
    }

    template <typename T, typename... Args, typename>
    HF_NODISCARD constexpr T or_equal(const T value, const T align, Args&&... rest) HF_NOEXCEPT {
        return or_equal(or_equal(value, align), std::forward<Args>(rest)...);
    }

    template <typename T, typename>
    HF_NODISCARD constexpr T round_up_to_power_of_2(const T value) HF_NOEXCEPT {
        return increment(or_equal(decrement(value), 1u, 2u, 4u, 8u, 16u));
    }
}

#endif // HELENA_CONCURRENCY_INTERNAL_IPP
