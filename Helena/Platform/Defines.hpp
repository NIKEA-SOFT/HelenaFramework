#ifndef HELENA_PLATFORM_DEFINES_HPP
#define HELENA_PLATFORM_DEFINES_HPP

#include <Helena/Platform/Compiler.hpp>
#include <Helena/Platform/Processor.hpp>

/* ----------- [Debug detect] ----------- */
#if defined(DEBUG) || defined(_DEBUG)
    #define HELENA_DEBUG
#endif

/* ----------- [Diagnostic Pragma] ----------- */
#if defined(HELENA_COMPILER_CLANG)
    #define HELENA_DIAGNOSTIC_CLANG_PUSH        _Pragma("GCC diagnostic push")
    #define HELENA_DIAGNOSTIC_CLANG_POP         _Pragma("GCC diagnostic pop")
    #define HELENA_DIAGNOSTIC_CLANG_IGNORE(id)  _Pragma("GCC diagnostic ignored " #id)
#else 
    #define HELENA_DIAGNOSTIC_CLANG_PUSH
    #define HELENA_DIAGNOSTIC_CLANG_POP
    #define HELENA_DIAGNOSTIC_CLANG_IGNORE(id)
#endif

#if defined(HELENA_COMPILER_GCC)
    #define HELENA_DIAGNOSTIC_GCC_PUSH          _Pragma("GCC diagnostic push")
    #define HELENA_DIAGNOSTIC_GCC_POP           _Pragma("GCC diagnostic pop")
    #define HELENA_DIAGNOSTIC_GCC_IGNORE(id)    _Pragma("GCC diagnostic ignored " #id)
#else
    #define HELENA_DIAGNOSTIC_GCC_PUSH
    #define HELENA_DIAGNOSTIC_GCC_POP
    #define HELENA_DIAGNOSTIC_GCC_IGNORE(id)
#endif

#if defined(HELENA_COMPILER_MSVC)
    #define HELENA_DIAGNOSTIC_MSVC_PUSH         __pragma warning(push)
    #define HELENA_DIAGNOSTIC_MSVC_POP          __pragma warning(pop)
    #define HELENA_DIAGNOSTIC_MSVC_IGNORE(id)   __pragma warning(disable: id)
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
//#if defined(HELENA_DEBUG)
//    #if defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
//        #define HELENA_FORCEINLINE              __attribute__((always_inline)
//    #elif defined(HELENA_COMPILER_MSVC)
//        #define HELENA_FORCEINLINE              __forceinline
//    #endif
//#else
//    #define HELENA_FORCEINLINE
//#endif

#if defined(HELENA_COMPILER_CLANG) || defined(HELENA_COMPILER_GCC)
    #define HELENA_FUNCTION             __PRETTY_FUNCTION__
#else
    #define HELENA_FUNCTION             __FUNCSIG__
#endif

//#if __has_cpp_attribute(likely)
//    #define HELENA_LIKELY               [[likely]]
//#else
//    #define HELENA_LIKELY
//#endif

//#if __has_cpp_attribute(unlikely)
//    #define HELENA_UNLIKELY             [[unlikely]]
//#else
//    #define HELENA_UNLIKELY
//#endif

//#if __has_cpp_attribute(no_unique_address)
//    #define HELENA_NO_UNIQUE_ADDRESS    [[no_unique_address]]
//#else
//    #define HELENA_NO_UNIQUE_ADDRESS
//#endif

//#if defined(HELENA_STANDARD_CPP11_OR_GREATER)
//    #define HELENA_NOEXCEPT             noexcept
//    #define HELENA_CONSTEXPR            constexpr
//    #define HELENA_FINAL                final
//#else
//    #define HELENA_NOEXCEPT             throw()
//    #define HELENA_CONSTEXPR
//    #define HELENA_FINAL
//#endif
//
//#if defined(HELENA_STANDARD_CPP17_OR_GREATER)
//    #define HELENA_NODISCARD            [[nodiscard]]
//#else
//    #define HELENA_NODISCARD
//#endif
//
//#if defined(HELENA_STANDARD_CPP20)
//    #define HELENA_NODISCARD_MSG(msg)   [[nodiscard(msg)]]
//#endif

#if defined(HELENA_COMPILER_MSVC)
    #define HELENA_CONSTEVAL                consteval
#elif defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
    #define HELENA_CONSTEVAL                constexpr
#endif

#if defined(HELENA_COMPILER_MSVC)
    #include <immintrin.h>
    #pragma intrinsic(_mm_pause)
    #define HELENA_PROCESSOR_YIELD()    _mm_pause()
#elif defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
    #define HELENA_PROCESSOR_YIELD()    __builtin_ia32_pause()
#endif

//#if defined(HELENA_PROCESSOR_AMD64) || defined(HELENA_PROCESSOR_X86)
//    #if defined(HELENA_COMPILER_MSVC)
//        #include <immintrin.h>
//        #pragma intrinsic(_mm_pause)
//        #define HELENA_PROCESSOR_YIELD()        _mm_pause()
//    #elif defined(HELENA_COMPILER_GCC) || defined(HELENA_COMPILER_CLANG)
//        #if __has_builtin (__builtin_ia32_pause)
//            #define HELENA_PROCESSOR_YIELD()    __builtin_ia32_pause()
//        #else
//            #define HELENA_PROCESSOR_YIELD()    __asm__ __volatile__ ("pause")
//        #endif
//    #endif
//#elif defined(HELENA_PROCESSOR_ARM)
//    #define HELENA_PROCESSOR_YIELD()            __asm__ __volatile__ ("isb" ::: "memory")
//#elif defined(HELENA_PROCESSOR_PPC64)
//    #define HELENA_PROCESSOR_YIELD()            __asm__ __volatile__ ("or 27,27,27" ::: "memory")
//#elif defined(HELENA_PROCESSOR_IA64)
//    #define HELENA_PROCESSOR_YIELD()            __asm__ __volatile__ ("hint @pause")
//#elif defined(HELENA_PROCESSOR_SPARC)
//    #define HELENA_PROCESSOR_YIELD()            __asm__ __volatile__ ("nop;");
//#endif

#define HELENA_NEW(type)                        new type
#define HELENA_NEW_NOTHROW(type)                new (std::nothrow) type
#define HELENA_NEW_ARRAY(type, size)            new type[size]
#define HELENA_NEW_ARRAY_NOTHROW(type, size)    new (std::nothrow) type[size]
#define HELENA_DELETE(type)                     delete type
#define HELENA_DELETE_ARRAY(type)               delete[] type

#define HELENA_CLASSNAME(type)                  #type
#define HELENA_CLASSNAME_RUNTIME(type)          typeid(type).name()

#endif  // HELENA_PLATFORM_DEFINES_HPP
