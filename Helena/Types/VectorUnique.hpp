#ifndef HELENA_TYPES_VECTORUNIQUE_HPP
#define HELENA_TYPES_VECTORUNIQUE_HPP

#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAs.hpp>
#include <Helena/Types/UniqueIndexer.hpp>

#include <memory>

namespace Helena::Types
{
    template <typename UniqueKey, typename Type>
    requires Traits::SameAs<Type, Traits::RemoveCVRP<Type>>
    class VectorUnique final
    {
    public:
        VectorUnique() : m_TypeIndexer{}, m_Storage{} {}
        ~VectorUnique() = default;
        VectorUnique(const VectorUnique&) = delete;
        VectorUnique(VectorUnique&&) noexcept = delete;
        VectorUnique& operator=(const VectorUnique&) = delete;
        VectorUnique& operator=(VectorUnique&&) noexcept = delete;

        template <typename Key, typename... Args>
        requires Traits::SameAs<Key, Traits::RemoveCVRP<Key>>
        void Create(Args&&... args)
        {
            const auto index = m_TypeIndexer.template Get<Key>();
            if(index >= m_Storage.size()) {
                m_Storage.resize(index + 1u);
            }

            HELENA_ASSERT(!m_Storage[index], "Key: {} already exist!", Traits::NameOf<Key>);
            m_Storage[index] = std::make_unique<Type>(std::forward<Args>(args)...);
        }

        template <typename... Key>
        requires (!Traits::Arguments<Key...>::Orphan && (Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...))
        [[nodiscard]] bool Has() const
        {
            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                return index < m_Storage.size() && m_Storage[index];
            } else {
                return (Has<Key>() && ...);
            }
        }

        template <typename... Key>
        requires (Traits::Arguments<Key...>::Size > 1 && (Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...))
        [[nodiscard]] bool Any() const {
            return (Has<Key>() || ...);
        }

        template <typename... Key>
        requires (!Traits::Arguments<Key...>::Orphan && (Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...))
        [[nodiscard]] decltype(auto) Get()
        {
            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>);
                return *m_Storage[index];
            } else {
                return std::forward_as_tuple(Get<Key>()...);
            }
        }

        template <typename... Key>
        requires (!Traits::Arguments<Key...>::Orphan && (Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...))
        [[nodiscard]] decltype(auto) Get() const
        {
            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>);
                return *m_Storage[index];
            } else {
                return std::forward_as_tuple(Get<Key>()...);
            }
        }

        template <typename Key>
        requires Traits::SameAs<Key, Traits::RemoveCVRP<Key>>
        [[nodiscard]] Type* Ptr()
        {
            if(const auto index = m_TypeIndexer.template Get<Key>(); index < m_Storage.size()) [[likely]] {
                return m_Storage[index].get();
            }

            return nullptr;
        }

        template <typename Key>
        requires Traits::SameAs<Key, Traits::RemoveCVRP<Key>>
        [[nodiscard]] const Type* Ptr() const
        {
            if(const auto index = m_TypeIndexer.template Get<Key>(); index < m_Storage.size()) [[likely]] {
                return m_Storage[index].get();
            }

            return nullptr;
        }

        template <typename... Key>
        requires (!Traits::Arguments<Key...>::Orphan && (Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...))
        void Remove()
        {
            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>);
                m_Storage[index].reset();
            } else (Remove<Key>(), ...);
        }

        void Clear() noexcept
        {
            for(auto& value : m_Storage) {
                value.reset();
            }
        }

    private:
        UniqueIndexer<UniqueKey> m_TypeIndexer;
        std::vector<std::unique_ptr<Type>> m_Storage;
    };
}

#endif // HELENA_TYPES_VECTORUNIQUE_HPP