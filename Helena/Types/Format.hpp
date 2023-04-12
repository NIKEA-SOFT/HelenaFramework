#ifndef HELENA_TYPES_FORMAT_HPP
#define HELENA_TYPES_FORMAT_HPP

#include <Helena/Dependencies/Fmt.hpp>
#include <Helena/Platform/Assert.hpp>

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
                AddTerminator();
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
        Format(Format&& other) noexcept {
            m_Buffer = std::move(other.m_Buffer);
            AddTerminator();
        };

        Format& operator=(const Format&) = delete;
        Format& operator=(Format&& other) {
            m_Buffer = std::move(other.m_Buffer);
            AddTerminator();
        }

        void Append(char c) noexcept {
            m_Buffer.push_back(c);
            AddTerminator();
        }

        void Append(const std::string_view buffer) noexcept {
            m_Buffer.append(buffer);
            AddTerminator();
        }

        template <std::convertible_to<std::string_view> T, typename... Args>
        void Edit(T&& msg, Args&&... args) noexcept {
            Formatting(std::forward<T>(msg), fmt::make_format_args(std::forward<Args>(args)...));
        }

        [[nodiscard]] const char* GetData() const noexcept {
            return m_Buffer.data();
        }

        [[nodiscard]] std::size_t GetSize() const noexcept {
            return m_Buffer.size();
        }

        [[nodiscard]] std::size_t Empty() const noexcept {
            return GetSize() == 0;
        }

        [[nodiscard]] operator std::string_view() const {
            return GetData();
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
            AddTerminator();
        }

        char* begin() noexcept {
            return m_Buffer.begin();
        }

        char* end() noexcept {
            return m_Buffer.end();
        }

        const char* begin() const noexcept {
            return m_Buffer.begin();
        }

        const char* end() const noexcept {
            return m_Buffer.end();
        }

    private:
        void AddTerminator() {
            m_Buffer.push_back('\0');
            m_Buffer.resize(m_Buffer.size() - 1);
        }

    private:
        memory_buffer m_Buffer;
    };
}

#endif // HELENA_TYPES_FORMAT_HPP