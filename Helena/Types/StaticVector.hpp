#ifndef HELENA_TYPES_STATICVECTOR_HPP
#define HELENA_TYPES_STATICVECTOR_HPP

#include <algorithm>
#include <optional>

#include <Helena/Types/AlignedStorage.hpp>

namespace Helena::Types
{
    template <typename T, std::size_t _Capacity = 32>
    class StaticVector
    {
        static_assert(_Capacity, "Array<T, 0> not support!");
        static constexpr auto m_Capacity = _Capacity;

    public:
        using TValue            = T;
        using TPointer          = T*;
        using TPointerConst     = const T*;
        using TReference        = T&;
        using TReferenceConst   = const T&;

        using TSize             = std::size_t;
        using TDifference       = std::ptrdiff_t;

        using iterator          = T*;
        using const_iterator    = const T*;
        using reverse_iterator  = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    public:
        StaticVector() : m_Storage{}, m_Size{} {}

        template <typename... Args>
        requires (((Traits::Specialization<Args, std::tuple> && AlignedStorage::Constructible<T, Args>) && ...)
                 && Traits::Arguments<Args...>::Size <= m_Capacity)
        explicit StaticVector(std::piecewise_construct_t, Args&&... tuples) : m_Size{sizeof...(Args)} {
            AlignedStorage::Construct(m_Storage, std::piecewise_construct, std::forward<Args>(tuples)...);
        }

