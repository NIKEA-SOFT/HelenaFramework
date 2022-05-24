#ifndef HELENA_TYPES_SOURCELOCATION_HPP
#define HELENA_TYPES_SOURCELOCATION_HPP

#include <Helena/Platform/Defines.hpp>

#include <cstdint>
#include <string_view>
#include <algorithm>

namespace Helena::Types
{
    /**
    * @brief SourceLocation implementation
    * @warning For optimization used POD and members are not initialized
    */
    struct SourceLocation
    {
        [[nodiscard]] static constexpr const char* GetSourceName(const std::string_view file, const std::string_view delimeter) noexcept {
            const auto it = std::find_first_of(file.crbegin(), file.crend(), delimeter.cbegin(), delimeter.cend());
            return it == file.crend() ? std::addressof(*file.cbegin()) : std::addressof(*it.base());
        }

        [[nodiscard]] static constexpr auto Create(const char* file = __builtin_FILE(),
            const char* function = __builtin_FUNCTION(), const std::uint_least32_t line = __builtin_LINE()) noexcept {
            SourceLocation location;
            location.m_File = GetSourceName(file, "\\/");
            location.m_Function = function;
            location.m_Line = line;
            return location;
        }

        [[nodiscard]] static constexpr auto Create(const std::string_view file, const std::uint_least32_t line) noexcept {
            SourceLocation location;
            location.m_File = GetSourceName(file, "\\/");
            location.m_Function = "";
            location.m_Line = line;
            return location;
        }

        constexpr SourceLocation() noexcept = default;

        [[nodiscard]] constexpr const char* GetFile() const noexcept {
            return m_File;
        }

        [[nodiscard]] constexpr const char* GetFunction() const noexcept {
            return m_Function;
        }

        [[nodiscard]] constexpr std::uint_least32_t GetLine() const noexcept {
            return m_Line;
        }

        const char* m_File;
        const char* m_Function;
        std::uint_least32_t m_Line;
    };

    static_assert(std::is_trivial_v<SourceLocation> && std::is_standard_layout_v<SourceLocation>, "Type is not pod");
}

#endif // HELENA_TYPES_SOURCELOCATION_HPP