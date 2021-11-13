#ifndef HELENA_TYPES_FIXEDSTRING_HPP
#define HELENA_TYPES_FIXEDSTRING_HPP

#include <Helena/Types/Hash.hpp>
#include <Helena/Dependencies/FixedString.hpp>

namespace Helena::Types
{
	template <size_t Size>
	struct FixedString : fixstr::basic_fixed_string<char, Size> {
		using fixstr::basic_fixed_string<char, Size>::basic_fixed_string;
	};

	template <std::size_t N>
	FixedString(const char (&)[N]) -> FixedString<N - 1>;
}

#endif // HELENA_TYPES_FIXEDSTRING_HPP
