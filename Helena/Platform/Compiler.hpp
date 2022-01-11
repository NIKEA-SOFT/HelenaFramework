#ifndef HELENA_PLATFORM_COMPILER_HPP
#define HELENA_PLATFORM_COMPILER_HPP

/* ----------- [Compiler detect] ----------- */
#if defined(__clang__)
    #define HELENA_COMPILER_NAME    "Clang"
    #define HELENA_STANDARD_VER     __cplusplus
    #define HELENA_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define HELENA_COMPILER_NAME    "GCC"
    #define HELENA_STANDARD_VER     __cplusplus
    #define HELENA_COMPILER_GCC
#elif defined(_MSC_VER)
    #define HELENA_COMPILER_NAME    "MSVC"
    #define HELENA_STANDARD_VER     _MSVC_LANG
    #define HELENA_COMPILER_MSVC
#else
    #error Unsupported compiler
#endif

#if HELENA_STANDARD_VER <= 201703L
    #error Framework support only C++20 and higher!
#endif

#endif  // HELENA_PLATFORM_COMPILER_HPP
