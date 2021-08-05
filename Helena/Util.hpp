#ifndef HELENA_UTIL_HPP
#define HELENA_UTIL_HPP

#include <Helena/Defines.hpp>
#include <Helena/Internal.hpp>
#include <Helena/Hash.hpp>

#include <cstdint>
#include <chrono>
#include <string_view>
#include <type_traits>
#include <optional>

namespace Helena::Util
{
    [[nodiscard]] inline constexpr std::string_view GetSourceName(const std::string_view file, const std::string_view delimeter) noexcept;

    inline void Sleep(const std::uint64_t milliseconds);

    template <typename Rep, typename Period>
    inline void Sleep(const std::chrono::duration<Rep, Period>& time);

    template <typename T>
    requires(Internal::is_scoped_enum_v<T>)
    inline constexpr decltype(auto) Cast(T value) noexcept {
        return static_cast<std::underlying_type_t<T>>(value);
    }
}

#include <Helena/Util.ipp>

#endif // HELENA_UTIL_HPP
