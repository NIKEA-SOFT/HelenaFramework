#ifndef HELENA_PLATFORM_ASSERT_HPP
#define HELENA_PLATFORM_ASSERT_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Types/BasicLoggerDefines.hpp>
#include <tuple>

#define HELENA_ASSERT_RUNTIME(cond, ...)                                        \
if(!std::is_constant_evaluated()) {                                             \
    if(!(cond)) {                                                               \
        Helena::Log::Message<Helena::Log::Assert>("Condition: " #cond);         \
                                                                                \
        using tuple = decltype(std::forward_as_tuple(__VA_ARGS__));             \
        if constexpr(std::tuple_size_v<tuple> > 0) {                            \
            Helena::Log::Message<Helena::Log::Assert>("Message: " __VA_ARGS__); \
        }                                                                       \
                                                                                \
        if(HELENA_DEBUGGING()) {                                                \
            HELENA_BREAKPOINT();                                                \
        }                                                                       \
                                                                                \
        std::exit(EXIT_FAILURE);                                                \
    }                                                                           \
}

#ifdef HELENA_DEBUG
    #define HELENA_ASSERT(cond, ...)    HELENA_ASSERT_RUNTIME(cond, __VA_ARGS__)
#else
    #define HELENA_ASSERT(cond, ...)
#endif

#endif // HELENA_PLATFORM_ASSERT_HPP
