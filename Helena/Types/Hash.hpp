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

    template <typename T = std::uint64_t>
    constexpr auto Get(const std::string_view str) noexcept 
    {
        using Hash = Helena::Traits::FNV1a<T>;

        auto hash{ Hash::Offset };
        for(std::size_t i = 0u; i < str.size(); ++i) {
            hash = (hash ^ static_cast<T>(str[i])) * Hash::Prime;
        }
        return hash;
    }

    template <typename T, typename U = std::uint64_t>
    constexpr auto Get() noexcept {
        return Get<U>(Helena::Traits::NameOf<T>::value);
    }
}

#endif // HELENA_TYPES_HASH_HPP
