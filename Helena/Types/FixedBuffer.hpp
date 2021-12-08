#ifndef HELENA_TYPES_FIXEDBUFFER_HPP
#define HELENA_TYPES_FIXEDBUFFER_HPP

#include <Helena/Types/Format.hpp>
#include <Helena/Util/Length.hpp>

namespace Helena::Types
{
	template <std::size_t Capacity, typename Char = char, Util::ELengthPolicy Policy = Util::ELengthPolicy::Fixed>
	struct FixedBuffer
	{
		static constexpr std::size_t max_size = std::numeric_limits<std::size_t>::max();

		constexpr void FillBuffer(const Char* data, std::size_t size = max_size) noexcept 
		{
			if(!data && size) {
				Clear();
				return;
			}

			if(size == max_size) {
				size = Util::Length(Policy, data, Capacity);
			} 
			else if(size > Capacity) 
			{
				if constexpr(Policy == Util::ELengthPolicy::Truncate) {
					size = Capacity;
				} else if constexpr(Policy == Util::ELengthPolicy::Fixed) {
					Clear();
					return;
				}
			}

			std::memcpy(m_Buffer, data, size);
			m_Buffer[size] = '\0';
			m_Size = static_cast<size_type>(size);
		}

		using size_type	 =	std::conditional_t<Capacity <= std::numeric_limits<std::uint8_t>::max(), std::uint8_t,
							std::conditional_t<Capacity <= std::numeric_limits<std::uint16_t>::max(), std::uint16_t,
							std::conditional_t<Capacity <= std::numeric_limits<std::uint32_t>::max(), std::uint32_t, std::uint64_t>>>;

		constexpr FixedBuffer() = default;
		constexpr ~FixedBuffer() = default;

		constexpr FixedBuffer(const FixedBuffer& other) noexcept {
			FillBuffer(other.m_Buffer, other.m_Size);
		}

		constexpr FixedBuffer(FixedBuffer&& other) noexcept {
			FillBuffer(other.m_Buffer, other.m_Size);
		}

		constexpr FixedBuffer(const Char* data) noexcept {
			FillBuffer(data);
		}

		template <std::size_t CapacityOther>
		constexpr FixedBuffer(const Format<CapacityOther>& other) noexcept {
			FillBuffer(other.GetData(), other.GetSize());
		}

		constexpr void SetData(const Char* data) noexcept {
			FillBuffer(data);
		}

		[[nodiscard]] constexpr const Char* GetData() const noexcept {
			return m_Buffer;
		}

		[[nodiscard]] constexpr std::size_t GetSize() const noexcept {
			return m_Size;
		}

		[[nodiscard]] static constexpr std::size_t GetCapacity() noexcept {
			return Capacity;
		}

		template <std::size_t CapacityOther>
		[[nodiscard]] constexpr bool IsEqual(const FixedBuffer<CapacityOther>& other) const noexcept {
			if(m_Size != other.m_Size) return false;
			return std::equal(m_Buffer, m_Buffer + m_Size, other.m_Buffer, other.m_Buffer + other.m_Size);
		}

		template <std::size_t CapacityOther>
		[[nodiscard]] constexpr bool IsEqual(const Format<CapacityOther>& other) const noexcept {
			if(m_Size != other.GetSize()) return false;
			return std::equal(m_Buffer, m_Buffer + m_Size, other.GetData(), other.GetData() + other.GetSize());
		}

		[[nodiscard]] constexpr bool IsEmpty() const noexcept {
			return !m_Size;
		}

		constexpr void Clear() noexcept {
			m_Size = 0;
			m_Buffer[m_Size] = '\0';
		}

		constexpr bool operator==(const FixedBuffer& other) const noexcept {
			return IsEqual(other);
		}

		constexpr bool operator!=(const FixedBuffer& other) const noexcept {
			return !IsEqual(other);
		}

		constexpr FixedBuffer& operator=(const FixedBuffer& other) noexcept {
			FillBuffer(other.m_Buffer, other.m_Size);
			return *this;
		}

		constexpr FixedBuffer& operator=(FixedBuffer&& other) noexcept {
			FillBuffer(other.m_Buffer, other.m_Size);
			return *this;
		}

		constexpr FixedBuffer& operator=(const Char* data) noexcept {
			FillBuffer(data);
			return *this;
		}

		template <std::size_t CapacityOther>
		constexpr FixedBuffer& operator=(const Format<CapacityOther>& other) noexcept  {
			FillBuffer(other.GetData(), other.GetSize());
			return *this;
		}

		[[nodiscard]] constexpr operator std::basic_string_view<Char>() const noexcept {
			return {m_Buffer, m_Size};
		}

		Char m_Buffer[Capacity + 1] {};
		size_type m_Size {};
	};

	template <std::size_t Capacity>
	FixedBuffer(Format<Capacity>&&) -> FixedBuffer<Capacity, char>;

	template <std::size_t Capacity, typename Char = char>
	FixedBuffer(const Char (&array)[Capacity]) -> FixedBuffer<Capacity, Char>;
}

#endif // HELENA_TYPES_FIXEDBUFFER_HPP
