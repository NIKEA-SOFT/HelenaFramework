#ifndef COMMON_PLATFORM_HPP
#define COMMON_PLATFORM_HPP

#define HF_PLATFORM_WIN     1
#define HF_PLATFORM_LINUX   2

#define HF_COMPILER_MSVC    1
#define HF_COMPILER_CLANG   2
#define HF_COMPILER_GCC     3

#define HF_STANDARD_CPP17   201703L

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

#if defined(_MSC_VER) 
    #define HF_COMPILER_NAME    "MSVC"
    #define HF_COMPILER         HF_COMPILER_MSVC
    #define HF_STANDARD_VER     _MSVC_LANG
#elif defined(__clang__)
    #define HF_COMPILER_NAME    "Clang"
    #define HF_COMPILER         HF_COMPILER_CLANG
    #define HF_STANDARD_VER     __cplusplus
#elif defined(__GNUC__) || defined(__GNUG__)
    #define HF_COMPILER_NAME    "GCC"
    #define HF_COMPILER         HF_COMPILER_GCC
    #define HF_STANDARD_VER     __cplusplus
#endif

#if defined(DEBUG) || defined(_DEBUG)
    #define HF_DEBUG
#else
    #define HF_RELEASE
#endif

#if HF_PLATFORM == HF_PLATFORM_WIN
    #pragma warning(disable:4091)
    #pragma warning(disable:4251)
    
    #define NOMINMAX
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #if _MSC_VER >= 1910
        #pragma execution_character_set("utf-8")
    #endif // _MSC_VER >= 1910

    // Including
    #include <Windows.h>
    #include <WinSock2.h>
    #include <minidumpapiset.h>

    inline const auto ENABLE_UNICODE_CONSOLE = []() {
        SetConsoleCP(65001);
        SetConsoleOutputCP(65001);
        return 0;
    }();

    // Definition
    #define HF_API                  extern "C" __declspec(dllexport)
    
    #define HF_MODULE_HANDLE        HINSTANCE
    #define HF_MODULE_LOAD(a)       LoadLibraryExA(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    #define HF_MODULE_CALLBACK      "HFMain"
    #define HF_MODULE_GETSYM(a, b)  GetProcAddress(a, b)
    #define HF_MODULE_UNLOAD(a)     FreeLibrary(a)

    #define HF_SEPARATOR '\\'
    
    #ifdef HF_DEBUG
        #define HF_DEBUG_BREAK()    __debugbreak()
		#define HF_ASSERT(cond, msg) {                          \
            do {                                                \
                if(!(cond)) {                                   \
	                char logInfo[1024];                         \
	                snprintf(logInfo, sizeof(logInfo),          \
	                    "File: %s | Line: %d\n"                 \
	                    "Condition: %s\n"                       \
	                    "Message: %s",                          \
	                   __FILE__, __LINE__, (#cond),             \
						std::string_view(msg).data());          \
	                printf("%s", logInfo);                      \
	                ::MessageBeep(MB_ICONERROR);                \
	                ::MessageBoxA(NULL, logInfo,                \
	                    "Assert happen, hey look at this!",     \
	                    MB_RETRYCANCEL | MB_ICONERROR);         \
	                HF_DEBUG_BREAK();                           \
                }                                               \
            } while(false);                                     \
        }
    #else
        #define HF_ASSERT(cond, msg)
    #endif // HF_DEBUG


#elif HF_PLATFORM == HF_PLATFORM_LINUX
    // Including
    #include <signal.h>
    #include <dlfcn.h>

    // Definition
    #define HF_API                  extern "C" __attribute__((visibility("default")))

    #define HF_MODULE_HANDLE        void*
    #define HF_MODULE_LOAD(a)       dlopen((a), RTLD_LAZY | RTLD_GLOBAL)
    #define HF_MODULE_CALLBACK      "HFMain"
    #define HF_MODULE_GETSYM(a, b)  dlsym(a, b)
    #define HF_MODULE_UNLOAD(a)     dlclose(a)

    #define HF_SEPARATOR '/'

    #ifdef HF_DEBUG
        #define HF_DEBUG_BREAK()    raise(SIGTRAP)
        #define HF_ASSERT(cond, msg) {                          \
            do {                                                \
                if(!(cond)) {                                   \
	                char logInfo[1024];                         \
	                snprintf(logInfo, sizeof(logInfo),          \
	                    "File: %s | Line: %d\n"                 \
	                    "Condition: %s\n"                       \
	                    "Message: %s",                          \
	                   __FILE__, __LINE__, (#cond),             \
						std::string_view(msg).data());          \
	                printf("%s", logInfo);                      \
	                HF_DEBUG_BREAK();                           \
                }                                               \
            } while(false);                                     \
        }
    #else
        #define HF_ASSERT(cond, msg)
    #endif // HF_DEBUG

#endif // HF_PLATFORM_WIN

#define HF_NEW                      new (std::nothrow)
#define HF_FREE(p)                  if(p) { delete p; p = nullptr; }

#define HF_NEW_ARRAY(type, size)    new (std::nothrow) type[size];
#define HF_FREE_ARRAY(type, p)      if(p) { delete[] static_cast<type*>(p); p = nullptr; }

#define HF_CLASSNAME(type)          (#type)
#define HF_CLASSNAME_RT(type)       typeid(type).name()

#endif  // COMMON_PLATFORM_HPP