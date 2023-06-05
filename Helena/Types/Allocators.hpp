#ifndef HELENA_TYPES_ALLOCATORS_HPP
#define HELENA_TYPES_ALLOCATORS_HPP

#include <Helena/Traits/NameOf.hpp>
#include <Helena/Types/FixedBuffer.hpp>
#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Platform.hpp>

#include <algorithm>
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
            return Allocate(bytes, PowerOf2(alignment));
        }

        void FreeMemory(void* ptr, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) noexcept {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            Free(ptr, bytes, PowerOf2(alignment));
        }

        [[nodiscard]] bool CompareResource(const IMemoryResource& other) const noexcept {
            return Equal(other);
        }

        [[nodiscard]] bool operator==(const IMemoryResource& other) const noexcept {
            return Equal(other);
        }

        [[nodiscard]] bool operator!=(const IMemoryResource& other) const noexcept {
            return !operator==(other);
        }

    protected:
        [[nodiscard]] static constexpr bool IsPowerOf2(std::size_t alignment) noexcept {
            return alignment && !(alignment & (alignment - 1));
        }

        [[nodiscard]] static constexpr auto PowerOf2(std::size_t alignment) noexcept -> decltype(alignment)
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
        virtual void Free(void* ptr, std::size_t bytes, std::size_t alignment) = 0;
        virtual bool Equal(const IMemoryResource& other) const = 0;
    };

    /**
    * @brief DefaultAllocator
    * Standard memory allocator with alignment support.
    *
    * @code{.cpp}
    * Types::DefaultAllocator upstreamAllocator;
    * Types::StackAllocator<10 * sizeof(std::string), alignof(std::string)> allocator{&upstreamAllocator};
    * @endcode
    *
    * @note
    * In the example above, we add a DefaultAllocator as an upstream resource allocator,
    * when the StackAllocator runs out of memory, it will try to request memory from an upstream memory resource,
    * which in the case of DefaultAllocator will lead to memory allocation on the heap.
    */
    class DefaultAllocator : public IMemoryResource
    {
        template <FixedBuffer<64>, typename>
        friend class LoggingAllocator;

    public:
        using IMemoryResource::IMemoryResource;

        [[nodiscard]] static IMemoryResource* Get() noexcept;

    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override {
            return ::operator new(bytes, std::align_val_t{
                (std::max<std::size_t>)(alignment, __STDCPP_DEFAULT_NEW_ALIGNMENT__)});
        }

        void Free(void* ptr, [[maybe_unused]] std::size_t bytes, std::size_t alignment) override {
            ::operator delete(ptr, std::align_val_t{(std::max<std::size_t>)(alignment, __STDCPP_DEFAULT_NEW_ALIGNMENT__)});
        }

        bool Equal(const IMemoryResource& other) const override {
            return this == &other;
        }
    };

    /**
    * @brief NulledAllocator
    * Used as a memory usage threshold exceeded detector.
    * In general it just throws an exception when someone calls Allocate.
    *
    * @code{.cpp}
    * Types::NulledAllocator upstreamAllocator;
    * Types::StackAllocator<10 * sizeof(std::string), alignof(std::string)> allocator{&upstreamAllocator};
    * @endcode
    *
    * @note
    * In the example above, we add a NulledAllocator as an upstream resource allocator,
    * when the StackAllocator runs out of memory, it will try to request memory from an upstream memory resource,
    * which in the case of a NulledAllocator will result in a std::bad_alloc exception being thrown.
    */
    class NulledAllocator : public IMemoryResource
    {
    public:
        using IMemoryResource::IMemoryResource;

        [[nodiscard]] static IMemoryResource* Get() noexcept;

    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override {
            HELENA_MSG_FATAL("Allocate memory bytes: {}, alignment: {} failed, buffer overflowed!", bytes, alignment);
            throw std::bad_alloc{};
        }

        void Free(void*, std::size_t, std::size_t) override {}
        bool Equal(const IMemoryResource& other) const override {
            return this == &other;
        }
    };

    /**
    * @brief StackAllocator
    * This allocator is the linear type of allocators, except that the stack memory is only freed when the resource is destroyed.
    * This rule is valid only for the memory of the stack itself, if the stack memory has out of memory, then the
    * allocating and freeing memory are managed by the upstream memory resource.
    *
    * @tparam std::size_t Size of bytes
    * @tparam std::size_t Memory alignment
    *
    * @code{.cpp}
    * Types::StackAllocator<10 * sizeof(std::string), alignof(std::string)> allocator;
    * @endcode
    *
    * @note
    * You need to be careful when working with the stack allocator, do not forget that the
    * memory that is allocated on the stack will be freed when the scope is exited.
    */
    template <std::size_t Stack, std::size_t Alignment>
    class StackAllocator : public IMemoryResource {
        template <FixedBuffer<64>, typename>
        friend class LoggingAllocator;

    public:
        StackAllocator(IMemoryResource* upstreamResource = DefaultAllocator::Get()) noexcept
            : m_UpstreamResource{upstreamResource}, m_Capacity{Stack}, m_Empty{} {}
        ~StackAllocator() = default;
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

            HELENA_ASSERT(m_UpstreamResource, "Upstream resource is nullptr!");
            return m_UpstreamResource->AllocateMemory(bytes, alignment);
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) override
        {
            if(std::bit_cast<std::uintptr_t>(ptr) < std::bit_cast<std::uintptr_t>(std::addressof(m_Buffer)) ||
                std::bit_cast<std::uintptr_t>(ptr) >= std::bit_cast<std::uintptr_t>(m_Buffer + Stack)) {
                HELENA_ASSERT(m_UpstreamResource, "Upstream resource is nullptr!");
                m_UpstreamResource->FreeMemory(ptr, bytes, alignment);
            }
        }

        bool Equal(const IMemoryResource& other) const override {
            return this == &other;
        }

    private:
        IMemoryResource* m_UpstreamResource;
        std::size_t m_Capacity;

        union alignas(Alignment) {
            std::byte m_Empty;
            std::byte m_Buffer[Stack];
        };
    };

    /**
    * @brief LoggingAllocator (wrapper)
    * @tparam FixedBuffer<64> Name identifier for debugging
    * @tparam Allocator Type of friendly Allocator
    *
    * @code{.cpp}
    * Types::LoggingAllocator<"Entity Allocator", Types::DefaultAllocator> allocator;
    * @endcode
    *
    * @note
    * The architecture of this class is slightly different from the previous ones,
    * this class takes the type of the allocator class as a template argument,
    * and then inherits from it, we do not want to take up extra bytes of memory,
    * and even more so to accept the allocators that we want to track as an upstream memory resource.
    */
    template <FixedBuffer<64> NameIdentifier, typename Allocator>
    class LoggingAllocator : public Allocator
    {
        template <FixedBuffer<64>, typename>
        friend class DebuggingAllocator;

        // Checking for access to private fields.
        static_assert(requires(LoggingAllocator* allocator) {
            { static_cast<Allocator*>(allocator)->Allocate(0, 0) } -> std::same_as<void*>;
            { static_cast<Allocator*>(allocator)->Free(nullptr, 0, 0) } -> std::same_as<void>;
        },  "LoggingAllocator does not have access to the private method of the Allocator class!"
            "Make LoggingAllocator friendly to Allocator class");

        template <typename T>
        static constexpr auto NameOf = []{
            constexpr std::string_view name = Traits::NameOf<Allocator>{};
            constexpr auto pos = name.find_last_of(':');
            constexpr auto offset = pos * (pos != name.npos) + (pos != name.npos);
            return name.substr(offset);
        }();

    public:
        template <typename... Args>
        requires std::constructible_from<Allocator, Args...>
        LoggingAllocator(Args&&... args) : Allocator{std::forward<Args>(args)...} {}
        ~LoggingAllocator() = default;
        LoggingAllocator(const LoggingAllocator&) = default;
        LoggingAllocator(LoggingAllocator&&) noexcept = default;
        LoggingAllocator& operator=(const LoggingAllocator&) = default;
        LoggingAllocator& operator=(LoggingAllocator&&) noexcept = default;

        [[nodiscard]] static constexpr const char* Name() noexcept {
            return NameIdentifier;
        }

    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override {
            HELENA_MSG_MEMORY("[ID: {} | {}] Alloc bytes: {} | alignment: {}", NameIdentifier, NameOf<Allocator>, bytes, alignment);
            return Allocator::Allocate(bytes, alignment);
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) override {
            HELENA_MSG_MEMORY("[ID: {} | {}] Free bytes: {} | alignment: {}", NameIdentifier, NameOf<Allocator>, bytes, alignment);
            Allocator::Free(ptr, bytes, alignment);
        }
    };

    template <FixedBuffer<64> NameIdentifier, typename Allocator>
    class DebuggingAllocator : public LoggingAllocator<NameIdentifier, Allocator>
    {
        using UsedAllocator = LoggingAllocator<NameIdentifier, Allocator>;

    public:
        struct BlockInfo {
            void* m_Address;
            std::size_t m_Bytes;
            std::size_t m_Alignment;
        };

    public:
        template <typename... Args>
        requires std::constructible_from<UsedAllocator, Args...>
        DebuggingAllocator(Args&&... args) : UsedAllocator{std::forward<Args>(args)...}
            , m_MaxUsedBlocks{}
            , m_MaxAllocatedBytes{}
            , m_MaxUsedBytes{}
            , m_UsedBytes {} {}
        ~DebuggingAllocator() = default;
        DebuggingAllocator(const DebuggingAllocator&) = default;
        DebuggingAllocator(DebuggingAllocator&&) noexcept = default;
        DebuggingAllocator& operator=(const DebuggingAllocator&) = default;
        DebuggingAllocator& operator=(DebuggingAllocator&&) noexcept = default;

        [[nodiscard]] decltype(auto) Blocks() const noexcept {
            return m_Blocks;
        }

        [[nodiscard]] std::size_t MaxUsedBlocks() const noexcept {
            return m_MaxUsedBlocks;
        }

        [[nodiscard]] std::size_t MaxAllocatedBytes() const noexcept {
            return m_MaxAllocatedBytes;
        }

        [[nodiscard]] std::size_t MaxUsedBytes() const noexcept {
            return m_MaxUsedBytes;
        }

        [[nodiscard]] std::size_t UsedBytes() const noexcept {
            return m_UsedBytes;
        }

    private:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            const auto ptr = UsedAllocator::Allocate(bytes, alignment);
            const auto it = std::lower_bound(m_Blocks.begin(), m_Blocks.end(), ptr, [](const auto& block, const auto ptr) {
                return std::bit_cast<std::uintptr_t>(block.m_Address) < std::bit_cast<std::uintptr_t>(ptr);
            });

            if(bytes > m_MaxAllocatedBytes) {
                m_MaxAllocatedBytes = bytes;
            }

            m_UsedBytes += bytes;
            if(m_UsedBytes > m_MaxUsedBytes) {
                m_MaxUsedBytes = m_UsedBytes;
            }

            m_Blocks.insert(it, BlockInfo{.m_Address = ptr, .m_Bytes = bytes, .m_Alignment = alignment});
            if(m_Blocks.size() > m_MaxUsedBlocks) {
                m_MaxUsedBlocks = m_Blocks.size();
            }

            return ptr;
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) override
        {
            const auto it = BinarySearch(ptr);
            HELENA_ASSERT(it != m_Blocks.cend(), "[ID: {} | {}] Block address: {} not found in blocks!",
                NameIdentifier, UsedAllocator::template NameOf<Allocator>, ptr);
            if(it == m_Blocks.cend()) [[unlikely]] {
                HELENA_MSG_ERROR("[ID: {} | {}] Block address: {} not found in blocks!",
                    NameIdentifier, UsedAllocator::template NameOf<Allocator>, ptr);
                throw std::invalid_argument("Free memory failed: invalid pointer!");
            }

            HELENA_ASSERT(it->m_Bytes == bytes, "[ID: {} | {}] Block address: {} bytes ({}) != ({}) free bytes!",
                NameIdentifier, UsedAllocator::template NameOf<Allocator>, ptr, it->m_Bytes, bytes);
            if(it->m_Bytes != bytes) {
                HELENA_MSG_ERROR("[ID: {} | {}] Block address: {} bytes ({}) != ({}) free bytes!",
                    NameIdentifier, UsedAllocator::template NameOf<Allocator>, ptr, it->m_Bytes, bytes);
                throw std::invalid_argument("Free memory failed: size mismatch!");
            }

            HELENA_ASSERT(it->m_Alignment == alignment, "[ID: {} | {}] Block address: {} alignment ({}) != ({}) free alignment!",
                NameIdentifier, UsedAllocator::template NameOf<Allocator>, ptr, it->m_Alignment, alignment);
            if(it->m_Alignment != alignment) {
                HELENA_MSG_ERROR("[ID: {} | {}] Block address: {} alignment ({}) != ({}) free alignment!",
                    NameIdentifier, UsedAllocator::template NameOf<Allocator>, ptr, it->m_Alignment, alignment);
                throw std::invalid_argument("Free memory failed: alignment mismatch!");
            }

            m_Blocks.erase(it);
            m_UsedBytes -= bytes;
            UsedAllocator::Free(ptr, bytes, alignment);
        }

    private:
        auto BinarySearch(void* const ptr) const noexcept {
            return std::lower_bound(m_Blocks.begin(), m_Blocks.end(), ptr, [](const auto& block, const auto ptr) {
                return std::bit_cast<std::uintptr_t>(block.m_Address) < std::bit_cast<std::uintptr_t>(ptr);
            });
        }

    private:
        std::vector<BlockInfo> m_Blocks;
        std::size_t m_MaxUsedBlocks;
        std::size_t m_MaxAllocatedBytes;
        std::size_t m_MaxUsedBytes;
        std::size_t m_UsedBytes;
    };


    template <typename T = std::byte>
    class MemoryAllocator
    {
        template <typename>
        friend class MemoryAllocator;

    public:
        using value_type = T;

        MemoryAllocator(IMemoryResource* const resource = DefaultAllocator::Get()) noexcept : m_Resource{resource} {
            HELENA_ASSERT(resource != nullptr, "Resource pointer cannot be nullptr");
        }

        MemoryAllocator(const MemoryAllocator&) = default;

        template <typename Other>
        MemoryAllocator(const MemoryAllocator<Other>& allocator) noexcept : m_Resource{allocator.m_Resource} {
            HELENA_ASSERT(allocator.m_Resource, "Resource pointer cannot be nullptr");
        }

        MemoryAllocator& operator=(const MemoryAllocator&) = delete;

        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        #if defined(HELENA_PLATFORM_WIN) && defined(HELENA_COMPILER_MSVC)
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
        #if defined(HELENA_PLATFORM_WIN) && defined(HELENA_COMPILER_MSVC)
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
        #if defined(HELENA_PLATFORM_WIN) && defined(HELENA_COMPILER_MSVC)
        // The allocator declaration specifier can be applied to custom memory-allocation functions
        // to make the allocations visible via Event Tracing for Windows (ETW).
        // URL: https://learn.microsoft.com/en-us/cpp/cpp/allocator?view=msvc-170
        __declspec(allocator)
        #endif
        U* allocate_object(const std::size_t count = 1)
        {
            if(HasOverflow(count, sizeof(U))) [[unlikely]] {
                HELENA_MSG_EXCEPTION("The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<U>{}, count);
                HELENA_ASSERT(!HasOverflow(count, sizeof(U)), "The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<U>{}, count);
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
        #if defined(HELENA_PLATFORM_WIN) && defined(HELENA_COMPILER_MSVC)
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
        [[nodiscard]] static bool HasOverflow(const std::size_t count, const std::size_t size) noexcept {
            return size > 1 && (count > ((std::numeric_limits<std::size_t>::max)() / size));
        }

        [[nodiscard]] static std::size_t GetSizeOfBytes(const std::size_t count, const std::size_t size) noexcept {
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