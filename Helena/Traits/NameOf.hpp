#ifndef HELENA_TRAITS_NAMEOF_HPP
#define HELENA_TRAITS_NAMEOF_HPP

#include <Helena/Platform/Compiler.hpp>
#include <array>
#include <string_view>
#include <utility>

namespace Helena::Traits
{
    namespace Internal {
        template <typename T>
        class NameOf final {
            template <std::size_t... Indexes>
            [[nodiscard]] static constexpr auto Substring(std::string_view str, std::index_sequence<Indexes...>) noexcept {
                return std::array{str[Indexes]..., '\0'};
            }

            template <typename>
            [[nodiscard]] static constexpr auto ParseName() noexcept
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
                constexpr auto prefix   = std::string_view{"ParseName<"};
                constexpr auto suffix   = std::string_view{">(void)"};
                constexpr auto function = std::string_view{__FUNCSIG__};
            #else
                # error Unsupported compiler
            #endif

                constexpr auto start = function.find(prefix) + prefix.size();
                constexpr auto end = function.rfind(suffix);
                static_assert(start < end);
                constexpr auto name = function.substr(start, (end - start));
                return Substring(name, std::make_index_sequence<name.size()>{});
            }

        public:
            static constexpr auto NameStorage = ParseName<T>();
        };
    }

    template <typename T>
    inline constexpr auto NameOf = Internal::NameOf<T>::NameStorage.data();
}

#endif // HELENA_TRAITS_NAMEOF_HPP