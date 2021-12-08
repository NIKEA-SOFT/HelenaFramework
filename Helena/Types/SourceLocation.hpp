#ifndef HELENA_TYPES_SOURCELOCATION_HPP
#define HELENA_TYPES_SOURCELOCATION_HPP

#include <cstdint>
#include <string_view>
#include <algorithm>

namespace Helena::Types
{
	struct SourceLocation
	{
		[[nodiscard]] static consteval std::string_view GetSourceName(const std::string_view file, const std::string_view delimeter) noexcept {
			const auto it = std::find_first_of(file.crbegin(), file.crend(), delimeter.cbegin(), delimeter.cend());
			return std::string_view(it == file.crend() ? file.cbegin() : it.base(), file.cend());
		}

		[[nodiscard]] static consteval auto Create(const char* const file = __builtin_FILE(),
			const char* const function = __builtin_FUNCTION(), const std::uint_least32_t line = __builtin_LINE()) noexcept {
			return SourceLocation{GetSourceName(file, std::string_view{"\\/"}).data(), function, line};
		}

		[[nodiscard]] static consteval auto Create(const std::string_view file, const std::uint_least32_t line) noexcept {
			return SourceLocation{GetSourceName(file, std::string_view{"\\/"}).data(), line};
		}

		[[nodiscard]] static consteval auto Create(const std::string_view file, const std::string_view function, const std::uint_least32_t line) noexcept {
			return SourceLocation{GetSourceName(file, std::string_view{"\\/"}).data(), line};
		}

		constexpr SourceLocation(const char* const file, const std::uint_least32_t line) noexcept
			: m_File{file}, m_Function{}, m_Line{line} {}
		constexpr SourceLocation(const char* const file, const char* const function, const std::uint_least32_t line) noexcept
			: m_File{file}, m_Function{function}, m_Line{line} {}
		constexpr ~SourceLocation() = default;
		constexpr SourceLocation(const SourceLocation&) = default;
		constexpr SourceLocation(SourceLocation&&) noexcept = default;

		[[nodiscard]] constexpr std::string_view GetFile() const noexcept {
			return m_File;
		}

		[[nodiscard]] constexpr std::string_view GetFunction() const noexcept {
			return m_Function;
		}

		[[nodiscard]] constexpr std::uint_least32_t GetLine() const noexcept {
			return m_Line;
		}

		constexpr SourceLocation& operator=(const SourceLocation&) = delete;
		constexpr SourceLocation& operator=(SourceLocation&&) noexcept = delete;

		const char* const m_File;
		const char* const m_Function;
		const std::uint_least32_t m_Line;
	};
}

#endif // HELENA_TYPES_SOURCELOCATION_HPP