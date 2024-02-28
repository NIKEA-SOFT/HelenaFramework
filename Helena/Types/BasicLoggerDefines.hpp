#ifndef HELENA_TYPES_BASICLOGGERSDEF_HPP
#define HELENA_TYPES_BASICLOGGERSDEF_HPP

#include <Helena/Platform/Platform.hpp>
#include <Helena/Types/SourceLocation.hpp>

#include <cstdint>
#include <cstdio>
#include <format>
#include <type_traits>
#include <concepts>

namespace Helena::Log
{
    enum class Color : std::uint16_t
    {
        Default = 0x3339,
        Black = 0x3330,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrightBlack = 0x3930,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightMagenta,
        BrightCyan,
        BrightWhite
    };

    class ColorStyle
    {
    public:
        static constexpr auto ColorSize = 10;
        static constexpr auto ColorSizeEnd = 4;

    public:
        constexpr ColorStyle(Color front = Color::Default, Color back = Color::Default)
            : m_Front{front}, m_Back{back} {}

        template <typename Char>
        void BeginColor(std::basic_string<Char>& buffer) const
        {
            if(!HELENA_ENABLE_VIRTUAL_TERMINAL_PROCESSING)
                return;

            const Char color[]{0x1B, 0x5B, 0x30,
                0x3B, static_cast<Char>(static_cast<std::underlying_type_t<Color>>(m_Front) >> 0x08),
                static_cast<Char>(static_cast<std::underlying_type_t<Color>>(m_Front) & 0xFF),
                0x3B, static_cast<Char>(((static_cast<std::underlying_type_t<Color>>(m_Back) >> 0x08) + 0x06) & 0xFF),
                static_cast<Char>(static_cast<std::underlying_type_t<Color>>(m_Back) & 0xFF),
                0x6D};

            static_assert(std::size(color) == ColorSize);
            buffer.append(color, std::size(color));
        }

        template <typename Char>
        static void EndColor(std::basic_string<Char>& buffer)
        {
            if(!HELENA_ENABLE_VIRTUAL_TERMINAL_PROCESSING)
                return;

            constexpr Char colorReset[]{0x1B, 0x5B, 0x30, 0x6D};
            static_assert(std::size(colorReset) == ColorSizeEnd);
            buffer.append(colorReset, std::size(colorReset));
        }

    private:
        Color m_Front;
        Color m_Back;
    };

    template <typename T>
    concept DefinitionLogger =
        std::same_as<std::remove_cvref_t<decltype(T::Prefix)>, std::string_view> &&
        std::same_as<std::remove_cvref_t<decltype(T::Style)>, ColorStyle>;

    // Class with an implicit constructor for working with formatting
    template <typename Char>
    struct Formatter
    {
        constexpr Formatter(const Char* message, const Types::SourceLocation location = Types::SourceLocation::Create()) noexcept
            : m_Location{location}
            , m_Message{message} {}

        constexpr Formatter(const std::basic_string_view<Char> message, const Types::SourceLocation location = Types::SourceLocation::Create()) noexcept
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

    template <typename Char>
    Formatter(const Char*) -> Formatter<Char>;

    // Structures defining how to print message for different Char types
    template <typename>
    struct Print;

    template <>
    struct Print<char>
    {
        using Context = std::format_context;

        static constexpr auto FormatStyle = "[{}][{}][{}:{}] ";
        static constexpr auto AllocateError =
            "\n----------------------------------------\n"
            "|| Error: alloc memory failed!\n"
            "|| Format: {}"
            "\n----------------------------------------";
        static constexpr auto FormatError =
            "\n----------------------------------------\n"
            "|| Error: format syntax invalid!\n"
            "|| Format: {}"
            "\n----------------------------------------";
        static constexpr auto Endline = '\n';

        static void Message(const std::string_view message) {
            (void)std::fputs(message.data(), stdout);
        }
    };

    template <>
    struct Print<wchar_t>
    {
        using Context = std::wformat_context;

        static constexpr auto FormatStyle = L"[{}][{}][{}:{}] ";
        static constexpr auto AllocateError =
            L"\n----------------------------------------\n"
            L"|| Error: alloc memory failed!\n"
            L"|| Format: {}"
            L"\n----------------------------------------";
        static constexpr auto FormatError =
            L"\n----------------------------------------\n"
            L"|| Error: format syntax invalid!\n"
            L"|| Format: {}"
            L"\n----------------------------------------";
        static constexpr auto Endline = L'\n';

        static void Message(const std::wstring_view message) {
            (void)std::fputws(message.data(), stdout);
        }
    };

    // Structure to override `Print<Char>::Show` behavior of specific logger using specialization
    template <DefinitionLogger>
    struct CustomPrint {
        template <typename Char>
        static void Message(std::basic_string<Char>& message) {
            Print<Char>::Message(message);
        }
    };

    template <DefinitionLogger>
    struct MuteController {
        // NOTE: Don't declare the given using in your own specializations (used for optimization)
        using DefaultFingerprint = void;

        static bool Muted() {
            return false;
        }
    };

    // Util functions for creating logging structures
    [[nodiscard]] static constexpr auto CreatePrefix(const std::string_view prefix) noexcept {
        return prefix;
    }

    [[nodiscard]] static constexpr auto CreateStyle(const Color color = Color::Default, const Color background = Color::Default) noexcept {
        return ColorStyle{color, background};
    }

    // Structures defining the type and color of the logged message
    struct Benchmark {
        static constexpr auto Prefix = CreatePrefix("BENCHMARK");
        static constexpr auto Style  = CreateStyle(Color::BrightMagenta);
    };

    struct Debug {
        static constexpr auto Prefix = CreatePrefix("DEBUG");
        static constexpr auto Style  = CreateStyle(Color::BrightBlue);
    };

    struct Info {
        static constexpr auto Prefix = CreatePrefix("INFO");
        static constexpr auto Style  = CreateStyle(Color::BrightGreen);
    };

    struct Notice {
        static constexpr auto Prefix = CreatePrefix("NOTICE");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite);
    };

    struct Warning {
        static constexpr auto Prefix = CreatePrefix("WARNING");
        static constexpr auto Style  = CreateStyle(Color::BrightYellow);
    };

    struct Error {
        static constexpr auto Prefix = CreatePrefix("ERROR");
        static constexpr auto Style  = CreateStyle(Color::BrightRed);
    };

    struct Fatal {
        static constexpr auto Prefix = CreatePrefix("FATAL");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Exception {
        static constexpr auto Prefix = CreatePrefix("EXCEPTION");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);

    };

    struct Assert {
        static constexpr auto Prefix = CreatePrefix("ASSERT");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);
    };

    struct Memory {
        static constexpr auto Prefix = CreatePrefix("MEMORY");
        static constexpr auto Style  = CreateStyle(Color::BrightCyan);
    };

    struct Shutdown {
        static constexpr auto Prefix = CreatePrefix("SHUTDOWN");
        static constexpr auto Style  = CreateStyle(Color::BrightWhite, Color::Red);
    };
}


// Forward declaration
namespace Helena::Log
{
    template <DefinitionLogger Logger, typename... Args>
    void Message(const Formatter<char> format, Args&&... args);

    template <DefinitionLogger Logger, typename... Args>
    void Message(const Formatter<wchar_t> format, Args&&... args);
}

#endif // HELENA_TYPES_BASICLOGGERSDEF_HPP