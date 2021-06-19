#ifndef HELENA_CONCURRENCY_INTERNAL_HPP
#define HELENA_CONCURRENCY_INTERNAL_HPP

#include <Helena/Defines.hpp>

#include <type_traits>

namespace Helena::Concurrency::Internal {
#ifdef __cpp_lib_hardware_interference_size
    constexpr std::size_t cache_line = std::hardware_destructive_interference_size;
#else
    constexpr std::size_t cache_line = 64;
#endif

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    HF_NODISCARD constexpr T log2(const T value) HF_NOEXCEPT;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    HF_NODISCARD constexpr T increment(const T value) HF_NOEXCEPT;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    HF_NODISCARD constexpr T decrement(const T value) HF_NOEXCEPT;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    HF_NODISCARD constexpr T or_equal(const T value, const T align) HF_NOEXCEPT;

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_integral_v<T>>>
    HF_NODISCARD constexpr T or_equal(const T value, const T align, Args&&... rest) HF_NOEXCEPT;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    HF_NODISCARD constexpr T round_up_to_power_of_2(const T value) HF_NOEXCEPT;
}

#include <Helena/Concurrency/Internal.ipp>

#endif // HELENA_CONCURRENCY_INTERNAL_HPP
