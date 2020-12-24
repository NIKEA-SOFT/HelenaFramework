#ifndef COMMON_UTIL_HPP
#define COMMON_UTIL_HPP

namespace Helena::Util
{
	inline void Sleep(const uint64_t milliseconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}
}

#endif // COMMON_UTIL_HPP