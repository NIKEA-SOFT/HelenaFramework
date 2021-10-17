#ifndef HELENA_ENGINE_LOG_HPP
#define HELENA_ENGINE_LOG_HPP

#include <Helena/Dependencies/Fmt.hpp>
#include <Helena/Platform/Platform.hpp>
#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Util/Cast.hpp>
#include <Helena/Util/SourceLocation.hpp>

namespace Helena::Log
{
    enum class Color : std::uint8_t
    {
        Black           = Util::Cast(fmt::terminal_color::black),
        Red             = Util::Cast(fmt::terminal_color::red),
        Green           = Util::Cast(fmt::terminal_color::green),
        Yellow          = Util::Cast(fmt::terminal_color::yellow),
        Blue            = Util::Cast(fmt::terminal_color::blue),
        Magenta         = Util::Cast(fmt::terminal_color::magenta),
        Cyan            = Util::Cast(fmt::terminal_color::cyan),
        White           = Util::Cast(fmt::terminal_color::white),
        BrightBlack     = Util::Cast(fmt::terminal_color::bright_black),
        BrightRed       = Util::Cast(fmt::terminal_color::bright_red),
        BrightGreen     = Util::Cast(fmt::terminal_color::bright_green),
        BrightYellow    = Util::Cast(fmt::terminal_color::bright_yellow),
        BrightBlue      = Util::Cast(fmt::terminal_color::bright_blue),
        BrightMagenta   = Util::Cast(fmt::terminal_color::bright_magenta),
        BrightCyan      = Util::Cast(fmt::terminal_color::bright_cyan),
        BrightWhite     = Util::Cast(fmt::terminal_color::bright_white)
    };


    [[nodiscard]] inline consteval auto CreatePrefix(const std::string_view prefix) noexcept {
        return prefix;
    }

    [[nodiscard]] inline consteval auto CreateStyle(const Color color) noexcept {
        return fmt::text_style(fmt::fg(static_cast<fmt::terminal_color>(Util::Cast(color))));
    }

    [[nodiscard]] inline consteval auto CreateStyle(const Color color, const Color background) noexcept {
        return fmt::text_style(fmt::fg(static_cast<fmt::terminal_color>(Util::Cast(color))) | fmt::bg(static_cast<fmt::terminal_color>(background)));
    }


    namespace Details 
    {
        struct Default {
            //using is_logging = std::false_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightWhite);
            }
        };

