#ifndef HELENA_UTIL_HPP
#define HELENA_UTIL_HPP

#include <cstdint>
#include <chrono>
#include <string_view>

namespace Helena::Util
{
    [[nodiscard]] inline constexpr const char* GetPrettyFile(const std::string_view file) noexcept;

    inline void Sleep(const uint64_t milliseconds);

    template <typename Rep, typename Period>
    void Sleep(const std::chrono::duration<Rep, Period>& time);

}

#define HF_FILE_LINE    Helena::Util::GetPrettyFile(__FILE__), __LINE__

#include <Helena/Util.ipp>

#endif // HELENA_UTIL_HPP
