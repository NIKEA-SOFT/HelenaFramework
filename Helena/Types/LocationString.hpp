#ifndef HELENA_TYPES_LOCATIONSTRING_HPP
#define HELENA_TYPES_LOCATIONSTRING_HPP

#include <Helena/Types/SourceLocation.hpp>

#include <concepts>

namespace Helena::Types
{
    // TODO: MSVC bad optimize source location, need trick
    struct LocationString
    {
        constexpr LocationString(const SourceLocation location = SourceLocation::Create()) noexcept
            : m_Location{location}, m_Msg{} {}

        template <std::convertible_to<std::string_view> T>
        constexpr LocationString(T&& msg, const SourceLocation location = SourceLocation::Create()) noexcept
            : m_Location{location}, m_Msg{msg} {}

        SourceLocation m_Location;
        std::string_view m_Msg;
    };
}

#endif // HELENA_TYPES_LOCATIONSTRING_HPP