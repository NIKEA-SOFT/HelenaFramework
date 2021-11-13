#ifndef HELENA_TYPES_VECTORKVANY_HPP
#define HELENA_TYPES_VECTORKVANY_HPP

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
	class VectorKVAny final
	{
		static constexpr auto Length = Traits::PowerOf2<Capacity>::value;

	public:
		using unique_type	= decltype(UUID);
		using index_type	= std::size_t;
		using any_type		= entt::basic_any<Length, alignof(typename std::aligned_storage_t<Length + !Length>)>;
		using vec_type		= std::vector<any_type>;
		using map_type		= std::unordered_map<index_type, index_type>;


		VectorKVAny() : m_Indexes{}, m_Storage{} {}
		~VectorKVAny() = default;
		VectorKVAny(const VectorKVAny&) = delete;
		VectorKVAny(VectorKVAny&&) noexcept = delete;
		VectorKVAny& operator=(const VectorKVAny&) = delete;
		VectorKVAny& operator=(VectorKVAny&&) noexcept = delete;

		template <typename Key, typename T, typename... Args>
		void Create(Args&&... args)
		{
			static_assert(std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>, "Type is const/ptr/ref");
			static_assert(std::is_same_v<T, Traits::RemoveCVRefPtr<T>>, "Type is const/ptr/ref");

			const auto index = Indexer<unique_type, Key>::GetIndex(m_Indexes);
			if(index >= m_Storage.size()) {
				m_Storage.resize(index + 1u);
			}

			HELENA_ASSERT(!m_Storage[index], "Key: {}, Type: {} already exist!", Traits::NameOf<Key>::value, Traits::NameOf<T>::value);
			m_Storage[index].template emplace<T>(std::forward<Args>(args)...);
		}

		template <typename... Key>
		[[nodiscard]] bool Has() noexcept 
		{
			static_assert(((std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(Key) == 1) {
				const auto index = Indexer<unique_type, Key...>::GetIndex(m_Indexes);
				return index < m_Storage.size() && m_Storage[index];
			} else {
				return (Has<Key>() && ...);
			}
		}

		template <typename... Key>
		[[nodiscard]] bool Any() noexcept 
		{
			static_assert(sizeof...(Key) > 1, "Exclusion-only Type are not supported");
			static_assert(((std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>) && ...), "Type is const/ptr/ref");

			return (Has<Key>() || ...);
		}

		template <typename Key, typename T>
		[[nodiscard]] decltype(auto) Get() noexcept
		{
			static_assert(std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>, "Type is const/ptr/ref");
			static_assert(std::is_same_v<T, Traits::RemoveCVRefPtr<T>>, "Type is const/ptr/ref");

			const auto index = Indexer<unique_type, Key...>::GetIndex(m_Indexes);

			HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {}, Type: {} not exist!", 
				Traits::NameOf<Key>::value, Traits::NameOf<T>::value);
			HELENA_ASSERT(entt::any_cast<T>(&m_Storage[index]), "Key: {}, Type: {} type mismatch!", 
				Traits::NameOf<Key>::value, Traits::NameOf<T>::value);

			return entt::any_cast<T&>(m_Storage[index]);
		}

		template <typename... Key>
		void Remove() 
		{
			static_assert(sizeof...(Key) > 0, "Pack is empty!");
			static_assert(((std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(Key) == 1) {
				const auto index = Indexer<unique_type, Key...>::GetIndex(m_Indexes);

				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key>::value);

				m_Storage[index].reset();
			} else {
				(Remove<Key>(), ...);
			}
		}

		void Clear() noexcept {
			m_Storage.clear();
		}

	private:
		template <typename Tag, typename Key>
		struct Indexer {
			[[nodiscard]] static auto GetIndex(map_type& map) {
				// Get a name of type T and generate a hash to use as a key for a hash map
				static const auto index = GetIndexByKey(map, Hash::Get<Key>());
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

#endif // HELENA_TYPES_VECTORKVANY_HPP