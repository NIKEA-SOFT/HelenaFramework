#ifndef HELENA_TYPES_VECTORANY_HPP
#define HELENA_TYPES_VECTORANY_HPP

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
    class VectorAny final
    {
    public:
        VectorAny() : m_TypeIndexer{}, m_Storage{} {}
        ~VectorAny() = default;
        VectorAny(const VectorAny&) = delete;
        VectorAny(VectorAny&&) noexcept = delete;
        VectorAny& operator=(const VectorAny&) = delete;
        VectorAny& operator=(VectorAny&&) noexcept = delete;

        template <typename T, typename... Args>
        void Create(Args&&... args)
        {
            static_assert(Traits::SameAs<T, Traits::RemoveCVRP<T>>, "Type is const/ptr/ref");

            const auto index = m_TypeIndexer.template Get<T>();
            if(index >= m_Storage.size()) {
                m_Storage.resize(index + 1);
            }

            HELENA_ASSERT(!m_Storage[index], "Type: {} already exist!", Traits::NameOf<T>{});
            m_Storage[index].template Create<T>(std::forward<Args>(args)...);
        }

        template <typename... T>
        [[nodiscard]] bool Has() const
        {
            static_assert(!Traits::Arguments<T...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...), "Type is const/ptr/ref");

            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                return index < m_Storage.size() && m_Storage[index];
            } else {
                return (Has<T>() && ...);
            }
        }

        template <typename... T>
        [[nodiscard]] bool Any() const
        {
            static_assert(Traits::Arguments<T...>::Size > 1, "Exclusion-only Type are not supported");
            static_assert((Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...), "Type is const/ptr/ref");

            return (Has<T>() || ...);
        }

        template <typename... T>
        [[nodiscard]] decltype(auto) Get()
        {
            static_assert(!Traits::Arguments<T...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...), "Type is const/ptr/ref");

            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();

                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>{});
                HELENA_ASSERT(m_Storage[index].template Equal<T...>(), "Type: {} type mismatch!", Traits::NameOf<T...>{});

                return m_Storage[index].template Ref<T...>();
            } else {
                return std::forward_as_tuple(Get<T>()...);
            }
        }

        template <typename... T>
        [[nodiscard]] decltype(auto) Get() const
        {
            static_assert(!Traits::Arguments<T...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...), "Type is const/ptr/ref");

            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();

                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>{});
                HELENA_ASSERT(m_Storage[index].template Equal<T...>(), "Type: {} type mismatch!", Traits::NameOf<T...>{});

                return m_Storage[index].template Ref<T...>();
            } else {
                return std::forward_as_tuple(Get<T>()...);
            }
        }

        template <typename... T>
        void Remove()
        {
            static_assert(!Traits::Arguments<T...>::Orphan, "Pack is empty!");
            static_assert((Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...), "Type is const/ptr/ref");

            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>{});

                m_Storage[index].Reset();
            } else {
                (Remove<T>(), ...);
            }
        }

        void Clear() noexcept
        {
            for(auto& value : m_Storage) {
                value.Reset();
            }
        }

    private:
        Types::UniqueIndexer<UniqueKey> m_TypeIndexer;
        std::vector<UndefinedContainer> m_Storage;
    };
}

#endif // HELENA_TYPES_VECTORANY_HPP