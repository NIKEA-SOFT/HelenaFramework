#ifndef HELENA_TYPES_FIXEDBUFFER_HPP
#define HELENA_TYPES_FIXEDBUFFER_HPP

#include <Helena/Util/Length.hpp>

#include <algorithm>
#include <string_view>

namespace Helena::Types
{
    template <std::size_t Capacity>
    class Format;

    template <std::size_t Capacity, Util::ELengthPolicy Policy = Util::ELengthPolicy::Fixed>
    struct FixedBuffer
    {
        constexpr void FillBuffer(const char* const data, std::size_t size = std::numeric_limits<std::size_t>::max()) noexcept
        {
            if(data)
            {
                if(size > Capacity)
                {
                    if(size == std::numeric_limits<std::size_t>::max()) {
                        size = Util::Length(Policy, data, Capacity);
                    }
                    else
                    {
                        switch(Policy)
                        {
                            case Util::ELengthPolicy::Truncate: {
                                size = Capacity;
                            } break;
                            case Util::ELengthPolicy::Fixed: {
                                size = 0;
                            } break;
                        }
                    }
                }

                *std::copy_n(data, size, m_Buffer) = '\0';
                m_Size = static_cast<size_type>(size);
                return;
            }

            Clear();
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

        constexpr FixedBuffer(const char* data) noexcept {
            FillBuffer(data);
        }

        constexpr FixedBuffer(const char* data, const std::size_t size) noexcept {
            FillBuffer(data, size);
        }

        template <std::size_t CapacityOther>
        FixedBuffer(const Format<CapacityOther>& other) noexcept {
            FillBuffer(other.GetData(), other.GetSize());
        }

        constexpr void SetData(const char* data) noexcept {
            FillBuffer(data);
        }

        [[nodiscard]] constexpr const char* GetData() const noexcept {
            return m_Buffer;
        }

        [[nodiscard]] constexpr std::size_t GetSize() const noexcept {
            return m_Size;
        }

        [[nodiscard]] static constexpr std::size_t GetCapacity() noexcept {
            return Capacity;
        }

        template <std::size_t CapacityOther>
        [[nodiscard]] constexpr bool Equal(const FixedBuffer<CapacityOther>& other) const noexcept {
            if(m_Size != other.m_Size) return false;
            return std::equal(m_Buffer, m_Buffer + m_Size, other.m_Buffer, other.m_Buffer + other.m_Size);
        }

        template <std::size_t CapacityOther>
        [[nodiscard]] constexpr bool Equal(const Format<CapacityOther>& other) const noexcept {
            if(m_Size != other.GetSize()) return false;
            return std::equal(m_Buffer, m_Buffer + m_Size, other.GetData(), other.GetData() + other.GetSize());
        }

        [[nodiscard]] constexpr bool Empty() const noexcept {
            return !m_Size;
        }

        constexpr void Clear() noexcept {
            m_Size = 0;
            m_Buffer[m_Size] = '\0';
        }

        [[nodiscard]] constexpr bool operator==(const FixedBuffer& other) const noexcept {
            return Equal(other);
        }

        [[nodiscard]] constexpr bool operator!=(const FixedBuffer& other) const noexcept {
            return !Equal(other);
        }

        constexpr FixedBuffer& operator=(const FixedBuffer& other) noexcept {
            FillBuffer(other.m_Buffer, other.m_Size);
            return *this;
        }

        constexpr FixedBuffer& operator=(FixedBuffer&& other) noexcept {
            FillBuffer(other.m_Buffer, other.m_Size);
            return *this;
        }

        constexpr FixedBuffer& operator=(const char* data) noexcept {
            FillBuffer(data);
            return *this;
        }

        template <std::size_t CapacityOther>
        constexpr FixedBuffer& operator=(const Format<CapacityOther>& other) noexcept  {
            FillBuffer(other.GetData(), other.GetSize());
            return *this;
        }

        [[nodiscard]] constexpr operator std::string_view() const noexcept {
            return {m_Buffer, m_Size};
        }

        char m_Buffer[Capacity + 1] {};
        size_type m_Size {};
    };

    template <std::size_t Capacity>
    FixedBuffer(Format<Capacity>&&) -> FixedBuffer<Capacity>;

    template <std::size_t Capacity>
    FixedBuffer(const char (&)[Capacity]) -> FixedBuffer<Capacity>;
}

#endif // HELENA_TYPES_FIXEDBUFFER_HPP
