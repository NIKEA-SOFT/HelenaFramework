#ifndef HELENA_TYPES_FORMAT_HPP
#define HELENA_TYPES_FORMAT_HPP

#include <Helena/Util/Length.hpp>

namespace Helena::Types
{
	template <std::size_t Capacity>
	class Format
	{
		using memory_buffer = fmt::basic_memory_buffer<char, Capacity + 1>;

	public:
		explicit Format(const char* data) noexcept {
			const auto size = Util::Length(Util::ELengthPolicy::Truncate, data, 1024);
			m_Buffer.append(data, data + size);
			m_Buffer.push_back('\0');
		}

		template <typename... Args>
		explicit Format(std::string_view msg, [[maybe_unused]] Args&&... args) 
		{
			try {
				if(!msg.empty()) {
					fmt::detail::vformat_to(m_Buffer, fmt::string_view{msg.data(), msg.size()}, fmt::make_format_args(std::forward<Args>(args)...));
				}
				m_Buffer.push_back('\0');
			} catch(const fmt::format_error&) {
				HELENA_MSG_EXCEPTION(
					"\n----------------------------------------\n"
					"|| Error: format syntax invalid!\n"
					"|| Format: {}"
					"\n----------------------------------------", msg);
				m_Buffer.clear();
				m_Buffer.push_back('\0');
			} catch(const std::bad_alloc&) {
				HELENA_MSG_EXCEPTION(
					"\n----------------------------------------\n"
					"|| Error: not enough memory for alloc\n"
					"|| Format: {}"
					"\n----------------------------------------", msg);
				m_Buffer.clear();
				m_Buffer.push_back('\0');
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
			return {GetData(), GetSize()};
		}

		[[nodiscard]] std::size_t GetSize() const noexcept {
			return m_Buffer.size() - 1;
		}

		[[nodiscard]] std::size_t IsEmpty() const noexcept {
			return !m_Buffer.size();
		}

		[[nodiscard]] operator std::string_view() const {
			return {GetData(), GetSize()};
		}

		void Clear() noexcept {
			m_Buffer.clear();
		}

		memory_buffer m_Buffer;
	};

	template <std::size_t Capacity>
	Format(const char (&array)[Capacity]) -> Format<Capacity>;
}

#endif // HELENA_TYPES_FORMAT_HPP