        struct Debug {
            //using is_logging = std::false_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("[DEBUG]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightBlue);
            }
        };

        struct Info {
            //using is_logging = std::false_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("[INFO]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightGreen);
            }
        };

        struct Warning {
            //using is_logging = std::false_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("[WARNING]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightYellow);
            }
        };

        struct Error {
            //using is_logging = std::true_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("[ERROR]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightRed);
            }
        };

        struct Fatal {
            //using is_logging = std::true_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("[FATAL]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightWhite, Color::BrightRed);
            }
        };

        struct Exception {
            //using is_logging = std::true_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("[EXCEPTION]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightWhite, Color::Red);
            }
        };

        struct Assert {
            //using is_logging = std::true_type;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return CreatePrefix("[ASSERT]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return CreateStyle(Color::BrightWhite, Color::Red);
            }
        };

        [[nodiscard]] inline bool MakeColor(fmt::memory_buffer& buffer, const fmt::text_style& style) 
        {
            bool has_style {};

            if (style.has_emphasis()) {
                has_style = true;
                auto emphasis = fmt::detail::make_emphasis<char>(style.get_emphasis());
                buffer.append(emphasis.begin(), emphasis.end());
            }

            if (style.has_foreground()) {
                has_style = true;
                auto foreground = fmt::detail::make_foreground_color<char>(style.get_foreground());
                buffer.append(foreground.begin(), foreground.end());
            }

            if (style.has_background()) {
                has_style = true;
                auto background = fmt::detail::make_background_color<char>(style.get_background());
                buffer.append(background.begin(), background.end());
            }
            
            return has_style;
        }

        inline void EndColor(fmt::memory_buffer& buffer) {
            fmt::detail::reset_color<char>(buffer);
        }

        template <typename T>
        concept PrefixTraits = requires {
            //{ T::is_logging }   -> Traits::AnyOf<std::true_type, std::false_type>;
            { T::GetPrefix() }  -> std::same_as<std::string_view>;
            { T::GetStyle() }   -> std::same_as<fmt::text_style>;
        };
    }

    template <Details::PrefixTraits Prefix, typename... Args>
    void Console(const Util::SourceLocation& source, const fmt::string_view msg, Args&&... args)
    {
        constexpr auto formatex = fmt::string_view("[{:%Y.%m.%d %H:%M:%S}][{}:{}]{} ");
        const auto time = fmt::localtime(std::time(nullptr));

        fmt::memory_buffer buffer;

        try 
        {
            constexpr auto style    = Prefix::GetStyle();
            constexpr auto prefix   = Prefix::GetPrefix();

            const auto has_style = Details::MakeColor(buffer, style);

            fmt::detail::vformat_to(buffer, formatex, fmt::make_format_args(time, source.GetFile(), source.GetLine(), prefix));
            fmt::detail::vformat_to(buffer, msg, fmt::make_format_args(args...));

            if (has_style) {
                Details::EndColor(buffer);
            }

        } catch(const fmt::format_error&) {

            buffer.clear();
            const auto has_style = Details::MakeColor(buffer, Details::Exception::GetStyle());

            fmt::detail::vformat_to(buffer, formatex, fmt::make_format_args(time, source.GetFile(), source.GetLine(), Details::Exception::GetPrefix()));
            fmt::detail::vformat_to(buffer, fmt::string_view{
                "\n----------------------------------------\n"
                "|| Error: format syntax invalid!\n"
                "|| Format: {}\n"
                "----------------------------------------"
                }, fmt::make_format_args(msg));

            if (has_style) {
                Details::EndColor(buffer);
            }
        }

        buffer.push_back('\n');
        buffer.push_back('\0');

        std::fputs(buffer.data(), stdout);
    }
}


// NOTE MSVC: If you get an error, make sure you add a preprocessor /Zc:preprocessor for support VA_OPT
// Read it here: https://docs.microsoft.com/en-us/cpp/build/reference/zc-preprocessor?view=msvc-160
#define HELENA_MSG(prefix, fmt, ...)    Helena::Log::Console<prefix>(Helena::Util::SourceLocation::Create(__FILE__, __LINE__), fmt __VA_OPT__(,) __VA_ARGS__)

#if defined(HELENA_DEBUG)
    #define HELENA_MSG_DEFAULT(fmt, ...)    HELENA_MSG(Helena::Log::Details::Default,   fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_DEBUG(fmt, ...)      HELENA_MSG(Helena::Log::Details::Debug,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_INFO(fmt, ...)       HELENA_MSG(Helena::Log::Details::Info,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Details::Warning,   fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Details::Error,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Details::Fatal,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ASSERT(fmt, ...)     HELENA_MSG(Helena::Log::Details::Assert,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_EXCEPTION(fmt, ...)  HELENA_MSG(Helena::Log::Details::Exception, fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define HELENA_MSG_DEFAULT(fmt, ...)
    #define HELENA_MSG_DEBUG(fmt, ...)
    #define HELENA_MSG_INFO(fmt, ...)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Details::Warning,   fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Details::Error,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Details::Fatal,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ASSERT(fmt, ...)     HELENA_MSG(Helena::Log::Details::Assert,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_EXCEPTION(fmt, ...)  HELENA_MSG(Helena::Log::Details::Exception, fmt __VA_OPT__(,) __VA_ARGS__)
#endif


#endif // HELENA_ENGINE_LOG_HPP
