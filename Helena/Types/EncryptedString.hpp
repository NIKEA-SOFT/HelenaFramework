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

        static constexpr auto Secret = Hash<std::uint64_t>::template From<EncryptedString>();
        static constexpr auto CharBits = std::numeric_limits<T>::digits;
        static constexpr auto CharMax = (std::numeric_limits<T>::max)();
        static constexpr auto SecretBits = std::numeric_limits<decltype(Secret)>::digits;

        static volatile inline auto NoOptimizeSecret = Secret;

    public:
        class DecryptedString
        {
        public:
            HELENA_FORCEINLINE DecryptedString(const T(&data)[N]) noexcept {
                const auto secret = NoOptimizeSecret;
                for(std::size_t i = 0; i < N; ++i) {
                    m_Data[i] = data[i] ^ static_cast<T>(secret >> (i * CharBits & (SecretBits - CharBits)));
                }
            }

            [[nodiscard]] HELENA_FORCEINLINE operator const T* () const noexcept {
                return m_Data;
            }

            [[nodiscard]] HELENA_FORCEINLINE operator std::basic_string_view<T>() const noexcept {
                return std::basic_string_view<T>{m_Data};
            }

        private:
            T m_Data[N];
        };

    public:
        constexpr EncryptedString(const T(&data)[N]) noexcept : m_Data{} {
            for(std::size_t i = 0; i < N; ++i) {
                m_Data[i] = data[i] ^ static_cast<T>(Secret >> (i * CharBits & (SecretBits - CharBits)));
            }
        }

        [[nodiscard]] HELENA_FORCEINLINE auto Decrypt() const & noexcept {
            return DecryptedString{m_Data};
        }

        [[nodiscard]] HELENA_FORCEINLINE auto operator*() const & noexcept {
            return Decrypt();
        }

        [[nodiscard]] HELENA_FORCEINLINE auto Decrypt() const && noexcept = delete;
        [[nodiscard]] HELENA_FORCEINLINE auto operator*() const && noexcept = delete;

    private:
        T m_Data[N];
    };
}

#endif // HELENA_TYPES_ENCRYPTEDSTRING_HPP