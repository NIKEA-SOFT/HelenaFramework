#ifndef HELENA_TYPES_SOURCELOCATION_HPP
#define HELENA_TYPES_SOURCELOCATION_HPP

#include <Helena/Platform/Compiler.hpp>

#include <cstdint>
#include <algorithm>
#include <string_view>

// Trick for optimization Truncate on MSVC compiler
// MSVC is unable to apply optimization here...
#if defined(HELENA_COMPILER_MSVC)
    #define HELENA_SOURCE_CONSTEXPREVAL consteval
#else
    #define HELENA_SOURCE_CONSTEXPREVAL constexpr
#endif

namespace Helena::Types
{
    /*! SourceLocation implementation */
    class SourceLocation
    {
        static constexpr std::string_view Separator = "\\/";

    public:
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

    public:
        [[nodiscard]] static HELENA_SOURCE_CONSTEXPREVAL auto Create(const char* file = __builtin_FILE(),
            const char* function = __builtin_FUNCTION(), const std::uint_least32_t line = __builtin_LINE()) noexcept {
            SourceLocation location;
            location.m_File = TruncatePath(file);
            location.m_Function = function;
            location.m_Line = line;
            return location;
        }

        [[nodiscard]] static HELENA_SOURCE_CONSTEXPREVAL auto Create(const std::string_view file, const std::uint_least32_t line) noexcept {
            SourceLocation location;
            location.m_File = TruncatePath(file);
            location.m_Function = "";
            location.m_Line = line;
            return location;
        }

        [[nodiscard]] static HELENA_SOURCE_CONSTEXPREVAL const char* TruncatePath(const std::string_view file) noexcept {
            const auto it = std::find_first_of(file.crbegin(), file.crend(), Separator.cbegin(), Separator.cend());
            return it == file.crend() ? std::addressof(*file.cbegin()) : std::addressof(*it.base());
        }

    private:
        const char* m_File{};
        const char* m_Function{};
        std::uint_least32_t m_Line{};
    };
}

#undef HELENA_SOURCE_CONSTEXPREVAL

#endif // HELENA_TYPES_SOURCELOCATION_HPP