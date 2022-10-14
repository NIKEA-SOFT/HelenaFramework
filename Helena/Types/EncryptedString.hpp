#ifndef HELENA_TYPES_ENCRYPTEDSTRING_HPP
#define HELENA_TYPES_ENCRYPTEDSTRING_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Types/Hash.hpp>

#include <cstdint>
#include <string>
#include <limits>

namespace Helena::Types
{
    template <typename T, std::size_t N, typename Traits = std::char_traits<T>>
    class EncryptedString
    {
        template <typename, std::size_t>
        friend class DecryptedString;

        static constexpr auto m_Secret = Hash<std::uint64_t>::template Get<decltype([] {})>();
        static constexpr auto m_Capacity = N;

    public:
        template <typename R, std::size_t Size>
        class DecryptedString
        {
        public:
        HELENA_OPTIMIZATION_DISABLE
            DecryptedString(const volatile R(&data)[Size]) noexcept {
                for(std::size_t i = 0; i < N; ++i) {
                    m_Data[i] = data[i] ^ ((m_Secret >> i * 8 % std::numeric_limits<std::uint64_t>::digits) & 0xFF);
                }
            }
        HELENA_OPTIMIZATION_ENABLE

            [[nodiscard]] operator const R* () const noexcept {
                return m_Data;
            }

            [[nodiscard]] operator std::basic_string_view<R>() const noexcept {
                return std::basic_string_view<R>{m_Data};
            }

        private:
            R m_Data[Size];
        };

    public:
        constexpr EncryptedString(const T(&data)[N]) noexcept : m_Data{} {
            for(std::size_t i = 0; i < N; ++i) {
                m_Data[i] = data[i] ^ ((m_Secret >> i * 8 % std::numeric_limits<std::uint64_t>::digits) & 0xFF);
            }
        }

        [[nodiscard]] auto Decrypt() const noexcept {
            return DecryptedString<T, N>{const_cast<const volatile T (&)[N]>(m_Data)};
        }

        [[nodiscard]] auto operator*() const noexcept {
            return Decrypt();
        }

    private:
        T m_Data[N];
    };
}

#endif // HELENA_TYPES_ENCRYPTEDSTRING_HPP