#ifndef HELENA_CONCURRENCY_INTERNAL_HPP
#define HELENA_CONCURRENCY_INTERNAL_HPP

#include <type_traits>

namespace Helena::Internal {
#ifdef __cpp_lib_hardware_interference_size
    inline constexpr std::size_t cache_line = std::hardware_destructive_interference_size;
#else
    inline constexpr std::size_t cache_line = 64;
#endif

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    [[nodiscard]] inline constexpr T log2(const T value) noexcept;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    [[nodiscard]] inline constexpr T increment(const T value) noexcept;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    [[nodiscard]] inline constexpr T decrement(const T value) noexcept;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    [[nodiscard]] inline constexpr T or_equal(const T value, const T align) noexcept;

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_integral_v<T>>>
    [[nodiscard]] inline constexpr T or_equal(const T value, const T align, Args&&... rest) noexcept;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    [[nodiscard]] inline constexpr T round_up_to_power_of_2(const T value) noexcept;
}

#include <Helena/Concurrency/Internal.ipp>

#endif // HELENA_CONCURRENCY_INTERNAL_HPP
