#ifndef HELENA_OS_WINDOWS_HPP
#define HELENA_OS_WINDOWS_HPP

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
#define HF_SLEEP(ms)            Sleep(ms)

#define HF_API                  extern "C" __declspec(dllexport)
#define HF_FORCEINLINE          __forceinline

#define HF_MODULE_HANDLE        HINSTANCE
#define HF_MODULE_LOAD(a)       LoadLibraryExA(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
#define HF_MODULE_ENTRYPOINT    "MainPlugin"
#define HF_MODULE_GETSYM(a, b)  GetProcAddress(a, b)
#define HF_MODULE_UNLOAD(a)     FreeLibrary(a)

#define HF_SEPARATOR '\\'

#endif // HELENA_OS_WINDOWS_HPP
