#ifndef HELENA_PLATFORM_DEFINES_HPP
#define HELENA_PLATFORM_DEFINES_HPP

#include <Helena/Platform/Compiler.hpp>
#include <Helena/Platform/Processor.hpp>

/* ----------- [Debug detect] ----------- */
#if !defined(NDEBUG)
    #define HELENA_DEBUG
#endif

/* ----------- [Utility] ----------- */
#define HELENA_STRINGIFY(str)                   #str

/* ----------- [Diagnostic Pragma] ----------- */
#if defined(HELENA_COMPILER_CLANG)
    #define HELENA_DIAGNOSTIC_CLANG_PUSH        _Pragma("clang diagnostic push")
    #define HELENA_DIAGNOSTIC_CLANG_POP         _Pragma("clang diagnostic pop")
    #define HELENA_DIAGNOSTIC_CLANG_IGNORE(id)  _Pragma(HELENA_STRINGIFY(clang diagnostic ignored id))
#else
    #define HELENA_DIAGNOSTIC_CLANG_PUSH
    #define HELENA_DIAGNOSTIC_CLANG_POP
    #define HELENA_DIAGNOSTIC_CLANG_IGNORE(id)
#endif

#if defined(HELENA_COMPILER_GCC)
    #define HELENA_DIAGNOSTIC_GCC_PUSH          _Pragma("GCC diagnostic push")
    #define HELENA_DIAGNOSTIC_GCC_POP           _Pragma("GCC diagnostic pop")
    #define HELENA_DIAGNOSTIC_GCC_IGNORE(id)    _Pragma(HELENA_STRINGIFY(GCC diagnostic ignored id))
#else
    #define HELENA_DIAGNOSTIC_GCC_PUSH
    #define HELENA_DIAGNOSTIC_GCC_POP
    #define HELENA_DIAGNOSTIC_GCC_IGNORE(id)
#endif

#if defined(HELENA_COMPILER_MSVC)
    #define HELENA_DIAGNOSTIC_MSVC_PUSH         _Pragma("warning(push)")
    #define HELENA_DIAGNOSTIC_MSVC_POP          _Pragma("warning(pop)")
    #define HELENA_DIAGNOSTIC_MSVC_IGNORE(id)   _Pragma(HELENA_STRINGIFY(warning(disable: id)))
#else
    #define HELENA_DIAGNOSTIC_MSVC_PUSH
    #define HELENA_DIAGNOSTIC_MSVC_POP
    #define HELENA_DIAGNOSTIC_MSVC_IGNORE(id)
#endif

#if defined(HELENA_COMPILER_CLANG)
    #define HELENA_DIAGNOSTIC_PUSH              HELENA_DIAGNOSTIC_CLANG_PUSH
    #define HELENA_DIAGNOSTIC_POP               HELENA_DIAGNOSTIC_CLANG_POP
    #define HELENA_DIAGNOSTIC_IGNORE(id)        HELENA_DIAGNOSTIC_CLANG_IGNORE(id)
#elif defined(HELENA_COMPILER_GCC)
    #define HELENA_DIAGNOSTIC_PUSH              HELENA_DIAGNOSTIC_GCC_PUSH
    #define HELENA_DIAGNOSTIC_POP               HELENA_DIAGNOSTIC_GCC_POP
    #define HELENA_DIAGNOSTIC_IGNORE(id)        HELENA_DIAGNOSTIC_GCC_IGNORE(id)
#elif defined(HELENA_COMPILER_MSVC)
    #define HELENA_DIAGNOSTIC_PUSH              HELENA_DIAGNOSTIC_MSVC_PUSH
    #define HELENA_DIAGNOSTIC_POP               HELENA_DIAGNOSTIC_MSVC_POP
    #define HELENA_DIAGNOSTIC_IGNORE(id)        HELENA_DIAGNOSTIC_MSVC_IGNORE(id)
#else
    #define HELENA_DIAGNOSTIC_PUSH
    #define HELENA_DIAGNOSTIC_POP
    #define HELENA_DIAGNOSTIC_IGNORE(id)
