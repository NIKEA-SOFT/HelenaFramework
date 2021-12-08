#ifndef HELENA_DEBUG_ASSERT_HPP
#define HELENA_DEBUG_ASSERT_HPP

#include <Helena/Platform/Platform.hpp>

//#include <filesystem>
//#include <fstream>
#include <string_view>
#include <tuple>
#include <cstdlib>

namespace Helena {

    namespace Types {
        struct SourceLocation;
    }

    namespace Log {
        enum class Color : std::uint8_t;

        namespace Details {
            struct Assert;
        }

        template <typename Prefix, typename... Args>
        void Console(const Types::SourceLocation& source, const std::string_view msg, Args&&... args) noexcept;
    }
}

#ifdef HELENA_DEBUG
    #define HELENA_ASSERT(cond, ...)                                                                        \
        do {                                                                                                \
            if(!(cond)) {                                                                                   \
                using assert = Helena::Log::Details::Assert;                                                \
                using tuple = decltype(std::forward_as_tuple(__VA_ARGS__));                                 \
                constexpr auto location = Helena::Types::SourceLocation::Create(__FILE__, __LINE__);        \
                Helena::Log::Console<assert>(location, "Condition: {}", #cond);                             \
                if constexpr (std::tuple_size_v<tuple> > 0) {                                               \
                    Helena::Log::Console<assert>(location, "Message: " __VA_OPT__(,) __VA_ARGS__);          \
                }                                                                                           \
                if(HELENA_DEBUGGING()) {                                                                    \
                    HELENA_BREAKPOINT();                                                                    \
                }                                                                                           \
                                                                                                            \
                std::exit(EXIT_FAILURE);                                                                    \
            }                                                                                               \
        } while(false)
#else
    #define HELENA_ASSERT(cond, ...)
#endif

#include <Helena/Engine/Log.hpp>

#endif // HELENA_DEBUG_ASSERT_HPP
