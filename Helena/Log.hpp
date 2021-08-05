#ifndef HELENA_LOG_HPP
#define HELENA_LOG_HPP

#include <Helena/Dependencies/Format.hpp>
#include <Helena/Util.hpp>
#include <Helena/Assert.hpp>
#include <Helena/Concurrency/Internal.hpp>

#include <array>
#include <iterator>
#include <iostream>
#include <string_view>

namespace Helena::Log
{
    using Style = fmt::text_style;
    using Level = std::uint8_t;

    enum class Color : std::uint32_t {
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

    enum class ELevel : std::uint8_t {
        Default = 250,
        Debug   = 251,
        Info    = 252,
        Warning = 253,
        Error   = 254,
        Fatal   = 255
    };

    [[nodiscard]] inline consteval std::string_view CreatePrefix(const std::string_view prefix) noexcept {
        return prefix;
    }

    [[nodiscard]] inline consteval auto CreateStyle(const Color color) noexcept {
        return Style(fmt::fg(static_cast<fmt::terminal_color>(Util::Cast(color))));
    }

    [[nodiscard]] inline consteval auto CreateStyle(const Color color, const Color background) noexcept {
        return Style(fmt::fg(static_cast<fmt::terminal_color>(Util::Cast(color))) | fmt::bg(static_cast<fmt::terminal_color>(background)));
    }

    struct Source final
    {
        [[nodiscard]] static consteval Source Create(const std::string_view file, const std::uint_least32_t line) noexcept {
            Source instance(Util::GetSourceName(file, std::string_view{"\\/"}), line);
            return instance;
        }

        constexpr Source(const std::string_view file, const std::uint_least32_t line) noexcept : m_File{file}, m_Line{line} {}

        [[nodiscard]] constexpr std::string_view GetFile() const noexcept {
            return m_File;
        }

        [[nodiscard]] constexpr std::uint_least32_t GetLine() const noexcept {
            return m_Line;
        }

    private:
        const std::string_view m_File;
        std::uint_least32_t m_Line;
    };

    struct Default final {
        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return CreatePrefix("");
        }

        [[nodiscard]] static consteval auto GetColor() noexcept {
            return CreateStyle(Color::BrightWhite);
        }
    };

    struct Debug final {
        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return CreatePrefix("[DEBUG]");
        }

        [[nodiscard]] static consteval auto GetColor() noexcept {
            return CreateStyle(Color::BrightBlue);
        }
    };

    struct Info final {
        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return CreatePrefix("[INFO]");
        }

        [[nodiscard]] static consteval auto GetColor() noexcept {
            return CreateStyle(Color::BrightGreen);
        }
    };

    struct Warning final {
        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return CreatePrefix("[WARNING]");
        }

        [[nodiscard]] static consteval auto GetColor() noexcept {
            return CreateStyle(Color::BrightYellow);
        }
    };

    struct Error final {
        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return CreatePrefix("[ERROR]");
        }

        [[nodiscard]] static consteval auto GetColor() noexcept {
            return CreateStyle(Color::BrightRed);
        }
    };

    struct Fatal final {
        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return CreatePrefix("[FATAL]");
        }

        [[nodiscard]] static consteval auto GetColor() noexcept {
            return CreateStyle(Color::BrightWhite, Color::BrightRed);
        }
    };

    struct Exception {
        [[nodiscard]] static consteval auto GetPrefix() noexcept {
            return CreatePrefix("[EXCEPTION]");
        }

        [[nodiscard]] static consteval auto GetColor() noexcept {
            return CreateStyle(Color::BrightWhite, Color::Red);
        }

        template <typename Iterator>
        static void ExceptionInfo(const Source& source, const Iterator begin, const std::size_t length) {
            constexpr const auto prefix = GetPrefix();
            constexpr const auto style  = GetColor();

            const auto end = fmt::format_to(begin + length, "Format syntax incorrect!\n");
            fmt::print(style, &*begin, fmt::localtime(std::time(nullptr)), source.GetFile(), source.GetLine(), prefix);
        }
    };

    namespace Details {
        template <typename T>
        concept PrefixTraits = requires {
            { T::GetPrefix() } -> std::same_as<std::string_view>;
            { T::GetColor() } -> std::same_as<Style>;
        };
    }


    template <Details::PrefixTraits Prefix, typename... Args>
    inline void Console(const Source& source, const std::string_view msg, Args&&... args)
    {
        static std::array<char, 4096> buffer {"[{:%Y.%m.%d %H:%M:%S}][{}:{}]{} \n"};

        constexpr std::size_t length = 32u;  // Length of format string in buffer
        constexpr auto prefix = Prefix::GetPrefix();
        constexpr auto style  = Prefix::GetColor();

        std::size_t offset = 0u;

        if constexpr (sizeof... (Args))
        {
            const auto size = msg.size() / 2 * 2;

            if(size)
            {
                while(offset < size) {
                    buffer[length + offset] = msg[offset]; ++offset;
                    buffer[length + offset] = msg[offset]; ++offset;
                }

                auto remainingSize = msg.size() - size;
                while(remainingSize) {
                    buffer[length + offset] = msg[offset]; ++offset;
                    --remainingSize;
                }

            } else {
                while(offset < msg.size()) {
                    buffer[length + offset] = msg[offset]; ++offset;
                }
            }

            buffer[length + offset] = '\n'; ++offset;
            buffer[length + offset] = '\0'; ++offset;

            try {
                fmt::print(style, buffer.data(), fmt::localtime(std::time(nullptr)), source.GetFile(), source.GetLine(), prefix, std::forward<Args>(args)...);
            } catch (const fmt::format_error&) {
                Exception::ExceptionInfo(source, buffer.begin(), length);
            }
        } else {
            buffer[length + offset] = '{';  ++offset;
            buffer[length + offset] = '}';  ++offset;
            buffer[length + offset] = '\n'; ++offset;
            buffer[length + offset] = '\0'; ++offset;

            try {
                fmt::print(style, buffer.data(), fmt::localtime(std::time(nullptr)), source.GetFile(), source.GetLine(), prefix, msg);
            } catch (const fmt::format_error&) {
                Exception::ExceptionInfo(source, buffer.begin(), length);
            }
        }
    }

    // NOTE MSVC: If you get an error, make sure you add a preprocessor /Zc:preprocessor
    // Read it here: https://docs.microsoft.com/en-us/cpp/build/reference/zc-preprocessor?view=msvc-160
    #define HF_MSG(prefix, fmt, ...)    Helena::Log::Console<prefix>(Helena::Log::Source::Create(__FILE__, __LINE__), fmt __VA_OPT__(,) __VA_ARGS__)

    #define HF_MSG_DEFAULT(fmt, ...)    HF_MSG(Helena::Log::Default,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HF_MSG_DEBUG(fmt, ...)      HF_MSG(Helena::Log::Debug,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HF_MSG_INFO(fmt, ...)       HF_MSG(Helena::Log::Info,       fmt __VA_OPT__(,) __VA_ARGS__)
    #define HF_MSG_WARNING(fmt, ...)    HF_MSG(Helena::Log::Warning,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HF_MSG_ERROR(fmt, ...)      HF_MSG(Helena::Log::Error,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HF_MSG_FATAL(fmt, ...)      HF_MSG(Helena::Log::Fatal,      fmt __VA_OPT__(,) __VA_ARGS__)
}

#endif // HELENA_LOG_HPP
