#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Types/Hash.hpp>
#include <Helena/Platform/Assert.hpp>

#include <algorithm>
#include <vector>

namespace Helena::Types
{
    template <typename UniqueKey>
    class UniqueIndexer final
    {
        using Hasher    = Hash<std::uint64_t>;
        using Storage   = std::vector<std::size_t>;

        template <typename T>
        static inline std::size_t m_TypeIndex{(std::numeric_limits<std::size_t>::max)()};

    public:
        UniqueIndexer() = default;
        ~UniqueIndexer() = default;
        UniqueIndexer(const UniqueIndexer&) = delete;
        UniqueIndexer(UniqueIndexer&&) noexcept = delete;
        UniqueIndexer& operator=(const UniqueIndexer&) = delete;
        UniqueIndexer& operator=(UniqueIndexer&&) noexcept = delete;

        template <typename T>
        [[nodiscard]] std::size_t Get() const {
            if(m_TypeIndex<T> == (std::numeric_limits<std::size_t>::max)()) [[unlikely]] {
                m_TypeIndex<T> = TypeIndexer<T>::GetIndex(m_Indexes);
                HELENA_ASSERT(m_TypeIndex<T> < m_Indexes->size(), "UniqueIndexer with same UniqueKey should not be in multiple instances!");
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
            [[nodiscard]] static std::size_t GetIndex(const std::unique_ptr<Storage>& storage)
            {
                if(const auto it = std::find(storage->cbegin(), storage->cend(), m_Key); it != storage->cend()) {
                    return std::distance(storage->cbegin(), it);
                }

                storage->emplace_back(m_Key);
                return storage->size() - 1uLL;
            }

            static constexpr auto m_Key = Hasher::template From<T>();
        };

        std::unique_ptr<Storage> m_Indexes{std::make_unique<Storage>()};
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP