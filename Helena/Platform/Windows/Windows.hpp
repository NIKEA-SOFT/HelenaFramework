#ifndef HELENA_OS_WINDOWS_HPP
#define HELENA_OS_WINDOWS_HPP

#if defined(HELENA_PLATFORM_WIN)

    #if defined(HELENA_COMPILER_MSVC)
        #pragma warning(disable:4091)
        #pragma warning(disable:4251)
        #pragma warning(disable:4068)
    #endif

    #pragma comment(lib, "winmm.lib")
    #pragma comment(lib, "dbghelp.lib")

    #define NOMINMAX
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <Windows.h>
    #include <WinSock2.h>
    #include <timeapi.h>
    #include <Dbghelp.h>
    #include <exception>

    #if defined(HELENA_COMPILER_MSVC)
        #if _MSC_VER >= 1910
            #pragma execution_character_set("utf-8")
        #endif // _MSC_VER >= 1910
    #endif

    //extern "C" 
    //{                                                              
    //    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    //    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
    //}

    inline const auto ENABLE_TIME_BEGIN_PERIOD_1MS = []() {
        timeBeginPeriod(1);
        return 0;
    }();

    inline const auto ENABLE_UNICODE_AND_VIRTUAL_TERMINAL = []() {
        // Set UTF-8
        SetConsoleCP(65001);
        SetConsoleOutputCP(65001);

        // Fix Windows fmt.print, enable virtual terminal processing
        if(HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); hStdOut != INVALID_HANDLE_VALUE)
        {
            DWORD mode {};
            if(!GetConsoleMode(hStdOut, &mode)) {
                MessageBoxA(NULL, "Get console mode failed!", "Error", MB_OK);
                std::terminate();
            }

            if(!SetConsoleMode(hStdOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                MessageBoxA(NULL, "Set console handle virtual terminal processing failed!", "Error", MB_OK);
                std::terminate();
            }

        } else {
            MessageBoxA(NULL, "Get console handle failed!", "Error", MB_OK);
            std::terminate();
        }

        return 0;
    }();

    // Definition
    #define HELENA_SLEEP(ms)            Sleep(ms)
    #define HELENA_DEBUGGING()          IsDebuggerPresent()
    #define HELENA_BREAKPOINT()         __debugbreak()

    #define HELENA_API_EXPORT           __declspec(dllexport)
    #define HELENA_API_IMPORT           __declspec(dllimport)

    #define HELENA_MODULE_HANDLE        HINSTANCE
    #define HELENA_MODULE_LOAD(a)       LoadLibraryExA(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    #define HELENA_MODULE_GETSYM(a, b)  GetProcAddress(a, b)
    #define HELENA_MODULE_UNLOAD(a)     FreeLibrary(a)

    #define HELENA_SEPARATOR '\\'

#endif // HELENA_PLATFORM_WIN
#endif // HELENA_OS_WINDOWS_HPP
