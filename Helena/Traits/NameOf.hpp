#ifndef HELENA_TRAITS_NAMEOF_HPP
#define HELENA_TRAITS_NAMEOF_HPP

#include <Helena/Platform/Defines.hpp>
#include <string_view>

namespace Helena::Traits 
{
	template <typename T>
	struct NameOf final {
	    static constexpr auto value = []() noexcept {
	        constexpr std::string_view name { HELENA_FUNCTION };
	        constexpr auto first = name.find_first_not_of(' ', name.find_first_of('<') + 1u);
	        constexpr auto value = name.substr(first, name.find_last_of('<') - first - 3u);
	        return value;
	    }();
	};
}

#endif // HELENA_TRAITS_NAMEOF_HPP