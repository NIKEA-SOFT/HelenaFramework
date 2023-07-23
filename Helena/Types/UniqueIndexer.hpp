#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Platform/Assert.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Types/Spinlock.hpp>

#include <algorithm>
#include <vector>

namespace Helena::Types
{
    template <typename UniqueKey>
    class UniqueIndexer final
    {
        using Hasher = Hash<std::uint64_t>;

        struct Container {
            std::vector<typename Hasher::value_type> m_Keys;
        };

        template <typename T>
        static inline std::size_t m_TypeIndex{(std::numeric_limits<std::size_t>::max)()};

    public:
        UniqueIndexer() = default;
        ~UniqueIndexer() = default;
        UniqueIndexer(const UniqueIndexer&) = delete;
        UniqueIndexer(UniqueIndexer&&) noexcept = default;
        UniqueIndexer& operator=(const UniqueIndexer&) = delete;
        UniqueIndexer& operator=(UniqueIndexer&&) noexcept = default;

        template <typename T>
        [[nodiscard]] HELENA_FORCEINLINE std::size_t Get() const {
            if(m_TypeIndex<T> == (std::numeric_limits<std::size_t>::max)()) [[unlikely]] {
                m_TypeIndex<T> = TypeIndexer<T>::GetIndex(m_Indexes);
                HELENA_ASSERT(m_TypeIndex<T> < m_Indexes->m_Keys.size(), "UniqueIndexer with same UniqueKey should not be in multiple instances!");
            }
            return m_TypeIndex<T>;
        }

        [[nodiscard]] std::size_t Size() const noexcept {
            return m_Indexes->m_Keys.size();
        }

    private:
        template <typename T>
        struct TypeIndexer
        {
            [[nodiscard]] HELENA_NOINLINE static std::size_t GetIndex(const std::unique_ptr<Container>& storage)
            {
                if(const auto it = std::find(storage->m_Keys.cbegin(), storage->m_Keys.cend(), m_Key); it != storage->m_Keys.cend()) {
                    return std::distance(storage->m_Keys.cbegin(), it);
                }

                storage->m_Keys.emplace_back(m_Key);
                return storage->m_Keys.size() - 1;
            }

            static constexpr auto m_Key = Hasher::template From<T>();
        };

        std::unique_ptr<Container> m_Indexes{std::make_unique<Container>()};
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP