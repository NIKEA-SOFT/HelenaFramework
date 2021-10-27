#ifndef HELENA_TYPES_FIXEDBUFFER_HPP
#define HELENA_TYPES_FIXEDBUFFER_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Dependencies/FixedString.hpp>
#include <Helena/Types/Hash.hpp>

namespace Helena::Types
{
	template <std::size_t Capacity, typename Char = char>
	class FixedBuffer
	{
		constexpr void FillBuffer(const Char* data, std::size_t size) noexcept 
		{
			if(!std::is_constant_evaluated()) {
				HELENA_ASSERT(data, "Data ptr is null");
			}

			std::copy_n(data, size, m_Buffer.data());

			if(m_Size > size) {
				std::fill_n(m_Buffer.begin() + size, size - m_Size, '\0');
			}

			m_Size = size;
		}

	public:
		constexpr FixedBuffer() = default;

		constexpr FixedBuffer(const Char* data, std::size_t length) noexcept {
			FillBuffer(data, length);
		}

		template <std::size_t Size>
		constexpr FixedBuffer(const Char (&data)[Size]) noexcept {
			FillBuffer(data, Size);
		}

		constexpr FixedBuffer(std::basic_string_view<Char> data) noexcept {
			FillBuffer(data.data(), data.length());
		}

		constexpr FixedBuffer(const FixedBuffer& other) noexcept {
			FillBuffer(other.GetData(), other.m_Size);
		}

		constexpr ~FixedBuffer() = default;

		constexpr FixedBuffer& operator=(const FixedBuffer& other) noexcept {
			FillBuffer(other.GetData(), other.m_Size);
		}

		template <std::size_t Size>
		constexpr FixedBuffer& operator=(const Char (&data)[Size]) noexcept {
			FillBuffer(data, Size);
		}

		constexpr FixedBuffer& operator=(const std::basic_string_view<Char> data) noexcept {
			FillBuffer(data.data(), data.length());
		}

		constexpr void SetData(const Char* data, std::size_t length) noexcept {
			FillBuffer(data, length);
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

		[[nodiscard]] static constexpr std::size_t GetCapacity() noexcept {
			return Capacity;
		}

		[[nodiscard]] constexpr bool IsEmpty() const noexcept {
			return !m_Size;
		}

		[[nodiscard]] constexpr Char& operator[](std::size_t position) { 
			if(!std::is_constant_evaluated()) {
				HELENA_ASSERT(position < m_Size, "Position offset: {} is overflowed, capacity: {}", position, Capacity);
			}
			return m_Buffer[position]; 
		}

		[[nodiscard]] constexpr const Char& operator[](std::size_t position) const { 
			if(!std::is_constant_evaluated()) {
				HELENA_ASSERT(position < Capacity, "Position offset: {} is overflowed, capacity: {}", position, Capacity);
			}
			return m_Buffer[position]; 
		}

		[[nodiscard]] constexpr operator std::basic_string_view<Char>() const noexcept {
			return {GetData(), m_Size};
		}

		fixstr::basic_fixed_string<Char, Capacity> m_Buffer;
		std::size_t m_Size {};
	};
}

#endif // HELENA_TYPES_FIXEDBUFFER_HPP
