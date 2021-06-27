#ifndef HELENA_UTIL_IPP
#define HELENA_UTIL_IPP

#include <Helena/Util.hpp>
#include <Helena/Internal.hpp>

#include <thread>
#include <algorithm>

namespace Helena::Util
{
    [[nodiscard]] inline constexpr const char* GetPrettyFile(const std::string_view file) noexcept {
        constexpr char symbols[]{'\\', '/'};
    #ifdef HF_STANDARD_CPP17
        const auto it = Internal::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
    #elif defined(HF_STANDARD_CPP20)
        const auto it = std::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
    #endif
        return it == file.rend() ? file.data() : &(*std::prev(it));
    }

    inline void Sleep(const uint64_t milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    template <typename Rep, typename Period>
    inline void Sleep(const std::chrono::duration<Rep, Period>& time) {
        std::this_thread::sleep_for(time);
    }
}

#endif // HELENA_UTIL_IPP
