#ifndef HELENA_PLATFORM_PLATFORM_HPP
#define HELENA_PLATFORM_PLATFORM_HPP

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

    /* ----------- [Standard detect] ----------- */
#if HELENA_STANDARD_VER > 201703L

    #define HELENA_STANDARD_CPP20

    /* ----------- [Debug detect] ----------- */
    #if defined(DEBUG) || defined(_DEBUG)
        #define HELENA_DEBUG
    #endif


    /* ----------- [Platform detect] ----------- */
    #if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
        #define HELENA_PLATFORM_NAME    "Windows"
        #define HELENA_PLATFORM_WIN

        #include <Helena/Platform/Windows/Windows.hpp>
    #elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
        #define HELENA_PLATFORM_NAME    "Linux"
        #define HELENA_PLATFORM_LINUX

        #include <Helena/Platform/Linux/Linux.hpp>
    #else
        #error Unsupported platform
    #endif

    // Definition
    #define HELENA_MODULE_ENTRYPOINT                "MainModule"


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

    #define HELENA_NEW(type)                        new type
    #define HELENA_NEW_NOTHROW(type)                new (std::nothrow) type
    #define HELENA_NEW_ARRAY(type, size)            new type[size]
    #define HELENA_NEW_ARRAY_NOTHROW(type, size)    new (std::nothrow) type[size]
    #define HELENA_DELETE(type)                     delete type
    #define HELENA_DELETE_ARRAY(type)               delete[] type

    #define HELENA_CLASSNAME(type)                  #type
    #define HELENA_CLASSNAME_RUNTIME(type)          typeid(type).name()
#endif  // HELENA_STANDARD_VER

#endif  // HELENA_PLATFORM_PLATFORM_HPP
