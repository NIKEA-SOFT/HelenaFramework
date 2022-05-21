#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Traits/CVRefPtr.hpp>
#include <Helena/Types/Hash.hpp>

#include <algorithm>
#include <vector>

namespace Helena::Types
{
    template <typename UniqueKey>
    class UniqueIndexer
    {
    public:
        using hash_type     = Hash<std::uint64_t>;
        using index_type    = std::size_t;
        using storage_type  = std::vector<index_type>;

    public:
        UniqueIndexer() = default;
        ~UniqueIndexer() = default;
        UniqueIndexer(const UniqueIndexer&) = delete;
        UniqueIndexer(UniqueIndexer&&) noexcept = delete;
        UniqueIndexer& operator=(const UniqueIndexer&) = delete;
        UniqueIndexer& operator=(UniqueIndexer&&) noexcept = delete;

        template <typename T>
        [[nodiscard]] index_type Get() const {
            static_assert(std::is_same_v<T, Traits::RemoveCVRefPtr<T>>, "Type is const/ptr/ref");
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
            [[nodiscard]] static std::size_t GetIndex(const storage_type& storage)
            {
                if(const auto it = std::find(storage.cbegin(), storage.cend(), m_Key); it != storage.cend()) {
                    return std::distance(storage.cbegin(), it);
                }

                const_cast<storage_type&>(storage).emplace_back(m_Key);
                return storage.size() - 1uLL;
            }

            static constexpr auto m_Key = hash_type::template Get<T>();
        };

        storage_type m_Indexes;
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP
