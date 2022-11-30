#ifndef HELENA_PLATFORM_ASSERT_HPP
#define HELENA_PLATFORM_ASSERT_HPP

#include <Helena/Types/BasicLoggersDef.hpp>

namespace Helena::Log
{
    template <Traits::DefinitionLogger Logger, typename... Args>
    void Console(const Formater<Logger> format, Args&&... args) noexcept;

    template <Traits::DefinitionLogger Logger, typename... Args>
    void Console(const Logger logger, const Formater<std::remove_cvref_t<decltype(logger)>> format, Args&&... args) noexcept;
}

#ifdef HELENA_DEBUG
    #include <Helena/Platform/Platform.hpp>
    #include <tuple>

    #define HELENA_ASSERT(cond, ...)                                                    \
        do {                                                                            \
            if(!(cond)) {                                                               \
                Helena::Log::Console<Helena::Log::Assert>("Condition: " #cond);         \
                                                                                        \
                using tuple = decltype(std::forward_as_tuple(__VA_ARGS__));             \
                if constexpr(std::tuple_size_v<tuple> > 0) {                            \
                    Helena::Log::Console<Helena::Log::Assert>("Message: " __VA_ARGS__); \
                }                                                                       \
                                                                                        \
                if(HELENA_DEBUGGING()) {                                                \
                    HELENA_BREAKPOINT();                                                \
                }                                                                       \
                                                                                        \
                std::exit(EXIT_FAILURE);                                                \
            }                                                                           \
        } while(false)
#else
    #define HELENA_ASSERT(cond, ...)
#endif

    #include <Helena/Engine/Log.hpp>

#endif // HELENA_PLATFORM_ASSERT_HPP
