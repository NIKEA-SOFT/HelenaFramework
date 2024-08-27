#ifndef HELENA_LOGGING_INTERNAL_LOGGERBUFFER_HPP
#define HELENA_LOGGING_INTERNAL_LOGGERBUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Helena::Logging::Internal
{
    class LoggerBuffer
    {
        static constexpr auto m_BufferCapacity{4000};

    public:
        LoggerBuffer()
            : m_InitData{new std::byte[m_BufferCapacity]}
            , m_Data{m_InitData}
            , m_Size{}
            , m_Capacity{m_BufferCapacity} {
        }
        ~LoggerBuffer() = default;
        LoggerBuffer(const LoggerBuffer&) = delete;
        LoggerBuffer(LoggerBuffer&&) = delete;
        LoggerBuffer& operator=(const LoggerBuffer&) = delete;
        LoggerBuffer& operator=(LoggerBuffer&& other) = delete;

        template <typename Char>
        void push_back(const Char value) {
            reallocate(m_Size + sizeof(Char));
            std::memcpy(m_Data + m_Size, std::addressof(value), sizeof(Char));
            m_Size += sizeof(Char);
        }

        template <typename Char>
        void append(const Char* data, std::size_t size) {
            const auto bytes = size * sizeof(Char);
            reallocate(m_Size + bytes);
            std::memcpy(m_Data + m_Size, data, bytes);
            m_Size += bytes;
        }

        void resize(std::size_t size) {
            reallocate(size);
            m_Size = size;
        }

        void erase(std::size_t offset, std::size_t size) {
            if(!offset && size == 10) { // specific impl for colors (needed for optimize)
                m_Data = m_Data + offset;
                m_Capacity -= offset;
            } else {
                std::memmove(m_Data + offset, m_Data + offset + size, m_Size - offset - size);
            }
            m_Size -= size;
        }

        template <typename Char>
        Char* data() const noexcept {
            return reinterpret_cast<Char*>(m_Data);
        }

        template <typename Char>
        std::size_t size() const noexcept {
            return m_Size / sizeof(Char);
        }

        std::size_t bytes() const noexcept {
            return m_Size;
        }

        void Reset() noexcept
        {
            if(m_Data < m_InitData || m_Data >= m_InitData + m_BufferCapacity) [[unlikely]] {
                delete[] m_Data;
            }

            m_Data = m_InitData;
            m_Size = 0;
            m_Capacity = m_BufferCapacity;
        }

        template <typename Char>
        [[nodiscard]] std::basic_string_view<Char> View() noexcept {
            return std::basic_string_view<Char>{reinterpret_cast<Char*>(m_Data), m_Size / sizeof(Char)};
        }

    private:
        void reallocate(std::size_t requiredBytes) {
            if(requiredBytes > m_Capacity) [[unlikely]] {
                m_Capacity = (std::max)(m_Capacity * 2, requiredBytes);
                std::memcpy(m_Data, std::exchange(m_Data, new std::byte[m_Capacity]), m_Size);
            }
        }

    private:
        std::byte* m_InitData;
        std::byte* m_Data;
        std::size_t m_Size;
        std::size_t m_Capacity;
    };

    inline thread_local LoggerBuffer m_LoggerBuffer;

    template <typename Char>
    LoggerBuffer& GetCachedBuffer() noexcept {
        auto& buffer = m_LoggerBuffer; buffer.Reset();
        return buffer;
    }
}

template <>
class std::back_insert_iterator<Helena::Logging::Internal::LoggerBuffer>
{
public:
    using iterator_category = output_iterator_tag;
    using value_type = void;
    using pointer = void;
    using reference = void;

    using container_type = Helena::Logging::Internal::LoggerBuffer;
    using difference_type = ptrdiff_t;

    constexpr explicit back_insert_iterator(container_type& container) noexcept
        : m_Container(std::addressof(container)) {}

    template <typename T>
    constexpr back_insert_iterator& operator=(const T& value) {
        m_Container->push_back(value);
        return *this;
    }

    template <typename T>
    constexpr back_insert_iterator& operator=(T&& value) {
        m_Container->push_back(std::move(value));
        return *this;
    }

    [[nodiscard]] constexpr back_insert_iterator& operator*() noexcept {
        return *this;
    }

    constexpr back_insert_iterator& operator++() noexcept {
        return *this;
    }

    constexpr back_insert_iterator operator++(int) noexcept {
        return *this;
    }

protected:
    container_type* m_Container;
};

#endif // HELENA_LOGGING_INTERNAL_LOGGERBUFFER_HPP
