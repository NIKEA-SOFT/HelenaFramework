#ifndef HELENA_UTIL_HPP
#define HELENA_UTIL_HPP

#include <cstdint>
#include <chrono>

#include <Helena/Internal.hpp>

namespace Helena::Util
{
    #define HF_FILE_LINE    Helena::Util::GetPrettyFile(__FILE__), __LINE__

    template <typename Container,
        typename Key = typename Container::key_type,
        typename Ret = typename Container::mapped_type,
        typename = std::enable_if_t<Internal::is_mapping_v<Internal::remove_cvref_t<Container>>, void>>
    auto AddOrGetTypeIndex(Container& container, const Key typeIndex) -> decltype(auto);

    [[nodiscard]] inline constexpr const char* GetPrettyFile(const std::string_view file) noexcept;

    inline void Sleep(const uint64_t milliseconds);

    template <typename Rep, typename Period>
    void Sleep(const std::chrono::duration<Rep, Period>& time);

}

#include <Helena/Util.ipp>

#endif // HELENA_UTIL_HPP
