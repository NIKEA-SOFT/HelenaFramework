#ifndef HELENA_UTIL_SOURCELOCATION_HPP
#define HELENA_UTIL_SOURCELOCATION_HPP

#include <cstdint>
#include <string_view>
#include <algorithm>

namespace Helena::Util
{
    struct SourceLocation
    {
        [[nodiscard]] static constexpr std::string_view GetSourceName(const std::string_view file, const std::string_view delimeter) noexcept {
            const auto it = std::find_first_of(file.crbegin(), file.crend(), delimeter.cbegin(), delimeter.cend());
            return std::string_view(it == file.crend() ? file.cbegin() : it.base(), file.cend());
        }

        [[nodiscard]] static consteval auto Create(const std::string_view file, const std::uint_least32_t line) noexcept {
            return SourceLocation{GetSourceName(file, std::string_view{"\\/"}).data(), line};
        }

        constexpr SourceLocation(const char* const file, const std::uint_least32_t line) noexcept : m_File { file }, m_Line { line } {}
        constexpr ~SourceLocation() = default;
        constexpr SourceLocation(const SourceLocation&) = delete;
        constexpr SourceLocation(SourceLocation&&) noexcept = delete;

        [[nodiscard]] constexpr std::string_view GetFile() const noexcept {
            return m_File;
        }

        [[nodiscard]] constexpr std::uint_least32_t GetLine() const noexcept {
            return m_Line;
        }

        constexpr SourceLocation& operator=(const SourceLocation&) = delete;
        constexpr SourceLocation& operator=(SourceLocation&&) noexcept = delete;

        const char* const m_File;
        const std::uint_least32_t m_Line;
    };
}

#endif // HELENA_UTIL_SOURCELOCATION_HPP