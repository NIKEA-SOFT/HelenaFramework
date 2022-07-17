#ifndef HELENA_UTIL_SLEEP_HPP
#define HELENA_UTIL_SLEEP_HPP

#include <cstdint>
#include <chrono>
#include <thread>

namespace Helena::Util
{
    inline void Sleep(const std::uint64_t milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    template <typename Rep, typename Period>
    void Sleep(const std::chrono::duration<Rep, Period>& time) {
        std::this_thread::sleep_for(time);
    }
}

#endif // HELENA_CORE_UTIL_SLEEP_HPP