#ifndef COMMON_UTIL_IPP
#define COMMON_UTIL_IPP

#include <thread>
#include <type_traits>

namespace Helena::Util
{
	namespace Internal {
	#if HF_STANDARD_VER == HF_STANDARD_CPP17
		template<class InputIt, class ForwardIt>
		[[nodiscard]] constexpr InputIt find_first_of(InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last) noexcept {
			for(; first != last; ++first) {
				for(ForwardIt it = s_first; it != s_last; ++it) {
					if(*first == *it) {
						return first;
					}
				}
			}
			return last;
		}
	#endif
	}

	template <typename... Args>
	void Console(const Internal::Location& location, const ELevelLog level, const std::string_view format, Args&&... args)
	{
		static constexpr const char* logName[] = {"Info ", "Warn ", "Error"};
		const auto buffer = fmt::format("[{:%Y.%m.%d %H:%M:%S}][{}:{}][{}] {}\n", 
			fmt::localtime(std::time(nullptr)), 
			location.m_Filename, 
			location.m_Line, 
			logName[std::underlying_type<ELevelLog>::type(level)], 
			fmt::format(format, args...));

		switch(level) 
		{
			case ELevelLog::Info: {
				fmt::print(fg(fmt::color::white), buffer);
			} break;

			case ELevelLog::Warn: {
				fmt::print(fg(fmt::color::yellow), buffer);
			} break;

			case ELevelLog::Error: {
				fmt::print(fg(fmt::color::crimson), buffer);
			} break;
		}
	}

	inline void Sleep(const uint64_t milliseconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	template <typename Type>
	[[nodiscard]] std::vector<Type> Split(std::string_view input, std::string_view delimeters, bool trim)
	{
		static_assert(std::is_same_v<Type, std::string> || std::is_same_v<Type, std::string_view>);
		std::vector<Type> v_splitted;
		for (auto first = input.data(), second = input.data(), last = first + input.size(); second != last && first != last; first = second + 1)
		{
			second = std::find_first_of(first, last, std::cbegin(delimeters), std::cend(delimeters));
			if (first != second) 
			{
				if(trim) {
					while(std::isspace(*first) && first != second) { 
						++first; 
					}

					if(first == second) { 
						continue;	
					}

					std::size_t offset = 1;
					while(std::isspace(*(second - offset)) && first != (second - offset)) { 
						++offset; 
					}

					if(first != (second - offset)) {
						v_splitted.emplace_back(first, second - first - (--offset));
					}
				} else {
					v_splitted.emplace_back(first, second - first);
				}
			}
		}
			
		return v_splitted;
	}


	[[nodiscard]] constexpr const char* GetFileName(const std::string_view file) {
		constexpr char symbols[]{'\\', '/'};
	#if HF_STANDARD_VER == HF_STANDARD_CPP17
		const auto it = Internal::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
	#else
		const auto it = std::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
	#endif
		return it == file.rend() ? file.data() : &(*std::prev(it));
	}

	template <typename TypeLeft, typename TypeRight>
	void BitAssign(TypeLeft& left, TypeRight right) {
		left = static_cast<TypeLeft>(right);
	}
		
	template <typename TypeLeft, typename TypeRight>
	void BitSet(TypeLeft& left, TypeRight right) {
		left |= static_cast<TypeLeft>(right);
	}

	template <typename TypeLeft, typename TypeRight>
	[[nodiscard]] bool BitGet(TypeLeft left, TypeRight right) {
		return left & static_cast<TypeLeft>(right);
	}

	template <typename TypeLeft, typename TypeRight>
	void BitRemove(TypeLeft& left, TypeRight right) {
		left &= ~(static_cast<TypeLeft>(right));
	}
		
	template <typename TypeLeft, typename TypeRight>
	void BitXor(TypeLeft& left, TypeRight right) {
		left ^= static_cast<TypeLeft>(right);
	}

	template <typename TypeLeft, typename TypeRight>
	[[nodiscard]] bool BitHas(TypeLeft left, TypeRight right) {
		return (left & static_cast<TypeLeft>(right)) == static_cast<TypeLeft>(right);
	}

	template <typename TypeLeft, typename TypeRight>
	[[nodiscard]] bool BitCompare(TypeLeft left, TypeRight right) {
		return left == static_cast<TypeLeft>(right);
	}
}

#endif	// COMMON_UTIL_IPP