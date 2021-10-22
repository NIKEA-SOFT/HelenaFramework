#ifndef HELENA_UTIL_CAST_HPP
#define HELENA_UTIL_CAST_HPP

#pragma message( "Compiling " __FILE__ )

#include <Helena/Traits/ScopedEnum.hpp>

#include <string_view>
#include <optional>

namespace Helena::Util
{
    template <Helena::Traits::ScopedEnum T> 
    [[nodiscard]] constexpr auto Cast(const T value) noexcept {
        return static_cast<std::underlying_type_t<T>>(value);
    }

    template <typename T> 
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
    [[nodiscard]] constexpr auto Cast(std::string_view str) noexcept;
}

#include <Helena/Util/Cast.ipp>

#endif // HELENA_CORE_UTIL_CAST_HPP