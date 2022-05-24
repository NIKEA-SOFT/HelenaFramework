#ifndef HELENA_TRAITS_POWEROF2_HPP
#define HELENA_TRAITS_POWEROF2_HPP

#include <cstdint>
#include <type_traits>

namespace Helena::Traits
{
    template <std::size_t Size>
    struct PowerOf2
    {
    private:
        [[nodiscard]] static constexpr std::size_t Inc(const std::size_t size) noexcept {
            return size + 1u;
        }

        [[nodiscard]] static constexpr std::size_t Dec(const std::size_t size) noexcept {
            return size - 1u;
        }

        [[nodiscard]] static constexpr std::size_t Equal(const std::size_t size, const std::size_t align) noexcept {
            return (size | size >> align);
        }

        template <typename... Rounds>
        [[nodiscard]] static constexpr std::size_t Equal(const std::size_t size, const std::size_t align, Rounds&&... rounds) noexcept {
            return Equal(Equal(size, align), std::forward<Rounds>(rounds)...);
        }

        [[nodiscard]] static constexpr std::size_t GetPowerOf2(const std::size_t size) noexcept {
            return Inc(Equal(Dec(size), 1u, 2u, 4u, 8u, 16u));
        }

    public:
        static constexpr auto value = GetPowerOf2(Size);
    };
}

#endif // HELENA_TRAITS_POWEROF2_HPP