#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Types/Hash.hpp>

#include <algorithm>
#include <vector>
#include <memory>

namespace Helena::Types
{
    template <typename UniqueKey>
    class UniqueIndexer
    {
        using Hasher = Hash<std::uint64_t>;
        using Container = std::vector<typename Hasher::value_type>;

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
        [[nodiscard]] HELENA_FORCEINLINE std::size_t Get() const noexcept
        {
            if(m_TypeIndex<T> == (std::numeric_limits<std::size_t>::max)()) [[unlikely]] {
                TypeIndexer<T>::CacheIndex(m_Indexes);
            }

            return m_TypeIndex<T>;
        }

        [[nodiscard]] std::size_t Size() const noexcept {
            return m_Indexes->size();
        }

    private:
        template <typename T>
        struct TypeIndexer
        {
            HELENA_NOINLINE static void CacheIndex(const std::unique_ptr<Container>& storage) noexcept
            {
                if(const auto it = std::find(storage->cbegin(), storage->cend(), m_Key); it != storage->cend()) {
                    m_TypeIndex<T> = std::distance(storage->cbegin(), it);
                } else {
                    storage->emplace_back(m_Key);
                    m_TypeIndex<T> = storage->size() - 1;
                }
            }

            static constexpr auto m_Key = Hasher::template From<T>();
        };

        std::unique_ptr<Container> m_Indexes{std::make_unique<Container>()};
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP