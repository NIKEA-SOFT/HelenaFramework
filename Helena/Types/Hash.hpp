#ifndef HELENA_TYPES_HASH_HPP
#define HELENA_TYPES_HASH_HPP

#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/FNV1a.hpp>

namespace Helena::Hash
{
    template <typename>
    struct Hasher;

    template <typename>
    struct Equaler;

    template <typename T = std::uint64_t, typename = std::enable_if<std::is_integral_v<T>>>
    [[nodiscard]] constexpr auto Get(const std::string_view str) noexcept 
    {
        using Hash = Traits::FNV1a<T>;

        auto value { Hash::Offset };
        for(std::size_t i = 0; i < str.size(); ++i) {
            value = (value ^ static_cast<T>(str[i])) * Hash::Prime;
        }

        return value;
    }

    template <typename T, typename P = std::uint64_t>
    [[nodiscard]] constexpr auto Get() noexcept {
        return Get<P>(Helena::Traits::NameOf<T>::value);
    }
}

#endif // HELENA_TYPES_HASH_HPP
