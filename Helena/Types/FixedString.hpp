#ifndef HELENA_TYPES_FIXEDSTRING_HPP
#define HELENA_TYPES_FIXEDSTRING_HPP

#include <Helena/Types/Hash.hpp>
#include <Helena/Dependencies/FixedString.hpp>

namespace Helena::Types
{
	template <std::size_t Size, typename Char = char>
	using FixedString = fixstr::basic_fixed_string<Char, Size>;
}

#endif // HELENA_TYPES_FIXEDSTRING_HPP
