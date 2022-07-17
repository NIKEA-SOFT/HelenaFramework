#ifndef HELENA_TRAITS_ADD_HPP
#define HELENA_TRAITS_ADD_HPP

#include <type_traits>

namespace Helena::Traits
{
    template <typename T>
    using AddConst = std::add_const_t<T>;

    template <typename T>
    using AddVol = std::add_volatile_t<T>;

    template <typename T>
    using AddRefLV = std::add_lvalue_reference_t<T>;

    template <typename T>
    using AddRefRV = std::add_rvalue_reference_t<T>;

    template <typename T>
    using AddPtr = std::add_pointer_t<T>;

    template <typename T>
    using AddCV = std::add_cv_t<T>;

    template <typename T>
    using AddCRLV = AddRefLV<AddConst<T>>;

    template <typename T>
    using AddCRRV = AddRefRV<AddConst<T>>;

    template <typename T>
    using AddCP = AddPtr<AddConst<T>>;
}

#endif // HELENA_TRAITS_ADD_HPP
