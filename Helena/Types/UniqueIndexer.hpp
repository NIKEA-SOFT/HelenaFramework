#ifndef HELENA_TYPES_UNIQUEINDEXER_HPP
#define HELENA_TYPES_UNIQUEINDEXER_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Types/Hash.hpp>

#include <algorithm>
#include <vector>
#include <utility>

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
        ~UniqueIndexer() {
            delete m_Indexes;
        }

        UniqueIndexer(const UniqueIndexer&) = delete;
        UniqueIndexer(UniqueIndexer&& other) noexcept {
            m_Indexes = std::exchange(other.m_Indexes, nullptr);
        }

        UniqueIndexer& operator=(const UniqueIndexer&) = delete;
        UniqueIndexer& operator=(UniqueIndexer&& other) noexcept {
            delete m_Indexes;
            m_Indexes = std::exchange(other.m_Indexes, nullptr);
            return *this;
        }

        template <typename T>
        [[nodiscard]] HELENA_FORCEINLINE std::size_t Get() const noexcept
        {
            if(m_TypeIndex<T> == (std::numeric_limits<std::size_t>::max)()) [[unlikely]] {
                TypeIndexer<T>::CacheIndex(*m_Indexes);
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
            HELENA_NOINLINE static void CacheIndex(Container& indexes) noexcept
            {
                if(const auto it = std::find(indexes.cbegin(), indexes.cend(), m_Key); it != indexes.cend()) {
                    m_TypeIndex<T> = std::distance(indexes.cbegin(), it);
                } else {
                    indexes.emplace_back(m_Key);
                    m_TypeIndex<T> = indexes.size() - 1;
                }
            }

            static constexpr auto m_Key = Hasher::template From<T>();
        };

        Container* const m_Indexes{new (std::nothrow) Container()};
    };

}

#endif // HELENA_TYPES_UNIQUEINDEXER_HPP