#ifndef HELENA_TYPES_FUNCTION_HPP
#define HELENA_TYPES_FUNCTION_HPP

#include <Helena/Dependencies/Function2.hpp>

namespace Helena::Types 
{
	template <std::size_t CapacitySBO, typename... Signatures>
	using Function			= fu2::function_base<true, true, fu2::capacity_fixed<CapacitySBO>, true, false, Signatures...>;

	template <std::size_t CapacitySBO, typename... Signatures>
	using FunctionUnique	= fu2::function_base<true, false, fu2::capacity_fixed<CapacitySBO>, true, false, Signatures...>;

	template <std::size_t CapacitySBO, typename... Signatures>
	using FunctionView		= fu2::function_base<false, true, fu2::capacity_fixed<CapacitySBO>, true, false, Signatures...>;
}

#endif // HELENA_TYPES_FUNCTION_HPP