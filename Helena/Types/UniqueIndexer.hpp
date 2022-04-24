#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Traits/CVRefPtr.hpp>
#include <Helena/Types/Hash.hpp>

#include <vector>

namespace Helena::Types
{
    template <typename UniqueKey>
    class UniqueIndexer
    {
    public:
        using key_type      = typename Traits::FNV1a<std::uint64_t>::value_type;
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
            return index;
        }

        [[nodiscard]] std::size_t GetSize() const noexcept {
            return m_Indexes.size();
        }

    private:
        template <typename T>
        struct TypeIndexer
        {
            static constexpr auto m_Key = Types::Hash::template Get<T, std::uint64_t>();

            [[nodiscard]] static auto GetIndex(storage_type& storage) {
                // Get a name of type T and generate a hash to use as a key for a hash map
                const auto index = GetIndexByKey(storage, m_Key);
                HELENA_ASSERT(index < storage.size(), "UniqueIndexer with same UniqueKey should not be in multiple instances!");
                return index;
            }
        };

        [[nodiscard]] static auto GetIndexByKey(storage_type& storage, const index_type key) {

            for(std::size_t i = 0; i < storage.size(); ++i) 
            {
                if(storage[i] == key) {
                    return i;
                }
            }
            
            storage.emplace_back(key);
            return storage.size() - 1;
        }

        mutable storage_type m_Indexes;
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP
