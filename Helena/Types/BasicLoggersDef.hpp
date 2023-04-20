#ifndef HELENA_TYPES_BASICLOGGERSDEF_HPP
#define HELENA_TYPES_BASICLOGGERSDEF_HPP

#include <Helena/Dependencies/Fmt.hpp>
#include <Helena/Types/SourceLocation.hpp>
#include <Helena/Traits/Underlying.hpp>

#include <concepts>

namespace Helena::Traits
{
    template <typename T>
    concept DefinitionLogger = requires {
        T::Prefix;
        T::Style;
        requires    std::is_same_v<std::remove_const_t<decltype(T::Prefix)>, std::string_view> &&
                    std::is_same_v<std::remove_const_t<decltype(T::Style)>, fmt::text_style>;
    };
}

namespace Helena::Log
{
    enum class Color    : Traits::Underlying<fmt::terminal_color>
    {
        Black           = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::black),
        Red             = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::red),
        Green           = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::green),
        Yellow          = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::yellow),
        Blue            = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::blue),
        Magenta         = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::magenta),
        Cyan            = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::cyan),
        White           = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::white),
        BrightBlack     = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_black),
        BrightRed       = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_red),
        BrightGreen     = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_green),
        BrightYellow    = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_yellow),
        BrightBlue      = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_blue),
        BrightMagenta   = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_magenta),
        BrightCyan      = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_cyan),
        BrightWhite     = Traits::Underlying<fmt::terminal_color>(fmt::terminal_color::bright_white)
    };

    template <Helena::Traits::DefinitionLogger Prefix>
    struct Formater {
        template <std::convertible_to<std::string_view> T>
        constexpr Formater(const T& msg, const Types::SourceLocation location = Types::SourceLocation::Create()) noexcept
            : m_Location{location}
            , m_Msg{msg} {}

        Types::SourceLocation m_Location;
        std::string_view m_Msg;
    };

    [[nodiscard]] static constexpr auto CreatePrefix(const std::string_view prefix) noexcept {
        return prefix;
    }

    [[nodiscard]] static constexpr auto CreateStyle(const Log::Color color) noexcept {
        return fmt::text_style{fmt::fg(static_cast<fmt::terminal_color>(color))};
    }

    [[nodiscard]] static constexpr auto CreateStyle(const Log::Color color, const Log::Color background) noexcept {
        return fmt::text_style{fmt::fg(static_cast<fmt::terminal_color>(color)) | fmt::bg(static_cast<fmt::terminal_color>(background))};
    }

    struct Benchmark {
        static constexpr auto Prefix = CreatePrefix("[BENCHMARK][FUNCTION:");
        static constexpr auto Style  = CreateStyle(Color::BrightMagenta);
    };

    struct Debug {
        static constexpr auto Prefix = CreatePrefix("[DEBUG]");
        static constexpr auto Style  = CreateStyle(Color::BrightBlue);
    };

    struct Info {
        static constexpr auto Prefix = CreatePrefix("[INFO]");
        static constexpr auto Style  = CreateStyle(Color::BrightGreen);
    };

    struct Notice {
        static constexpr auto Prefix = CreatePrefix("[NOTICE]");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite);
    };

    struct Warning {
        static constexpr auto Prefix = CreatePrefix("[WARNING]");
        static constexpr auto Style  = CreateStyle(Color::BrightYellow);
    };

    struct Error {
        static constexpr auto Prefix = CreatePrefix("[ERROR]");
        static constexpr auto Style  = CreateStyle(Color::BrightRed);
    };

    struct Fatal {
        static constexpr auto Prefix = CreatePrefix("[FATAL]");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Exception {
        static constexpr auto Prefix = CreatePrefix("[EXCEPTION]");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Assert {
        static constexpr auto Prefix = CreatePrefix("[ASSERT]");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Memory {
        static constexpr auto Prefix = CreatePrefix("[MEMORY]");
        static constexpr auto Style  = CreateStyle(Color::BrightCyan);
    };

    struct Shutdown {
        static constexpr auto Prefix = CreatePrefix("[SHUTDOWN]");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);
    };
}

#endif // HELENA_TYPES_BASICLOGGERSDEF_HPP