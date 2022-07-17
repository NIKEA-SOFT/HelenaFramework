#ifndef HELENA_TRAITS_CONDITIONAL_HPP
#define HELENA_TRAITS_CONDITIONAL_HPP

#include <type_traits>

namespace Helena::Traits
{
    template <bool Condition, typename Success, typename Failure>
    using Conditional = std::conditional_t<Condition, Success, Failure>;
}

#endif // HELENA_TRAITS_CONDITIONAL_HPP
