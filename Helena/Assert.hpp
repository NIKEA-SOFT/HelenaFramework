#ifndef HELENA_ASSERT_HPP
#define HELENA_ASSERT_HPP

#include <fstream>

#include <Helena/Log.hpp>

#ifdef HF_DEBUG
    #if HF_PLATFORM == HF_PLATFORM_WIN
        #define HF_DEBUG_BREAK()    _CrtDbgBreak()
    #elif HF_PLATFORM == HF_PLATFORM_LINUX
        #define HF_DEBUG_BREAK()    raise(SIGTRAP)
    #endif

    #define HF_ASSERT(cond, ...)                                                                            \
        do {                                                                                                \
            if(!(cond)) {                                                                                   \
                constexpr auto size = std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value;       \
                std::ofstream file;                                                                         \
                file.open("Assert.txt", std::ios::out | std::ios::app);                                     \
                std::string log; log.reserve(1024);                                                         \
                                                                                                            \
                log += HF_FORMAT("[{:%Y.%m.%d %H:%M:%S}][{}:{}] Assert: " #cond,                            \
                    fmt::localtime(std::time(nullptr)), HF_FILE_LINE);                                      \
                                                                                                            \
                if(size) {                                                                                  \
                    log += HF_FORMAT("\nMessage: {}", HF_FORMAT(__VA_ARGS__));                              \
                }                                                                                           \
                                                                                                            \
                if(file.is_open()) {                                                                        \
                    file << log << std::endl;                                                               \
                    file.close();                                                                           \
                }                                                                                           \
                                                                                                            \
                HF_MSG(fg(fmt::terminal_color::bright_white) | bg(fmt::terminal_color::bright_red),         \
                    "{}\n", log);                                                                           \
                HF_DEBUG_BREAK();                                                                           \
                std::terminate();                                                                           \
            }                                                                                               \
        } while(false)
#else
    #define HF_DEBUG_BREAK()
    #define HF_ASSERT(cond, ...)
#endif

#endif // HELENA_ASSERT_HPP
