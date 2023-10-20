#ifndef HELENA_TYPES_FIXEDBUFFER_HPP
#define HELENA_TYPES_FIXEDBUFFER_HPP

#include <Helena/Util/String.hpp>
#include <format>
#include <memory>
#include <utility>

namespace Helena::Types
{
    template <std::size_t _Capacity, typename Char = char, std::size_t SearchDepth = (std::numeric_limits<std::size_t>::max)(), typename = std::char_traits<Char>>
    struct FixedBuffer
    {
        static_assert(_Capacity > 0, "Capacity is too small");

        using size_type = std::conditional_t<_Capacity <= (std::numeric_limits<std::uint8_t>::max)(), std::uint8_t,
            std::conditional_t<_Capacity <= (std::numeric_limits<std::uint16_t>::max)(), std::uint16_t,
            std::conditional_t<_Capacity <= (std::numeric_limits<std::uint32_t>::max)(), std::uint32_t, std::uint64_t>>>;

        constexpr void FillBuffer(const Char* const data, std::size_t size = SearchDepth) noexcept
        {
            if(data) {
                size = Util::String::LengthTruncated(data, (std::min)(_Capacity, size));
                *std::copy_n(data, size, m_Buffer) = 0;
                m_Size = static_cast<size_type>(size);
                return;
            }

            Clear();
        }

        constexpr FixedBuffer() = default;
        constexpr ~FixedBuffer() = default;

        constexpr FixedBuffer(const FixedBuffer& other) noexcept {
            operator=(other);
        }

        constexpr FixedBuffer(FixedBuffer&& other) noexcept {
            operator=(std::move(other));
        }

        constexpr FixedBuffer(const Char* data) noexcept {
            FillBuffer(data);
        }

        constexpr FixedBuffer(const Char* data, const std::size_t size) noexcept {
            FillBuffer(data, size);
        }

        constexpr FixedBuffer& operator=(const FixedBuffer& other) noexcept
        {
            if(this != std::addressof(other)) [[likely]] {
                m_Size = other.m_Size;
                *std::copy_n(other.m_Buffer, m_Size, m_Buffer) = 0;
            }
            return *this;
        }

        constexpr FixedBuffer& operator=(FixedBuffer&& other) noexcept
        {
            if(this != std::addressof(other)) [[likely]] {
                m_Size = std::exchange(other.m_Size, 0);
                *std::copy_n(other.m_Buffer, m_Size, m_Buffer) = 0;
                other.m_Buffer[other.m_Size] = 0;
            }
            return *this;
        }

        constexpr FixedBuffer& operator=(const Char* data) noexcept {
            FillBuffer(data);
            return *this;
        }

        [[nodiscard]] constexpr const Char* Data() const noexcept {
            return m_Buffer;
        }

        [[nodiscard]] constexpr size_type Size() const noexcept {
            return m_Size;
        }

        [[nodiscard]] static constexpr size_type Capacity() noexcept {
            return _Capacity;
        }

        template <std::size_t CapacityOther>
        [[nodiscard]] constexpr bool Equal(const FixedBuffer<CapacityOther>& other) const noexcept {
            if(m_Size != other.m_Size) return false;
            return std::equal(m_Buffer, m_Buffer + m_Size, other.m_Buffer, other.m_Buffer + other.m_Size);
        }

        [[nodiscard]] constexpr bool Equal(const std::size_t expectedSize) const noexcept {
            return m_Size == expectedSize;
        }

        [[nodiscard]] constexpr bool Empty() const noexcept {
            return !m_Size;
        }

        constexpr void Clear() noexcept {
            m_Size = 0;
            m_Buffer[m_Size] = 0;
        }

        [[nodiscard]] constexpr bool operator==(const FixedBuffer& other) const noexcept {
            return Equal(other);
        }

        [[nodiscard]] constexpr bool operator!=(const FixedBuffer& other) const noexcept {
            return !Equal(other);
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return !Empty();
        }

        [[nodiscard]] constexpr operator const Char*() const noexcept {
            return m_Buffer;
        }

        [[nodiscard]] constexpr operator std::basic_string_view<Char>() const noexcept {
            return {m_Buffer, m_Size};
        }

        Char m_Buffer[_Capacity + 1]{};
        size_type m_Size{};
    };

    template <typename Char, std::size_t Capacity>
    FixedBuffer(const Char(&)[Capacity]) -> FixedBuffer<Capacity, Char>;
}

namespace std
{
    template <std::size_t N, typename Char>
    struct formatter<Helena::Types::FixedBuffer<N, Char>, Char>
    {
        constexpr auto parse(auto& ctx) {
            return ctx.begin();
        }

        auto format(const Helena::Types::FixedBuffer<N, Char>& name, auto& ctx) const {
            return std::format_to(ctx.out(), "{}", static_cast<const Char*>(name));
        }
    };
}

#endif // HELENA_TYPES_FIXEDBUFFER_HPP