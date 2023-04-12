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
        requires Traits::SameAs<T, Traits::RemoveCVRP<T>>
        void Create(Args&&... args)
        {
            const auto index = m_TypeIndexer.template Get<T>();
            if(index >= m_Storage.size()) {
                m_Storage.resize(index + 1);
            }

            HELENA_ASSERT(!m_Storage[index], "Type: {} already exist!", Traits::NameOf<T>{});
            m_Storage[index].template Create<T>(std::forward<Args>(args)...);
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...))
        [[nodiscard]] bool Has() const {
            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                return index < m_Storage.size() && m_Storage[index].template Equal<T...>();
            } else return (Has<T>() && ...);
        }

        template <typename... T>
        requires (Traits::Arguments<T...>::Size > 1 && (Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...))
        [[nodiscard]] bool Any() const {
            return (Has<T>() || ...);
        }

        template <typename T>
        requires Traits::SameAs<T, Traits::RemoveCVRP<T>>
        [[nodiscard]] bool Contain() const {
            const auto index = m_TypeIndexer.template Get<T>();
            return m_Storage[index].template Equal<T>();
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...))
        [[nodiscard]] decltype(auto) Get()
        {
            if constexpr(Traits::Arguments<T...>::Single && Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>{});
                HELENA_ASSERT(m_Storage[index].template Equal<T...>(), "Type: {} type mismatch!", Traits::NameOf<T...>{});
                return m_Storage[index].template Ref<T...>();
            } else return std::forward_as_tuple(Get<T>()...);
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...))
        [[nodiscard]] decltype(auto) Get() const
        {
            if constexpr(Traits::Arguments<T...>::Single && Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>{});
                HELENA_ASSERT(m_Storage[index].template Equal<T...>(), "Type: {} type mismatch!", Traits::NameOf<T...>{});
                return m_Storage[index].template Ref<T...>();
            } else return std::forward_as_tuple(Get<T>()...);
        }

        template <typename T>
        requires Traits::SameAs<T, Traits::RemoveCVRP<T>>
        [[nodiscard]] decltype(auto) Ptr() {
            const auto index = m_TypeIndexer.template Get<T>();
            return index < m_Storage.size() ? m_Storage[index].template Ptr<T>() : nullptr;
        }

        template <typename T>
        requires Traits::SameAs<T, Traits::RemoveCVRP<T>>
        [[nodiscard]] decltype(auto) Ptr() const {
            const auto index = m_TypeIndexer.template Get<T>();
            return index < m_Storage.size() ? m_Storage[index].template Ptr<T>() : nullptr;
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (Traits::SameAs<T, Traits::RemoveCVRP<T>> && ...))
        void Remove()
        {
            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>{});
                m_Storage[index].Reset();
            } else (Remove<T>(), ...);
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

#endif // HELENA_TYPES_VECTORANY_HPP