#endif

/* ----------- [Features] ----------- */
#if defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
    #define HELENA_LINUX_FASTCALL               __attribute__((fastcall))
    #define HELENA_WIN_FASTCALL
#elif defined(HELENA_COMPILER_MSVC)
    #define HELENA_WIN_FASTCALL                 __fastcall
    #define HELENA_LINUX_FASTCALL
#endif

#if defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
    #define HELENA_FORCEINLINE                  __attribute__((always_inline))
#elif defined(HELENA_COMPILER_MSVC)
    #define HELENA_FORCEINLINE                  __forceinline
#endif

#if defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
    #define HELENA_NOINLINE                     __attribute__((noinline))
#elif defined(HELENA_COMPILER_MSVC)
    #define HELENA_NOINLINE                     __declspec(noinline)
#endif

#if defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
#define HELENA_NORETURN                         __attribute__((noreturn))
#elif defined(HELENA_COMPILER_MSVC)
#define HELENA_NORETURN                         __declspec(noreturn)
#endif

#if defined(HELENA_COMPILER_CLANG) || defined(HELENA_COMPILER_GCC)
    #define HELENA_FUNCTION                     __PRETTY_FUNCTION__
#else
    #define HELENA_FUNCTION                     __FUNCSIG__
#endif

#if defined(HELENA_COMPILER_CLANG) || defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_MSVC)
    #define HELENA_RESTRICT                     __restrict
#else
    #define HELENA_RESTRICT
#endif

#if defined(HELENA_COMPILER_CLANG)
    #define HELENA_OPTIMIZATION_ENABLE          _Pragma("clang optimize on")
    #define HELENA_OPTIMIZATION_DISABLE         _Pragma("clang optimize off")
#elif defined(HELENA_COMPILER_GCC)
    #define HELENA_OPTIMIZATION_ENABLE          _Pragma("GCC pop_options")
    #define HELENA_OPTIMIZATION_DISABLE         _Pragma("GCC push_options") \
                                                _Pragma("GCC optimize (\"-O0\")")
#elif defined(HELENA_COMPILER_MSVC)
    #define HELENA_OPTIMIZATION_ENABLE          _Pragma("optimize(\"\", on)")
    #define HELENA_OPTIMIZATION_DISABLE         _Pragma("optimize(\"\", off)")
#else
    #define HELENA_OPTIMIZATION_ENABLE
    #define HELENA_OPTIMIZATION_DISABLE
#endif

#if __has_cpp_attribute(likely)
    #define HELENA_LIKELY                       [[likely]]
#else
    #define HELENA_LIKELY
#endif

#if __has_cpp_attribute(unlikely)
    #define HELENA_UNLIKELY                     [[unlikely]]
#else
    #define HELENA_UNLIKELY
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1929)
    #define HELENA_NO_UNIQUE_ADDRESS            [[msvc::no_unique_address]]
#elif __has_cpp_attribute(no_unique_address)
    #define HELENA_NO_UNIQUE_ADDRESS            [[no_unique_address]]
#else
    #define HELENA_NO_UNIQUE_ADDRESS
#endif

#if defined(HELENA_COMPILER_MSVC)
    #include <immintrin.h>
    #pragma intrinsic(_mm_pause)
    #define HELENA_PROCESSOR_YIELD()            _mm_pause()
#elif defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
    #define HELENA_PROCESSOR_YIELD()            __builtin_ia32_pause()
#endif

#define HELENA_NEW(type)                        new type
#define HELENA_NEW_NOTHROW(type)                new (std::nothrow) type
#define HELENA_NEW_ARRAY(type, size)            new type[size]
#define HELENA_NEW_ARRAY_NOTHROW(type, size)    new (std::nothrow) type[size]
#define HELENA_DELETE(type)                     delete type
#define HELENA_DELETE_ARRAY(type)               delete[] type

#define HELENA_CLASSNAME(type)                  #type
#define HELENA_CLASSNAME_RUNTIME(type)          typeid(type).name()

#endif  // HELENA_PLATFORM_DEFINES_HPP
