#ifndef HELENA_TRAITS_UNDERLYING_HPP
#define HELENA_TRAITS_UNDERLYING_HPP

#include <type_traits>

namespace Helena::Traits
{
    template <typename T>
    using Underlying = std::underlying_type_t<T>;
}


#endif // HELENA_TRAITS_IDENTITY_HPP