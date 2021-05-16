#ifndef HELENA_UTIL_IPP
#define HELENA_UTIL_IPP

#include <Helena/Util.hpp>
#include <Helena/Log.hpp>

#include <chrono>
#include <thread>

namespace Helena::Util
{
    template <typename Container, typename Key, typename Ret, typename>
    auto AddOrGetTypeIndex(Container& container, const Key typeIndex) -> decltype(auto)
    {
        if(const auto it = container.find(typeIndex); it != container.cend()) {
            return static_cast<Ret>(it->second);
        }

        if(const auto [it, result] = container.emplace(typeIndex, container.size()); !result) {
            HF_MSG_FATAL("Type index emplace failed!");
            std::terminate();
        }

        return static_cast<Ret>(container.size()) - 1u;
    }

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
    void Sleep(const std::chrono::duration<Rep, Period>& time) {
        std::this_thread::sleep_for(time);
    }
}

#endif // HELENA_UTIL_IPP
