#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Types/Hash.hpp>

#include <algorithm>
#include <vector>

namespace Helena::Types
{
    template <typename UniqueKey>
    class UniqueIndexer
    {
    public:
        using Hash      = Hash<std::uint64_t>;
        using Storage   = std::vector<std::size_t>;

    public:
        UniqueIndexer() = default;
        ~UniqueIndexer() = default;
        UniqueIndexer(const UniqueIndexer&) = delete;
        UniqueIndexer(UniqueIndexer&&) noexcept = delete;
        UniqueIndexer& operator=(const UniqueIndexer&) = delete;
        UniqueIndexer& operator=(UniqueIndexer&&) noexcept = delete;

        template <typename T>
        [[nodiscard]] std::size_t Get() const {
            static const auto index = TypeIndexer<T>::GetIndex(m_Indexes);
            HELENA_ASSERT(index < m_Indexes.size(), "UniqueIndexer with same UniqueKey should not be in multiple instances!");
            return index;
        }

        [[nodiscard]] std::size_t Size() const noexcept {
            return m_Indexes.size();
        }

    private:
        template <typename T>
        struct TypeIndexer
        {
            [[nodiscard]] static std::size_t GetIndex(const Storage& storage)
            {
                if(const auto it = std::find(storage.cbegin(), storage.cend(), m_Key); it != storage.cend()) {
                    return std::distance(storage.cbegin(), it);
                }

                const_cast<Storage&>(storage).emplace_back(m_Key);
                return storage.size() - 1uLL;
            }

            static constexpr auto m_Key = Hash::template Get<T>();
        };

        Storage m_Indexes;
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP