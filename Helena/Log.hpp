#ifndef HELENA_LOG_HPP
#define HELENA_LOG_HPP

#include <Helena/Util.hpp>
#include <Helena/Internal.hpp>
#include <Helena/Format.hpp>

namespace Helena
{
    #define HF_MSG(...)                                                                                     \
        fmt::print(__VA_ARGS__)

    #define HF_MSG_DEBUG(...)                                                                               \
        fmt::print(fg(fmt::terminal_color::bright_blue),                                                    \
        "[{:%Y.%m.%d %H:%M:%S}][{}:{}][DEBUG] {}\n",                                                        \
        fmt::localtime(std::time(nullptr)), HF_FILE_LINE, HF_FORMAT(__VA_ARGS__))

    #define HF_MSG_INFO(...)                                                                                \
        fmt::print(fg(fmt::terminal_color::bright_green),                                                   \
        "[{:%Y.%m.%d %H:%M:%S}][{}:{}][INFO] {}\n",                                                         \
        fmt::localtime(std::time(nullptr)), HF_FILE_LINE, HF_FORMAT(__VA_ARGS__))

    #define HF_MSG_WARN(...)                                                                                \
        fmt::print(fg(fmt::terminal_color::bright_yellow),                                                  \
        "[{:%Y.%m.%d %H:%M:%S}][{}:{}][WARNING] {}\n",                                                      \
        fmt::localtime(std::time(nullptr)), HF_FILE_LINE, HF_FORMAT(__VA_ARGS__))

    #define HF_MSG_ERROR(...)                                                                               \
        fmt::print(fg(fmt::terminal_color::bright_red),                                                     \
        "[{:%Y.%m.%d %H:%M:%S}][{}:{}][ERROR] {}\n",                                                        \
        fmt::localtime(std::time(nullptr)), HF_FILE_LINE, HF_FORMAT(__VA_ARGS__))

    #define HF_MSG_FATAL(...)                                                                               \
        fmt::print(fg(fmt::terminal_color::bright_white) | bg(fmt::terminal_color::bright_red),             \
        "[{:%Y.%m.%d %H:%M:%S}][{}:{}][FATAL] {}\n",                                                        \
        fmt::localtime(std::time(nullptr)), HF_FILE_LINE, HF_FORMAT(__VA_ARGS__))
}

#endif // HELENA_LOG_HPP
