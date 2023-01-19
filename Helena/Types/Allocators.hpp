#ifndef HELENA_TYPES_ALLOCATORS_HPP
#define HELENA_TYPES_ALLOCATORS_HPP

#include <Helena/Traits/NameOf.hpp>
#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Platform.hpp>

#include <bit>
#include <limits>
#include <memory>
#include <new>
#include <utility>

namespace Helena::Types
{
    class IMemoryResource
    {
        /* TODO:
            Align Pool | Block Pool | Block's
            [4]  -> [32]   -> [][][][][][]
                    [64]   -> [][][][][][]
                    [128]  -> [][][][][][]
                    [256]  -> [][][][][][]
            [8]  -> [32]   -> [][][][][][]
                    [64]   -> [][][][][][]
                    [128]  -> [][][][][][]
                    [256]  -> [][][][][][]
            [16] -> [32]   -> [][][][][][]
                    [64]   -> [][][][][][]
                    [128]  -> [][][][][][]
                    [256]  -> [][][][][][]
            [32] -> [32]   -> [][][][][][]
                    [64]   -> [][][][][][]
                    [128]  -> [][][][][][]
                    [256]  -> [][][][][][]
            [64] -> [32]   -> [][][][][][]
                    [64]   -> [][][][][][]
                    [128]  -> [][][][][][]
                    [256]  -> [][][][][][]
            [128]-> [32]   -> [][][][][][]
                    [64]   -> [][][][][][]
                    [128]  -> [][][][][][]
                    [256]  -> [][][][][][]

        Complexity:
            O(log(log(n)) + O(log(log(n)) + O(log(n))
            O(1) + O(1) + O(1)
        */

    public:
        IMemoryResource() = default;
        virtual ~IMemoryResource() noexcept = default;
        IMemoryResource(const IMemoryResource&) = default;
        IMemoryResource(IMemoryResource&&) noexcept = default;
        IMemoryResource& operator=(const IMemoryResource&) = default;
        IMemoryResource& operator=(IMemoryResource&&) noexcept = default;

        [[nodiscard("This function allocates raw memory. Ignoring the return result will leak memory.")]]
        void* AllocateMemory(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            alignment = PowerOf2(alignment);
            return Allocate(bytes, alignment);
        }

        void FreeMemory(void* ptr, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) noexcept {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            alignment = PowerOf2(alignment);
            Free(ptr, bytes, alignment);
        }

        [[nodiscard]] bool CompareResource(const IMemoryResource& other) const noexcept {
            return Equal(other);
        }

        [[nodiscard]] bool operator==(const IMemoryResource& other) const noexcept {
            return this == &other && Equal(other);
        }

        [[nodiscard]] bool operator!=(const IMemoryResource& other) const noexcept {
            return !operator==(other);
        }

    protected:
        [[nodiscard]] static bool IsPowerOf2(std::size_t alignment) noexcept {
            return alignment && !(alignment & (alignment - 1));
        }

        [[nodiscard]] static auto PowerOf2(std::size_t alignment) noexcept -> decltype(alignment)
        {
            if(IsPowerOf2(alignment)) {
                return alignment;
            }

            alignment |= alignment >> 1;
            alignment |= alignment >> 2;
            alignment |= alignment >> 4;
            alignment |= alignment >> 8;
            alignment |= alignment >> 16;
            return ++alignment;
        }

        [[nodiscard]] static void* Align(void* ptr, std::size_t& space, std::size_t size, std::size_t alignment) {
            const auto distance = AlignDistance(ptr, alignment);
            const auto success = space >= (distance + size);
            const auto result = std::bit_cast<void*>(std::bit_cast<std::uintptr_t>(ptr) * success + distance * success);
            space -= distance * success;
            return result;
        }

        [[nodiscard]] static void* AlignForward(void* ptr, std::size_t alignment) noexcept {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment requirements are not met.");
            return std::bit_cast<void*>((std::bit_cast<std::uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
        }

        [[nodiscard]] static std::size_t AlignDistance(void* ptr, void* alignedPtr) noexcept {
            return std::bit_cast<std::uintptr_t>(alignedPtr) - std::bit_cast<std::uintptr_t>(ptr);
        }

        [[nodiscard]] static std::size_t AlignDistance(void* ptr, std::size_t alignment) noexcept {
            return AlignDistance(ptr, AlignForward(ptr, alignment));
        }

    private:
        virtual void* Allocate(std::size_t bytes, std::size_t alignment) = 0;
        virtual void Free(void* ptr, std::size_t bytes, std::size_t alignment) noexcept = 0;
        virtual bool Equal(const IMemoryResource& other) const noexcept = 0;
    };

    class DefaultAllocator : public IMemoryResource {
    public:
        using IMemoryResource::IMemoryResource;

        [[nodiscard]] static IMemoryResource* Get() noexcept;

    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            HELENA_MSG_MEMORY("Allocate memory bytes: {}, alignment: {}", bytes, alignment);
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                return ::operator new(bytes, std::align_val_t{alignment});
            }

            return ::operator new(bytes);
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) noexcept override
        {
            HELENA_MSG_MEMORY("Free memory bytes: {}, alignment: {}", bytes, alignment);
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                return ::operator delete(ptr, std::align_val_t{alignment});
            }

