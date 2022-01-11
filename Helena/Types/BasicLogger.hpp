#ifndef HELENA_TYPES_BASICLOGGER_HPP
#define HELENA_TYPES_BASICLOGGER_HPP

#include <Helena/Dependencies/Fmt.hpp>
#include <Helena/Types/SourceLocation.hpp>

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
    struct Exception {
        [[nodiscard]] static consteval auto GetPrefix() noexcept;
        [[nodiscard]] static consteval auto GetStyle() noexcept;
    };
}

namespace Helena::Types
{
    class BasicLogger
    {
        [[nodiscard]] static bool MakeColor(fmt::memory_buffer& buffer, const fmt::text_style style)
        {
            bool has_style{};
            if(style.has_emphasis()) {
                has_style = true;
                const auto emphasis = fmt::detail::make_emphasis<char>(style.get_emphasis());
                buffer.append(emphasis.begin(), emphasis.end());
            }

            if(style.has_foreground()) {
                has_style = true;
                const auto foreground = fmt::detail::make_foreground_color<char>(style.get_foreground());
                buffer.append(foreground.begin(), foreground.end());
            }

            if(style.has_background()) {
                has_style = true;
                const auto background = fmt::detail::make_background_color<char>(style.get_background());
                buffer.append(background.begin(), background.end());
            }

            return has_style;
        }

        static void EndColor(fmt::memory_buffer& buffer) noexcept {
            fmt::detail::reset_color<char>(buffer);
        }

    public:
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

        template <Helena::Traits::DefinitionLogger Prefix>
        struct Formater {
            template <std::convertible_to<std::string_view> T>
            constexpr Formater(T&& msg, const SourceLocation location = SourceLocation::Create()) noexcept
                : m_Location{location}
                , m_Msg{msg} {}

            Types::SourceLocation m_Location;
            std::string_view m_Msg;
        };

        [[nodiscard]] static consteval auto CreatePrefix(const std::string_view prefix) noexcept {
            return prefix;
        }

        [[nodiscard]] static consteval auto CreateStyle(const Color color) noexcept {
            return fmt::text_style{fmt::fg(static_cast<fmt::terminal_color>(color))};
        }

        [[nodiscard]] static consteval auto CreateStyle(const Color color, const Color background) noexcept {
            return fmt::text_style(fmt::fg(static_cast<fmt::terminal_color>(color)) | fmt::bg(static_cast<fmt::terminal_color>(background)));
        }

    public:
        template <typename Logger, typename... Args>
        BasicLogger(const Formater<Logger>& format, Args&&... args) {
            PrintConsole<Logger>(format, std::forward<Args>(args)...);
        }

    private:
        template <Helena::Traits::DefinitionLogger Logger, typename... Args>
        static void PrintConsole(const Formater<Logger> format, [[maybe_unused]] Args&&... args)
        {
            constexpr auto formatex = fmt::string_view("[{:%Y.%m.%d %H:%M:%S}][{}:{}]{} ");
            const auto msg = fmt::string_view{format.m_Msg};
            const auto time = fmt::localtime(std::time(nullptr));
            bool has_style = false;

            fmt::memory_buffer buffer{};
            try {
                constexpr auto style = Logger::GetStyle();
                has_style = MakeColor(buffer, style);
                const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Logger::GetPrefix());
                fmt::detail::vformat_to(buffer, formatex, args1);
                const auto args2 = fmt::make_format_args(std::forward<Args>(args)...);
                fmt::detail::vformat_to(buffer, msg, args2);
            } catch(const fmt::format_error&) {
                buffer.clear();

                constexpr auto style = Log::Exception::GetStyle();
                has_style = MakeColor(buffer, style);
                const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Log::Exception::GetPrefix());
                fmt::detail::vformat_to(buffer, formatex, args1);
                fmt::detail::vformat_to(buffer, fmt::string_view{
                    "\n----------------------------------------\n"
                    "|| Error: format syntax invalid!\n"
                    "|| Format: {}"
                    "\n----------------------------------------"
                    }, fmt::make_format_args(msg));
            } catch(const std::bad_alloc&) {
                buffer.clear();

                constexpr auto style = Log::Exception::GetStyle();
                has_style = MakeColor(buffer, style);
                const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Log::Exception::GetPrefix());
                fmt::detail::vformat_to(buffer, formatex, args1);
                fmt::detail::vformat_to(buffer, fmt::string_view{
                    "\n----------------------------------------\n"
                    "|| Error: alloc memory failed!\n"
                    "|| Format: {}"
                    "\n----------------------------------------"
                    }, fmt::make_format_args(msg));
            }

            if(has_style) {
                EndColor(buffer);
            }

            buffer.push_back('\n');
            buffer.push_back('\0');

            std::fputs(buffer.data(), stdout);
        }
    };
}

namespace Helena::Log
{
    [[nodiscard]] inline consteval auto Exception::GetPrefix() noexcept {
        return Types::BasicLogger::CreatePrefix("[EXCEPTION]");
    }

    [[nodiscard]] inline consteval auto Exception::GetStyle() noexcept {
        return Types::BasicLogger::CreateStyle(Types::BasicLogger::Color::BrightWhite, Types::BasicLogger::Color::Red);
    }
}

#endif // HELENA_TYPES_BASICLOGGER_HPP