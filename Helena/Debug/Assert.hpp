#ifndef HELENA_DEBUG_ASSERT_HPP
#define HELENA_DEBUG_ASSERT_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Engine/Log.hpp>

#ifdef HELENA_DEBUG
    #include <Helena/Platform/Platform.hpp>
    #include <tuple>

    namespace Helena::Log
    {
        struct Assert {
            using Logger = Types::BasicLogger;

            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return Logger::CreatePrefix("[ASSERT]");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return Logger::CreateStyle(Logger::Color::BrightWhite, Logger::Color::Red);
            }
        };
    }

    #define HELENA_ASSERT(cond, ...)                                                    \
        do {                                                                            \
            if(!(cond)) {                                                               \
                Helena::Log::Console<Helena::Log::Assert>("Condition: " #cond);         \
                                                                                        \
                using tuple = decltype(std::forward_as_tuple(__VA_ARGS__));             \
                if constexpr (std::tuple_size_v<tuple> > 0) {                           \
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

#endif // HELENA_DEBUG_ASSERT_HPP
