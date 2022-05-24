#ifndef HELENA_TYPES_BASICLOGGERSDEF_HPP
#define HELENA_TYPES_BASICLOGGERSDEF_HPP

#include <Helena/Dependencies/Fmt.hpp>
#include <Helena/Types/SourceLocation.hpp>

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename T>
    concept DefinitionLogger = requires {
        //{ T::is_logging }   -> Traits::AnyOf<std::true_type, std::false_type>;
        { T::GetPrefix() }  -> std::same_as<std::string_view>;
        { T::GetStyle() }   -> std::same_as<fmt::text_style>;
    };
}

namespace Helena::Log
{
    enum class Color : std::underlying_type_t<fmt::terminal_color> {
        Black = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::black),
        Red = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::red),
        Green = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::green),
        Yellow = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::yellow),
        Blue = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::blue),
        Magenta = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::magenta),
        Cyan = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::cyan),
        White = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::white),
        BrightBlack = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_black),
        BrightRed = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_red),
        BrightGreen = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_green),
        BrightYellow = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_yellow),
        BrightBlue = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_blue),
        BrightMagenta = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_magenta),
        BrightCyan = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_cyan),
        BrightWhite = std::underlying_type_t<fmt::terminal_color>(fmt::terminal_color::bright_white)
    };

    [[nodiscard]] static constexpr auto CreatePrefix(const std::string_view prefix) noexcept {
        return prefix;
    }

    [[nodiscard]] static constexpr auto CreateStyle(const Log::Color color) noexcept {
        return fmt::text_style{fmt::fg(static_cast<fmt::terminal_color>(color))};
    }

    [[nodiscard]] static constexpr auto CreateStyle(const Log::Color color, const Log::Color background) noexcept {
        return fmt::text_style(fmt::fg(static_cast<fmt::terminal_color>(color)) | fmt::bg(static_cast<fmt::terminal_color>(background)));
    }

    template <Helena::Traits::DefinitionLogger Prefix>
    struct Formater {
        template <std::convertible_to<std::string_view> T>
        constexpr Formater(T&& msg, const Types::SourceLocation location = Types::SourceLocation::Create()) noexcept
            : m_Location{location}
            , m_Msg{msg} {}

        Types::SourceLocation m_Location;
        std::string_view m_Msg;
    };

    struct Debug {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[DEBUG]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightBlue);
        }
    };

    struct Info {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[INFO]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightGreen);
        }
    };

    struct Notice {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[NOTICE]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightWhite);
        }
    };

    struct Warning {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[WARNING]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightYellow);
        }
    };

    struct Error {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[ERROR]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightRed);
        }
    };

    struct Fatal {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[FATAL]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightWhite, Color::Red);
        }
    };

    struct Exception {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[EXCEPTION]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightWhite, Color::Red);
        }
    };

    struct Assert {
        [[nodiscard]] static constexpr auto GetPrefix() noexcept {
            return CreatePrefix("[ASSERT]");
        }

        [[nodiscard]] static constexpr auto GetStyle() noexcept {
            return CreateStyle(Color::BrightWhite, Color::Red);
        }
    };
}

#endif // HELENA_TYPES_BASICLOGGERSDEF_HPP