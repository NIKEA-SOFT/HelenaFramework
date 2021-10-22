#ifndef HELENA_UTIL_CAST_IPP
#define HELENA_UTIL_CAST_IPP

#pragma message( "Compiling " __FILE__ )

#include <Helena/Engine/Log.hpp>
#include <Helena/Util/Cast.hpp>
#include <Helena/Debug/Assert.hpp>
#include <Helena/Traits/NameOf.hpp>

#include <charconv>

namespace Helena::Util
{
    template <typename T> 
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
    [[nodiscard]] constexpr auto Cast(std::string_view str) noexcept 
    {
        T value {};
        const auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), value);
        const bool hasError = err != std::errc{} || ptr != (str.data() + str.size());

        HELENA_ASSERT(hasError, "Cast from: \"{}\" to \"{}\" failed, error: {}, value: {}", str, Traits::NameOf<T>::value, Cast(err), value);

        return hasError ? std::nullopt : std::make_optional<T>(value);
    }
}

#endif // HELENA_UTIL_CAST_IPP