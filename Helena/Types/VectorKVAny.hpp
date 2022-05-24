#ifndef HELENA_TYPES_VECTORKVANY_HPP
#define HELENA_TYPES_VECTORKVANY_HPP

#include <Helena/Traits/Add.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAS.hpp>
#include <Helena/Types/Any.hpp>
#include <Helena/Types/UniqueIndexer.hpp>

namespace Helena::Types
{
    template <typename UniqueKey, std::size_t Capacity = sizeof(double)>
    class VectorKVAny final
    {
        using any_type = Types::Any<Capacity, alignof(typename std::aligned_storage_t<Capacity + !Capacity>)>;

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
            static_assert(Traits::SameAS<Key, Traits::RemoveCVRP<Key>>, "Type is const/ptr/ref");
            static_assert(Traits::SameAS<T, Traits::RemoveCVRP<T>>, "Type is const/ptr/ref");

            const auto index = m_TypeIndexer.template Get<Key>();
            if(index >= m_Storage.size()) {
                m_Storage.resize(index + 1u);
            }

            HELENA_ASSERT(!m_Storage[index], "Key: {}, Type: {} already exist!", Traits::NameOf<Key>{}, Traits::NameOf<T>{});
            m_Storage[index].template Create<T>(std::forward<Args>(args)...);
        }

        template <typename... Key>
        [[nodiscard]] bool Has() const
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Type is const/ptr/ref");

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
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Type is const/ptr/ref");

            return (Has<Key>() || ...);
        }

        template <typename Key, typename T>
        [[nodiscard]] decltype(auto) Get()
        {
            static_assert(Traits::SameAS<Key, Traits::RemoveCVRP<Key>>, "Type is const/ptr/ref");
            static_assert(Traits::SameAS<T, Traits::RemoveCVRP<T>>, "Type is const/ptr/ref");

            const auto index = m_TypeIndexer.template Get<Key>();

            HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {}, Type: {} not exist!",
                Traits::NameOf<Key>{}, Traits::NameOf<T>{});
            HELENA_ASSERT(AnyCast<T>(&m_Storage[index]), "Key: {}, Type: {} type mismatch!",
                Traits::NameOf<Key>{}, Traits::NameOf<T>{});

            return AnyCast<T&>(m_Storage[index]);
        }

        template <typename Key, typename T>
        [[nodiscard]] decltype(auto) Get() const
        {
            static_assert(Traits::SameAS<Key, Traits::RemoveCVRP<Key>>, "Type is const/ptr/ref");
            static_assert(Traits::SameAS<T, Traits::RemoveCVRP<T>>, "Type is const/ptr/ref");

            const auto index = m_TypeIndexer.template Get<Key>();

            HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {}, Type: {} not exist!",
                Traits::NameOf<Key>{}, Traits::NameOf<T>{});
            HELENA_ASSERT(AnyCast<Traits::AddConst<T>>(&m_Storage[index]), "Key: {}, Type: {} type mismatch!",
                Traits::NameOf<Key>{}, Traits::NameOf<T>{});

            return AnyCast<Traits::AddCRLV<T>>(m_Storage[index]);
        }

        template <typename... Key>
        void Remove()
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Type is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();

                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>{});

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