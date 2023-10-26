#ifndef HELENA_UTIL_MATH_HPP
#define HELENA_UTIL_MATH_HPP

#include <limits>
#include <bit>

namespace Helena::Util
{
    class Math
    {
    public:
        static constexpr bool IsPowerOf2(std::size_t value) noexcept {
            return value && !(value & (value - 1));
        }

        static constexpr auto PowerOf2(std::size_t value) noexcept {
            const std::size_t values[]{(value | (value >> 1) | (value >> 2) | (value >> 4) | (value >> 16)) + 1, value};
            return values[IsPowerOf2(value)];
        }

        static constexpr auto Log2(std::size_t value) noexcept {
            return std::numeric_limits<decltype(value)>::digits - std::countl_zero(value) - 1;
        }
    };
}

#endif // HELENA_UTIL_MATH_HPP