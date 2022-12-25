#ifndef HELENA_TYPES_ALLOCATORS_HPP
#define HELENA_TYPES_ALLOCATORS_HPP

#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Platform.hpp>

#include <limits>
#include <memory>
#include <new>

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
            HELENA_MSG_MEMORY("Memory alloc: {} bytes, {} alignment!", bytes, alignment);
            return Allocate(bytes, alignment);
        }

        void FreeMemory(void* ptr, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            alignment = PowerOf2(alignment);
            HELENA_MSG_MEMORY("Memory free: {} bytes, {} alignment!", bytes, alignment);
            Free(ptr, bytes, alignment);
        }

        [[nodiscard]] bool CompareResource(const IMemoryResource& other) const noexcept {
            return Equal(other);
        }

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

        [[nodiscard]] bool operator==(const IMemoryResource& other) const noexcept {
            return this == &other && Equal(other);
        }

        [[nodiscard]] bool operator!=(const IMemoryResource& other) const noexcept {
            return !operator==(other);
        }

    private:
        virtual void* Allocate(std::size_t bytes, std::size_t alignment) = 0;
        virtual void Free(void* ptr, std::size_t bytes, std::size_t alignment) = 0;
        virtual bool Equal(const IMemoryResource& other) const noexcept = 0;
    };

    class DefaultAllocator : public IMemoryResource {
    public:
        using IMemoryResource::IMemoryResource;

    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                return ::operator new(bytes, std::align_val_t{alignment});
            }

            return ::operator new(bytes);
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) override
        {
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                return ::operator delete(ptr, std::align_val_t{alignment});
            }

            return ::operator delete(ptr);
        }

        bool Equal(const IMemoryResource& other) const noexcept override {
            return this == &other;
        }
    };


    template <typename T = std::byte>
    class IMemoryAllocator
    {
        template <typename>
        friend class IMemoryAllocator;

    public:
        using value_type = T;

        IMemoryAllocator() noexcept = default;
        IMemoryAllocator(IMemoryResource* const resource) noexcept : m_Resource{resource} {
            HELENA_ASSERT(resource != nullptr, "Resource pointer cannot be nullptr");
        }

        IMemoryAllocator(const IMemoryAllocator&) = default;

        template <typename Other>
        IMemoryAllocator(const IMemoryAllocator<Other>& allocator) noexcept : m_Resource{allocator.m_Resource} {
            HELENA_ASSERT(allocator.m_Resource, "Resource pointer cannot be nullptr");
        }

        IMemoryAllocator& operator=(const IMemoryAllocator&) = delete;

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
            std::allocator_traits<IMemoryAllocator>::destroy(*this, ptr);
            deallocate_object(ptr);
        }

        [[nodiscard]] IMemoryAllocator select_on_container_copy_construction() const noexcept {
            return {};
        }

        [[nodiscard]] IMemoryResource* resource() const noexcept {
            return m_Resource;
        }

        template <typename U>
        [[nodiscard]] IMemoryAllocator& operator==(const IMemoryAllocator<U>& other) const noexcept {
            HELENA_ASSERT(m_Resource, "Memory resource is nullptr");
            HELENA_ASSERT(other.m_Resource, "Memory resource is nullptr");
            return *m_Resource == *other.m_Resource;
        }

        template <typename U>
        [[nodiscard]] IMemoryAllocator& operator!=(const IMemoryAllocator<U>& other) const noexcept {
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
}
#endif // HELENA_TYPES_STACKALLOCATOR_HPP