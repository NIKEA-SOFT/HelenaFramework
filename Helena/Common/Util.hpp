#ifndef COMMON_UTIL_HPP
#define COMMON_UTIL_HPP

#include <vector>
#include <string_view>
#include <type_traits>
#include <algorithm>

#include "Platform.hpp"

#define HF_FILE_LINE		Util::GetFileName(__FILE__), __LINE__

namespace Helena
{
	namespace Internal {
	#if HF_STANDARD_VER == HF_STANDARD_CPP17
		template<class InputIt, class ForwardIt>
		[[nodiscard]] static constexpr InputIt find_first_of(InputIt first, InputIt last,
			ForwardIt s_first, ForwardIt s_last) noexcept
		{
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

	class Util final
	{
	public:
		/**
		 * @brief	Split string using delimeter (support trim) 
		 * 
		 * @tparam	Type		Type of vector value 
		 * @param	str			Input string data for split 
		 * @param	delims		Delimeter (default: ",") 
		 * @param	trim_space	Remove space from data 
		 * 
		 * @details	Split input data using delimiter and return std::vector<Type> 
		 * @note	Trim remove space char's from start and end position. 
		 *			Example: " Hello World " "Hello World" 
		 */
		template <typename Type, typename = std::enable_if_t<std::is_same_v<Type, std::string> || std::is_same_v<Type, std::string_view>>>
		[[nodiscard]] static std::vector<Type> Split(std::string_view str, std::string_view delims = ",", bool trim_space = true)
		{
			std::vector<Type> v_splitted;
			for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last; first = second + 1) 
			{
				second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));
				if (first != second) 
				{
					if(trim_space) {
						while(std::isspace(*first) && first != second) { ++first; }
						if(first == second) { continue;	}
						uint32_t offset = 1;
						while(std::isspace(*(second - offset)) && first != (second - offset)) { ++offset; }
						if(first != (second - offset)) v_splitted.emplace_back(first, second - first - (--offset));
					} else {
						v_splitted.emplace_back(first, second - first);
					}
				}
			}
			
			return v_splitted;
		}

		/**
		 * @brief	Return cutted filename in constexpr
		 * 
		 * @param	file	Macros of __FILE__
		 * 
		 * @return	@code{.cpp} const char* @endocde
		 */
		[[nodiscard]] static constexpr const char* GetFileName(const std::string_view file) {
			constexpr char symbols[]{'\\', '/'};
		#if HF_STANDARD_VER == HF_STANDARD_CPP17
			const auto it = Internal::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
		#else
			const auto it = std::find_first_of(file.rbegin(), file.rend(), std::begin(symbols), std::end(symbols));
		#endif
			return it == file.rend() ? file.data() : &(*std::prev(it));
		}

		//------------[Bit Operations]---------------//
		/**
		 * @brief	Assign bit flags from right to left side 
		 *
		 * @tparam	TypeLeft	Type of data with flags 
		 * @tparam	TypeRight	Type of bit flag 
		 * @param	left		Data with flags 
		 * @param	right		Bit flag 
		 *
		 * @note	This function assigns new flags to <left>. 
		 *			It's remove old data from <left> variable. 
		 *			Support multiple flags in <right>. 
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitAssign(TypeLeft& left, TypeRight right) {
			left = static_cast<TypeLeft>(right);
		}
		
		/**
		 * @brief	Add bit flag from right to left side 
		 * 
		 * @tparam	TypeLeft	Type of data with flags 
		 * @tparam	TypeRight	Type of bit flag 
		 * @param	left		Data with flags 
		 * @param	right		Bit flag 
		 * 
		 * @note	Support multiple flags in <right>
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitSet(TypeLeft& left, TypeRight right) {
			left |= static_cast<TypeLeft>(right);
		}

		/**
		 * @brief	Get bit flag in left from right 
		 *
		 * @tparam	TypeLeft	Type of data with flags 
		 * @tparam	TypeRight	Type of bit flag 
		 * @param	left		Data with flags 
		 * @param	right		Bit flag 
		 *
		 * @return	Return false if right flag disabled 
		 * 
		 * @note	This function check a <left> on flag. 
		 *			Support only single flag in <right>. 
		 */
		template <typename TypeLeft, typename TypeRight>
		[[nodiscard]] static bool BitGet(TypeLeft left, TypeRight right) {
			return left & static_cast<TypeLeft>(right);
		}

		/**
		 * @brief	Remove bit flag from left using right 
		 * 
		 * @tparam	TypeLeft	Type of data with flags 
		 * @tparam	TypeRight	Type of bit flag  
		 * @param	left		Data with flags 
		 * @param	right		Bit flag 
		 * 
		 * @note	Support multiple flags in <right>. 
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitRemove(TypeLeft& left, TypeRight right) {
			left &= ~(static_cast<TypeLeft>(right));
		}
		
		/**
		 * @brief	Xor bit flag in left from right 
		 *
		 * @tparam	TypeLeft	Type of data with flags 
		 * @tparam	TypeRight	Type of bit flag 
		 * @param	left		Data with flags 
		 * @param	right		Bit flag 
		 * 
		 * @note	This function invert flags in <left>. \n 
		 *			Support multiple flags in <right>.
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitXor(TypeLeft& left, TypeRight right) {
			left ^= static_cast<TypeLeft>(right);
		}

		/**
		 * @brief	Check left on has multiple flags from right 
		 *
		 * @tparam	TypeLeft	Type of data with flags 
		 * @tparam	TypeRight	Type of bit flag 
		 * @param	left		Data with flags 
		 * @param	right		Bit flag 
		 * 
		 * @return	Returns false if at least one of the right flags is missing. 
		 *
		 * @note	Analog BitGet, support multiple flags in <right>. 
		 */
		template <typename TypeLeft, typename TypeRight>
		[[nodiscard]] static bool BitHas(TypeLeft left, TypeRight right) {
			return (left & static_cast<TypeLeft>(right)) == static_cast<TypeLeft>(right);
		}

		/**
		 * @brief	Compare left and right on equality 
		 *
		 * @tparam	TypeLeft	Type of data with flags 
		 * @tparam	TypeRight	Type of bit flag 
		 * @param	left		Data with flags 
		 * @param	right		Bit flag 
		 *
		 * @return	Returns false if at least one of the flags is missing. 
		 *
		 * @note	This code is identical to the following. 
		 *			@code{.cpp} 
		 *			return left == right; 
		 *			@endcode 
		 */ 
		template <typename TypeLeft, typename TypeRight>
		[[nodiscard]] static bool BitCompare(TypeLeft left, TypeRight right) {
			return left == static_cast<TypeLeft>(right);
		}

	};

}

#endif	// COMMON_UTIL_HPP