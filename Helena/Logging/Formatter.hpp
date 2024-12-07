#ifndef HELENA_LOGGING_FORMATTER_HPP
#define HELENA_LOGGING_FORMATTER_HPP

#include <Helena/Types/SourceLocation.hpp>
#include <string>
#include <string_view>

namespace Helena::Logging
{
    // Class with an implicit constructor for working with formatting of logging
    template <typename Char>
    struct Formatter
    {
        constexpr Formatter(const Char* message, const Types::SourceLocation location = Types::SourceLocation::Create()) noexcept
            : m_Location{location}
            , m_Message{message} {}

        constexpr Formatter(const std::basic_string_view<Char> message, const Types::SourceLocation location = Types::SourceLocation::Create()) noexcept
            : m_Location{location}
            , m_Message{message} {}

        // the message must meet lvalue reference requirements because the formatter uses a string view for m_Message member
        template <typename Traits, typename Allocator>
        constexpr Formatter(const std::basic_string<Char, Traits, Allocator>& message, const Types::SourceLocation location = Types::SourceLocation::Create()) noexcept
            : m_Location{location}
            , m_Message{message} {}

        [[nodiscard]] constexpr decltype(auto) File() const noexcept {
            return m_Location.GetFile();
        }

        [[nodiscard]] constexpr decltype(auto) Function() const noexcept {
            return m_Location.GetFunction();
        }

        [[nodiscard]] constexpr decltype(auto) Line() const noexcept {
            return m_Location.GetLine();
        }

        [[nodiscard]] constexpr auto Message() const noexcept {
            return m_Message;
        }

        const Types::SourceLocation m_Location;
        const std::basic_string_view<Char> m_Message;
    };
}

#endif // HELENA_LOGGING_FORMATTER_HPP
