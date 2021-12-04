#ifndef HELENA_TYPES_VECTORKVANY_HPP
#define HELENA_TYPES_VECTORKVANY_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Types/Any.hpp>
#include <Helena/Types/UniqueIndexer.hpp>
#include <Helena/Traits/NameOf.hpp>

namespace Helena::Types 
{
	template <typename UniqueKey, std::size_t Capacity>
	class VectorKVAny final
	{
		using any_type = Any<Capacity, alignof(typename std::aligned_storage_t<Capacity + !Capacity>)>;

	public:
		VectorKVAny() : m_TypeIndexer{}, m_Storage{} {}
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

			const auto index = m_TypeIndexer.Get<Key>();
			if(index >= m_Storage.size()) {
				m_Storage.resize(index + 1u);
			}

			HELENA_ASSERT(!m_Storage[index], "Key: {}, Type: {} already exist!", Traits::NameOf<Key>::value, Traits::NameOf<T>::value);
			m_Storage[index].template Create<T>(std::forward<Args>(args)...);
		}

		template <typename... Key>
		[[nodiscard]] bool Has() const noexcept
		{
			static_assert(((std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(Key) == 1) {
				const auto index = m_TypeIndexer.Get<Key...>();
				return index < m_Storage.size() && m_Storage[index];
			} else {
				return (Has<Key>() && ...);
			}
		}

		template <typename... Key>
		[[nodiscard]] bool Any() const noexcept 
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

			const auto index = m_TypeIndexer.Get<Key>();

			HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {}, Type: {} not exist!", 
				Traits::NameOf<Key>::value, Traits::NameOf<T>::value);
			HELENA_ASSERT(AnyCast<T>(&m_Storage[index]), "Key: {}, Type: {} type mismatch!",
				Traits::NameOf<Key>::value, Traits::NameOf<T>::value);

			return AnyCast<T&>(m_Storage[index]);
		}

		template <typename Key, typename T>
		[[nodiscard]] decltype(auto) Get() const noexcept
		{
			static_assert(std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>, "Type is const/ptr/ref");
			static_assert(std::is_same_v<T, Traits::RemoveCVRefPtr<T>>, "Type is const/ptr/ref");

			const auto index = m_TypeIndexer.Get<Key>();

			HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {}, Type: {} not exist!",
				Traits::NameOf<Key>::value, Traits::NameOf<T>::value);
			HELENA_ASSERT(AnyCast<T>(&m_Storage[index]), "Key: {}, Type: {} type mismatch!",
				Traits::NameOf<Key>::value, Traits::NameOf<T>::value);

			return AnyCast<T&>(m_Storage[index]);
		}

		template <typename... Key>
		void Remove() 
		{
			static_assert(sizeof...(Key) > 0, "Pack is empty!");
			static_assert(((std::is_same_v<Key, Traits::RemoveCVRefPtr<Key>>) && ...), "Type is const/ptr/ref");

			if constexpr(sizeof...(Key) == 1) {
				const auto index = m_TypeIndexer.Get<Key>();

				HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key>::value);

				m_Storage[index].Reset();
			} else {
				(Remove<Key>(), ...);
			}
		}

		void Clear() noexcept 
		{
			for(auto& any : m_Storage) {
				any.Reset();
			}
		}

	private:
		Types::UniqueIndexer<UniqueKey> m_TypeIndexer;
		std::vector<any_type> m_Storage;
	};
}

#endif // HELENA_TYPES_VECTORKVANY_HPP