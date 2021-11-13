#ifndef HELENA_TYPES_VECTORANY_HPP
#define HELENA_TYPES_VECTORANY_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Dependencies/EnTT.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Traits/CVRefPtr.hpp>
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


		VectorAny() : m_Indexes{}, m_Storage{} {}
		~VectorAny() = default;
		VectorAny(const VectorAny&) = delete;
		VectorAny(VectorAny&&) noexcept = delete;
		VectorAny& operator=(const VectorAny&) = delete;
		VectorAny& operator=(VectorAny&&) noexcept = delete;

		template <typename T, typename... Args>
		void Create(Args&&... args)
		{
			static_assert(std::is_same_v<T, Traits::RemoveCVRefPtr<T>>, "Type is const/ptr/ref");

			const auto index = Indexer<unique_type, T>::GetIndex(m_Indexes);
			if(index >= m_Storage.size()) {
				m_Storage.resize(index + 1u);
			}

			HELENA_ASSERT(!m_Storage[index], "Type: {} already exist!", Traits::NameOf<T>::value);
			m_Storage[index].template emplace<T>(std::forward<Args>(args)...);
		}

		template <typename... T>
		[[nodiscard]] bool Has() 
		{
			static_assert(sizeof...(T) > 0, "Pack is empty!");
			static_assert(((std::is_same_v<T, Traits::RemoveCVRefPtr<T>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);
				return index < m_Storage.size() && m_Storage[index];
			} else {
				return (Has<T>() && ...);
			}
		}

		template <typename... T>
		[[nodiscard]] bool Any() 
		{
			static_assert(sizeof...(T) > 1, "Exclusion-only Type are not supported");
			static_assert(((std::is_same_v<T, Traits::RemoveCVRefPtr<T>>) && ...), "Type is const/ptr/ref");

			return (Has<T>() || ...);
		}

		template <typename... T>
		[[nodiscard]] decltype(auto) Get()
		{
			static_assert(sizeof...(T) > 0, "Pack is empty!");
			static_assert(((std::is_same_v<T, Traits::RemoveCVRefPtr<T>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);

				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>::value);
				HELENA_ASSERT(entt::any_cast<T...>(&m_Storage[index]), "Type: {} type mismatch!", Traits::NameOf<T...>::value); 

				return entt::any_cast<T&...>(m_Storage[index]);
			} else {
				return std::forward_as_tuple(Get<T>()...);
			}
		}

		template <typename... T>
		void Remove() 
		{
			static_assert(sizeof...(T) > 0, "Pack is empty!");
			static_assert(((std::is_same_v<T, Traits::RemoveCVRefPtr<T>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);
				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>::value);

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

		[[nodiscard]] static auto GetIndexByKey(map_type& map, const index_type key) {
			const auto [it, result] = map.try_emplace(key, map.size());
			return it->second;
		}

		map_type m_Indexes;			// Used for storage T indexes (for support dll/so across boundary)
		vec_type m_Storage;			// Used for storage T elements
	};
}

#endif // HELENA_TYPES_VECTORANY_HPP