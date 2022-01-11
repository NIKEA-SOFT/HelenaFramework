#ifndef HELENA_TYPES_SOURCELOCATION_HPP
#define HELENA_TYPES_SOURCELOCATION_HPP

#include <Helena/Platform/Defines.hpp>

#include <cstdint>
#include <string_view>
#include <algorithm>

namespace Helena::Types
{
    struct SourceLocation
    {
        [[nodiscard]] static HELENA_CONSTEVAL const std::string_view GetSourceName(const std::string_view file, const std::string_view delimeter) noexcept {
            const auto it = std::find_first_of(file.crbegin(), file.crend(), delimeter.cbegin(), delimeter.cend());
            return std::string_view{it == file.crend() ? file.cbegin() : it.base(), file.cend()};
        }

        [[nodiscard]] static HELENA_CONSTEVAL const auto Create(const std::string_view file = __builtin_FILE(),
            const std::string_view function = __builtin_FUNCTION(), const std::uint_least32_t line = __builtin_LINE()) noexcept {
            SourceLocation location;
            location.m_File = GetSourceName(file, std::string_view{"\\/"});
            location.m_Function = function;
            location.m_Line = line;
            return location;
        }

        [[nodiscard]] static HELENA_CONSTEVAL const auto Create(const std::string_view file, const std::uint_least32_t line) noexcept {
            SourceLocation location;
            location.m_File = GetSourceName(file, std::string_view{"\\/"});
            location.m_Line = line;
            return location;
        }

        constexpr SourceLocation() noexcept = default;

        [[nodiscard]] constexpr std::string_view GetFile() const noexcept {
            return m_File;
        }

        [[nodiscard]] constexpr std::string_view GetFunction() const noexcept {
            return m_Function;
        }

        [[nodiscard]] constexpr std::uint_least32_t GetLine() const noexcept {
            return m_Line;
        }

        std::string_view m_File {};
        std::string_view m_Function {};
        std::uint_least32_t m_Line {};
    };
}

#endif // HELENA_TYPES_SOURCELOCATION_HPP