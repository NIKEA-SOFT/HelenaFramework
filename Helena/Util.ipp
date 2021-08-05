#ifndef HELENA_UTIL_IPP
#define HELENA_UTIL_IPP

#include <Helena/Util.hpp>
#include <thread>

namespace Helena::Util
{
    [[nodiscard]] inline constexpr std::string_view GetSourceName(const std::string_view file, const std::string_view delimeter) noexcept {
        const auto it = std::find_first_of(file.crbegin(), file.crend(), std::cbegin(delimeter), std::cend(delimeter));
        return std::string_view(it == file.crend() ? file.cbegin() : it.base(), file.cend());
    }

    inline void Sleep(const std::uint64_t milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    template <typename Rep, typename Period>
    inline void Sleep(const std::chrono::duration<Rep, Period>& time) {
        std::this_thread::sleep_for(time);
    }
}

#endif // HELENA_UTIL_IPP
