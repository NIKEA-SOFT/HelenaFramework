#ifndef HELENA_TRAITS_CACHELINE_HPP
#define HELENA_TRAITS_CACHELINE_HPP

#include <new>

namespace Helena::Traits {
#ifdef __cpp_lib_hardware_interference_size
    inline constexpr std::size_t Cacheline = std::hardware_destructive_interference_size;
#else
    inline constexpr std::size_t Cacheline = 64;
#endif
}

#endif // HELENA_TRAITS_CACHELINE_HPP