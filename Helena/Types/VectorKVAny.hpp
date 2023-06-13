#ifndef HELENA_TYPES_VECTORKVANY_HPP
#define HELENA_TYPES_VECTORKVANY_HPP

#include <Helena/Traits/Add.hpp>
#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAs.hpp>
#include <Helena/Types/UndefinedContainer.hpp>
#include <Helena/Types/UniqueIndexer.hpp>

namespace Helena::Types
{
    template <typename UniqueKey>
    class VectorKVAny final
    {
    public:
        VectorKVAny() : m_TypeIndexer{}, m_Storage{} {}
        ~VectorKVAny() = default;
        VectorKVAny(const VectorKVAny&) = delete;
        VectorKVAny(VectorKVAny&&) noexcept = delete;
        VectorKVAny& operator=(const VectorKVAny&) = delete;
        VectorKVAny& operator=(VectorKVAny&&) noexcept = delete;

        template <typename K, typename T, typename... Args>
        requires (Traits::SameAs<K, Traits::RemoveCVRP<K>> && Traits::SameAs<T, Traits::RemoveCVRP<T>>)
        void Create(Args&&... args)
        {
            const auto index = m_TypeIndexer.template Get<K>();
            if(index >= m_Storage.size()) {
                m_Storage.resize(index + 1);
            }

            HELENA_ASSERT(!m_Storage[index], "Type: {} already exist!", Traits::NameOf<T>);
            m_Storage[index].template Create<T>(std::forward<Args>(args)...);
        }

        template <typename... K>
        requires (!Traits::Arguments<K...>::Orphan && (Traits::SameAs<K, Traits::RemoveCVRP<K>> && ...))
        [[nodiscard]] bool Has() const {
            if constexpr(Traits::Arguments<K...>::Single) {
                const auto index = m_TypeIndexer.template Get<K...>();
                return index < m_Storage.size() && m_Storage[index];
            } else return (Has<K>() && ...);
        }

        template <typename... K>
        requires (Traits::Arguments<K...>::Size > 1 && (Traits::SameAs<K, Traits::RemoveCVRP<K>> && ...))
        [[nodiscard]] bool Any() const {
            return (Has<K>() || ...);
        }

        template <typename K, typename T>
        requires (Traits::SameAs<K, Traits::RemoveCVRP<K>> && Traits::SameAs<T, Traits::RemoveCVRP<T>>)
        [[nodiscard]] bool Contain() const {
            const auto index = m_TypeIndexer.template Get<K>();
            return index < m_Storage.size() && m_Storage[index].template Equal<T>();
        }

        template <typename K, typename T>
        requires (Traits::SameAs<K, Traits::RemoveCVRP<K>> && Traits::SameAs<T, Traits::RemoveCVRP<T>>)
        [[nodiscard]] decltype(auto) Get() {
            const auto index = m_TypeIndexer.template Get<K>();
            HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T>);
            HELENA_ASSERT(m_Storage[index].template Equal<T>(), "Type: {} type mismatch!", Traits::NameOf<T>);
            return m_Storage[index].template Ref<T>();
        }

        template <typename K, typename T>
        requires (Traits::SameAs<K, Traits::RemoveCVRP<K>> && Traits::SameAs<T, Traits::RemoveCVRP<T>>)
        [[nodiscard]] decltype(auto) Get() const {
            const auto index = m_TypeIndexer.template Get<K>();
            HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T>);
            HELENA_ASSERT(m_Storage[index].template Equal<T>(), "Type: {} type mismatch!", Traits::NameOf<T>);
            return m_Storage[index].template Ref<T>();
        }

        template <typename K, typename T>
        requires (Traits::SameAs<K, Traits::RemoveCVRP<K>> && Traits::SameAs<T, Traits::RemoveCVRP<T>>)
        [[nodiscard]] decltype(auto) Ptr() {
            const auto index = m_TypeIndexer.template Get<K>();
            return index < m_Storage.size() ? m_Storage[index].template Ptr<T>() : nullptr;
        }

        template <typename K, typename T>
        requires (Traits::SameAs<K, Traits::RemoveCVRP<K>> && Traits::SameAs<T, Traits::RemoveCVRP<T>>)
        [[nodiscard]] decltype(auto) Ptr() const {
            const auto index = m_TypeIndexer.template Get<K>();
            return index < m_Storage.size() ? m_Storage[index].template Ptr<T>() : nullptr;
        }

        template <typename... K>
        requires (!Traits::Arguments<K...>::Orphan && (Traits::SameAs<K, Traits::RemoveCVRP<K>> && ...))
        void Remove()
        {
            if constexpr(Traits::Arguments<K...>::Single) {
                const auto index = m_TypeIndexer.template Get<K...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<K...>);
                m_Storage[index].Reset();
            } else (Remove<K>(), ...);
        }

        void Clear() noexcept
        {
            for(auto& value : m_Storage) {
                value.Reset();
            }
        }

    private:
        UniqueIndexer<UniqueKey> m_TypeIndexer;
        std::vector<UndefinedContainer> m_Storage;
    };
}

#endif // HELENA_TYPES_VECTORKVANY_HPP