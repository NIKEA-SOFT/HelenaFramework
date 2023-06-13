#ifndef HELENA_TYPES_HASH_HPP
#define HELENA_TYPES_HASH_HPP

#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/FNV1a.hpp>
#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Traits/Specialization.hpp>

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

    private:
        template <std::input_iterator Iterator>
        [[nodiscard]] static constexpr auto Calculate(Iterator begin, const Iterator end) noexcept
        {
            auto value{hash_type::Offset};
            while(begin != end) {
                value = (value ^ static_cast<T>(*begin)) * hash_type::Prime;
                ++begin;
            }

            return value;
        }

    public:
        Hash() = delete;
        ~Hash() = delete;
        Hash(const Hash&) = delete;
        Hash(Hash&&) noexcept = delete;
        Hash& operator=(const Hash&) = delete;
        Hash& operator=(Hash&&) noexcept = delete;

        template <typename Container>
        requires requires(Container container) {
            container.begin();
            container.end();
            container.begin() != container.end();
            std::convertible_to<std::remove_cvref_t<decltype(*container.begin())>, T>;
        }
        [[nodiscard]] static constexpr auto From(const Container& container) noexcept {
            return Calculate(container.begin(), container.end());
        }


        [[nodiscard]] static constexpr auto From(const char* str) noexcept {
            return From(std::string_view{str});
        }

        [[nodiscard]] static constexpr auto From(const wchar_t* str) noexcept {
            return From(std::wstring_view{str});
        }

        template <typename Type>
        [[nodiscard]] static constexpr auto From() noexcept {
            return From(Traits::NameOf<Type>);
        }
    };

    // Heterogeneous lookup
    template <Traits::Specialization<std::basic_string> T>
    requires requires(T str) {
        { std::hash<std::basic_string_view<typename T::value_type>>{}(str) } -> std::convertible_to<std::size_t>;
    }
    struct Hasher<T> {
        using char_type = typename T::value_type;
        using string_view = std::basic_string_view<char_type>;
        using hash_type = std::hash<string_view>;
        using is_transparent = void;

        [[nodiscard]] std::size_t operator()(const char_type* str) const { return hash_type{}(str); }
        [[nodiscard]] std::size_t operator()(string_view str) const { return hash_type{}(str); }
        [[nodiscard]] std::size_t operator()(const T& str) const { return hash_type{}(str); }
    };
}

namespace Helena::Literals::Hash {
    [[nodiscard]] constexpr auto operator""_Hash32(const char* data, std::size_t size) noexcept {
        return Types::Hash<std::uint32_t>::From(std::string_view{data, size});
    }

    [[nodiscard]] constexpr auto operator""_Hash64(const char* data, std::size_t size) noexcept {
        return Types::Hash<std::uint64_t>::From(std::string_view{data, size});
    }

    [[nodiscard]] constexpr auto operator""_Hash32(const wchar_t* data, std::size_t size) noexcept {
        return Types::Hash<std::uint32_t>::From(std::wstring_view{data, size});
    }

    [[nodiscard]] constexpr auto operator""_Hash64(const wchar_t* data, std::size_t size) noexcept {
        return Types::Hash<std::uint64_t>::From(std::wstring_view{data, size});
    }
}

#endif // HELENA_TYPES_HASH_HPP