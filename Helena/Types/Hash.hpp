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

    [[nodiscard]] constexpr auto Get(const std::string_view str) noexcept 
    {
        using Hash = Traits::FNV1a<std::uint64_t>;

        auto value { Hash::Offset };
        for(std::size_t i = 0; i < str.size(); ++i) {
            value = (value ^ static_cast<std::uint64_t>(str[i])) * Hash::Prime;
        }

        return value;
    }

    template <typename T>
    [[nodiscard]] constexpr auto Get() noexcept {
        return Get(Helena::Traits::NameOf<T>::value);
    }
}

#endif // HELENA_TYPES_HASH_HPP
