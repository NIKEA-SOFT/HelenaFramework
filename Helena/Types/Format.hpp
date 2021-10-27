#ifndef HELENA_TYPES_FORMAT_HPP
#define HELENA_TYPES_FORMAT_HPP

#include <Helena/Engine/Log.hpp>
#include <Helena/Types/FixedBuffer.hpp>

namespace Helena::Types
{
	template <std::size_t Capacity>
	class Format
	{
		using memory_buffer = fmt::basic_memory_buffer<char, Capacity>;

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

		[[nodiscard]] const char* GetData() const noexcept {
			return m_Buffer.data();
		}

		[[nodiscard]] std::string_view GetBuffer() const noexcept {
			return {m_Buffer.data(), m_Buffer.size()};
		}

		[[nodiscard]] std::size_t GetSize() const noexcept {
			return m_Buffer.size();
		}

		[[nodiscard]] std::size_t Empty() const noexcept {
			return !m_Buffer.size();
		}

		void Clear() noexcept {
			m_Buffer.clear();
		}

		[[nodiscard]] operator FixedBuffer<Capacity>() const noexcept {
			return FixedBuffer<Capacity>(m_Buffer.data(), m_Buffer.size());
		}

		memory_buffer m_Buffer;
	};
}

#endif // HELENA_TYPES_FORMAT_HPP