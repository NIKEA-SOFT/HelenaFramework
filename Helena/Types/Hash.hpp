#ifndef HELENA_TYPES_HASH_HPP
#define HELENA_TYPES_HASH_HPP

#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/FNV1a.hpp>
#include <Helena/Traits/AnyOf.hpp>

namespace Helena::Types
{
    template <typename>
    struct Hasher;

    template <typename>
    struct Equaler;

    template <typename T>
    requires Traits::AnyOf<T, std::uint32_t, std::uint64_t>
    class Hash
    {
    public:
        using Hasher = Traits::FNV1a<T>;
        using Value = T;

    public:
        Hash() = delete;
        Hash(const Hash&) = delete;
        Hash(Hash&&) noexcept = delete;
        Hash& operator=(const Hash&) = delete;
        Hash& operator=(Hash&&) noexcept = delete;

        template <typename Char>
        [[nodiscard]] static constexpr auto Get(std::basic_string_view<Char> str) noexcept
        {
            auto value{Hasher::Offset};
            for(std::size_t i = 0; i < str.size(); ++i) {
                value = (value ^ static_cast<T>(str[i])) * Hasher::Prime;
            }

            return value;
        }

        template <typename Type>
        [[nodiscard]] static constexpr auto Get() noexcept {
            return Get(Helena::Traits::NameOf<Type>::Value);
        }
    };
}

#endif // HELENA_TYPES_HASH_HPP