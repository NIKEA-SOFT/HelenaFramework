#ifndef HELENA_UTIL_FORMAT_HPP
#define HELENA_UTIL_FORMAT_HPP

#include <Helena/Engine/Log.hpp>

#include <format>

namespace Helena::Util
{
    namespace Internal {
        namespace FormatStorage {
            /*
            * Let's give some memory to get rid of allocations.
            * This is a little trick to optimize the formatting.
            * WARNING: m_RingCache defines the maximum number of buffers that can be used at the same time.
            * This means that if more than one m_RingCache buffer is used at the same time, the data will be overwritten.
            * This is an efficient solution to use Format for temporary formatted strings,
            * e.g. before outputting something or immediately after receiving a formatted string.
            *
            * Usage example:
            * 1) If you need to store formatted string permanently, then it's better to copy the result
            * auto data = Format("Hello {}", "world");
            *
            * 2) If you need a format string to use right now
            * const auto& data = Format("Hello {}", "world");
            * print_message(data);
            *
            * The allocator is not used here for backwards compatibility
            *
            * WARNING: Using this method at the time of freeing variables with static lifetimes is undefined behavior!
            * For example: destructors of static variables are such. In this case, you can use std::vformat
            */
            static constexpr std::size_t m_RingCache = 20;
            template <typename Char>
            static thread_local std::basic_string<Char> m_Buffer[m_RingCache]{};
            static thread_local std::size_t m_BufferID{};

            template <typename Char, typename... Args>
            [[nodiscard]] const std::basic_string<Char>& Format(const std::basic_string_view<Char> msg, Args&&... args) noexcept
            {
                auto& buffer = m_Buffer<Char>[std::exchange(m_BufferID, (m_BufferID + 1) % m_RingCache)];

                try {
                    buffer.resize(0);
                    std::vformat_to(std::back_inserter(buffer), msg, std::make_format_args(std::forward<Args>(args)...));
                } catch(const std::format_error&) {
                    buffer.resize(0);
                    Log::Message<Log::Exception>(
                        "\n----------------------------------------\n"
                        "|| Error: format syntax invalid!\n"
                        "|| Format: {}"
                        "\n----------------------------------------", msg);
                } catch(const std::bad_alloc&) {
                    buffer.resize(0);
                    Log::Message<Log::Exception>(
                        "\n----------------------------------------\n"
                        "|| Error: not enough memory for alloc\n"
                        "|| Format: {}"
                        "\n----------------------------------------", msg);
                }

                return buffer;
            }
        };
    }

    template <typename... Args>
    [[nodiscard]] decltype(auto) Format(const std::string_view msg, Args&&... args) {
        return Internal::FormatStorage::Format(msg, std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[nodiscard]] decltype(auto) Format(const std::wstring_view msg, Args&&... args) {
        return Internal::FormatStorage::Format(msg, std::forward<Args>(args)...);
    }
}

#endif // HELENA_UTIL_FORMAT_HPP