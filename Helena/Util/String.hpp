#ifndef HELENA_UTIL_STRING_HPP
#define HELENA_UTIL_STRING_HPP

#include <Helena/Engine/Log.hpp>

#include <algorithm>
#include <cstring>
#include <format>
#include <iterator>
#include <utility>

namespace Helena::Util
{
    namespace Internal::FormatImpl {
        template <typename Char>
        [[nodiscard]] auto& GetCachedBuffer() noexcept;
    }

    class String
    {
    public:
        /**
        * @brief Formatting message
        * @tparam Char Type of characters
        * @tparam Trait Type of std::char_traits<Char>
        * @param msg Formatting text
        * @param args Arguments for formatter
        * @return std::basic_string<Char, Traits> with data or empty if exception is throwed.
        * @note Result can be moved and stored.
        */
        template <typename Char, typename Traits, typename... Args>
        static auto Format(const std::basic_string_view<Char, Traits> msg, Args&&... args) noexcept
        {
            try {
                return std::vformat(msg, std::make_format_args<typename Log::Print<Char>::Context>(args...));
            } catch(const std::format_error&) {
                Log::Message<Log::Exception>(Log::Print<Char>::FormatError, msg);
            } catch(const std::bad_alloc&) {
                Log::Message<Log::Exception>(Log::Print<Char>::AllocateError, msg);
            }

            return std::basic_string<Char, Traits>{};
        }

        template <typename Char, typename... Args>
        static auto Format(const Char* msg, Args&&... args) noexcept {
            return Format(std::basic_string_view<Char>{msg}, std::forward<Args>(args)...);
        }

        /**
        * @brief Formatting message into temp view buffer (for use result right now and forgot).
        * @tparam Char Type of characters
        * @tparam Trait Type of Char traits
        * @param msg Formatting text
        * @param args Arguments for formatter
        * @return std::basic_string_view<Char, Traits> with data or empty if exception is throwed.
        * @note This function is effective for cases when you need to immediately use the formatting result and forget about it.
        * The implementation is built around a ring cached buffer, which can be overwritten if you store the result of a FormatView,
        * so you should never do this.
        * Implementation: Each thread has 20 ring buffers with a cache size of 512 bytes.
        * This is efficient, you won't have to allocate memory for formatting, but this implementation has a side effect - you should
        * not store the result returned by this function, otherwise after 20 calls to FormatView the memory may be overwritten because ring buffer.
        * @warning You should not store the result returned by this function, it is unsafe.
        */
        template <typename Char, typename Traits, typename... Args>
        [[nodiscard]] static std::basic_string_view<Char, Traits> FormatView(std::basic_string_view<Char, Traits> msg, Args&&... args) noexcept
        {
            try {
                auto& buffer = Internal::FormatImpl::GetCachedBuffer<Char>();
                std::vformat_to(std::back_inserter(buffer), msg, std::make_format_args<typename Log::Print<Char>::Context>(args...));
                return buffer.template View<Char, Traits>();
            } catch(const std::format_error&) {
                Log::Message<Log::Exception>(Log::Print<Char>::FormatError, msg);
            } catch(const std::bad_alloc&) {
                Log::Message<Log::Exception>(Log::Print<Char>::AllocateError, msg);
            }

            return std::basic_string_view<Char, Traits>{};
        }

        template <typename Char, typename... Args>
        static auto FormatView(const Char* msg, Args&&... args) noexcept {
            return FormatView(std::basic_string_view<Char>{msg}, std::forward<Args>(args)...);
        }

        /**
        * @brief Get the truncated length of the data
        * @tparam Char Type of characters
        * @tparam Trait Type of std::char_traits<Char>
        * @param data Pointer to characters
        * @param max Maximum search range (with null terminator)
        * @return Returns 0 if data is nullptr. Otherwise, the length is returned.
        */
        template <typename Char, typename Trait = std::char_traits<Char>>
        [[nodiscard]] static constexpr std::size_t LengthTruncated(const Char* data, std::size_t max) {
            if(!data || !max) return 0;
            const auto found = Trait::find(data, max, 0);
            return found ? std::distance(data, found) : max;
        }

        /**
        * @brief Get the length of the data
        * @tparam Char Type of characters
        * @tparam Trait Type of std::char_traits<Char>
        * @param data Pointer to characters
        * @return Returns 0 if data is nullptr. Otherwise, the length is returned.
        */
        template <typename Char, typename Trait = std::char_traits<Char>>
        [[nodiscard]] static constexpr std::size_t Length(const Char* data) {
            return data ? Trait::length(data) : 0;
        }

