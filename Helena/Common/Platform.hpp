#ifndef COMMON_PLATFORM_HPP
#define COMMON_PLATFORM_HPP

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
    #error USED UNSUPPORTED PLATFORM
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

#ifdef HF_STANDARD_CPP17
namespace Helena::Internal {
    template<class InputIt, class ForwardIt>
    [[nodiscard]] constexpr InputIt find_first_of(InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last) noexcept {
        for(; first != last; ++first) {
            for(ForwardIt it = s_first; it != s_last; ++it) {
                if(*first == *it) {
                    return first;
                }
            }
        }
        return last;
    }
}
#endif

namespace Helena::Internal {
    [[nodiscard]] constexpr const char* GetPrettyFile(const std::string_view file) {
        constexpr char symbols[]{'\\', '/'};
    #ifdef HF_STANDARD_CPP17
        const auto it = Internal::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
    #elif defined(HF_STANDARD_CPP20)
        const auto it = std::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
    #endif
        return it == file.rend() ? file.data() : &(*std::prev(it));
    }
}

#define HF_FILE_LINE                    Helena::Internal::GetPrettyFile(__FILE__), __LINE__
#define HF_FORMAT(msg, ...)             fmt::format(msg, ##__VA_ARGS__)
#define HF_MSG(msg, ...)                fmt::print(msg "\n", ##__VA_ARGS__)
#define HF_MSG_DEBUG(msg, ...)          fmt::print(fg(fmt::terminal_color::bright_blue), "[{:%Y.%m.%d %H:%M:%S}][{}:{}][DEBUG] " msg "\n", fmt::localtime(std::time(nullptr)), HF_FILE_LINE, ##__VA_ARGS__)
#define HF_MSG_INFO(msg, ...)           fmt::print(fg(fmt::terminal_color::bright_green), "[{:%Y.%m.%d %H:%M:%S}][{}:{}][INFO] " msg "\n", fmt::localtime(std::time(nullptr)), HF_FILE_LINE, ##__VA_ARGS__)
#define HF_MSG_WARN(msg, ...)           fmt::print(fg(fmt::terminal_color::bright_yellow), "[{:%Y.%m.%d %H:%M:%S}][{}:{}][WARN] " msg "\n", fmt::localtime(std::time(nullptr)), HF_FILE_LINE, ##__VA_ARGS__)
#define HF_MSG_ERROR(msg, ...)          fmt::print(fg(fmt::terminal_color::bright_red), "[{:%Y.%m.%d %H:%M:%S}][{}:{}][ERROR] " msg "\n", fmt::localtime(std::time(nullptr)), HF_FILE_LINE, ##__VA_ARGS__)
#define HF_MSG_FATAL(msg, ...)          fmt::print(fg(fmt::terminal_color::bright_white) | bg(fmt::terminal_color::bright_red), "[{:%Y.%m.%d %H:%M:%S}][{}:{}][FATAL] " msg "\n", fmt::localtime(std::time(nullptr)), HF_FILE_LINE, ##__VA_ARGS__)