        template <typename... Args>
        requires std::constructible_from<T, Args...>
        explicit StaticVector(std::size_t count, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : m_Size{count} {
            HELENA_ASSERT(count);
            HELENA_ASSERT(count <= m_Capacity);
            for(std::size_t index = 0; index < count - 1; ++index) {
                AlignedStorage::Construct(m_Storage, index, std::as_const(args)...);
            }
            AlignedStorage::Construct(m_Storage, count - 1, std::forward<Args>(args)...);
        }

        StaticVector(const StaticVector& other) noexcept(
            std::is_nothrow_copy_constructible_v<T>) : m_Size{other.Size()} {
            AlignedStorage::ConstructCopy(other.m_Storage, m_Storage, 0, other.Size());
        }

        StaticVector(StaticVector&& other) noexcept(
            std::is_nothrow_move_constructible_v<T> &&
            std::is_nothrow_destructible_v<T>) : m_Size{other.Size()} {
            AlignedStorage::ConstructMove(other.m_Storage, m_Storage, 0, other.Size());
            other.Clear();
        }

        StaticVector& operator=(const StaticVector& other)
        {
            std::size_t copy = (std::min)(m_Size, other.Size());
            std::size_t left = (std::max)(m_Size, other.Size()) - copy;
            AlignedStorage::OperatorCopy(other.m_Storage, m_Storage, 0, copy);

            if(m_Size < other.Size()) {
                AlignedStorage::ConstructCopy(other.m_Storage, m_Storage, copy, left);
            } else if(m_Size > other.Size()) {
                AlignedStorage::Destruct(m_Storage, copy, left);
            }

            m_Size = other.Size();
            return *this;
        }

        StaticVector& operator=(StaticVector&& other) noexcept
        {
            std::size_t copy = (std::min)(m_Size, other.Size());
            std::size_t left = (std::max)(m_Size, other.Size()) - copy;
            AlignedStorage::OperatorMove(other.m_Storage, m_Storage, 0, copy);

            if(m_Size < other.Size()) {
                AlignedStorage::ConstructMove(other.m_Storage, m_Storage, copy, left);
            } else if(m_Size > other.Size()) {
                AlignedStorage::Destruct(m_Storage, copy, left);
            }

            m_Size = other.Size();
            other.Clear();
            return *this;
        }

        ~StaticVector() noexcept(std::is_nothrow_destructible_v<T>) {
            Clear();
        }

        [[nodiscard]] TReference At(std::size_t pos) noexcept {
            HELENA_ASSERT(pos < m_Size, "Out of bounds!");
            return AlignedStorage::Ref(m_Storage)[pos];
        }

        [[nodiscard]] TReferenceConst At(std::size_t pos) const noexcept {
            HELENA_ASSERT(pos < m_Size, "Out of bounds!");
            return AlignedStorage::Ref(m_Storage)[pos];
        }

        template <typename... Args>
        requires std::constructible_from<T, Args...>
        void PushBack(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            AlignedStorage::Construct(m_Storage, m_Size++, std::forward<Args>(args)...);
        }

        void PushBack(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            AlignedStorage::ConstructCopy(m_Storage, m_Size++, value);
        }

        void PushBack(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
            AlignedStorage::ConstructMove(m_Storage, m_Size++, std::move(value));
        }

        void PopBack() noexcept(std::is_nothrow_destructible_v<T>) {
            HELENA_ASSERT(m_Size, "Out of bounds!");
            AlignedStorage::Destruct(m_Storage, --m_Size);
        }

        auto Insert(const_iterator pos, const T& value) -> std::optional<T>
        {
            std::size_t index = pos - begin();
            HELENA_ASSERT(index < m_Capacity, "Out of bounds!");

            if(index == m_Size) {
                AlignedStorage::Construct(m_Storage, index, value);
                ++m_Size;
                return std::nullopt;
            }

            if(index > m_Size) {
                AlignedStorage::Construct(m_Storage, m_Size, index - m_Size);
                AlignedStorage::Construct(m_Storage, index, value);
                m_Size = index + 1;
                return std::nullopt;
            }

            std::size_t offset = m_Size - 1;
            std::optional<T> extracted;

            if(m_Size < m_Capacity) {
                AlignedStorage::Construct(m_Storage, m_Size, std::move(Back()));
                ++m_Size;
            } else {
                extracted = std::move(Back());
            }

            for(; offset != index; --offset) {
                At(offset) = std::move(At(offset - 1));
            }

            At(index) = value;
            return extracted;
        }

        auto Insert(const_iterator pos, T&& value) -> std::optional<T>
        {
            std::size_t index = pos - begin();
            HELENA_ASSERT(index < m_Capacity, "Out of bounds!");

            if(index == m_Size) {
                AlignedStorage::Construct(m_Storage, index, std::move(value));
                ++m_Size;
                return std::nullopt;
            }

            if(index > m_Size) {
                AlignedStorage::Construct(m_Storage, m_Size, index - m_Size);
                AlignedStorage::Construct(m_Storage, index, std::move(value));
                m_Size = index + 1;
                return std::nullopt;
            }

            std::size_t offset = m_Size - 1;
            std::optional<T> extracted;
            auto temp = std::move(value);

            if(m_Size < m_Capacity) {
                AlignedStorage::Construct(m_Storage, m_Size, std::move(Back()));
                ++m_Size;
            } else {
                extracted = std::move(Back());
            }

            for(; offset != index; --offset) {
                At(offset) = std::move(At(offset - 1));
            }

            At(index) = std::move(temp);
            return extracted;
        }

        void Swap(StaticVector& other) {
            HELENA_ASSERT(this != std::addressof(other));
            if(this != std::addressof(other)) {
                auto tmp = std::move(other);
                other = std::move(*this);
                *this = std::move(tmp);
            }
        }

        void Clear() noexcept(std::is_nothrow_destructible_v<T>) {
            AlignedStorage::Destruct(m_Storage, 0, m_Size);
            m_Size = 0;
        }

        void Resize(std::size_t size) requires std::is_default_constructible_v<T> {
            HELENA_ASSERT(size <= m_Capacity);
            if(size < m_Size) {
                AlignedStorage::Destruct(m_Storage, size, m_Size - size);
            } else if(size > m_Size) {
                AlignedStorage::Construct(m_Storage, m_Size, size - m_Size);
            }

            m_Size = size;
        }


        void Resize(std::size_t size, const T& value) requires std::is_copy_constructible_v<T> {
            HELENA_ASSERT(size <= m_Capacity);
            if(size < m_Size) {
                AlignedStorage::Destruct(m_Storage, size, m_Size - size);
            } else if(size > m_Size) {
                const std::size_t count = size - m_Size;
                for(std::size_t i = 0; i < count; ++i) {
                    AlignedStorage::Construct(m_Storage, m_Size + i, value);
                }
            }

            m_Size = size;
        }

        void Remove(std::size_t index) {
            HELENA_ASSERT(index < m_Size, "Out of bounds!");
            AlignedStorage::Destruct(m_Storage, index);
            AlignedStorage::Move(m_Storage, index + 1, index, m_Size - 1);
            --m_Size;
        }

        void Remove(std::size_t index, std::size_t size) {
            HELENA_ASSERT(index + size <= m_Size, "Out of bounds!");
            AlignedStorage::Destruct(m_Storage, index, size);
            AlignedStorage::Move(m_Storage, index + size, index, m_Size - size);
            m_Size -= size;
        }

        iterator Erase(const_iterator pos) {
            if(pos != end()) {
                std::uintptr_t index = pos - begin();
                Remove(index);
            }
            return iterator(pos);
        }

        iterator Erase(const_iterator first, const_iterator last)
        {
            if(first != last) {
                std::uintptr_t index = first - begin();
                std::uintptr_t size = last - first;
                HELENA_ASSERT(index + size <= m_Size, "Out of bounds!");
                AlignedStorage::Destruct(m_Storage, index, size);
                AlignedStorage::Move(m_Storage, index + size, index, m_Size - size);
                m_Size -= size;
            }
            return iterator(first);
        }

        [[nodiscard]] TReference Front() noexcept {
            HELENA_ASSERT(m_Size, "Container is empty!");
            return AlignedStorage::Ref(m_Storage)[0];
        }

        [[nodiscard]] TReferenceConst Front() const noexcept {
            HELENA_ASSERT(m_Size, "Container is empty!");
            return AlignedStorage::Ref(m_Storage)[0];
        }

        [[nodiscard]] TReference Back() noexcept {
            HELENA_ASSERT(m_Size, "Container is empty!");
            return AlignedStorage::Ref(m_Storage)[m_Size - 1];
        }

        [[nodiscard]] TReferenceConst Back() const noexcept {
            HELENA_ASSERT(m_Size, "Container is empty!");
            return AlignedStorage::Ref(m_Storage)[m_Size - 1];
        }

        [[nodiscard]] TPointer Data() noexcept {
            return AlignedStorage::Ref(m_Storage);
        }

        [[nodiscard]] TPointerConst Data() const noexcept {
            return AlignedStorage::Ref(m_Storage);
        }

        [[nodiscard]] TPointer Data(std::size_t fromIndex) noexcept {
            return AlignedStorage::Ref(m_Storage) + fromIndex;
        }

        [[nodiscard]] TPointerConst Data(std::size_t fromIndex) const noexcept {
            return AlignedStorage::Ref(m_Storage) + fromIndex;
        }

        [[nodiscard]] constexpr bool Empty() const noexcept {
            return !m_Size;
        }

        [[nodiscard]] constexpr bool Enough(std::size_t size) const noexcept {
            return size <= m_Capacity - m_Size;
        }

        [[nodiscard]] constexpr std::size_t Size() const noexcept {
            return m_Size;
        }

        [[nodiscard]] static constexpr std::size_t Capacity() noexcept {
            return m_Capacity;
        }

        [[nodiscard]] iterator begin() noexcept {
            return iterator(Data());
        }

        [[nodiscard]] const_iterator begin() const noexcept {
            return const_iterator(Data());
        }

        [[nodiscard]] iterator end() noexcept {
            return iterator(Data(m_Size));
        }

        [[nodiscard]] const_iterator end() const noexcept {
            return const_iterator(Data(m_Size));
        }

        [[nodiscard]] reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        [[nodiscard]] reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        [[nodiscard]] const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        [[nodiscard]] const_iterator cbegin() const noexcept {
            return begin();
        }

        [[nodiscard]] const_iterator cend() const noexcept {
            return end();
        }

        [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        [[nodiscard]] const_reverse_iterator crend() const noexcept {
            return rend();
        }

        [[nodiscard]] TReference operator[](std::size_t index) noexcept {
            HELENA_ASSERT(index < m_Size, "Out of bounds!");
            return AlignedStorage::Ref(m_Storage)[index];
        }

        [[nodiscard]] TReferenceConst operator[](std::size_t index) const noexcept {
            HELENA_ASSERT(index < m_Size, "Out of bounds!");
            return AlignedStorage::Ref(m_Storage)[index];
        }

    private:
        AlignedStorage::Storage<T[m_Capacity]> m_Storage;
        std::size_t m_Size;
    };
}

#endif // HELENA_TYPES_STATICVECTOR_HPP