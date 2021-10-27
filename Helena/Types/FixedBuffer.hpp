#ifndef HELENA_TYPES_FIXEDBUFFER_HPP
#define HELENA_TYPES_FIXEDBUFFER_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Dependencies/FixedString.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Types/Format.hpp>

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

			if(size > Capacity) {
				Clear();
				return;
			}

			std::copy_n(data, size, m_Buffer.data());
			if(m_Size > size) {
				std::fill_n(m_Buffer.begin() + size, m_Size - size, '\0');
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

		template <std::size_t Cap>
		constexpr FixedBuffer(const Format<Cap>& other) noexcept {
			FillBuffer(other.GetData(), other.GetSize());
		}

		constexpr FixedBuffer(const FixedBuffer& other) noexcept {
			FillBuffer(other.GetData(), other.m_Size);
		}

		constexpr ~FixedBuffer() = default;

		constexpr FixedBuffer& operator=(const FixedBuffer& other) noexcept {
			FillBuffer(other.GetData(), other.m_Size);
			return *this;
		}

		template <std::size_t CapacityOther>
		constexpr FixedBuffer& operator=(const Format<CapacityOther>& other) noexcept  {
			FillBuffer(other.GetData(), other.GetSize());
			return *this;
		}

		constexpr void SetData(const Char* data, std::size_t length) noexcept {
			FillBuffer(data, length);
		}

		template <std::size_t Size>
		constexpr FixedBuffer& SetData(const Char (&data)[Size]) noexcept {
			FillBuffer(data, Size);
			return *this;
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

		[[nodiscard]] static constexpr std::size_t GetCapacity() noexcept {
			return Capacity;
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
			std::fill_n(m_Buffer.data(), m_Size, '\0');
			m_Size = 0;
		}

		[[nodiscard]] constexpr operator std::basic_string_view<Char>() const noexcept {
			return {GetData(), m_Size};
		}

		fixstr::basic_fixed_string<Char, Capacity> m_Buffer;
		std::size_t m_Size {};
	};

	template <std::size_t Capacity>
	FixedBuffer(const Format<Capacity>&) -> FixedBuffer<Capacity, char>;

	template <std::size_t Size, typename Char = char>
	FixedBuffer(const Char (&)[Size]) -> FixedBuffer<Size, Char>;
}

#endif // HELENA_TYPES_FIXEDBUFFER_HPP