#if HF_PLATFORM == HF_PLATFORM_WIN
    #if HF_COMPILER == HF_COMPILER_MSVC
        #pragma warning(disable:4091)
        #pragma warning(disable:4251)
        #pragma warning(disable:4068)
    #endif
    
    #pragma comment(lib, "winmm.lib")
    
    #define NOMINMAX
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    
    #if HF_COMPILER == HF_COMPILER_MSVC
        #if _MSC_VER >= 1910
            #pragma execution_character_set("utf-8")
        #endif // _MSC_VER >= 1910
    #endif

    // Including
    #include <Windows.h>
    #include <WinSock2.h>
    #include <timeapi.h>
    #include <Dbghelp.h>
    #include <minidumpapiset.h>

    inline const auto ENABLE_UNICODE_AND_VIRTUAL_TERMINAL = []() {
        // Set UTF-8
        SetConsoleCP(65001);
        SetConsoleOutputCP(65001);

        timeBeginPeriod(1);
        
        // Fix Windows fmt.print, enable virtual terminal processing
        if(HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); hStdOut != INVALID_HANDLE_VALUE) 
        {
            DWORD mode {};
            if(!GetConsoleMode(hStdOut, &mode)) {
                HF_MSG_ERROR("Get console mode failed!");
                std::terminate();
            }

            if(!SetConsoleMode(hStdOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                HF_MSG_ERROR("Set console handle virtual terminal processing failed!");
                std::terminate();
            }

        } else {
            HF_MSG_ERROR("Get console handle failed!");
            std::terminate();
        }

        return 0;
    }();

    // Definition
    #define HF_SLEEP(ms)            Sleep(ms)

    #define HF_API                  extern "C" __declspec(dllexport)
    #define HF_FORCEINLINE          __forceinline

    #define HF_MODULE_HANDLE        HINSTANCE
    #define HF_MODULE_LOAD(a)       LoadLibraryExA(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    #define HF_MODULE_ENTRYPOINT    "MainPlugin"
    #define HF_MODULE_GETSYM(a, b)  GetProcAddress(a, b)
    #define HF_MODULE_UNLOAD(a)     FreeLibrary(a)

    #define HF_SEPARATOR '\\'
    
    #ifdef HF_DEBUG
        #define HF_DEBUG_BREAK()    _CrtDbgBreak()
        #define HF_ASSERT(cond, ...)                                    \
            do {                                                        \
                if(!(cond)) {                                           \
                    constexpr auto size = std::tuple_size<decltype      \
                        (std::make_tuple(__VA_ARGS__))>::value;         \
                    HF_MSG_FATAL("Assert: " #cond);                     \
                    if(size) {                                          \
                        HF_MSG_FATAL("LOG: " __VA_ARGS__);              \
                    }                                                   \
                    ::MessageBeep(MB_ICONERROR);                        \
                    std::terminate();                                   \
                }                                                       \
            } while(false)
    #else
        #define HF_DEBUG_BREAK() 
        #define HF_ASSERT(cond, msg, ...) 
    #endif // HF_DEBUG


#elif HF_PLATFORM == HF_PLATFORM_LINUX
    // Including
    #include <signal.h>
    #include <dlfcn.h>
    #include <unistd.h>

    // Definition
    #define HF_SLEEP(ms)            usleep(ms * 1000)

    #define HF_API                  extern "C" __attribute__((visibility("default")))
    #define HF_FORCEINLINE          __attribute__((always_inline))
    
    #define HF_MODULE_HANDLE        void*
    #define HF_MODULE_LOAD(a)       dlopen((a), RTLD_LAZY | RTLD_GLOBAL)
    #define HF_MODULE_ENTRYPOINT    "MainPlugin"
    #define HF_MODULE_GETSYM(a, b)  dlsym(a, b)
    #define HF_MODULE_UNLOAD(a)     dlclose(a)

    #define HF_SEPARATOR '/'

    // todo: check assert
    #ifdef HF_DEBUG
        #define HF_DEBUG_BREAK()    raise(SIGTRAP)
        #define HF_ASSERT(cond, ...)                                    \
            do {                                                        \
                if(!(cond)) {                                           \
                    constexpr auto size = std::tuple_size<decltype      \
                        (std::make_tuple(__VA_ARGS__))>::value;         \
                    HF_MSG_FATAL("Assert: " #cond);                     \
                    if(size) {                                          \
                        HF_MSG_FATAL("LOG: " __VA_ARGS__);              \
                    }                                                   \
                    HF_DEBUG_BREAK();                                   \
                    std::terminate();                                   \
                }                                                       \
            } while(false)
    #else
        #define HF_DEBUG_BREAK()
        #define HF_ASSERT(cond, msg, ...)
    #endif // HF_DEBUG

#endif

#if defined(HF_STANDARD_CPP11_OR_GREATER)
    #define HF_NOEXCEPT             noexcept
    #define HF_CONSTEXPR            constexpr
    #define HF_FINAL                final
#else 
    #define HF_NOEXCEPT             throw()
    #define HF_CONSTEXPR 
    #define HF_FINAL 
#endif

#define HF_NEW                      new (std::nothrow)
#define HF_FREE(p)                  if(p) { delete p; p = nullptr; }

#define HF_NEW_ARRAY(type, size)    new (std::nothrow) type[size];
#define HF_FREE_ARRAY(type, p)      if(p) { delete[] static_cast<type*>(p); p = nullptr; }

#define HF_CLASSNAME(type)          (#type)
#define HF_CLASSNAME_RT(type)       typeid(type).name()

#endif  // COMMON_PLATFORM_HPP
