#ifndef HELENA_OS_LINUX_HPP
#define HELENA_OS_LINUX_HPP

#if defined(HELENA_PLATFORM_LINUX)

	#include <signal.h>
	#include <dlfcn.h>
	#include <unistd.h>
	#include <sys/ptrace.h>

	// Definition
	#define HELENA_SLEEP(ms)            usleep(ms * 1000)
	#define HELENA_DEBUGGING()          (ptrace(PT_TRACE_ME, 0, 0, 0) < 0)
	#define HELENA_BREAKPOINT()			asm("int3")

	#define HELENA_API_EXPORT			__attribute__((visibility("default")))
	#define HELENA_API_IMPORT			__attribute__((visibility("default")))

	#define HELENA_MODULE_HANDLE        void*
	#define HELENA_MODULE_LOAD(a)       dlopen((a), RTLD_LAZY | RTLD_GLOBAL)
	#define HELENA_MODULE_GETSYM(a, b)  dlsym(a, b)
	#define HELENA_MODULE_UNLOAD(a)     dlclose(a)
	#define HELENA_MODULE_EXTENSION     ".so"

	#define HELENA_SEPARATOR '/'

#endif // HELENA_PLATFORM_LINUX
#endif // HELENA_OS_LINUX_HPP
