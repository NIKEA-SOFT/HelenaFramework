#ifndef HELENA_TYPES_UNIQUE_VECTOR_HPP
#define HELENA_TYPES_UNIQUE_VECTOR_HPP

#include <Helena/Dependencies/EnTT.hpp>
#include <Helena/Debug/Assert.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Types/Hash.hpp>

#include <vector>
#include <unordered_map>
#include <optional>

namespace Helena::Types 
{
	template <typename T, auto UUID = []{}>
	class VectorUnique final
	{
		using unique_type	= decltype(UUID);
		using value_type	= T;
		using index_type	= std::size_t;
		using vec_type		= std::vector<std::optional<value_type>>;
		using map_type		= std::unordered_map<index_type, index_type>;

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

			HELENA_ASSERT(!m_Storage[index], "Instance of {} already exist in {}", Traits::NameOf<T>::value, Traits::NameOf<VectorUnique>::value);

			if(!m_Storage[index].has_value()) {
				m_Storage[index].emplace(std::forward<Args>(args)...);
				m_Size++;
			}
		}

		template <typename... T>
		[[nodiscard]] bool Has() const noexcept 
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
		[[nodiscard]] bool Any() const noexcept 
		{
			static_assert(sizeof...(T) > 1, "Exclusion-only Type are not supported");
			static_assert(((std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>) && ...), "Type cannot be const/ptr/ref");

			return (Has<T>() || ...);
		}

		template <typename... T>
		[[nodiscard]] decltype(auto) Get() const noexcept
		{
			static_assert(sizeof...(T) > 0, "Type pack is empty!");
			static_assert(((std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>) && ...), "Type cannot be const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);

				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Instance of {} not exist in {}", 
					Traits::NameOf<T...>::value, Traits::NameOf<VectorUnique>::value);

				return m_Storage[index].value();
			} else {
				return std::forward_as_tuple(Get<T>()...);
			}
		}

		template <typename Callback>
		void Each(Callback func, bool reset = false) noexcept 
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

		template <typename... T>
		void Remove() noexcept 
		{
			static_assert(sizeof...(T) > 0, "Type pack is empty!");
			static_assert(((std::is_same_v<T, std::remove_cvref_t<T>> && !std::is_pointer_v<T>) && ...), "Type cannot be const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = Indexer<unique_type, T...>::GetIndex(m_Indexes);

				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Instance of {} not exist in {}", 
					Traits::NameOf<T...>::value, Traits::NameOf<VectorUnique>::value);

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

		mutable map_type m_Indexes;	// Used for storage T indexes (for support dll/so across boundary)
		vec_type m_Storage;			// Used for storage T elements
		std::size_t m_Size;			// Used size
	};
}

#endif // HELENA_TYPES_UNIQUE_VECTOR_HPP