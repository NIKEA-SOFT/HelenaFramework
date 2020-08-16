#ifndef COMMON_HFUTIL_HPP
#define COMMON_HFUTIL_HPP

#include <vector>
#include <string_view>
#include <type_traits>
#include <algorithm>

namespace Helena
{
	class HFUtil final
	{
	public:
		/**
		 * @brief Split string using delimeter (support trim)
		 *
		 * @tparam	Type		Type of vector value
		 * @param	str			Input string data for split
		 * @param	delims		Delimeter (default: ",")
		 * @param	trim_space	Remove space from data
		 *
		 * @details
		 * Split input data using delimiter and return std::vector<Type>
		 *
		 * @note
		 * Trim remove space char's from start and end position \n
		 * Example: " Hello World " -> "Hello World"
		 */
		template <typename Type, typename = std::enable_if_t<std::is_same_v<Type, std::string> || std::is_same_v<Type, std::string_view>>>
		static std::vector<Type> Split(std::string_view str, std::string_view delims = ",", bool trim_space = true)
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

	#if HF_STANDARD_VER > HF_STANDARD_CPP17
		/**
		 * @brief Get current filename without path, only filename (used __FILE__ for split)
		 * @return const char*
		 */
		static constexpr const char* GetFileName() {
			constexpr std::string_view filename = __FILE__;
			constexpr char symbols[]{'\\', '/'};
			const auto it = std::find_first_of(filename.rbegin(), filename.rend(), std::begin(symbols), std::end(symbols));
			return it == filename.rend() ? filename.data() : &(*std::prev(it));
		}
	#endif
		//------------[Bit Operations]---------------//
		/**
		 * @brief Assign bit flags from right to left side
		 *
		 * @tparam	TypeLeft	Type of data with flags
		 * @tparam	TypeRight	Type of bit flag
		 * @param	left		Data with flags
		 * @param	right		Bit flag
		 *
		 * @note
		 * This function assigns new flags to <left>. \n
		 * It's remove old data from <left> variable. \n
		 * Support multiple flags in <right>.
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitAssign(TypeLeft& left, TypeRight right) {
			left = static_cast<TypeLeft>(right);
		}
		
		/**
		 * @brief Add bit flag from right to left side
		 *
		 * @tparam	TypeLeft	Type of data with flags
		 * @tparam	TypeRight	Type of bit flag
		 * @param	left		Data with flags
		 * @param	right		Bit flag
		 *
		 * @note
		 * Support multiple flags in <right>.
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitSet(TypeLeft& left, TypeRight right) {
			left |= static_cast<TypeLeft>(right);
		}

		/**
		 * @brief Get bit flag in left from right
		 *
		 * @tparam	TypeLeft	Type of data with flags
		 * @tparam	TypeRight	Type of bit flag
		 * @param	left		Data with flags
		 * @param	right		Bit flag
		 *
		 * @return
		 * Success: @code{.cpp} true
		 * @endcode
		 * Failure: @code{.cpp} false
		 * @endcode
		 * 
		 * @note
		 * This function check a <left> on flag. \n
		 * Support only single flag in <right>.
		 */
		template <typename TypeLeft, typename TypeRight>
		static bool BitGet(TypeLeft left, TypeRight right) {
			return left & static_cast<TypeLeft>(right);
		}

		/**
		 * @brief Remove bit flag from left using right
		 *
		 * @tparam	TypeLeft	Type of data with flags
		 * @tparam	TypeRight	Type of bit flag
		 * @param	left		Data with flags
		 * @param	right		Bit flag
		 *
		 * @note
		 * Support multiple flags in <right>.
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitRemove(TypeLeft& left, TypeRight right) {
			left &= ~(static_cast<TypeLeft>(right));
		}
		
		/**
		 * @brief Xor bit flag in left from right
		 *
		 * @tparam	TypeLeft	Type of data with flags
		 * @tparam	TypeRight	Type of bit flag
		 * @param	left		Data with flags
		 * @param	right		Bit flag
		 *
		 * @note
		 * This function invert flags in <left>. \n
		 * Support multiple flags in <right>.
		 */
		template <typename TypeLeft, typename TypeRight>
		static void BitXor(TypeLeft& left, TypeRight right) {
			left ^= static_cast<TypeLeft>(right);
		}

		/**
		 * @brief Check left on has multiple flags from right
		 *
		 * @tparam	TypeLeft	Type of data with flags
		 * @tparam	TypeRight	Type of bit flag
		 * @param	left		Data with flags
		 * @param	right		Bit flag
		 *
		 * @return
		 * Success: @code{.cpp} true
		 * @endcode
		 * Failure: @code{.cpp} false
		 * @endcode
		 * Returns false if at least one of the flags is missing.
		 *
		 * @note
		 * Analog BitGet, support multiple flags in <right>.
		 */
		template <typename TypeLeft, typename TypeRight>
		static bool BitHas(TypeLeft left, TypeRight right) {
			return (left & static_cast<TypeLeft>(right)) == static_cast<TypeLeft>(right);
		}

		/**
		 * @brief Compare left and right on equality
		 *
		 * @tparam	TypeLeft	Type of data with flags
		 * @tparam	TypeRight	Type of bit flag
		 * @param	left		Data with flags
		 * @param	right		Bit flag
		 *
		 * @return
		 * Success: @code{.cpp} true
		 * @endcode
		 * Failure: @code{.cpp} false
		 * @endcode
		 * Returns false if at least one of the flags is missing.
		 *
		 * @note
		 * This code is identical to the following.
		 * @code{.cpp}
		 * return left == right;
		 * @endcode
		 */	
		template <typename TypeLeft, typename TypeRight>
		static bool BitCompare(TypeLeft left, TypeRight right) {
			return left == static_cast<TypeLeft>(right);
		}
	};

}

#endif