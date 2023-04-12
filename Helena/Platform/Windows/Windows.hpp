#ifndef HELENA_OS_WINDOWS_HPP
#define HELENA_OS_WINDOWS_HPP

#if defined(HELENA_PLATFORM_WIN)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <iphlpapi.h>
    #include <fcntl.h>
    #include <userenv.h>
    #include <timeapi.h>
    #include <dbghelp.h>
    #include <exception>
    #include <cstdio>

    #ifdef min
        #undef min
    #endif

    #ifdef max
        #undef max
    #endif

    #if !defined(HELENA_COMPILER_MINGW)
        #pragma comment(lib, "winmm.lib")
        #pragma comment(lib, "dbghelp.lib")
        #pragma comment(lib, "ws2_32.lib")
        #pragma comment(lib, "iphlpapi.lib")
        #pragma comment(lib, "userenv.lib")
    #else
        #pragma message("Current compiler does not support #pragma comment, linking libs: winmm.lib, dbghelp.lib, ws2_32.lib, iphlpapi.lib, userenv.lib")
    #endif

    #if defined(HELENA_COMPILER_MSVC)
        #if _MSC_VER >= 1910
            #pragma execution_character_set("utf-8")
        #endif // _MSC_VER >= 1910
    #endif

    #ifndef _WINDLL
        extern "C"
        {
            __declspec(dllexport) inline DWORD NvOptimusEnablement = 0x00000001;
            __declspec(dllexport) inline int AmdPowerXpressRequestHighPerformance = 0x00000001;
        }

        inline auto ENABLE_SUPPORT_GAMES = []() noexcept {
            return NvOptimusEnablement && AmdPowerXpressRequestHighPerformance;
        }();
    #endif

    inline auto ENABLE_TIME_BEGIN_PERIOD_1MS = []() noexcept  {
        ::timeBeginPeriod(1);
        return 0;
    }();

    inline auto ENABLE_UNICODE_AND_VIRTUAL_TERMINAL = []() noexcept {

        if(!::GetConsoleWindow()) {
            return 0;
        }

        // Set UTF-8
        ::SetConsoleCP(CP_UTF8);
        ::SetConsoleOutputCP(CP_UTF8);

        // Fix Windows fmt.print, enable virtual terminal processing
        if(HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE); hStdOut != INVALID_HANDLE_VALUE)
        {
            DWORD mode{};
            if(!::GetConsoleMode(hStdOut, &mode)) {
                ::MessageBoxA(nullptr, "Get console mode failed!", "Error", MB_OK | MB_ICONERROR);
                std::terminate();
            }

            if(!::SetConsoleMode(hStdOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                ::MessageBoxA(nullptr, "Set console handle virtual terminal processing failed!", "Error", MB_OK | MB_ICONERROR);
                std::terminate();
            }

        } else {
            ::MessageBoxA(nullptr, "Get console handle failed!", "Error", MB_OK | MB_ICONERROR);
            std::terminate();
        }

        return 0;
    }();

    // Definition
    #define HELENA_SLEEP(ms)            ::Sleep(ms)
    #define HELENA_DEBUGGING()          ::IsDebuggerPresent()
    #define HELENA_BREAKPOINT()         ::__debugbreak()

    #define HELENA_API_EXPORT           __declspec(dllexport)
    #define HELENA_API_IMPORT           __declspec(dllimport)

    #define HELENA_MODULE_HANDLE        HINSTANCE
    #define HELENA_MODULE_LOAD(a)       ::LoadLibraryExA(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    #define HELENA_MODULE_GETSYM(a, b)  ::GetProcAddress(a, b)
    #define HELENA_MODULE_UNLOAD(a)     ::FreeLibrary(a)
    #define HELENA_MODULE_EXTENSION     ".dll"

    #define HELENA_SEPARATOR '\\'

#endif // HELENA_PLATFORM_WIN
#endif // HELENA_OS_WINDOWS_HPP
