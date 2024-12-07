#ifndef HELENA_OS_LINUX_HPP
#define HELENA_OS_LINUX_HPP

#if defined(HELENA_PLATFORM_LINUX)
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/ptrace.h>
    #include <sys/stat.h>
    #include <poll.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <netdb.h>
    #include <signal.h>
    #include <dlfcn.h>
    #include <unistd.h>
    #include <errno.h>
    #include <stdio.h>
    #include <fcntl.h>
    #include <execinfo.h>
    #include <cxxabi.h>
    #include <ucontext.h>
    #include <linux/limits.h>

    inline constexpr auto HELENA_ENABLE_VIRTUAL_TERMINAL_PROCESSING = true;
    inline auto HELENA_PLATFORM_HAS_CONSOLE = []() {
        return ::isatty(::fileno(stdout)) || ::isatty(::fileno(stderr));
    };

    // Definition
    #define HELENA_SLEEP(ms)            ::usleep(ms * 1000)
    #define HELENA_DEBUGGING()          (::ptrace(PT_TRACE_ME, 0, 0, 0) < 0)
    #define HELENA_BREAKPOINT()         asm("int3")

    #define HELENA_API_EXPORT           __attribute__((visibility("default")))
    #define HELENA_API_IMPORT           __attribute__((visibility("default")))

    #define HELENA_MODULE_HANDLE        void*
    #define HELENA_MODULE_LOAD(a)       ::dlopen((a), RTLD_NOW)
    #define HELENA_MODULE_ISLOAD(a)     ::dlopen((a), RTLD_NOW | RTLD_NOLOAD)
    #define HELENA_MODULE_GETSYM(a, b)  ::dlsym(a, b)
    #define HELENA_MODULE_UNLOAD(a)     ::dlclose(a)
    #define HELENA_MODULE_EXTENSION     ".so"

    #define HELENA_SEPARATOR            '/'
    #define HELENA_SEPARATOR_QUOTED     "/"
    #define HELENA_MAX_PATH_LENGTH      PATH_MAX

#endif // HELENA_PLATFORM_LINUX
#endif // HELENA_OS_LINUX_HPP