            return ::operator delete(ptr);
        }

        bool Equal(const IMemoryResource& other) const noexcept override {
            return this == &other;
        }
    };

    class NulledAllocator : public IMemoryResource {
    public:
        using IMemoryResource::IMemoryResource;

        [[nodiscard]] static IMemoryResource* Get() noexcept;
    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override {
            HELENA_MSG_FATAL("Allocate memory bytes: {}, alignment: {} failed, buffer overflowed!", bytes, alignment);
            throw std::bad_alloc{};
            return nullptr;
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) noexcept override {}
        bool Equal(const IMemoryResource& other) const noexcept override {
            return this == &other;
        }
    };

    template <std::size_t Stack>
    class StackAllocator : public IMemoryResource {
    public:
        StackAllocator() = default;
        ~StackAllocator() = default;
        explicit StackAllocator(IMemoryResource* const upstreamResource) noexcept : m_UpstreamResource{upstreamResource} {}
        StackAllocator(const StackAllocator&) = default;
        StackAllocator(StackAllocator&&) noexcept = default;
        StackAllocator& operator=(const StackAllocator&) = default;
        StackAllocator& operator=(StackAllocator&&) noexcept = default;

        [[nodiscard]] IMemoryResource* UpstreamResource() const noexcept {
            return m_UpstreamResource;
        }

        [[nodiscard]] std::size_t Size() const noexcept {
            return Stack - m_Capacity;
        }

        [[nodiscard]] static constexpr std::size_t Capacity() noexcept {
            return Stack;
        }

    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment requirements are not met.");
            if(const auto address = Align(m_Buffer + Size(), m_Capacity, bytes, alignment)) {
                return m_Capacity -= bytes, address;
            }

            return m_UpstreamResource->AllocateMemory(bytes, alignment);
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) noexcept override
        {
            if(std::cmp_less(std::bit_cast<std::uintptr_t>(ptr), std::bit_cast<std::uintptr_t>(static_cast<void*>(m_Buffer))) ||
                std::cmp_greater_equal(std::bit_cast<std::uintptr_t>(ptr), std::bit_cast<std::uintptr_t>(m_Buffer + Stack))) {
                m_UpstreamResource->FreeMemory(ptr, bytes, alignment);
            }
        }

        bool Equal(const IMemoryResource& other) const noexcept override {
            return this == &other;
        }

    private:
        union {
            std::byte m_Empty{};
            std::byte m_Buffer[Stack];
        };
        std::size_t m_Capacity{Stack};
        IMemoryResource* m_UpstreamResource{DefaultAllocator::Get()};
    };

    template <typename T = std::byte>
    class MemoryAllocator
    {
        template <typename>
        friend class MemoryAllocator;

    public:
        using value_type = T;

        MemoryAllocator() noexcept = default;
        MemoryAllocator(IMemoryResource* const resource) noexcept : m_Resource{resource} {
            HELENA_ASSERT(resource != nullptr, "Resource pointer cannot be nullptr");
        }

        MemoryAllocator(const MemoryAllocator&) = default;

        template <typename Other>
        MemoryAllocator(const MemoryAllocator<Other>& allocator) noexcept : m_Resource{allocator.m_Resource} {
            HELENA_ASSERT(allocator.m_Resource, "Resource pointer cannot be nullptr");
        }

        MemoryAllocator& operator=(const MemoryAllocator&) = delete;

        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        #ifdef HELENA_PLATFORM_WIN
        // The allocator declaration specifier can be applied to custom memory-allocation functions
        // to make the allocations visible via Event Tracing for Windows (ETW).
        // URL: https://learn.microsoft.com/en-us/cpp/cpp/allocator?view=msvc-170
        __declspec(allocator)
        #endif
        T* allocate(const std::size_t count)
        {
            if(HasOverflow(count, sizeof(T))) [[unlikely]] {
                HELENA_MSG_EXCEPTION("The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<T>{}, count);
                HELENA_ASSERT(!HasOverflow(count, sizeof(T)), "The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<T>{}, count);
                return nullptr;
            }

            HELENA_ASSERT(m_Resource, "Memory resource is nullptr");
            return static_cast<T*>(m_Resource->AllocateMemory(GetSizeOfBytes(count, sizeof(T)), alignof(T)));
        }

        void deallocate(T* const ptr, const std::size_t count) noexcept {
            HELENA_ASSERT(m_Resource, "Memory resource is nullptr");
            m_Resource->FreeMemory(ptr, count * sizeof(T), alignof(T));
        }

        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        #ifdef HELENA_PLATFORM_WIN
        // The allocator declaration specifier can be applied to custom memory-allocation functions
        // to make the allocations visible via Event Tracing for Windows (ETW).
        // URL: https://learn.microsoft.com/en-us/cpp/cpp/allocator?view=msvc-170
        __declspec(allocator)
        #endif
        void* allocate_bytes(const std::size_t bytes, const std::size_t alignment = alignof(std::max_align_t)) {
            HELENA_ASSERT(m_Resource, "Memory resource is nullptr");
            return m_Resource->AllocateMemory(bytes, alignment);
        }

        void deallocate_bytes(void* const ptr, const std::size_t bytes, const std::size_t alignment = alignof(std::max_align_t)) noexcept {
            HELENA_ASSERT(m_Resource, "Memory resource is nullptr");
            m_Resource->FreeMemory(ptr, bytes, alignment);
        }

        template <typename U>
        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        #ifdef HELENA_PLATFORM_WIN
        // The allocator declaration specifier can be applied to custom memory-allocation functions
        // to make the allocations visible via Event Tracing for Windows (ETW).
        // URL: https://learn.microsoft.com/en-us/cpp/cpp/allocator?view=msvc-170
        __declspec(allocator)
        #endif
        U* allocate_object(const std::size_t count = 1)
        {
            if(HasOverflow(count, sizeof(U))) [[unlikely]] {
                HELENA_MSG_EXCEPTION("The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<U>{}, count);
                HELENA_ASSERT(!HasOverflow<sizeof(U)>(count), "The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<U>{}, count);
                return nullptr;
            }

            return static_cast<U*>(allocate_bytes(GetSizeOfBytes(count, sizeof(U)), alignof(U)));
        }

        template <typename U>
        void deallocate_object(U* const ptr, const std::size_t count = 1) noexcept {
            deallocate_bytes(ptr, count * sizeof(U), alignof(U));
        }

        template <typename U, typename... Args>
        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        #ifdef HELENA_PLATFORM_WIN
        // The allocator declaration specifier can be applied to custom memory-allocation functions
        // to make the allocations visible via Event Tracing for Windows (ETW).
        // URL: https://learn.microsoft.com/en-us/cpp/cpp/allocator?view=msvc-170
        __declspec(allocator)
        #endif
        U* new_object(Args&&... args)
        {
            U* const ptr = allocate_object<U>();

            try {
                std::uninitialized_construct_using_allocator(ptr, *this, std::forward<Args>(args)...);
            } catch(...) {
                deallocate_object(ptr);
                throw;
            }

            return ptr;
        }

        template <typename U>
        void delete_object(U* const ptr) noexcept {
            std::allocator_traits<MemoryAllocator>::destroy(*this, ptr);
            deallocate_object(ptr);
        }

        [[nodiscard]] MemoryAllocator select_on_container_copy_construction() const noexcept {
            return {};
        }

        [[nodiscard]] IMemoryResource* resource() const noexcept {
            return m_Resource;
        }

        template <typename U>
        [[nodiscard]] MemoryAllocator& operator==(const MemoryAllocator<U>& other) const noexcept {
            HELENA_ASSERT(m_Resource, "Memory resource is nullptr");
            HELENA_ASSERT(other.m_Resource, "Memory resource is nullptr");
            return *m_Resource == *other.m_Resource;
        }

        template <typename U>
        [[nodiscard]] MemoryAllocator& operator!=(const MemoryAllocator<U>& other) const noexcept {
            return !(*this == other);
        }

    private:
        [[nodiscard]] static bool HasOverflow(std::size_t count, std::size_t size) noexcept {
            return size > 1 && (count > ((std::numeric_limits<std::size_t>::max)() / size));
        }

        [[nodiscard]] static std::size_t GetSizeOfBytes(std::size_t count, std::size_t size) noexcept {
            return count * size;
        }

    private:
        IMemoryResource* m_Resource;
    };

    [[nodiscard]] inline IMemoryResource* DefaultAllocator::Get() noexcept {
        static constinit DefaultAllocator allocator{};
        return &allocator;
    }

    [[nodiscard]] inline IMemoryResource* NulledAllocator::Get() noexcept {
        static constinit NulledAllocator allocator{};
        return &allocator;
    }
}
#endif // HELENA_TYPES_STACKALLOCATOR_HPP