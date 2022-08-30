#ifndef HELENA_TRAITS_POWEROF2_HPP
#define HELENA_TRAITS_POWEROF2_HPP

#include <cstdint>
#include <type_traits>

namespace Helena::Traits
{
    template <std::size_t Size>
    static constexpr auto PowerOf2 = []{
        constexpr auto isPowerOf2 = Size && !(Size & (Size - 1));
        if constexpr(isPowerOf2) {
            return Size;
        } else {
            auto value = Size;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            return ++value;
        }
    }();
}

#endif // HELENA_TRAITS_POWEROF2_HPP