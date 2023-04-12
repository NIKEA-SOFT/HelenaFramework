#ifndef HELENA_UTIL_CAST_HPP
#define HELENA_UTIL_CAST_HPP

#include <Helena/Traits/ScopedEnum.hpp>
#include <Helena/Types/FixedBuffer.hpp>

#include <charconv>
#include <optional>

namespace Helena::Util
{
    template <Traits::ScopedEnum T>
    [[nodiscard]] constexpr auto Cast(const T value) noexcept {
        return static_cast<std::underlying_type_t<T>>(value);
    }

    template <typename T> 
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
    [[nodiscard]] auto Cast(const std::string_view str) noexcept {
        T value{};
        const auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), value);
        return err != std::errc{} ? std::nullopt : std::make_optional<T>(value);
    }

    template <std::size_t Capacity = 24, typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
    [[nodiscard]] auto Cast(const T value) noexcept {
        static_assert(!std::is_same_v<std::remove_cvref_t<T>, bool>, "std::to_chars overload for bool deleted");
        char m_Buffer[Capacity];
        const auto [ptr, err] = std::to_chars(m_Buffer, m_Buffer + Capacity, value);
        return err != std::errc{} ? std::nullopt : std::make_optional<Types::FixedBuffer<Capacity>>(m_Buffer, ptr - m_Buffer);
    }
}

#endif // HELENA_UTIL_CAST_HPP