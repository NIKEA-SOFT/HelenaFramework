#ifndef HELENA_ENGINE_LOG_HPP
#define HELENA_ENGINE_LOG_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Types/BasicLoggersDef.hpp>

namespace Helena::Log
{
    template <Traits::DefinitionLogger Logger, typename... Args>
    void Console(const Formater<Logger> format, Args&&... args) noexcept;

    template <Traits::DefinitionLogger Logger, typename... Args>
    void Console(const Logger logger, const Formater<std::remove_cvref_t<decltype(logger)>> format, Args&&... args) noexcept;
}

#include <Helena/Engine/Log.ipp>

// NOTE MSVC: If you get an error, make sure you add a preprocessor /Zc:preprocessor for support VA_OPT
// Read it here: https://docs.microsoft.com/en-us/cpp/build/reference/zc-preprocessor?view=msvc-160
#define HELENA_MSG(prefix, fmt, ...)        Helena::Log::Console<prefix>(fmt __VA_OPT__(,) __VA_ARGS__)

#if defined(HELENA_DEBUG)
    #define HELENA_MSG_ASSERT(fmt, ...)     HELENA_MSG(Helena::Log::Assert,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_DEBUG(fmt, ...)      HELENA_MSG(Helena::Log::Debug,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_INFO(fmt, ...)       HELENA_MSG(Helena::Log::Info,       fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_NOTICE(fmt, ...)     HELENA_MSG(Helena::Log::Notice,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Warning,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Error,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Fatal,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_EXCEPTION(fmt, ...)  HELENA_MSG(Helena::Log::Exception,  fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_MEMORY(fmt, ...)     HELENA_MSG(Helena::Log::Memory,     fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define HELENA_MSG_ASSERT(fmt, ...)
    #define HELENA_MSG_DEBUG(fmt, ...)
    #define HELENA_MSG_INFO(fmt, ...)
    #define HELENA_MSG_NOTICE(fmt, ...)     HELENA_MSG(Helena::Log::Notice,     fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_WARNING(fmt, ...)    HELENA_MSG(Helena::Log::Warning,    fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_ERROR(fmt, ...)      HELENA_MSG(Helena::Log::Error,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_FATAL(fmt, ...)      HELENA_MSG(Helena::Log::Fatal,      fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_EXCEPTION(fmt, ...)  HELENA_MSG(Helena::Log::Exception,  fmt __VA_OPT__(,) __VA_ARGS__)
    #define HELENA_MSG_MEMORY(fmt, ...)
#endif

#endif // HELENA_ENGINE_LOG_HPP
