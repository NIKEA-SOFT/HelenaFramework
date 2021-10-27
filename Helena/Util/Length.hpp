#ifndef HELENA_UTIL_LENGTH_HPP
#define HELENA_UTIL_LENGTH_HPP

#include <Helena/Debug/Assert.hpp>

namespace Helena::Util
{
	enum class ELengthPolicy : std::uint8_t {
		Truncate,
		Fixed
	};

	template <typename Char = char>
	[[nodiscard]] constexpr std::size_t Length(const Char* data, 
		std::size_t max_size = std::numeric_limits<std::size_t>::max(), 
		ELengthPolicy policy = ELengthPolicy::Truncate) noexcept 
	{
		if(!std::is_constant_evaluated()) {
			HELENA_ASSERT(max_size, "Max size cannot be null");
		}

		std::size_t offset = 0;

		if(data)
		{ 
			while(data[offset] != '\0') 
			{
				if(++offset > max_size) 
				{
					if(policy == ELengthPolicy::Truncate) {
						return offset - 1;
					} else if(policy == ELengthPolicy::Fixed) {
						return std::size_t{};
					}
				}
			}
		}

		return offset;
	}
}

#endif // HELENA_UTIL_LENGTH_HPP