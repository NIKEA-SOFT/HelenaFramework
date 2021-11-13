#ifndef HELENA_TYPES_VECTORUNIQUE_HPP
#define HELENA_TYPES_VECTORUNIQUE_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Dependencies/EnTT.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/CVRefPtr.hpp>
#include <Helena/Types/Hash.hpp>

#include <vector>
#include <unordered_map>
#include <optional>

namespace Helena::Types 
{
	template <typename T, auto UUID = []{}>
	class VectorUnique final
	{
	public:
		using unique_type	= decltype(UUID);
		using value_type	= T;
		using index_type	= std::size_t;
		using vec_type		= std::vector<std::optional<value_type>>;
		using map_type		= std::unordered_map<index_type, index_type>;


		VectorUnique() : m_Indexes{}, m_Storage{}, m_Size{} {}
		~VectorUnique() = default;
		VectorUnique(const VectorUnique&) = delete;
		VectorUnique(VectorUnique&&) noexcept = delete;
		VectorUnique& operator=(const VectorUnique&) = delete;
		VectorUnique& operator=(VectorUnique&&) noexcept = delete;

		template <typename T, typename... Args>
		void Create(Args&&... args)
		{
			static_assert(std::is_same_v<T, Traits::RemoveCVRefPtr<T>>, "Type is const/ptr/ref");

			const auto index = Indexer<unique_type, T>::GetIndex(m_Indexes);
			if(index >= m_Storage.size()) {
				m_Storage.resize(index + 1u);
			}

			HELENA_ASSERT(!m_Storage[index], "Type: {} already exist!", Traits::NameOf<T>::value);
			if(!m_Storage[index].has_value()) {
				m_Storage[index].emplace(std::forward<Args>(args)...);
				m_Size++;
			}
		}

		template <typename... T>
		[[nodiscard]] bool Has() const 
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
		[[nodiscard]] bool Any() const 
		{
			static_assert(sizeof...(T) > 1, "Exclusion-only Type are not supported");
			static_assert(((std::is_same_v<T, Traits::RemoveCVRefPtr<T>>) && ...), "Type is const/ptr/ref");
			return (Has<T>() || ...);
		}

		template <typename... T>
		[[nodiscard]] decltype(auto) Get() const
		{
			static_assert(sizeof...(T) > 0, "Pack is empty!");
			static_assert(((std::is_same_v<T, Traits::RemoveCVRefPtr<T>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);
				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>::value);

				return m_Storage[index].value();
			} else {
				return std::forward_as_tuple(Get<T>()...);
			}
		}

		template <typename Callback>
		void Each(Callback func, bool reset = false) 
		{
			for(std::size_t i = 0; i < m_Storage.size(); ++i)
			{
				if(auto& data = m_Storage[i]; data.has_value()) 
				{
					func(data.value());

					if(reset) {
						data.reset();
					}
				}
			}
		}

		template <typename Callback>
		void RemoveEach(Callback&& func) 
		{
			for(std::size_t i = 0; i < m_Storage.size(); ++i)
			{
				if(auto& data = m_Storage[i]; data.has_value()) {
					func(data.value());
					data.reset();
				}
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

				if(m_Storage[index].has_value()) {
					m_Storage[index].reset();
					m_Size--;
				}
			} else {
				(Remove<T>(), ...);
			}
		}

		bool Empty() const noexcept {
			return !m_Size;
		}

		std::size_t GetSize() const noexcept {
			return m_Size;
		}

		std::size_t GetCapacity() const noexcept {
			return m_Storage.size();
		}

		void Clear() noexcept {
			m_Storage.clear();
			m_Size = 0;
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

		mutable map_type m_Indexes;	// Used for storage T indexes (for support dll/so across boundary)
		vec_type m_Storage;			// Used for storage T elements
		std::size_t m_Size;			// Used size
	};
}

#endif // HELENA_TYPES_VECTORUNIQUE_HPP