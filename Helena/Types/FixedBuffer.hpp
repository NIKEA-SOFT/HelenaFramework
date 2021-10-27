#ifndef HELENA_TYPES_FIXEDBUFFER_HPP
#define HELENA_TYPES_FIXEDBUFFER_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Dependencies/FixedString.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Types/Format.hpp>
#include <Helena/Util/Length.hpp>

namespace Helena::Types
{
	template <std::size_t Capacity, typename Char = char, Util::ELengthPolicy Policy = Util::ELengthPolicy::Fixed>
	class FixedBuffer
	{
	public:
		using value_type =	fixstr::basic_fixed_string<Char, Capacity>;
		using size_type	 =	std::conditional_t<Capacity <= std::numeric_limits<std::uint8_t>::max(), std::uint8_t,
							std::conditional_t<Capacity <= std::numeric_limits<std::uint16_t>::max(), std::uint16_t,
							std::conditional_t<Capacity <= std::numeric_limits<std::uint32_t>::max(), std::uint32_t, std::uint64_t>>>;

		[[nodiscard]] static constexpr std::size_t GetCapacity() noexcept {
			return Capacity;
		}

	private:
		constexpr void FillBuffer(const Char* data, std::size_t size) noexcept 
		{
			if(!data && size || size > Capacity) {
				Clear();
				return;
			}

			std::copy_n(data, size, m_Buffer.data());
			if(m_Size > static_cast<size_type>(size)) {
				std::fill_n(m_Buffer.begin() + size, m_Size - static_cast<size_type>(size), '\0');
			}

			m_Size = static_cast<size_type>(size);
		}

	public:

		constexpr FixedBuffer() = default;

		explicit constexpr FixedBuffer(const Char* data, std::size_t length) noexcept {
			FillBuffer(data, length);
		}

		constexpr FixedBuffer(const Char* data) noexcept {
			FillBuffer(data, Util::Length(data, Capacity, Policy));
		}

		constexpr FixedBuffer(std::basic_string_view<Char> data) noexcept {
			FillBuffer(data.data(), data.length());
		}

		template <std::size_t Cap>
		constexpr FixedBuffer(const Format<Cap>& other) noexcept {
			FillBuffer(other.GetData(), other.GetSize());
		}

		explicit constexpr FixedBuffer(const FixedBuffer& other) noexcept {
			FillBuffer(other.GetData(), other.GetSize());
		}

		constexpr ~FixedBuffer() = default;

		constexpr FixedBuffer& operator=(const FixedBuffer& other) noexcept {
			FillBuffer(other.GetData(), other.GetSize());
			return *this;
		}

		template <std::size_t Capacity>
		constexpr FixedBuffer& operator=(const Format<Capacity>& other) noexcept  {
			FillBuffer(other.GetData(), other.GetSize());
			return *this;
		}

		constexpr void SetData(const Char* data, std::size_t length) noexcept {
			FillBuffer(data, length);
		}

		template <std::size_t Size>
		constexpr void SetData(const Char (&data)[Size]) noexcept {
			FillBuffer(data, Size);
		}

		constexpr void SetData(std::basic_string_view<Char> data) noexcept {
			FillBuffer(data.data(), data.length());
		}

		[[nodiscard]] constexpr Char* GetData() noexcept {
			return m_Buffer.data();
		}

		[[nodiscard]] constexpr const Char* GetData() const noexcept {
			return m_Buffer.data();
		}

		[[nodiscard]] constexpr std::size_t GetSize() const noexcept {
			return m_Size;
		}

		template <std::size_t Capacity>
		[[nodiscard]] constexpr bool IsEqual(const FixedBuffer<Capacity>& other) const noexcept {
			if(m_Size != other.m_Size) return false;
			return std::equal(m_Buffer.cbegin(), m_Buffer.cbegin() + m_Size, other.m_Buffer.cbegin(), other.m_Buffer.cbegin() + other.m_Size);
		}

		[[nodiscard]] constexpr bool IsEmpty() const noexcept {
			return !m_Size;
		}

		[[nodiscard]] constexpr void Clear() noexcept {
			std::fill_n(GetData(), GetSize(), '\0');
			m_Size = 0;
		}

		[[nodiscard]] constexpr operator std::basic_string_view<Char>() const noexcept {
			return {GetData(), GetSize()};
		}

		value_type m_Buffer;
		size_type m_Size {};
	};

	template <std::size_t Capacity>
	FixedBuffer(const Format<Capacity>&) -> FixedBuffer<Capacity, char>;
}

#endif // HELENA_TYPES_FIXEDBUFFER_HPP
