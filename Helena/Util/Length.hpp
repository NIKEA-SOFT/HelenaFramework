#ifndef HELENA_UTIL_LENGTH_HPP
#define HELENA_UTIL_LENGTH_HPP

#include <cstdint>
#include <cstddef>
#include <limits>

namespace Helena::Util
{
    enum class ELengthPolicy : std::uint8_t {
        Truncate,
        Fixed
    };

    template <typename Char = char>
    [[nodiscard]] constexpr std::size_t Length(ELengthPolicy policy, const Char* data, std::size_t max_size = std::numeric_limits<std::size_t>::max()) noexcept
    {
        std::size_t offset = 0;
        if(data) [[likely]]
        {
            while(true)
            {
                if(offset < max_size) [[likely]]
                {
                    if(data[offset] != '\0') [[likely]]
                        ++offset;
                    else [[unlikely]]
                        break;
                }
                else [[unlikely]]
                {
                    switch(policy)
                    {
                        case ELengthPolicy::Truncate:   return offset;
                        case ELengthPolicy::Fixed:      return 0uLL;
                    }
                }
            }
        }

        return offset;
    }
}

#endif // HELENA_UTIL_LENGTH_HPP