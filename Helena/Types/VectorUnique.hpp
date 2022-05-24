#ifndef HELENA_TYPES_VECTORUNIQUE_HPP
#define HELENA_TYPES_VECTORUNIQUE_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAS.hpp>
#include <Helena/Types/UniqueIndexer.hpp>

#include <cstddef>
#include <optional>
#include <vector>

namespace Helena::Types
{
    template <typename UniqueKey, typename Type>
    class VectorUnique final
    {
        static_assert(Traits::SameAS<Type, Traits::RemoveCVRP<Type>>, "Type is const/ptr/ref");

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
            static_assert(Traits::SameAS<Key, Traits::RemoveCVRP<Key>>, "Key is const/ptr/ref");

            const auto index = m_TypeIndexer.template Get<Key>();
            if(index >= m_Storage.size()) {
                m_Storage.resize(index + 1u);
            }

            HELENA_ASSERT(!m_Storage[index], "Key: {} already exist!", Traits::NameOf<Key>{});
            if(!m_Storage[index].has_value()) {
                m_Storage[index].emplace(std::forward<Args>(args)...);
                m_Size++;
            }
        }

        template <typename... Key>
        [[nodiscard]] bool Has() const
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Key is const/ptr/ref");

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
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Key is const/ptr/ref");
            return (Has<Key>() || ...);
        }

        template <typename... Key>
        [[nodiscard]] decltype(auto) Get()
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Key is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>{});
                return m_Storage[index].value();
            } else {
                return std::forward_as_tuple(Get<Key>()...);
            }
        }

        template <typename... Key>
        [[nodiscard]] decltype(auto) Get() const
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Key is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>{});
                return m_Storage[index].value();
            } else {
                return std::forward_as_tuple(Get<Key>()...);
            }
        }

        template <typename Callback>
        void Each(Callback func)
        {
            for(std::size_t i = 0; i < m_Storage.size(); ++i)
            {
                if(auto& data = m_Storage[i]) {
                    func(data.value());
                }
            }
        }

        template <typename... Key>
        void Remove()
        {
            static_assert(!Traits::Arguments<Key...>::Orphan, "Pack is empty!");
            static_assert(((Traits::SameAS<Key, Traits::RemoveCVRP<Key>>) && ...), "Key is const/ptr/ref");

            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Key: {} not exist!", Traits::NameOf<Key...>{});
                if(m_Storage[index].has_value()) {
                    m_Storage[index].reset();
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
            for(auto& opt : m_Storage) {
                opt.reset();
            }

            m_Size = 0;
        }

    private:
        Types::UniqueIndexer<UniqueKey> m_TypeIndexer;
        std::vector<std::optional<Type>> m_Storage;
        std::size_t m_Size;
    };
}

#endif // HELENA_TYPES_VECTORUNIQUE_HPP