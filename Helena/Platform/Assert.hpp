#ifndef HELENA_PLATFORM_ASSERT_HPP
#define HELENA_PLATFORM_ASSERT_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Types/BasicLoggerDefines.hpp>

#define HELENA_ASSERT_RUNTIME(cond, ...)                                        \
if(!std::is_constant_evaluated() && !(cond)) {                                  \
    [](auto&&... args) HELENA_NOINLINE HELENA_NORETURN {                        \
        Helena::Log::Message<Helena::Log::Assert>("Condition: " #cond);         \
                                                                                \
        if constexpr(sizeof...(args)) {                                         \
            Helena::Log::Message<Helena::Log::Assert, false>("Message: ");      \
            Helena::Log::Message<Helena::Log::Assert>(                          \
                std::forward<decltype(args)>(args)...);                         \
        }                                                                       \
                                                                                \
        if(HELENA_DEBUGGING()) {                                                \
            HELENA_BREAKPOINT();                                                \
        }                                                                       \
                                                                                \
        std::exit(EXIT_FAILURE);                                                \
    }(__VA_ARGS__);\
}                                                                               \


#ifdef HELENA_DEBUG
    #define HELENA_ASSERT(cond, ...)    HELENA_ASSERT_RUNTIME(cond, __VA_ARGS__)
#else
    #define HELENA_ASSERT(cond, ...)
#endif

#endif // HELENA_PLATFORM_ASSERT_HPP