        /**
        * @brief Get the length of the data
        * @tparam Char Type of characters
        * @tparam Trait Type of std::char_traits<Char>
        * @param data Pointer to characters
        * @param max Maximum search range (with null terminator)
        * @return Returns 0 if data is nullptr or null terminator was not found in search range.
        * Otherwise, the length is returned.
        */
        template <typename Char, typename Trait = std::char_traits<Char>>
        [[nodiscard]] static constexpr std::size_t Length(const Char* data, std::size_t max) {
            if(!data || !max) return 0;
            const auto found = Trait::find(data, max, 0);
            return found ? std::distance(data, found) : 0;
        }
    };
}

namespace Helena::Util::Internal::FormatImpl
{
    /*
    * Let's give some memory to get rid of allocations.
    * This is a little trick to optimize the formatting.
    * WARNING: m_BufferCount defines the maximum number of buffers that can be used at the same time from some thread.
    * This means that if more than one m_BufferCount buffer is used at the same time, the data will be overwritten.
    * This is an efficient solution to use Format for temporary formatted strings,
    * e.g. before outputting something or immediately after receiving a formatted string.
    *
    * Usage example:
    * 1) If you need to store formatted string permanently, then it's better to copy the result
    * auto data = Format("Hello {}", "world");
    *
    * 2) If you need a format string to use right now
    * const auto data = FormatView("Hello {}", "world");
    * print_message(data);
    */
    class FormatBuffer
    {
        static constexpr auto m_BufferCapacity{512};
        static constexpr auto m_BufferCount{20};

    public:
        FormatBuffer()
            : m_InitData{new std::byte[m_BufferCount * m_BufferCapacity]}
            , m_Data{m_InitData}
            , m_RingID{}
            , m_Size{}
            , m_Capacity{m_BufferCapacity} {}
        FormatBuffer(const FormatBuffer&) = delete;
        FormatBuffer(FormatBuffer&&) = delete;
        FormatBuffer& operator=(const FormatBuffer&) = delete;
        FormatBuffer& operator=(FormatBuffer&& other) = delete;
        ~FormatBuffer() {}

        template <typename Char>
        void push_back(Char&& value) {
            reallocate(m_Size + sizeof(Char));
            std::memcpy(static_cast<std::byte*>(m_Data) + m_Size, &value, sizeof(Char));
            m_Size += sizeof(Char);
        }

        template <typename Char>
        void append(const Char* data, std::size_t size) {
            const auto bytes = size * sizeof(Char);
            reallocate(m_Size + bytes);
            std::memcpy(static_cast<std::byte*>(m_Data) + m_Size, data, bytes);
            m_Size += bytes;
        }

        template <typename Char>
        void Reset()
        {
            if(m_Capacity != m_BufferCapacity) [[unlikely]] {
                delete[] static_cast<std::byte*>(m_Data);
                m_Data = m_InitData;
                m_Capacity = m_BufferCapacity;
            }

            const auto id = std::exchange(m_RingID, (m_RingID + 1) % m_BufferCount);
            m_Data = static_cast<std::byte*>(m_InitData) + id * m_BufferCapacity;
            m_Size = 0;
        }

        template <typename Char, typename Traits = std::char_traits<Char>>
        [[nodiscard]] std::basic_string_view<Char, Traits> View() noexcept {
            push_back<Char>(Char{}); m_Size -= sizeof(Char);
            return std::basic_string_view<Char, Traits>{static_cast<Char*>(m_Data), m_Size / sizeof(Char)};
        }

    private:
        void reallocate(std::size_t requiredBytes) {
            if(requiredBytes > m_Capacity) [[unlikely]] {
                m_Capacity = (std::max)(m_Capacity * 2, requiredBytes);
                std::memcpy(m_Data, std::exchange(m_Data, new std::byte[m_Capacity]), m_Size);
            }
        }

    private:
        void* m_InitData;
        void* m_Data;
        std::size_t m_RingID;
        std::size_t m_Size;
        std::size_t m_Capacity;
    };

    inline thread_local FormatBuffer m_RingBuffer;

    template <typename Char>
    [[nodiscard]] auto& GetCachedBuffer() noexcept {
        auto& buffer = m_RingBuffer; buffer.Reset<Char>();
        return buffer;
    }
}

template <>
class std::back_insert_iterator<Helena::Util::Internal::FormatImpl::FormatBuffer>
{
public:
    using iterator_category = output_iterator_tag;
    using value_type = void;
    using pointer = void;
    using reference = void;

    using container_type = Helena::Util::Internal::FormatImpl::FormatBuffer;
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

#endif // HELENA_UTIL_STRING_HPP