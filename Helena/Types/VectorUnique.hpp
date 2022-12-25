#ifndef HELENA_TYPES_VECTORUNIQUE_HPP
#define HELENA_TYPES_VECTORUNIQUE_HPP

#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAs.hpp>
#include <Helena/Types/UniqueIndexer.hpp>

#include <optional>

namespace Helena::Types
{
    template <typename UniqueKey, typename Type>
    class VectorUnique final
    {
        static_assert(Traits::SameAs<Type, Traits::RemoveCVRP<Type>>, "Type is const/ptr/ref");

    public:
        VectorUnique() : m_TypeIndexer{}, m_Storage{}, m_Size{} {}
        ~VectorUnique() = default;
        VectorUnique(const VectorUnique&) = delete;
        VectorUnique(VectorUnique&&) noexcept = delete;
        VectorUnique& operator=(const VectorUnique&) = delete;
        VectorUnique& operator=(VectorUnique&&) noexcept = delete;

        template <typename Key, typename... Args>
        void Create(Args&&... args)
        {
            static_assert(Traits::SameAs<Key, Traits::RemoveCVRP<Key>>, "Key is const/ptr/ref");

            const auto index = m_TypeIndexer.template Get<Key>();
            if(index >= m_Storage.size()) {
                m_Storage.resize(index + 1u);
            }

            HELENA_ASSERT(!m_Storage[index], "Key: {} already exist!", Traits::NameOf<Key>{});
            if(!m_Storage[index]) {
                m_Storage[index] = std::make_unique<Type>(std::forward<Args>(args)...);
                m_Size++;
            }
        }

        template <typename... Key>
        [[nodiscard]] bool Has() const
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...), "Key is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                return index < m_Storage.size() && m_Storage[index];
            } else {
                return (Has<Key>() && ...);
            }
        }

        template <typename... Key>
        [[nodiscard]] bool Any() const
        {
            static_assert(Traits::Arguments<Key...>::Size > 1, "Exclusion-only Type are not supported");
            static_assert((Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...), "Key is const/ptr/ref");
            return (Has<Key>() || ...);
        }

        template <typename... Key>
        [[nodiscard]] decltype(auto) Get()
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...), "Key is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>{});
                return *m_Storage[index];
            } else {
                return std::forward_as_tuple(Get<Key>()...);
            }
        }

        template <typename... Key>
        [[nodiscard]] decltype(auto) Get() const
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...), "Key is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>{});
                return *m_Storage[index];
            } else {
                return std::forward_as_tuple(Get<Key>()...);
            }
        }

        template <typename Key>
        [[nodiscard]] auto GetStorage() {
            static_assert(Traits::SameAs<Key, Traits::RemoveCVRP<Key>>, "Key is const/ptr/ref");
            const auto index = m_TypeIndexer.template Get<Key>();
            return index < m_Storage.size() && m_Storage[index] ? m_Storage[index].get() : nullptr;
        }

        template <typename... Key>
        void Remove()
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<Key, Traits::RemoveCVRP<Key>> && ...), "Key is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>{});
                if(m_Storage[index]) {
                    m_Storage[index]->reset();
                    m_Size--;
                }
            } else {
                (Remove<Key>(), ...);
            }
        }

        [[nodiscard]] bool Empty() const noexcept {
            return !m_Size;
        }

        [[nodiscard]] std::size_t Size() const noexcept {
            return m_Size;
        }

        [[nodiscard]] std::size_t Capacity() const noexcept {
            return m_Storage.size();
        }

        void Clear() noexcept
        {
            for(auto& ptr : m_Storage) {
                ptr.reset();
            }

            m_Size = 0;
        }

    private:
        Types::UniqueIndexer<UniqueKey> m_TypeIndexer;
        std::vector<std::unique_ptr<Type>> m_Storage;
        std::size_t m_Size;
    };
}

#endif // HELENA_TYPES_VECTORUNIQUE_HPP