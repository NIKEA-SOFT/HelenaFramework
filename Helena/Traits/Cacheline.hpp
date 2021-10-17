#ifndef HELENA_TRAITS_CACHELINE_HPP
#define HELENA_TRAITS_CACHELINE_HPP

#include <new>

namespace Helena::Traits {

    struct Cacheline {
    #ifdef __cpp_lib_hardware_interference_size
        static constexpr std::size_t value = std::hardware_destructive_interference_size;
    #else
        static constexpr std::size_t value = 64;
    #endif
    };
}

#endif // HELENA_TRAITS_CACHELINE_HPP