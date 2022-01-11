#ifndef HELENA_PLATFORM_PLATFORM_HPP
#define HELENA_PLATFORM_PLATFORM_HPP

/* ----------- [Platform detect] ----------- */
#if defined(_WIN32) || defined(_WIN64)
    #define HELENA_PLATFORM_NAME    "Windows"

    #define HELENA_PLATFORM_WIN
    #if defined(_WIN32)
        #define HELENA_PLATFORM_WIN32
    #elif defined(_WIN64)
        #define HELENA_PLATFORM_WIN64
    #endif

    #include <Helena/Platform/Windows/Windows.hpp>

#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    #define HELENA_PLATFORM_NAME    "Linux"
    #define HELENA_PLATFORM_LINUX

    #include <Helena/Platform/Linux/Linux.hpp>
#else
    #error Unsupported platform
#endif

// Definition
#define HELENA_PLUGIN_API           extern "C" HELENA_API_EXPORT

#endif  // HELENA_PLATFORM_PLATFORM_HPP
