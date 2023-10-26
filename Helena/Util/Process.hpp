#ifndef HELENA_UTIL_PROCESS_HPP
#define HELENA_UTIL_PROCESS_HPP

#include <Helena/Platform/Platform.hpp>

#include <chrono>
#include <thread>

namespace Helena::Util
{
    class Process
    {
        static inline auto m_ExecutablePath = []{
        #if defined(HELENA_PLATFORM_LINUX)
            static char path[PATH_MAX]{};
            auto length = ::readlink("/proc/self/exe", path, sizeof(path));
            length = length * !(length == -1 || length == sizeof(path));
        #elif defined(HELENA_PLATFORM_WIN)
            static char path[MAX_PATH]{};
            auto length = ::GetModuleFileNameA(nullptr, path, MAX_PATH);
        #else
            #error Unsupported platform
        #endif
            while(length--) {
                const auto notFound = !(path[length] == HELENA_SEPARATOR);
                if(path[length] = path[length] * notFound; !notFound) break;
            }
            return path;
        }();

    public:
        static const char* ExecutablePath() {
            return m_ExecutablePath;
        }

        static void Sleep(const std::uint64_t milliseconds) {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }

        template <typename Rep, typename Period>
        static void Sleep(const std::chrono::duration<Rep, Period>& time) {
            std::this_thread::sleep_for(time);
        }
    };
}

#endif // HELENA_UTIL_PROCESS_HPP