#ifndef HELENA_UTIL_LENGTH_HPP
#define HELENA_UTIL_LENGTH_HPP

#include <algorithm>
#include <string>

namespace Helena::Util
{
    struct String
    {
        /**
        * @brief Get the truncated length of the data
        * @tparam Char Type of characters
        * @tparam Trait Type of std::char_traits<Char>
        * @param data Pointer to characters
        * @param max Maximum search range (with null terminator)
        * @return Returns 0 if data is nullptr. Otherwise, the length is returned.
        */
        template <typename Char, typename Trait = std::char_traits<Char>>
        [[nodiscard]] static constexpr std::size_t LengthTruncated(const Char* data, std::size_t max) {
            if(!data || !max) return 0;
            const auto found = Trait::find(data, max, 0);
            return found ? std::distance(data, found) : max;
        }

        /**
        * @brief Get the length of the data
        * @tparam Char Type of characters
        * @tparam Trait Type of std::char_traits<Char>
        * @param data Pointer to characters
        * @return Returns 0 if data is nullptr. Otherwise, the length is returned.
        */
        template <typename Char, typename Trait = std::char_traits<Char>>
        [[nodiscard]] static constexpr std::size_t Length(const Char* data) {
            return data ? Trait::length(data) : 0;
        }

        /**
        * @brief Get the length of the data
        * @tparam Char Type of characters
        * @tparam Trait Type of std::char_traits<Char>
        * @param data Pointer to characters
        * @param max Maximum search range (with null terminator)
        * @return Returns 0 if data is nullptr or null terminator was not found in search range.
        * Otherwise, the length is returned.
        */
        template <typename Char, typename Trait = std::char_traits<Char>>
        [[nodiscard]] static constexpr std::size_t Length(const Char* data, std::size_t max) {
            if(!data || !max) return 0;
            const auto found = Trait::find(data, max, 0);
            return found ? std::distance(data, found) : 0;
        }
    };
}

#endif // HELENA_UTIL_LENGTH_HPP