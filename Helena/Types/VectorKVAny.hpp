#ifndef HELENA_TYPES_VECTORKVANY_HPP
#define HELENA_TYPES_VECTORKVANY_HPP

#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Types/UniqueIndexer.hpp>

#include <vector>
#include <memory>

namespace Helena::Types
{
    template <typename UniqueKey>
    class VectorKVAny final
    {
    public:
        using UniquePointer = std::unique_ptr<void, void (*)(const void*)>;

        template <typename T>
        static constexpr bool AllowedParam = std::conjunction_v<
            std::is_same<std::remove_cvref_t<std::remove_pointer_t<T>>, T>,
            std::negation<std::is_same<T, void>>,
            std::negation<std::is_function<std::remove_pointer_t<T>>>,
            std::negation<std::is_member_function_pointer<T>>>;

        template <typename Key, typename T, typename... Args>
        static constexpr bool RequiredParams = std::conjunction_v<
            std::bool_constant<AllowedParam<Key>>,
            std::bool_constant<AllowedParam<T>>,
            std::conditional_t<Traits::Arguments<Args...>::Orphan,
                std::is_default_constructible<std::decay_t<T>>,
                std::is_constructible<std::decay_t<T>, Args...>>>;

    public:
        VectorKVAny() : m_TypeIndexer{}, m_Storage{} {}
        ~VectorKVAny() = default;
        VectorKVAny(const VectorKVAny&) = delete;
        VectorKVAny(VectorKVAny&&) noexcept = default;
        VectorKVAny& operator=(const VectorKVAny&) = delete;
        VectorKVAny& operator=(VectorKVAny&&) noexcept = default;

        template <typename Key, typename T, typename... Args>
        requires RequiredParams<Key, T, Args...>
        void Create(Args&&... args)
        {
            const auto index = m_TypeIndexer.template Get<Key>();
            if(index >= m_Storage.size()) {
                m_Storage.emplace_back(nullptr, nullptr);
            }

            HELENA_ASSERT(!m_Storage[index], "Type: {} already exist!", Traits::NameOf<T>);
            m_Storage[index] = UniquePointer(new T(std::forward<Args>(args)...), +[](const void* ptr) {
                delete static_cast<const T*>(ptr);
            });
        }

        template <typename... Key>
        requires (!Traits::Arguments<Key...>::Orphan && (AllowedParam<Key> && ...))
        [[nodiscard]] bool Has() const {
            if constexpr(Traits::Arguments<Key...>::Single) {
                const auto index = m_TypeIndexer.template Get<Key...>();
                return index < m_Storage.size() && m_Storage[index];
            } else return (Has<Key>() && ...);
        }

        template <typename... Key>
        requires (Traits::Arguments<Key...>::Size > 1 && (AllowedParam<Key> && ...))
        [[nodiscard]] bool Any() const {
            return (Has<Key>() || ...);
        }

        template <typename Key, typename T>
        requires (AllowedParam<Key> && AllowedParam<T>)
        [[nodiscard]] decltype(auto) Get() {
            const auto index = m_TypeIndexer.template Get<Key>();
            HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T>);
            return *static_cast<T*>(m_Storage[index].get());
        }

        template <typename Key, typename T>
        requires (AllowedParam<Key> && AllowedParam<T>)
        [[nodiscard]] decltype(auto) Get() const {
            const auto index = m_TypeIndexer.template Get<Key>();
            HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T>);
            return *static_cast<const T*>(m_Storage[index].get());
        }

        template <typename... Key>
        requires (!Traits::Arguments<Key...>::Orphan && (AllowedParam<Key> && ...))
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
        std::vector<UniquePointer> m_Storage;
    };
}

#endif // HELENA_TYPES_VECTORKVANY_HPP