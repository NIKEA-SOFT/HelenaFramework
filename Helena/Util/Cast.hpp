#ifndef HELENA_UTIL_CAST_HPP
#define HELENA_UTIL_CAST_HPP

#include <Helena/Traits/ScopedEnum.hpp>

namespace Helena::Util
{
    template <Helena::Traits::ScopedEnum T> 
	[[nodiscard]] constexpr auto Cast(const T value) noexcept {
        return static_cast<std::underlying_type_t<T>>(value);
    }
}

#endif // HELENA_CORE_UTIL_CAST_HPP