#ifndef HELENA_TYPES_FORMAT_HPP
#define HELENA_TYPES_FORMAT_HPP

#include <Helena/Debug/Assert.hpp>

#include <string_view>
#include <concepts>

namespace Helena::Types
{
    template <std::size_t Capacity>
    class Format
    {
        using memory_buffer = fmt::basic_memory_buffer<char, Capacity + 1>;

        void Formatting(std::string_view msg, fmt::format_args args) noexcept 
        {
            try {
                Clear();
                fmt::detail::vformat_to(m_Buffer, fmt::string_view{msg.data(), msg.size()}, args);
                m_Buffer.push_back('\0');
            } catch(const fmt::format_error&) {
                Clear();
                Log::Console<Log::Exception>(
                    "\n----------------------------------------\n"
                    "|| Error: format syntax invalid!\n"
                    "|| Format: {}"
                    "\n----------------------------------------", msg);
            } catch(const std::bad_alloc&) {
                Clear();
                Log::Console<Log::Exception>(
                    "\n----------------------------------------\n"
                    "|| Error: alloc memory failed!\n"
                    "|| Format: {}"
                    "\n----------------------------------------", msg);
            }
        }

    public:
        template <std::convertible_to<std::string_view> T, typename... Args>
        Format(T&& msg, Args&&... args) noexcept {
            Formatting(std::forward<T>(msg), fmt::make_format_args(std::forward<Args>(args)...));
        }

        Format() = default;
        ~Format() = default;
        Format(const Format&) = delete;
        Format(Format&&) noexcept = default;
        Format& operator=(const Format&) = delete;
        Format& operator=(Format&&) = default;

        void Append(char c) noexcept {
            m_Buffer.push_back(c);
        }

        void Append(const std::string_view buffer) noexcept {
            m_Buffer.append(buffer);
        }

        template <std::convertible_to<std::string_view> T, typename... Args>
        void Edit(T&& msg, Args&&... args) noexcept {
            Formatting(std::forward<T>(msg), fmt::make_format_args(std::forward<Args>(args)...));
        }

        [[nodiscard]] const char* GetData() const noexcept {
            return m_Buffer.data();
        }

        [[nodiscard]] std::string_view GetBuffer() const noexcept {
            return {GetData(), GetSize()};
        }

        [[nodiscard]] std::size_t GetSize() const noexcept {
            return m_Buffer.size();
        }

        [[nodiscard]] std::size_t Empty() const noexcept {
            return !GetSize();
        }

        [[nodiscard]] operator std::string_view() const {
            return {GetData(), GetSize()};
        }

        char& operator[](std::size_t index) const {
            HELENA_ASSERT(index < m_Buffer.size(), "Buffer overflowed");
            return m_Buffer[index];
        }

        char& operator[](std::size_t index) {
            HELENA_ASSERT(index < m_Buffer.size(), "Buffer overflowed");
            return m_Buffer[index];
        }

        void Clear() noexcept {
            m_Buffer.clear();
        }

    private:
        memory_buffer m_Buffer;
    };

    template <std::size_t Size>
    Format(const char(&)[Size]) -> Format<Size>;
}

#endif // HELENA_TYPES_FORMAT_HPP