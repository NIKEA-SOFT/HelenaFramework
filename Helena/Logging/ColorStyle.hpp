#ifndef HELENA_LOGGING_COLORSTYLE_HPP
#define HELENA_LOGGING_COLORSTYLE_HPP

#include <Helena/Platform/Platform.hpp>
#include <Helena/Logging/Internal/LoggerBuffer.hpp>

namespace Helena::Logging
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
        void BeginColor(Internal::LoggerBuffer& buffer) const
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
        void EndColor(Internal::LoggerBuffer& buffer) const
        {
            if(!HELENA_ENABLE_VIRTUAL_TERMINAL_PROCESSING)
                return;

            static constexpr Char colorReset[]{0x1B, 0x5B, 0x30, 0x6D};
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

            static constexpr Char colorHeader[]{0x1B, 0x5B, 0x30, 0x3B};
            if(std::memcmp(str.data(), colorHeader, sizeof(colorHeader)) == 0) {
                str = std::basic_string_view<Char, Traits>(str.data() + ColorSize, str.size() - ColorSize);
            }

            static constexpr Char colorEnd[]{0x1B, 0x5B, 0x30, 0x6D};
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
        std::same_as<std::remove_cvref_t<decltype(T::Prefix)>, std::string_view>&&
        std::same_as<std::remove_cvref_t<decltype(T::Style)>, class ColorStyle>;

    [[nodiscard]] inline constexpr auto CreateStyle(const Color color = Color::Default, const Color background = Color::Default) noexcept {
        return ColorStyle{color, background};
    }

    [[nodiscard]] static constexpr auto CreatePrefix(const std::string_view prefix) noexcept {
        return prefix;
    }
}

#include <Helena/Logging/Print.hpp>

#endif // HELENA_LOGGING_COLORSTYLE_HPP
