#ifndef HELENA_TRAITS_CVREFPTR_HPP
#define HELENA_TRAITS_CVREFPTR_HPP

#include <type_traits>
#include <concepts>

namespace Helena::Traits
{
    template <typename Type>
    using RemoveCVRefPtr = std::conditional_t<std::is_array_v<Type>, std::remove_all_extents_t<Type>,
        std::conditional_t<std::is_pointer_v<Type>, std::remove_cvref_t<std::remove_pointer_t<Type>>, std::remove_cvref_t<Type>>>;
}


#endif // HELENA_TRAITS_CVREFPTR_HPP