#ifndef HELENA_TYPES_VECTORANY_HPP
#define HELENA_TYPES_VECTORANY_HPP

#include <Helena/Dependencies/EnTT.hpp>
#include <Helena/Debug/Assert.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Types/Hash.hpp>

#include <vector>
#include <unordered_map>

namespace Helena::Types 
{
	template <std::size_t Capacity, auto UUID = []{}>
	class VectorAny final
	{
		static constexpr auto Length = Traits::PowerOf2<Capacity>::value;

	public:
		using unique_type	= decltype(UUID);
		using index_type	= std::size_t;
		using any_type		= entt::basic_any<Length, alignof(typename std::aligned_storage_t<Length + !Length>)>;
		using vec_type		= std::vector<any_type>;
		using map_type		= std::unordered_map<index_type, index_type>;

	private:
		[[nodiscard]] static auto GetIndexByKey(map_type& map, const index_type key) {
			const auto [it, result] = map.try_emplace(key, map.size());
			return it->second;
		}

	public:
		template <typename T, typename... Args>
		void Create([[maybe_unused]] Args&&... args)
		{
			static_assert(std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>, "Type cannot be const/ptr/ref");
			//static_assert(std::is_constructible_v<T, Args...>, "Type cannot be constructable from args");

			const auto index = Indexer<unique_type, T>::GetIndex(m_Indexes);
			if(index >= m_Storage.size()) {
				m_Storage.resize(index + 1u);
			}

			HELENA_ASSERT(!m_Storage[index], "Instance of {} already exist in {}", Traits::NameOf<T>::value, Traits::NameOf<VectorAny>::value);
			m_Storage[index].template emplace<T>(std::forward<Args>(args)...);
		}

		template <typename... T>
		[[nodiscard]] bool Has() noexcept 
		{
			static_assert(sizeof...(T) > 0, "Type pack is empty!");
			static_assert(((std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>) && ...), "Type cannot be const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);
				return index < m_Storage.size() && m_Storage[index];
			} else {
				return (Has<T>() && ...);
			}
		}

		template <typename... T>
		[[nodiscard]] bool Any() noexcept 
		{
			static_assert(sizeof...(T) > 1, "Exclusion-only Type are not supported");
			static_assert(((std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>) && ...), "Type cannot be const/ptr/ref");

			return (Has<T>() || ...);
		}

		template <typename... T>
		[[nodiscard]] decltype(auto) Get() noexcept
		{
			static_assert(sizeof...(T) > 0, "Type pack is empty!");
			static_assert(((std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>) && ...), "Type cannot be const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);

				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Instance of {} not exist in {}", 
					Traits::NameOf<T...>::value, Traits::NameOf<VectorAny>::value);
				HELENA_ASSERT(entt::any_cast<T...>(&m_Storage[index]), "Instance of {} type mismatch in {}", 
					Traits::NameOf<T...>::value, Traits::NameOf<VectorAny>::value);

				return entt::any_cast<T&...>(m_Storage[index]);
			} else {
				return std::forward_as_tuple(Get<T>()...);
			}
		}

		template <typename... T>
		void Remove() noexcept 
		{
			static_assert(sizeof...(T) > 0, "Type pack is empty!");
			static_assert(((std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>) && ...), "Type cannot be const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);

				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Instance of {} not exist in {}", 
					Traits::NameOf<T...>::value, Traits::NameOf<VectorAny>::value);

				m_Storage[index].reset();
			} else {
				(Remove<T>(), ...);
			}
		}

		void Clear() noexcept {
			m_Storage.clear();
		}

	private:
		template <typename Tag, typename T>
		struct Indexer {
			[[nodiscard]] static auto GetIndex(map_type& map) {
				// Get a name of type T and generate a hash to use as a key for a hash map
				static const auto index = GetIndexByKey(map, Hash::Get<T>());
				return index;
			}
		};

		map_type m_Indexes;			// Used for storage T indexes (for support dll/so across boundary)
		vec_type m_Storage;			// Used for storage T elements
	};
}

#endif // HELENA_TYPES_VECTORANY_HPP