#ifndef HELENA_TRAITS_POWEROF2_HPP
#define HELENA_TRAITS_POWEROF2_HPP

#include <cstddef>

namespace Helena::Traits
{
    //! Power of 2 (warn: false for value 0)
    template <std::size_t Size>
    static constexpr auto IsPowerOf2 = Size && !(Size & (Size - 1));

    template <std::size_t Size>
    static constexpr auto PowerOf2 = []{
        if constexpr(IsPowerOf2<Size>) {
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