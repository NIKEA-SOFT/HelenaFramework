#ifndef HELENA_TRAITS_NAMEOF_HPP
#define HELENA_TRAITS_NAMEOF_HPP

#include <Helena/Platform/Defines.hpp>
#include <array>
#include <string_view>

namespace Helena::Traits 
{
    template <typename T>
    class NameOf final {
        template <std::size_t... Indexes>
        [[nodiscard]] static constexpr auto substring_as_array(std::string_view str, std::index_sequence<Indexes...>) noexcept {
            return std::array{str[Indexes]..., '\0'};
        }

        template <typename>
        [[nodiscard]] static constexpr auto type_name_array() noexcept
        {
        #if defined(HELENA_COMPILER_CLANG)
            constexpr auto prefix   = std::string_view{"[T = "};
            constexpr auto suffix   = std::string_view{"]"};
            constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
        #elif defined(HELENA_COMPILER_GCC)
            constexpr auto prefix   = std::string_view{"; T = "};
            constexpr auto suffix   = std::string_view{"]"};
            constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
        #elif defined(HELENA_COMPILER_MSVC)
            constexpr auto prefix   = std::string_view{"type_name_array<"};
            constexpr auto suffix   = std::string_view{">(void)"};
            constexpr auto function = std::string_view{__FUNCSIG__};
        #else
            # error Unsupported compiler
        #endif

            constexpr auto start = function.find(prefix) + prefix.size();
            constexpr auto end = function.rfind(suffix);

            static_assert(start < end);

            constexpr auto name = function.substr(start, (end - start));
            return substring_as_array(name, std::make_index_sequence<name.size()>{});
        }

        template <typename Type>
        struct type_name_holder {
            static constexpr auto value = type_name_array<T>();
        };

        template <typename Type>
        [[nodiscard]] static constexpr auto type_name() {
            constexpr auto& value = type_name_holder<Type>::value;
            return std::string_view{value.data()};
        }

    public:
        constexpr operator std::string_view() const noexcept {
            return value;
        }

        static constexpr auto value = type_name<T>();
    };
}

#endif // HELENA_TRAITS_NAMEOF_HPP