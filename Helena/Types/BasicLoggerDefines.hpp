#ifndef HELENA_TYPES_BASICLOGGERSDEF_HPP
#define HELENA_TYPES_BASICLOGGERSDEF_HPP

#include <Helena/Platform/Platform.hpp>
#include <Helena/Types/SourceLocation.hpp>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <concepts>
#include <format>
#include <string>
#include <type_traits>
#include <utility>

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

    class LoggerBuffer
    {
        static constexpr auto m_BufferCapacity{4000};

    public:
        LoggerBuffer()
            : m_InitData{new std::byte[m_BufferCapacity]}
            , m_Data{m_InitData}
            , m_Size{}
            , m_Capacity{m_BufferCapacity} {}
        LoggerBuffer(const LoggerBuffer&) = delete;
        LoggerBuffer(LoggerBuffer&&) = delete;
        LoggerBuffer& operator=(const LoggerBuffer&) = delete;
        LoggerBuffer& operator=(LoggerBuffer&& other) = delete;
        ~LoggerBuffer() {}

        template <typename Char>
        void push_back(Char&& value) {
            reallocate(m_Size + sizeof(Char));
            std::memcpy(static_cast<std::byte*>(m_Data) + m_Size, &value, sizeof(Char));
            m_Size += sizeof(Char);
        }

        template <typename Char>
        void append(const Char* data, std::size_t size) {
            const auto bytes = size * sizeof(Char);
            reallocate(m_Size + bytes);
            std::memcpy(static_cast<std::byte*>(m_Data) + m_Size, data, bytes);
            m_Size += bytes;
        }

        void resize(std::size_t size) {
            reallocate(size);
            m_Size = size;
        }

        void erase(std::size_t offset, std::size_t size) {
            if(!offset && size == 10) { // specific impl for colors (needed for optimize)
                m_Data = static_cast<std::byte*>(m_Data) + offset;
                m_Capacity -= offset;
            } else {
                std::memmove(static_cast<std::byte*>(m_Data) + offset, static_cast<std::byte*>(m_Data) + offset + size, m_Size - offset - size);
            }
            m_Size -= size;
        }

        template <typename Char>
        std::size_t size() const noexcept {
            return m_Size / sizeof(Char);
        }

        std::size_t bytes() const noexcept {
            return m_Size;
        }

        template <typename Char>
        void Reset() noexcept
        {
            if(m_Data < m_InitData || m_Data >= static_cast<std::byte*>(m_InitData) + m_BufferCapacity) [[unlikely]] {
                delete[] static_cast<std::byte*>(m_Data);
            }

            m_Data = m_InitData;
            m_Size = 0;
            m_Capacity = m_BufferCapacity;
        }

        template <typename Char>
        [[nodiscard]] std::basic_string_view<Char> View() noexcept {
            push_back<Char>(Char{}); m_Size -= sizeof(Char);
            return std::basic_string_view<Char>{static_cast<Char*>(m_Data), m_Size / sizeof(Char)};
        }

    private:
        void reallocate(std::size_t requiredBytes) {
            if(requiredBytes > m_Capacity) [[unlikely]] {
                m_Capacity = (std::max)(m_Capacity * 2, requiredBytes);
                std::memcpy(m_Data, std::exchange(m_Data, new std::byte[m_Capacity]), m_Size);
            }
        }

    private:
        void* m_InitData;
        void* m_Data;
        std::size_t m_Size;
        std::size_t m_Capacity;
    };

    template <typename>
    struct Print;

    class ColorStyle
    {
    public:
        static constexpr std::size_t ColorSize = 10;
        static constexpr std::size_t ColorSizeEnd = 4;

    public:
        constexpr ColorStyle(Color front = Color::Default, Color back = Color::Default)
            : m_Front{front}, m_Back{back} {}

        template <typename Char>
        void BeginColor(LoggerBuffer& buffer) const
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
        void EndColor(LoggerBuffer& buffer) const
        {
            if(!HELENA_ENABLE_VIRTUAL_TERMINAL_PROCESSING)
                return;

            constexpr Char colorReset[]{0x1B, 0x5B, 0x30, 0x6D};
            static_assert(std::size(colorReset) == ColorSizeEnd);
            buffer.append(colorReset, std::size(colorReset));
        }

        template <typename Char, typename Traits>
        static void RemoveColor(std::basic_string_view<Char, Traits>& str)
        {
            if(!HELENA_ENABLE_VIRTUAL_TERMINAL_PROCESSING)
                return;

            if(str.size() < ColorSize + ColorSizeEnd)
                return;

            constexpr Char colorHeader[]{0x1B, 0x5B, 0x30, 0x3B};
            if(std::memcmp(str.data(), colorHeader, sizeof(colorHeader)) == 0) {
                str = std::basic_string_view<Char, Traits>(str.data() + ColorSize, str.size() - ColorSize);
            }

            constexpr Char colorEnd[]{0x1B, 0x5B, 0x30, 0x6D};
            const auto hasEndline = *str.rbegin() == Print<Char>::Endline;
            if(std::memcmp(str.data() + str.size() - (ColorSizeEnd + hasEndline), colorEnd, sizeof(colorEnd)) == 0) {
                str = std::basic_string_view<Char, Traits>(str.data(), str.size() - ColorSizeEnd);
                *(const_cast<Char*>(str.data()) + str.size()) = Char{};
                if(hasEndline) [[likely]] {
                    const_cast<Char&>(str.back()) = Print<Char>::Endline;
                }
            }
        }

    private:
        Color m_Front;
        Color m_Back;
    };

    template <typename T>
    concept DefinitionLogger =
        std::same_as<std::remove_cvref_t<decltype(T::Prefix)>, std::string_view> &&
        std::same_as<std::remove_cvref_t<decltype(T::Style)>, ColorStyle>;

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
        // NOTE: Don't declare the given using in your own specializations (used for optimization)
        using DefaultFingerprint = void;

        template <typename Char>
        static void Message(std::basic_string_view<Char> message) {
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

// Forward declaration and specialization
namespace Helena::Log
{
    template <DefinitionLogger Logger, bool Endline = true, typename Char, typename... Args>
    void Message(const Formatter<Char> format, Args&&... args);

    template <DefinitionLogger Logger, bool Endline = true, typename... Args>
    void Message(const Formatter<char> format, Args&&... args) {
        Message<Logger, Endline, char>(format, std::forward<Args>(args)...);
    }

    template <DefinitionLogger Logger, bool Endline = true, typename... Args>
    void Message(const Formatter<wchar_t> format, Args&&... args) {
        Message<Logger, Endline, wchar_t>(format, std::forward<Args>(args)...);
    }
}

namespace Helena::Log::Internal
{
    inline thread_local LoggerBuffer m_LoggerBuffer;

    template <typename Char>
    LoggerBuffer& GetCachedBuffer() noexcept {
        auto& buffer = m_LoggerBuffer; buffer.Reset<Char>();
        return buffer;
    }
}

template <>
class std::back_insert_iterator<Helena::Log::LoggerBuffer>
{
public:
    using iterator_category = output_iterator_tag;
    using value_type = void;
    using pointer = void;
    using reference = void;

    using container_type = Helena::Log::LoggerBuffer;
    using difference_type = ptrdiff_t;

    constexpr explicit back_insert_iterator(container_type& container) noexcept
        : m_Container(std::addressof(container)) {}

    template <typename T>
    constexpr back_insert_iterator& operator=(const T& value) {
        m_Container->push_back(value);
        return *this;
    }

    template <typename T>
    constexpr back_insert_iterator& operator=(T&& value) {
        m_Container->push_back(std::move(value));
        return *this;
    }

    [[nodiscard]] constexpr back_insert_iterator& operator*() noexcept {
        return *this;
    }

    constexpr back_insert_iterator& operator++() noexcept {
        return *this;
    }

    constexpr back_insert_iterator operator++(int) noexcept {
        return *this;
    }

protected:
    container_type* m_Container;
};

#endif // HELENA_TYPES_BASICLOGGERSDEF_HPP