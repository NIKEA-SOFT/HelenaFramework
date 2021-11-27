#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Traits/CVRefPtr.hpp>
#include <Helena/Types/Hash.hpp>

#include <unordered_map>

namespace Helena::Types
{
    // UniqueIndexer cannot be used in storage where used multiple object with same type
    template <auto UUID = []{}>
    class UniqueIndexer
    {
    public:
        using key_type      = typename Traits::FNV1a<std::uint64_t>::value_type;
        using index_type    = std::size_t;
        using map_type      = std::unordered_map<key_type, index_type>;

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
            return TypeIndexer<T>::GetIndex(m_Indexes);
        }

    private:
        template <typename T>
        struct TypeIndexer
        {
            [[nodiscard]] static auto GetIndex(map_type& map) {
                // Get a name of type T and generate a hash to use as a key for a hash map
                static const auto index = GetIndexByKey(map, Hash::Get<T>());
                return index;
            }
        };

        [[nodiscard]] static auto GetIndexByKey(map_type& map, const index_type key) {
            const auto [it, result] = map.try_emplace(key, map.size());
            return it->second;
        }

        mutable map_type m_Indexes;
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP
