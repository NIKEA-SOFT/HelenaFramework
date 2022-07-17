#ifndef HELENA_TRAITS_FNV1A_HPP
#define HELENA_TRAITS_FNV1A_HPP

#include <cstdint>

namespace Helena::Traits
{
	template <typename>
	struct FNV1a;

	template<>
	struct FNV1a<std::uint32_t> final {
	    using value_type = std::uint32_t;

	    static constexpr std::uint32_t Offset	= 2166136261u;
	    static constexpr std::uint32_t Prime	= 16777619u;
	};


	template<>
	struct FNV1a<std::uint64_t> final {
	    using value_type = std::uint64_t;

	    static constexpr std::uint64_t Offset	= 14695981039346656037ull;
	    static constexpr std::uint64_t Prime	= 1099511628211ull;
	};
}

#endif // HELENA_TRAITS_FNV1A_HPP