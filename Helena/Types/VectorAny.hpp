#ifndef HELENA_TYPES_VECTORANY_HPP
#define HELENA_TYPES_VECTORANY_HPP

#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/Constructible.hpp>
#include <Helena/Types/UniqueIndexer.hpp>

#include <vector>
#include <memory>

namespace Helena::Types
{
    template <typename UniqueKey>
    class VectorAny final
    {
    public:
        using UniquePointer = std::unique_ptr<void, void (*)(const void*)>;

        template <typename T>
        static constexpr bool AllowedParam = std::conjunction_v<
            std::is_same<std::remove_cvref_t<std::remove_pointer_t<T>>, T>,
            std::negation<std::is_same<T, void>>,
            std::negation<std::is_function<std::remove_pointer_t<T>>>,
            std::negation<std::is_member_function_pointer<T>>>;

        template <typename T, typename... Args>
        static constexpr bool RequiredParams = std::conjunction_v<
            std::bool_constant<AllowedParam<T>>,
            std::conditional_t<Traits::Arguments<Args...>::Orphan,
                std::is_default_constructible<std::decay_t<T>>,
                std::bool_constant<Traits::ConstructibleAggregateFrom<std::decay_t<T>, Args...>>>>;

    public:
        VectorAny() : m_TypeIndexer{}, m_Storage{} {}
        ~VectorAny() = default;
        VectorAny(const VectorAny&) = delete;
        VectorAny(VectorAny&&) noexcept = default;
        VectorAny& operator=(const VectorAny&) = delete;
        VectorAny& operator=(VectorAny&&) noexcept = default;

        template <typename T, typename... Args>
        requires RequiredParams<T, Args...>
        void Create(Args&&... args)
        {
            const auto index = m_TypeIndexer.template Get<T>();
            if(index >= m_Storage.size()) {
                m_Storage.emplace_back(nullptr, nullptr);
            }

            HELENA_ASSERT(!m_Storage[index], "Type: {} already exist!", Traits::NameOf<T>);

            // Clang doesn't support aggregate initialization
            T* instance{};
            if constexpr(std::is_aggregate_v<T>) {
                instance = new T{std::forward<Args>(args)...};
            } else {
                instance = new T(std::forward<Args>(args)...);
            }

            m_Storage[index] = UniquePointer(instance, +[](const void* ptr) {
                delete static_cast<const T*>(ptr);
            });
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (AllowedParam<T> && ...))
        [[nodiscard]] bool Has() const {
            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                return index < m_Storage.size() && m_Storage[index];
            } else return (Has<T>() && ...);
        }

        template <typename... T>
        requires (Traits::Arguments<T...>::Size > 1)
        [[nodiscard]] bool Any() const {
            return (Has<T>() || ...);
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (AllowedParam<T> && ...))
        [[nodiscard]] decltype(auto) Get()
        {
            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>);
                return (*static_cast<T*>(m_Storage[index].get()), ...);
            } else return std::forward_as_tuple(Get<T>()...);
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (AllowedParam<T> && ...))
        [[nodiscard]] decltype(auto) Get() const
        {
            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                HELENA_ASSERT(index < m_Storage.size() && m_Storage[index], "Type: {} not exist!", Traits::NameOf<T...>);
                return (*static_cast<const T*>(m_Storage[index].get()), ...);
            } else return std::forward_as_tuple(Get<T>()...);
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (AllowedParam<T> && ...))
        void Remove()
        {
            if constexpr(Traits::Arguments<T...>::Single) {
                const auto index = m_TypeIndexer.template Get<T...>();
                m_Storage[index].reset();
            } else (Remove<T>(), ...);
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

#endif // HELENA_TYPES_VECTORANY_HPP