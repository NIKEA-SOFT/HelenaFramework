#ifndef HELENA_TYPES_HASH_HPP
#define HELENA_TYPES_HASH_HPP

#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/FNV1a.hpp>
#include <concepts>

namespace Helena::Types
{
    template <typename>
    struct Hasher;

    template <typename>
    struct Equaler;

    class Hash 
    {
    public:
        Hash() = delete;
        Hash(const Hash&) = delete;
        Hash(Hash&&) noexcept = delete;
        Hash& operator=(const Hash&) = delete;
        Hash& operator=(Hash&&) noexcept = delete;

        template <typename T>
        requires std::is_integral_v<T>
        [[nodiscard]] static  constexpr auto Get(const std::string_view str) noexcept
        {
            using Hash = Traits::FNV1a<T>;

            auto value{Hash::Offset};
            for(std::size_t i = 0; i < str.size(); ++i) {
                value = (value ^ static_cast<T>(str[i])) * Hash::Prime;
            }

            return value;
        }

        template <typename T, typename P>
        [[nodiscard]] static constexpr auto Get() noexcept {
            return Get<P>(Helena::Traits::template NameOf<T>::value);
        }
    };
}

#endif // HELENA_TYPES_HASH_HPP
