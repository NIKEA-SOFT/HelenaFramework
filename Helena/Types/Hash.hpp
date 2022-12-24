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

    template <Traits::AnyOf<std::uint32_t, std::uint64_t> T>
    class Hash final
    {
    public:
        using hash_type = Traits::FNV1a<T>;
        using value_type = T;

    public:
        Hash() = delete;
        Hash(const Hash&) = delete;
        Hash(Hash&&) noexcept = delete;
        Hash& operator=(const Hash&) = delete;
        Hash& operator=(Hash&&) noexcept = delete;

        template <typename Char>
        [[nodiscard]] static constexpr auto From(const std::basic_string_view<Char> str) noexcept
        {
            auto value{hash_type::Offset};
            for(std::size_t i = 0; i < str.size(); ++i) {
                value = (value ^ static_cast<T>(str[i])) * hash_type::Prime;
            }

            return value;
        }

        template <typename Char>
        requires std::convertible_to<std::add_pointer_t<Char>, std::basic_string_view<Char>>
        [[nodiscard]] static constexpr auto From(const Char* str) noexcept {
            return From(std::basic_string_view<Char>{str});
        }

        template <typename Type>
        [[nodiscard]] static constexpr auto From() noexcept {
            return From(static_cast<std::string_view>(Helena::Traits::NameOf<Type>{}));
        }
    };
}

#endif // HELENA_TYPES_HASH_HPP