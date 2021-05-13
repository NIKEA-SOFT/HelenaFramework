#ifndef COMMON_UTIL_HPP
#define COMMON_UTIL_HPP

namespace Helena
{
	namespace Internal 
	{
		template <typename Type>
		constexpr auto type_name_t = entt::type_name<Type>().value();

		template <typename Type>
		struct remove_cvref {
			using type = std::remove_cv_t<std::remove_reference_t<Type>>;
		};

		template <typename Type>
		using remove_cvref_t = typename remove_cvref<Type>::type;

		template <typename Type>
		using remove_cvrefptr_t = std::conditional_t<std::is_array_v<Type>, std::remove_all_extents_t<Type>, 
			std::conditional_t<std::is_pointer_v<Type>, remove_cvref_t<std::remove_pointer_t<Type>>, remove_cvref_t<Type>>>;

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

		template <typename T, typename = void>
		struct is_pair : std::false_type {};
 
		template <typename T>
		struct is_pair<T, std::void_t<decltype(std::declval<typename T::first_type>()), decltype(std::declval<typename T::second_type>())>> : std::true_type {};
 
		template <typename T>
		constexpr bool is_pair_v = is_pair<T>::value;
 
		template<typename, typename = void>
		struct is_mapping : std::false_type {};
 
		template <typename Container>
		struct is_mapping<Container, std::enable_if_t<is_pair_v<typename std::iterator_traits<typename Container::iterator>::value_type>>> : std::true_type {};
 
		template <typename T>
		constexpr bool is_mapping_v = is_mapping<T>::value;

		template <typename T, bool B = std::is_enum<T>::value>
		struct is_scoped_enum : std::false_type {};

		template <typename T>
		struct is_scoped_enum<T, true> : std::integral_constant<bool, !std::is_convertible<T, typename std::underlying_type<T>::type>::value> {};

		template <typename T>
		constexpr bool is_scoped_enum_t = is_scoped_enum<T>::value;

		template <typename T> 
		struct is_integral_constant : std::false_type {};

		template <typename T, T V> 
		struct is_integral_constant<std::integral_constant<T, V>> : std::true_type {};

		template <typename T> 
		constexpr bool is_integral_constant_v = is_integral_constant<T>::value;
	}

	namespace Util 
	{
		template <
			typename Container, 
			typename Key = typename Container::key_type, 
			typename Ret = typename Container::mapped_type, 
			typename = std::enable_if_t<Internal::is_mapping_v<Internal::remove_cvref_t<Container>>, void>>
		auto AddOrGetTypeIndex(Container& container, const Key typeIndex) -> decltype(auto)
		{
			if(const auto it = container.find(typeIndex); it != container.cend()) {
				return static_cast<Ret>(it->second);
			}

			if(const auto [it, result] = container.emplace(typeIndex, container.size()); !result) {
				HF_MSG_FATAL("Type index emplace failed!");
				std::terminate();
			}

			return static_cast<Ret>(container.size()) - 1u;
		}

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