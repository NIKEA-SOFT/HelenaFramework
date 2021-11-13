#ifndef HELENA_DEBUG_ASSERT_HPP
#define HELENA_DEBUG_ASSERT_HPP

#include <Helena/Platform/Platform.hpp>

//#include <filesystem>
//#include <fstream>
#include <string_view>
#include <tuple>
#include <cstdlib>

namespace Helena {

    namespace Util {
        struct SourceLocation;
    }

    namespace Log {
        enum class Color : std::uint8_t;

        namespace Details {
            struct Assert;
        }

        template <typename Prefix, typename... Args>
        void Console(const Util::SourceLocation& source, const std::string_view msg, Args&&... args);
    }
}

#ifdef HELENA_DEBUG
    #define HELENA_ASSERT(cond, ...)                                                                        \
        do {                                                                                                \
            if(!(cond)) {                                                                                   \
                using assert    = Helena::Log::Details::Assert;                                             \
                using tuple     = decltype(std::forward_as_tuple(__VA_ARGS__));                             \
                constexpr auto location = Helena::Util::SourceLocation::Create(__FILE__, __LINE__);         \
                Helena::Log::Console<assert>(location, "Condition: {}", #cond);                             \
                if constexpr (std::tuple_size_v<tuple> > 0) {                                               \
                    Helena::Log::Console<assert>(location, "Message: " __VA_ARGS__);                        \
                }                                                                                           \
                if(HELENA_DEBUGGING()) {                                                                    \
                    HELENA_BREAKPOINT();                                                                    \
                }                                                                                           \
                                                                                                            \
                std::exit(EXIT_FAILURE);                                                                    \
            }                                                                                               \
        } while(false)

    //#define HELENA_ASSERT(cond, ...)                                                                    \
    //    do {                                                                                        \
    //        if(!(cond)) {                                                                           \
    //            using tuple = decltype(std::forward_as_tuple(__VA_ARGS__));                         \
    //            constexpr auto source = Helena::Util::SourceLocation::Create(__FILE__, __LINE__);   \
    //            if constexpr (std::tuple_size_v<tuple> > 0) {                                       \
    //                HELENA_MSG_FATAL("Assert: " #cond "\nMessage: " __VA_OPT__(,) __VA_ARGS__);         \
    //            } else {                                                                            \
    //                HELENA_MSG_FATAL("Assert: " #cond);                                                 \
    //            }                                                                                   \
    //                                                                                                \
    //            fmt::memory_buffer buffer;                                                          \
    //            fmt::memory_buffer path;                                                            \
    //            fmt::detail::vformat_to(buffer,                                                     \
    //                fmt::string_view{"[{:%Y.%m.%d %H:%M:%S}][{}:{}] Assert: " #cond},               \
    //                fmt::make_format_args(fmt::localtime(std::time(nullptr)),                       \
    //                source.GetFile(), source.GetLine()));                                           \
    //                                                                                                \
    //            try {                                                                               \
    //                if constexpr (std::tuple_size_v<tuple> > 0) {                                   \
    //                    fmt::detail::vformat_to(buffer,                                             \
    //                        HELENA_ARGS_AS_FMT_WITH_ARGS(__VA_OPT__(,) __VA_ARGS__));                   \
    //                }                                                                               \
    //            } catch (const fmt::format_error&) {                                                \
    //                using Exception = Helena::Log::Details::Exception;                              \
    //                const auto has_style = Helena::Log::Details::MakeColor(buffer,                  \
    //                    Exception::GetStyle());                                                     \
    //                fmt::detail::vformat_to(buffer, fmt::string_view{" | [{}] {}"},                 \
    //                    fmt::make_format_args(Exception::GetPrefix(), "Format syntax incorrect!")); \
    //                                                                                                \
    //                if (has_style) {                                                                \
    //                    Helena::Log::Details::EndColor(buffer);                                     \
    //                }                                                                               \
    //            }                                                                                   \
    //                                                                                                \
    //            buffer.push_back('\0');                                                             \
    //                                                                                                \
    //            fmt::detail::vformat_to(path, fmt::string_view{"{}"},                               \
    //                fmt::make_format_args(std::filesystem::current_path().string()));               \
    //            path.push_back(HELENA_SEPARATOR);                                                       \
    //            path.append(fmt::string_view{"Assert.log"});                                        \
    //            path.push_back('\0');                                                               \
    //                                                                                                \
    //            std::ofstream file(path.data(), std::ios::out | std::ios::app);                     \
    //            if(file.is_open()) {                                                                \
    //                file << buffer.data() << '\n';                                                  \
    //                file.flush();                                                                   \
    //                file.close();                                                                   \
    //            } else {                                                                            \
    //                HELENA_MSG_FATAL("Open file: {} failed for save log assert", path.data());          \
    //            }                                                                                   \
    //                                                                                                \
    //            HELENA_BREAKPOINT;                                                                      \
    //            std::exit(EXIT_FAILURE);                                                            \
    //        }                                                                                       \
    //    } while(false)
#else
    #define HELENA_ASSERT(cond, ...)
#endif

#include <Helena/Engine/Log.hpp>

#endif // HELENA_DEBUG_ASSERT_HPP
