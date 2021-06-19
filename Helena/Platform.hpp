#ifndef COMMON_PLATFORM_HPP
#define COMMON_PLATFORM_HPP

#include <Helena/Defines.hpp>

#if HF_PLATFORM == HF_PLATFORM_WIN
    #include <Helena/OS/Windows.hpp>

#elif HF_PLATFORM == HF_PLATFORM_LINUX
    #include <Helena/OS/Linux.hpp>
#endif

#endif  // COMMON_PLATFORM_HPP
