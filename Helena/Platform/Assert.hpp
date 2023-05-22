#ifndef HELENA_PLATFORM_ASSERT_HPP
#define HELENA_PLATFORM_ASSERT_HPP

#include <Helena/Types/BasicLogger.hpp>

#ifdef HELENA_DEBUG
    #include <tuple>

    #define HELENA_ASSERT(cond, ...)                                                    \
    if(!std::is_constant_evaluated()) {                                                 \
        do {                                                                            \
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
        } while(false);                                                                 \
    }
#else
    #define HELENA_ASSERT(cond, ...)
#endif

#endif // HELENA_PLATFORM_ASSERT_HPP
