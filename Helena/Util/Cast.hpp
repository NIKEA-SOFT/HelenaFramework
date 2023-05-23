#ifndef HELENA_UTIL_CAST_HPP
#define HELENA_UTIL_CAST_HPP

#include <Helena/Traits/ScopedEnum.hpp>
#include <Helena/Types/FixedBuffer.hpp>

#include <charconv>
#include <optional>

namespace Helena::Util
{
    /**
    * @brief Cast enum class to underlying type
    * @tparam T Enum class type
    * @param value Enum class key
    * @return Return a value of underlying type
    */
    template <Traits::ScopedEnum T>
    [[nodiscard]] constexpr auto Cast(const T value) noexcept {
        return static_cast<std::underlying_type_t<T>>(value);
    }

    /**
    * @brief Cast string with value to integral or floating type
    * @tparam T Cast type
    * @param str String with value
    * @return Return a std::optional<T> value
    */
    template <typename T> 
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
    [[nodiscard]] auto Cast(const std::string_view str) noexcept {
        T value{};
        const auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), value);
        return err != std::errc{} ? std::nullopt : std::make_optional<T>(value);
    }

    /**
    * @brief Cast integral or floating type to string
    * @tparam Capacity Default buffer capacity
    * @tparam T Type of value
    * @param value Value of type T
    * @return Return a Types::FixedBuffer<Capacity> value
    * @note FixedBuffer can be empty when cast fails, use fixedBuffer.Empty() for check result.
    */
    template <std::size_t Capacity = 24, typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
    [[nodiscard]] auto Cast(const T value) noexcept {
        static_assert(!std::is_same_v<std::remove_cvref_t<T>, bool>, "std::to_chars overload for bool deleted");
        Types::FixedBuffer<Capacity> data;
        const auto [ptr, err] = std::to_chars(data.m_Buffer, data.m_Buffer + Capacity, value);
        if(err != std::errc()) data.Clear();
        return data;
    }
}

#endif // HELENA_UTIL_CAST_HPP