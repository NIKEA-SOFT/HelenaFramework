#ifndef HELENA_OS_LINUX_HPP
#define HELENA_OS_LINUX_HPP

#include <signal.h>
#include <dlfcn.h>
#include <unistd.h>

// Definition
#define HF_SLEEP(ms)            usleep(ms * 1000)

#define HF_API                  extern "C" __attribute__((visibility("default")))

#define HF_MODULE_HANDLE        void*
#define HF_MODULE_LOAD(a)       dlopen((a), RTLD_LAZY | RTLD_GLOBAL)
#define HF_MODULE_ENTRYPOINT    "MainPlugin"
#define HF_MODULE_GETSYM(a, b)  dlsym(a, b)
#define HF_MODULE_UNLOAD(a)     dlclose(a)

#define HF_SEPARATOR '/'

#endif // HELENA_OS_LINUX_HPP
