#ifndef HELENA_TYPES_VECTORUNIQUE_HPP
#define HELENA_TYPES_VECTORUNIQUE_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Types/UniqueIndexer.hpp>
#include <Helena/Traits/NameOf.hpp>

#include <optional>

namespace Helena::Types 
{
	template <typename T, auto UUID = []{}>
	class VectorUnique final
	{
	public:
		VectorUnique() : m_TypeIndexer{}, m_Storage{}, m_Size{} {}
		~VectorUnique() = default;
		VectorUnique(const VectorUnique&) = delete;
		VectorUnique(VectorUnique&&) noexcept = delete;
		VectorUnique& operator=(const VectorUnique&) = delete;
		VectorUnique& operator=(VectorUnique&&) noexcept = delete;

		template <typename T, typename... Args>
		void Create(Args&&... args)
		{
			static_assert(std::is_same_v<T, Traits::RemoveCVRefPtr<T>>, "Type is const/ptr/ref");

			const auto index = m_TypeIndexer.Get<T>();
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
				const auto index = m_TypeIndexer.Get<T...>();
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
				const auto index = m_TypeIndexer.Get<T...>();
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
				if(auto& data = m_Storage[i]) 
				{
					func(data.value());

					if(reset) {
						m_Storage[i].reset();
						m_Size--;
					}
				}
			}
		}

		template <typename... T>
		void Remove() 
		{
			static_assert(sizeof...(T) > 0, "Pack is empty!");
			static_assert(((std::is_same_v<T, Traits::RemoveCVRefPtr<T>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(T) == 1) {
				const auto index = m_TypeIndexer.Get<T...>();
				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>::value);

				if(m_Storage[index].has_value()) {
					m_Storage[index].reset();
					m_Size--;
				}
			} else {
				(Remove<T>(), ...);
			}
		}

		[[nodiscard]] bool Empty() const noexcept {
			return !m_Size;
		}

		[[nodiscard]] std::size_t GetSize() const noexcept {
			return m_Size;
		}

		[[nodiscard]] std::size_t GetCapacity() const noexcept {
			return m_Storage.size();
		}

		void Clear() noexcept {
			m_Storage.clear();
			m_Size = 0;
		}

	private:
		Types::UniqueIndexer<UUID> m_TypeIndexer;
		std::vector<std::optional<T>> m_Storage;
		std::size_t m_Size;
	};
}

#endif // HELENA_TYPES_VECTORUNIQUE_HPP