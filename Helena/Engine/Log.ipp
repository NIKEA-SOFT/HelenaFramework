#ifndef HELENA_ENGINE_LOG_IPP
#define HELENA_ENGINE_LOG_IPP

#include <Helena/Types/BasicLogger.hpp>

namespace Helena::Log
{
    template <Traits::DefinitionLogger Logger, typename... Args>
    void Console(const Formater<Logger> format, Args&&... args) noexcept {
    #if defined(HELENA_PLATFORM_WIN)
        if(_isatty(_fileno(stdout)))
    #elif defined(HELENA_PLATFORM_LINUX)
        if(isatty(fileno(stdout)))
    #endif
        Types::BasicLogger::PrintConsole(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Console(const Traits::DefinitionLogger auto logger, const Formater<std::decay_t<decltype(logger)>> format, Args&&... args) noexcept {
    #if defined(HELENA_PLATFORM_WIN)
        if(_isatty(_fileno(stdout)))
    #elif defined(HELENA_PLATFORM_LINUX)
        if(isatty(fileno(stdout)))
    #endif
        Types::BasicLogger::PrintConsole(format, std::forward<Args>(args)...);
    }
}

#endif // HELENA_ENGINE_LOG_IPP
