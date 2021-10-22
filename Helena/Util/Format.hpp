#ifndef HELENA_UTIL_FORMAT_HPP
#define HELENA_UTIL_FORMAT_HPP

#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Engine/Log.hpp>

namespace Helena::Util
{
	template <std::size_t Capacity>
	class Format
	{
		using memory_buffer = fmt::basic_memory_buffer<char, Traits::PowerOf2<Capacity>::value>;

	public:
		template <typename... Args>
		Format(const std::string_view msg, [[maybe_unused]] Args&&... args) 
		{
			try {
				fmt::detail::vformat_to(m_Buffer, fmt::string_view{msg.data(), msg.size()}, fmt::make_format_args(std::forward<Args>(args)...));
			} catch(const fmt::format_error&) {
				HELENA_MSG_EXCEPTION(
					"\n----------------------------------------\n"
					"|| Error: format syntax invalid!\n"
					"|| Format: {}"
					"\n----------------------------------------", msg);
				m_Buffer.clear();
			} catch(const std::bad_alloc&) {
				HELENA_MSG_EXCEPTION(
					"\n----------------------------------------\n"
					"|| Error: not enough memory for alloc\n"
					"|| Format: {}"
					"\n----------------------------------------", msg);
				m_Buffer.clear();
			}
		}

		Format() = default;
		~Format() = default;
		Format(const Format&) = delete;
		Format(Format&&) noexcept = default;
		Format& operator=(const Format&) = delete;
		Format& operator=(Format&&) = default;

		[[nodiscard]] std::string_view GetText() noexcept {
			return std::string_view{m_Buffer.data(), m_Buffer.size()};
		}

		[[nodiscard]] std::size_t GetSize() noexcept {
			return m_Buffer.size();
		}

		[[nodiscard]] std::size_t Empty() noexcept {
			return !m_Buffer.size();
		}

		void Clear() noexcept {
			m_Buffer.clear();
		}

	private:
		memory_buffer m_Buffer;
	};
}

#endif // HELENA_UTIL_FORMAT_HPP