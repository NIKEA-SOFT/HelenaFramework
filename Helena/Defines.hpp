#ifndef HELENA_DEFINES_HPP
#define HELENA_DEFINES_HPP

#define HF_PLATFORM_WIN     1
#define HF_PLATFORM_LINUX   2

#define HF_COMPILER_MSVC    1
#define HF_COMPILER_CLANG   2
#define HF_COMPILER_GCC     3

// Detect Platform
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
    #define HF_PLATFORM_NAME    "Windows"
    #define HF_PLATFORM         HF_PLATFORM_WIN
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    #define HF_PLATFORM_NAME    "Linux"
    #define HF_PLATFORM         HF_PLATFORM_LINUX
#else
    #error Unsupported platform
#endif

#if defined(__clang__)
    #define HF_COMPILER_NAME    "Clang"
    #define HF_COMPILER         HF_COMPILER_CLANG
    #define HF_STANDARD_VER     __cplusplus
#elif defined(__GNUC__) || defined(__GNUG__)
    #define HF_COMPILER_NAME    "GCC"
    #define HF_COMPILER         HF_COMPILER_GCC
    #define HF_STANDARD_VER     __cplusplus
#elif defined(_MSC_VER)
    #define HF_COMPILER_NAME    "MSVC"
    #define HF_COMPILER         HF_COMPILER_MSVC
    #define HF_STANDARD_VER     _MSVC_LANG
#endif

#if (__cplusplus == 201103L) || (defined(_MSVC_LANG) && _MSVC_LANG == 201103L)
    #define HF_STANDARD_CPP11
#endif

#if (__cplusplus == 201402L) || (defined(_MSVC_LANG) && _MSVC_LANG == 201402L)
    #define HF_STANDARD_CPP14
#endif

#if (__cplusplus == 201703L) || (defined(_MSVC_LANG) && _MSVC_LANG == 201703L)
    #define HF_STANDARD_CPP17
#endif

#if (__cplusplus > 201703L) || (defined(_MSVC_LANG) && _MSVC_LANG > 201703L)
    #define HF_STANDARD_CPP20
#endif

#if (__cplusplus >= 201103L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
    #define HF_STANDARD_CPP11_OR_GREATER
#endif

#if (__cplusplus >= 201402L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
    #define HF_STANDARD_CPP14_OR_GREATER
#endif

#if (__cplusplus >= 201703L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    #define HF_STANDARD_CPP17_OR_GREATER
#endif

#if defined(DEBUG) || defined(_DEBUG)
    #define HF_DEBUG
#endif

#if __has_cpp_attribute(likely)
    #define HF_LIKELY               [[likely]]
#else
    #define HF_LIKELY
#endif

#if __has_cpp_attribute(unlikely)
    #define HF_UNLIKELY             [[unlikely]]
#else
    #define HF_UNLIKELY
#endif

#if __has_cpp_attribute(no_unique_address)
    #define HF_NO_UNIQUE_ADDRESS    [[no_unique_address]]
#else
    #define HF_NO_UNIQUE_ADDRESS
#endif

//#if defined(HF_STANDARD_CPP11_OR_GREATER)
//    #define HF_NOEXCEPT             noexcept
//    #define HF_CONSTEXPR            constexpr
//    #define HF_FINAL                final
//#else
//    #define HF_NOEXCEPT             throw()
//    #define HF_CONSTEXPR
//    #define HF_FINAL
//#endif

//#if defined(HF_STANDARD_CPP17_OR_GREATER)
//    #define HF_NODISCARD            [[nodiscard]]
//#else
//    #define HF_NODISCARD
//#endif

//#if defined(HF_STANDARD_CPP20)
//    #define HF_NODISCARD_MSG(msg)   [[nodiscard(msg)]]
//#endif


#define HF_NEW                      new (std::nothrow)
#define HF_FREE(p)                  if(p) { delete p; p = nullptr; }

#define HF_NEW_ARRAY(type, size)    new (std::nothrow) type[size];
#define HF_FREE_ARRAY(type, p)      if(p) { delete[] static_cast<type*>(p); p = nullptr; }

#define HF_CLASSNAME(type)          (#type)
#define HF_CLASSNAME_RT(type)       typeid(type).name()

#endif // HELENA_DEFINES_HPP
