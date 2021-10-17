#ifndef HELENA_TRAITS_POWEROF2_HPP
#define HELENA_TRAITS_POWEROF2_HPP

#include <cstdint>
#include <type_traits>

namespace Helena::Traits 
{
    template <std::size_t Value>
    struct PowerOf2
    {
    private:
        [[nodiscard]] static constexpr std::size_t Inc(const std::size_t value) noexcept {
            return value + 1u;
        }

        [[nodiscard]] static constexpr std::size_t Dec(const std::size_t value) noexcept {
            return value - 1u;
        }

        [[nodiscard]] static constexpr std::size_t Equal(const std::size_t value, const std::size_t align) noexcept {
            return (value | value >> align);
        }

        template <typename... Rounds>
        [[nodiscard]] static constexpr std::size_t Equal(const std::size_t value, const std::size_t align, Rounds&&... rounds) noexcept {
            return Equal(Equal(value, align), std::forward<Rounds>(rounds)...);
        }

        [[nodiscard]] static constexpr std::size_t GetPowerOf2(const std::size_t value) noexcept {
            return Inc(Equal(Dec(value), 1u, 2u, 4u, 8u, 16u));
        }

    public:
        static constexpr auto value = GetPowerOf2(Value);
    };
}

#endif // HELENA_TRAITS_POWEROF2_HPP