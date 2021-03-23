#ifndef COMMON_UTIL_HPP
#define COMMON_UTIL_HPP

namespace Helena
{
	namespace Internal 
	{
		template <typename Default, typename AlwaysVoid, template<typename...> typename Op, typename... Args>
		struct detector {
			using value_t = std::false_type;
			using type = Default;
		};

		template <typename Default, template<typename...> typename Op, typename... Args>
		struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
			using value_t = std::true_type;
			using type = Op<Args...>;
		};

		struct nonesuch {
			nonesuch() = delete;
			~nonesuch() = delete;
			nonesuch(nonesuch const&) = delete;
			void operator=(nonesuch const&) = delete;
		};

		template <template<typename...> typename Op, typename... Args>
		using is_detected = typename Internal::detector<nonesuch, void, Op, Args...>::value_t;

		template <template<typename...> typename Op, typename... Args>
		constexpr bool is_detected_v = is_detected<Op, Args...>::value;

		template <template<typename...> typename Op, typename... Args>
		using detected_t = typename Internal::detector<nonesuch, void, Op, Args...>::type;

		template <typename Default, template<typename...> typename Op, typename... Args>
		using detected_or = Internal::detector<Default, void, Op, Args...>;
	}

	namespace Util 
	{
		inline void Sleep(const uint64_t milliseconds) {
			std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
		}

		template <typename Rep, typename Period>
		void Sleep(const std::chrono::duration<Rep, Period>& time) {
			std::this_thread::sleep_for(time);
		}
	}
}

#endif // COMMON_UTIL_HPP