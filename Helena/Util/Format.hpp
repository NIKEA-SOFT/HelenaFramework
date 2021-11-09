#ifndef HELENA_UTIL_FORMAT_HPP
#define HELENA_UTIL_FORMAT_HPP

#include <Helena/Dependencies/Fmt.hpp>
#include <Helena/Debug/Assert.hpp>

#include <string>

namespace Helena::Util
{
	template <typename... Args>
	[[nodiscard]] std::string Format(std::string_view msg, Args&&... args) noexcept 
	{
		try {
			if(!msg.empty()) {
				return fmt::format(msg, std::forward<Args>(args)...);
			}
		} catch(const fmt::format_error&) {
			HELENA_MSG_EXCEPTION(
				"\n----------------------------------------\n"
				"|| Error: format syntax invalid!\n"
				"|| Format: {}"
				"\n----------------------------------------", msg);
		} catch(const std::bad_alloc&) {
			HELENA_MSG_EXCEPTION(
				"\n----------------------------------------\n"
				"|| Error: not enough memory for alloc\n"
				"|| Format: {}"
				"\n----------------------------------------", msg);
		}

		return std::string{};
	}
}

#endif // HELENA_UTIL_FORMAT_HPP