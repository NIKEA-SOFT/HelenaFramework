#ifndef COMMON_HFPLATFORM_HPP
#define COMMON_HFPLATFORM_HPP

// Detect Platform
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
    #define HF_PLATFORM_WIN
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    #define HF_PLATFORM_LINUX
#else
    #error USED UNSUPPORTED PLATFORM
#endif

#ifdef HF_PLATFORM_WIN      // Windows
    #define NOMINMAX
    // Including
    #include <Windows.h>

    // Definition
    #define HF_API                  extern "C" __declspec(dllexport)

    #define HF_MODULE_HANDLE        HINSTANCE
    #define HF_MODULE_LOAD(a)       LoadLibraryExA(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    #define HF_MODULE_CALLBACK      "HFMain"
    #define HF_MODULE_GETSYM(a, b)  GetProcAddress(a, b)
    #define HF_MODULE_UNLOAD(a)     FreeLibrary(a)

    #define HF_SEPARATOR '\\'

#elif HF_PLATFORM_LINUX     // Linux
    // Including
    #include <dlfcn.h>

    // Definition
    #define HF_API                  __attribute__((visibility("default")))

    #define HF_MODULE_HANDLE        void*
    #define HF_MODULE_LOAD(a)       dlopen((a), RTLD_LAZY | RTLD_GLOBAL)
    #define HF_MODULE_GETSYM(a, b)  dlsym((a), (b))
    #define HF_MODULE_UNLOAD(a)     dlclose((a))

    #define HF_SEPARATOR '/'

#endif

#define HF_NEW                      new (std::nothrow)
#define HF_FREE(Ptr)                if(Ptr) { delete Ptr; Ptr = nullptr; }

#define HF_NEW_ARRAY(Type, Size)    new (std::nothrow) Type[Size];
#define HF_FREE_ARRAY(Type, Ptr)    if(Ptr) { delete[] Ptr; Ptr = nullptr; }

#define HF_CLASSNAME(Type)          #Type
#define HF_CLASSNAME_RT(Type)       typeid(Type).name()

#endif