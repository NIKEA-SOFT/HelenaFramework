#ifndef HELENA_UTIL_STRING_HPP
#define HELENA_UTIL_STRING_HPP

#include <Helena/Engine/Log.hpp>
#include <Helena/Types/ReferencePointer.hpp>

#include <array>
#include <algorithm>
#include <string>
#include <format>

namespace Helena::Util
{
    namespace Internal::FormatImpl
    {
        /*
        * Let's give some memory to get rid of allocations.
        * This is a little trick to optimize the formatting.
        * WARNING: m_BufferCount defines the maximum number of buffers that can be used at the same time.
        * This means that if more than one m_BufferCount buffer is used at the same time, the data will be overwritten.
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
        * The allocator is not used here for backwards compatibility.
        */
        inline constexpr std::size_t m_BufferCount{20};
        inline constexpr std::size_t m_BufferCapacity{512};
        inline thread_local std::size_t m_BufferID{};

        template <typename Char>
        inline thread_local auto m_Buffers = []{
            std::array<Types::ReferencePointer<std::basic_string<Char>>, m_BufferCount> array;
            for(auto& buffer : array) {
                buffer = decltype(array)::value_type::Create(m_BufferCapacity, 0);
            }
            return array;
        }();

        template <typename Char>
        [[nodiscard]] decltype(auto) BufferSwitch() noexcept
        {
            const auto id = std::exchange(m_BufferID, (m_BufferID + 1) % m_BufferCount);
            auto& buffer = m_Buffers<Char>[id];
            if(buffer) [[likely]] {
                return buffer;
            }

            return (buffer = std::remove_cvref_t<decltype(buffer)>::Create(m_BufferCapacity, 0));
        }

        template <typename Char, typename... Args>
        [[nodiscard]] const std::basic_string<Char>& Format(const std::basic_string_view<Char> msg, Args&&... args) noexcept
        {
            const auto& buffer = BufferSwitch<Char>();

            try {
                buffer->resize(0);
                std::vformat_to(std::back_inserter(*buffer), msg, std::make_format_args(args...));
            } catch(const std::format_error&) {
                buffer->resize(0);
                Log::Message<Log::Exception>(
                    "\n----------------------------------------\n"
                    "|| Error: format syntax invalid!\n"
                    "|| Format: {}"
                    "\n----------------------------------------", msg);
            } catch(const std::bad_alloc&) {
                buffer->resize(0);
                Log::Message<Log::Exception>(
                    "\n----------------------------------------\n"
                    "|| Error: not enough memory for alloc\n"
                    "|| Format: {}"
                    "\n----------------------------------------", msg);
            }

            return *buffer;
        }
    }

    class String
    {
    public:
        /*
        * WARNING:
        * The format is based on circular buffers that are reused.
        * You should not store returned strings by reference, otherwise, sooner or later the
        * buffer you store will be reused and the data will change.
        * For storage you can copy string by value.
        * This approach allows you not to allocate memory when formatting strings.
        */
        template <typename... Args>
        [[nodiscard]] static const std::string& Format(const std::string_view msg, Args&&... args) {
            return Internal::FormatImpl::Format(msg, std::forward<Args>(args)...);
        }

        template <typename... Args>
        [[nodiscard]] static const std::wstring& Format(const std::wstring_view msg, Args&&... args) {
            return Internal::FormatImpl::Format(msg, std::forward<Args>(args)...);
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

#endif // HELENA_UTIL_STRING_HPP