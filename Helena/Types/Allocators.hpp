#ifndef HELENA_TYPES_ALLOCATORS_HPP
#define HELENA_TYPES_ALLOCATORS_HPP

#include <Helena/Traits/NameOf.hpp>
#include <Helena/Types/FixedBuffer.hpp>
#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Platform.hpp>
#include <Helena/Util/Math.hpp>

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
    public:
        template <typename T>
        static constexpr auto NameOf = []{
            constexpr std::string_view name = Traits::NameOf<T>;
            constexpr auto pos = name.find_last_of(':');
            constexpr auto offset = pos * (pos != name.npos) + (pos != name.npos);
            return name.substr(offset);
        }();

    public:
        IMemoryResource() = default;
        virtual ~IMemoryResource() noexcept = default;
        IMemoryResource(const IMemoryResource&) = delete;
        IMemoryResource(IMemoryResource&&) noexcept = default;
        IMemoryResource& operator=(const IMemoryResource&) = delete;
        IMemoryResource& operator=(IMemoryResource&&) noexcept = default;

        [[nodiscard("This function allocates raw memory. Ignoring the return result will leak memory.")]]
        void* AllocateMemory(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
            HELENA_ASSERT(bytes, "Allocate a memory block of size: {} is impossible.", bytes);
            HELENA_ASSERT(Util::Math::IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            return Allocate(bytes, alignment);
        }

        void FreeMemory(void* ptr, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) noexcept {
            HELENA_ASSERT(Util::Math::IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            if(ptr) [[likely]] {
                Free(ptr, bytes, alignment);
            }
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
        [[nodiscard]] static constexpr void* Align(void* ptr, std::size_t& space, std::size_t size, std::size_t alignment) noexcept {
            const auto distance = AlignDistance(ptr, alignment);
            const auto success = space >= (distance + size);
            const auto result = std::bit_cast<void*>(std::bit_cast<std::uintptr_t>(ptr) * success + distance * success);
            return space -= distance * success, result;
        }

        [[nodiscard]] static constexpr void* AlignForward(void* ptr, std::size_t alignment) noexcept {
            HELENA_ASSERT(Util::Math::IsPowerOf2(alignment), "Alignment requirements are not met.");
            return std::bit_cast<void*>((std::bit_cast<std::uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
        }

        [[nodiscard]] static constexpr std::size_t AlignDistance(void* ptr, void* alignedPtr) noexcept {
            HELENA_ASSERT(alignedPtr >= ptr, "Aligned pointer cannot have an address less than ptr.")
            return std::bit_cast<std::uintptr_t>(alignedPtr) - std::bit_cast<std::uintptr_t>(ptr);
        }

        [[nodiscard]] static constexpr std::size_t AlignDistance(void* ptr, std::size_t alignment) noexcept {
            return AlignDistance(ptr, AlignForward(ptr, alignment));
        }

    protected:
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
        static DefaultAllocator Default;
        static inline IMemoryResource* m_Resource{};

    public:
        DefaultAllocator() = default;
        ~DefaultAllocator() noexcept = default;
        DefaultAllocator(const DefaultAllocator&) = delete;
        DefaultAllocator(DefaultAllocator&&) noexcept = delete;
        DefaultAllocator& operator=(const DefaultAllocator&) = delete;
        DefaultAllocator& operator=(DefaultAllocator&&) noexcept = delete;

        static void Set(IMemoryResource* resource) noexcept;
        [[nodiscard]] static IMemoryResource* Get() noexcept;

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
        #if defined(__cpp_aligned_new)
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                return ::operator new(bytes, std::align_val_t{alignment});
            }
        #endif

            return ::operator new(bytes);
        }

        void Free(void* ptr, [[maybe_unused]] std::size_t bytes, std::size_t alignment) override
        {
        #if defined(__cpp_aligned_new)
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
            #if defined(__cpp_sized_deallocation)
                return ::operator delete(ptr, bytes, std::align_val_t{alignment});
            #else
                return ::operator delete(ptr, std::align_val_t{alignment});
            #endif
            }
        #endif

        #if defined(__cpp_sized_deallocation)
            ::operator delete(ptr, bytes);
        #else
            ::operator delete(ptr);
        #endif
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
        static NulledAllocator Default;
        static inline IMemoryResource* m_Resource{};

    public:
        NulledAllocator() = default;
        ~NulledAllocator() noexcept = default;
        NulledAllocator(const NulledAllocator&) = delete;
        NulledAllocator(NulledAllocator&&) noexcept = delete;
        NulledAllocator& operator=(const NulledAllocator&) = delete;
        NulledAllocator& operator=(NulledAllocator&&) noexcept = delete;

        static void Set(IMemoryResource* resource) noexcept;
        [[nodiscard]] static IMemoryResource* Get() noexcept;

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override {
            HELENA_MSG_FATAL("Allocate memory bytes: {}, alignment: {} failed, buffer overflowed!", bytes, alignment);
            throw std::bad_alloc{};
        }

        void Free(void*, std::size_t, std::size_t) override {}
        bool Equal(const IMemoryResource& other) const override {
            return this == &other;
        }
    };

    template <typename T = std::byte>
    class MemoryAllocator
    {
        template <typename>
        friend class MemoryAllocator;

    public:
        using value_type = T;

    public:
        MemoryAllocator(IMemoryResource* const resource = DefaultAllocator::Get()) noexcept : m_Resource{resource} {
            HELENA_ASSERT(resource != nullptr, "Resource pointer cannot be nullptr");
        }

        ~MemoryAllocator() = default;
        MemoryAllocator(const MemoryAllocator&) = default;

        template <typename Other>
        MemoryAllocator(const MemoryAllocator<Other>& allocator) noexcept : m_Resource{allocator.m_Resource} {
            HELENA_ASSERT(allocator.m_Resource, "Resource pointer cannot be nullptr");
        }

        MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    public: // Common design compatible API (Recommended)
        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        T* Allocate(const std::size_t count) {
            return allocate(count);
        }

        void Free(T* const ptr, const std::size_t count) noexcept {
            deallocate(ptr, count);
        }

        template <typename U>
        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        U* AllocateObjects(const std::size_t count = 1) {
            return allocate_object<U>(count);
        }

        template <typename U>
        void FreeObjects(U* const ptr, const std::size_t count = 1) noexcept {
            deallocate_object(ptr, count);
        }

        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        void* AllocateBytes(const std::size_t bytes, const std::size_t alignment = alignof(std::max_align_t)) {
            return allocate_bytes(bytes, alignment);
        }

        void FreeBytes(void* const ptr, const std::size_t bytes, const std::size_t alignment = alignof(std::max_align_t)) noexcept {
            deallocate_bytes(ptr, bytes, alignment);
        }

        template <typename U, typename... Args>
        [[nodiscard("The function return a pointer to the allocated memory, ignoring it will lead to memory leaks.")]]
        U* CreateObject(Args&&... args) {
            return new_object<U>(std::forward<Args>(args)...);
        }

        template <typename U>
        void DeleteObject(U* const ptr) noexcept {
            delete_object(ptr);
        }

        template <typename U, typename... Args>
        void ConstructObject(U* const ptr, Args&&... args) {
            construct(ptr, std::forward<Args>(args)...);
        }

        template <typename U>
        void DestroyObject(U* const ptr) noexcept {
            destroy(ptr);
        }

        [[nodiscard]] IMemoryResource* MemoryResource() const noexcept {
            return m_Resource;
        }

    public: // Backwards compatible with STL
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
                HELENA_MSG_EXCEPTION("The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<T>, count);
                HELENA_ASSERT(!HasOverflow(count, sizeof(T)), "The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<T>, count);
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
                HELENA_MSG_EXCEPTION("The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<U>, count);
                HELENA_ASSERT(!HasOverflow(count, sizeof(U)), "The allocator has detected an overflow, type: {}, count: {}", Traits::NameOf<U>, count);
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
            destroy(ptr);
            deallocate_object(ptr);
        }

        template <typename U, typename... Args>
        void construct(U* const ptr, Args&&... args) {
            std::uninitialized_construct_using_allocator(ptr, *this, std::forward<Args>(args)...);
        }

        template <typename U>
        void destroy(U* const ptr) noexcept {
            std::destroy_at(ptr);
        }

        [[nodiscard]] MemoryAllocator select_on_container_copy_construction() const noexcept {
            return MemoryAllocator();
        }

        [[nodiscard]] IMemoryResource* resource() const noexcept {
            return m_Resource;
        }

        template <typename U>
        [[nodiscard]] bool operator==(const MemoryAllocator<U>& other) const noexcept {
            HELENA_ASSERT(m_Resource, "Memory resource is nullptr");
            HELENA_ASSERT(other.m_Resource, "Memory resource is nullptr");
            return *m_Resource == *other.m_Resource;
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

    /**
    * @brief MonotonicAllocator
    * Analog of the std::pmr::monotonic_buffer_resource (GCC) allocator.
    */
    class MonotonicAllocator : public IMemoryResource
    {
        struct MemoryHeader
        {
            // stack implementation
            MemoryHeader* m_Next;

            std::size_t m_Size;
            std::size_t m_Alignment;

            [[nodiscard]] void* Base() noexcept {
                return std::bit_cast<std::byte*>(this + 1) - m_Size;
            }
        };

        static constexpr std::size_t MinSize    = 1024 + sizeof(MemoryHeader);
        static constexpr std::size_t MaxSize    = (std::numeric_limits<std::size_t>::max)();

        static constexpr std::size_t GrowthSize(std::size_t size) noexcept {
            return size / 2 * 3; // grow factor: 1.5
        }

    public:
        explicit MonotonicAllocator(IMemoryResource* upstreamResource = DefaultAllocator::Get()) noexcept
            : m_UpstreamResource{upstreamResource}, m_MemoryHeader{}
            , m_Buffer{}, m_Space{}, m_NextBufferSize{MinSize}
            , m_InitBuffer{}, m_InitSize{} {
            HELENA_ASSERT(upstreamResource, "Resource is nullptr!");
        }

        MonotonicAllocator(std::size_t initSize, IMemoryResource* upstreamResource = DefaultAllocator::Get()) noexcept
            : m_UpstreamResource{upstreamResource}, m_MemoryHeader{}
            , m_Buffer{}, m_Space{}, m_NextBufferSize{std::clamp(initSize, MinSize, MaxSize)}
            , m_InitBuffer{}, m_InitSize{initSize} {
            HELENA_ASSERT(upstreamResource, "Resource is nullptr!");
            HELENA_ASSERT(initSize, "Size incorrect!");
        }

        MonotonicAllocator(void* const buffer, std::size_t size, IMemoryResource* upstreamResource = DefaultAllocator::Get()) noexcept
            : m_UpstreamResource{upstreamResource}, m_MemoryHeader{}
            , m_Buffer{buffer}, m_Space{size}, m_NextBufferSize{GrowthSize(size)}
            , m_InitBuffer{buffer}, m_InitSize{size} {
            HELENA_ASSERT(upstreamResource, "Resource is nullptr!");
            HELENA_ASSERT(buffer, "Buffer is nullptr!");
            HELENA_ASSERT(size, "Size incorrect!");
        }

        ~MonotonicAllocator() noexcept {
            Release();
        }

        MonotonicAllocator(const MonotonicAllocator&) = delete;
        MonotonicAllocator(MonotonicAllocator&&) noexcept = delete;
        MonotonicAllocator& operator=(const MonotonicAllocator&) = delete;
        MonotonicAllocator& operator=(MonotonicAllocator&&) noexcept = delete;

        [[nodiscard]] IMemoryResource* UpstreamResource() const noexcept {
            return m_UpstreamResource;
        }

        void Release() noexcept
        {
            if((m_Buffer = m_InitBuffer)) {
                m_Space = m_InitSize;
                m_NextBufferSize = GrowthSize(m_InitSize);
            } else {
                m_Space = 0;
                m_NextBufferSize = MinSize;
            }

            while(m_MemoryHeader) {
                const auto memory = std::exchange(m_MemoryHeader, m_MemoryHeader->m_Next);
                m_UpstreamResource->FreeMemory(memory->Base(), memory->m_Size, memory->m_Alignment);
            }
        }

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            if(m_Buffer = Align(m_Buffer, m_Space, bytes, alignment); !m_Buffer) {
                RequestMemory(bytes, alignment);
            }

            return m_Space -= bytes, std::exchange(m_Buffer, static_cast<std::byte*>(m_Buffer) + bytes);
        }

        void Free([[maybe_unused]] void* ptr, [[maybe_unused]] std::size_t bytes, [[maybe_unused]] std::size_t alignment) override {}

        bool Equal(const IMemoryResource& other) const override {
            return this == &other;
        }

    private:
        void RequestMemory(std::size_t size, std::size_t alignment)
        {
            if(size > MaxSize - (sizeof(MemoryHeader) + alignof(MemoryHeader))) [[unlikely]] {
                throw std::bad_alloc{};
            }

            const std::size_t requiredSize = (size + sizeof(MemoryHeader) + alignof(MemoryHeader) - 1) & ~(alignof(MemoryHeader) - 1);
            const std::size_t requiredAlignment = (std::max)(alignof(MemoryHeader), alignment);

            if(requiredSize > m_NextBufferSize) [[unlikely]] {
                size = requiredSize;
            } else {
                size = m_NextBufferSize;
            }

            m_Buffer = m_UpstreamResource->AllocateMemory(size, requiredAlignment);
            HELENA_ASSERT(AlignDistance(m_Buffer, requiredAlignment) == 0, "Upstream resource did not respect alignment requirement!");
            m_Space = size - sizeof(MemoryHeader);
            m_MemoryHeader = new (static_cast<std::byte*>(m_Buffer) + m_Space) MemoryHeader{m_MemoryHeader, size, requiredAlignment};
            m_NextBufferSize = GrowthSize(size);
        }

    private:
        IMemoryResource* m_UpstreamResource;
        MemoryHeader* m_MemoryHeader;

        void* m_Buffer;
        std::size_t m_Space;
        std::size_t m_NextBufferSize;

        void* m_InitBuffer;
        std::size_t m_InitSize;
    };

    /**
    * @brief StackAllocator
    * Based on top of the MonotonicAllocator, which means the memory is only released on Release calls.
    *
    * @tparam Stack Size of bytes
    *
    * @code{.cpp}
    * Types::StackAllocator<10 * sizeof(std::string)> allocator;
    * @endcode
    *
    * @note
    * You need to be careful when working with the stack allocator, do not forget that the
    * memory that is allocated on the stack will be freed when the scope is exited.
    */
    template <std::size_t Stack>
    class StackAllocator : public MonotonicAllocator
    {
    public:
        explicit StackAllocator(IMemoryResource* upstreamResource = NulledAllocator::Get()) noexcept
            : MonotonicAllocator(&m_Buffer, Stack, upstreamResource), m_Empty{} {}
        ~StackAllocator() = default;
        StackAllocator(const StackAllocator&) = delete;
        StackAllocator(StackAllocator&&) noexcept = delete;
        StackAllocator& operator=(const StackAllocator&) = delete;
        StackAllocator& operator=(StackAllocator&&) noexcept = delete;

        [[nodiscard]] static constexpr std::size_t Capacity() noexcept {
            return Stack;
        }

    private:
        union {
            std::byte m_Empty;
            std::byte m_Buffer[Stack];
        };
    };

    /**
    * @brief LoggingAllocator (wrapper)
    * @tparam NameIdentifier Name identifier for debugging
    * @tparam Allocator Type of Allocator
    * @tparam Callback for handle event
    *
    * @code{.cpp}
    * Types::LoggingAllocator<Types::DefaultAllocator, "Test Allocator", [](const char* name, bool allocate, void* ptr, std::size_t bytes, std::size_t alignment){
    *     HELENA_MSG_MEMORY("Allocator: {}, {} addr: {}, bytes: {}, alignment: {}", name, allocate ? "allocate" : "free", ptr, bytes, alignment);
    * }> allocator;
    * @endcode
    *
    * @note
    * The architecture of this class is slightly different from the previous ones,
    * this class takes the type of the allocator class as a template argument,
    * and then inherits from it, we do not want to take up extra bytes of memory,
    * and even more so to accept the allocators that we want to track as an upstream memory resource.
    */
    template <FixedBuffer<64> NameIdentifier, typename Allocator, auto Callback>
    requires requires{ Callback("", true, nullptr, std::size_t{}, std::size_t{}); }
    class LoggingAllocator : public Allocator
    {
        static_assert(!std::is_final_v<Allocator>, "Allocator type does not meet requirements!");
        static_assert(!std::is_same_v<Allocator, IMemoryResource>, "IMemoryResource is not allocator!");

    public:
        template <typename... Args>
        requires std::constructible_from<Allocator, Args...>
        LoggingAllocator(Args&&... args) : Allocator(std::forward<Args>(args)...) {}
        ~LoggingAllocator() = default;
        LoggingAllocator(const LoggingAllocator&) = delete;
        LoggingAllocator(LoggingAllocator&&) noexcept = delete;
        LoggingAllocator& operator=(const LoggingAllocator&) = delete;
        LoggingAllocator& operator=(LoggingAllocator&&) noexcept = delete;

        [[nodiscard]] static constexpr const char* Name() noexcept {
            return NameIdentifier;
        }

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override {
            const auto ptr = Allocator::Allocate(bytes, alignment);
            return Callback(Name(), /* allocate */ true, ptr, bytes, alignment), ptr;
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) override {
            Callback(Name(), /* allocate */ false, ptr, bytes, alignment);
            Allocator::Free(ptr, bytes, alignment);
        }
    };

    /**
    * @brief DebuggingAllocator (wrapper)
    * @tparam NameIdentifier Name identifier for debugging
    * @tparam Allocator Type of Allocator
    *
    * @code{.cpp}
    * Types::DebuggingAllocator<"Test Allocator", Types::DefaultAllocator> allocator;
    * @endcode
    *
    * @note
    * The architecture of this class is slightly different from the previous ones,
    * this class takes the type of the allocator class as a template argument,
    * and then inherits from it, we do not want to take up extra bytes of memory,
    * and even more so to accept the allocators that we want to track as an upstream memory resource.
    */
    template <FixedBuffer<64> NameIdentifier, typename Allocator>
    class DebuggingAllocator : public Allocator
    {
        static_assert(!std::is_final_v<Allocator>, "Allocator type does not meet requirements!");
        static_assert(!std::is_same_v<Allocator, IMemoryResource>, "IMemoryResource is not allocator!");

    public:
        struct BlockInfo {
            void* m_Address;
            std::size_t m_Bytes;
            std::size_t m_Alignment;
        };

    public:
        template <typename... Args>
        requires std::constructible_from<Allocator, Args...>
        DebuggingAllocator(Args&&... args) : Allocator(std::forward<Args>(args)...)
            , m_Blocks{}
            , m_MaxUsedBlocks{}
            , m_MaxAllocatedBytes{}
            , m_MaxUsedBytes{}
            , m_UsedBytes {} {}
        ~DebuggingAllocator() = default;
        DebuggingAllocator(const DebuggingAllocator&) = delete;
        DebuggingAllocator(DebuggingAllocator&&) noexcept = delete;
        DebuggingAllocator& operator=(const DebuggingAllocator&) = delete;
        DebuggingAllocator& operator=(DebuggingAllocator&&) noexcept = delete;

        [[nodiscard]] static constexpr const char* Name() noexcept {
            return NameIdentifier;
        }

        [[nodiscard]] decltype(auto) GetBlocks() const noexcept {
            return m_Blocks;
        }

        [[nodiscard]] std::size_t UsedBlocks() const noexcept {
            return m_Blocks.size();
        }

        [[nodiscard]] std::size_t MaxUsedBlocks() const noexcept {
            return m_MaxUsedBlocks;
        }

        [[nodiscard]] std::size_t MaxAllocatedBytes() const noexcept {
            return m_MaxAllocatedBytes;
        }

        [[nodiscard]] std::size_t UsedBytes() const noexcept {
            return m_UsedBytes;
        }

        [[nodiscard]] std::size_t MaxUsedBytes() const noexcept {
            return m_MaxUsedBytes;
        }

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            const auto ptr = Allocator::Allocate(bytes, alignment);
            const auto it = std::lower_bound(m_Blocks.begin(), m_Blocks.end(), ptr, [](const auto& block, const auto ptr) {
                return block.m_Address < ptr;
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
                NameIdentifier, IMemoryResource::NameOf<Allocator>, ptr);
            if(it == m_Blocks.cend()) [[unlikely]] {
                HELENA_MSG_ERROR("[ID: {} | {}] Block address: {} not found in blocks!",
                    NameIdentifier, IMemoryResource::NameOf<Allocator>, ptr);
                throw std::invalid_argument("Free memory failed: invalid pointer!");
            }

            HELENA_ASSERT(it->m_Bytes == bytes, "[ID: {} | {}] Block address: {} bytes ({}) != ({}) free bytes!",
                NameIdentifier, IMemoryResource::NameOf<Allocator>, ptr, it->m_Bytes, bytes);
            if(it->m_Bytes != bytes) {
                HELENA_MSG_ERROR("[ID: {} | {}] Block address: {} bytes ({}) != ({}) free bytes!",
                    NameIdentifier, IMemoryResource::NameOf<Allocator>, ptr, it->m_Bytes, bytes);
                throw std::invalid_argument("Free memory failed: size mismatch!");
            }

            HELENA_ASSERT(it->m_Alignment == alignment, "[ID: {} | {}] Block address: {} alignment ({}) != ({}) free alignment!",
                NameIdentifier, IMemoryResource::NameOf<Allocator>, ptr, it->m_Alignment, alignment);
            if(it->m_Alignment != alignment) {
                HELENA_MSG_ERROR("[ID: {} | {}] Block address: {} alignment ({}) != ({}) free alignment!",
                    NameIdentifier, IMemoryResource::NameOf<Allocator>, ptr, it->m_Alignment, alignment);
                throw std::invalid_argument("Free memory failed: alignment mismatch!");
            }

            m_Blocks.erase(it);
            m_UsedBytes -= bytes;
            Allocator::Free(ptr, bytes, alignment);
        }

    private:
        auto BinarySearch(void* const ptr) const noexcept {
            return std::lower_bound(m_Blocks.begin(), m_Blocks.end(), ptr, [](const auto& block, const auto ptr) {
                return block.m_Address < ptr;
            });
        }

    private:
        std::vector<BlockInfo> m_Blocks;
        std::size_t m_MaxUsedBlocks;
        std::size_t m_MaxAllocatedBytes;
        std::size_t m_MaxUsedBytes;
        std::size_t m_UsedBytes;
    };

    /**
    * @brief ArenaAllocator
    * Incredibly fast memory allocator 
    * @tparam AlignmentBucketsGrowthFactor The minimum alignment of memory chunk.
    * @tparam MemoryBucketsGrowthFactor The subsequent bucket sizes of memory.
    * @tparam MemoryBucketsMaxSize The maximum size of memory bucket.
    * @tparam ChunkBucketsMaxChunks The maximum number of memory chunks per bucket.
    * @tparam FreeOrphanMemory If true, the memory chunks that are not used are free.
    *
    * @code{.cpp}
    * Types::MemoryArena<8, 32, 1024, 1024, true> allocator;
    * @endcode
    *
    * @note
    * The architecture of this class is slightly different from the previous ones,
    * this class takes the type of the allocator class as a template argument,
    * and then inherits from it, we do not want to take up extra bytes of memory,
    * and even more so to accept the allocators that we want to track as an upstream memory resource.
    *
    *   Memory Arena:
    *   Note:   Efficient and fast for allocate, free, clear chunks, but not thread safe!
    *           This arena stores pools and chunks (memory) until clear is called (the destructor also frees memory).
    *           Focused on performance, recommended for use with other allocators where ArenaAllocator
    *           is used as an upstream resource.
    *   Architecture:
    *   AlignmentsBuckets | MemoryBuckets | ChunkBuckets
    *           [4]     ->      [32]    ->  [32  byte] [32  byte] [32  byte]
    *                           [64]    ->  [64  byte] [64  byte] [64  byte]
    *                           [96]    ->  [96  byte] [96  byte] [96  byte]
    *                           [128]   ->  [128 byte] [128 byte] [128 byte]
    *           [8]     ->      [32]    ->  [32  byte] [32  byte] [32  byte]
    *                           [64]    ->  [64  byte] [64  byte] [64  byte]
    *                           [96]    ->  [96  byte] [96  byte] [96  byte]
    *                           [128]   ->  [128 byte] [128 byte] [128 byte]
    *           [16]    ->      [32]    ->  [32  byte] [32  byte] [32  byte]
    *                           [64]    ->  [64  byte] [64  byte] [64  byte]
    *                           [96]    ->  [96  byte] [96  byte] [96  byte]
    *                           [128]   ->  [128 byte] [128 byte] [128 byte]
    *           [32]    ->      [32]    ->  [32  byte] [32  byte] [32  byte]
    *                           [64]    ->  [64  byte] [64  byte] [64  byte]
    *                           [96]    ->  [96  byte] [96  byte] [96  byte]
    *                           [128]   ->  [128 byte] [128 byte] [128 byte]
    *           [64]    ->      [32]    ->  [32  byte] [32  byte] [32  byte]
    *                           [64]    ->  [64  byte] [64  byte] [64  byte]
    *                           [96]    ->  [96  byte] [96  byte] [96  byte]
    *                           [128]   ->  [128 byte] [128 byte] [128 byte]
    *           [128]   ->      [32]    ->  [32  byte] [32  byte] [32  byte]
    *                           [64]    ->  [64  byte] [64  byte] [64  byte]
    *                           [96]    ->  [96  byte] [96  byte] [96  byte]
    *                           [128]   ->  [128 byte] [128 byte] [128 byte]
    *           ...
    */
    template <
        std::size_t AlignmentBucketsGrowthFactor = 8,
        std::size_t MemoryBucketsGrowthFactor = 32,
        std::size_t MemoryBucketsMaxSize = 1024,
        std::size_t ChunkBucketsMaxChunks = 128,
        bool FreeOrphanMemory = true
    >
    requires Traits::IsPowerOf2<AlignmentBucketsGrowthFactor>
    class ArenaAllocator : public IMemoryResource
    {
        using BitSetType = std::uint64_t;
        static constexpr auto BitSetSize = std::numeric_limits<BitSetType>::digits;
        static constexpr auto CountChunks = (ChunkBucketsMaxChunks - 1) / BitSetSize + 1;

        static_assert(MemoryBucketsGrowthFactor >= 8, "MemoryBucketsGrowthFactor cannot be less than 8");
        static_assert(MemoryBucketsMaxSize >= MemoryBucketsGrowthFactor, "MemoryBucketsMaxSize cannot be less than MemoryBucketsGrowthFactor");
        static_assert(MemoryBucketsMaxSize % MemoryBucketsGrowthFactor == 0, "MemoryBucketsMaxSize must be a multiple of MemoryBucketsGrowthFactor");

        template <typename T>
        using ContainerOf = std::vector<T>;

        template <typename T>
        using AlignmentsBuckets = ContainerOf<T>;

        template <typename T>
        using MemoryBuckets = ContainerOf<T>;

        struct ChunkBuckets {
            std::size_t m_Count{};
            BitSetType m_BitSet[CountChunks]{};
            std::byte* m_Memory{};
        };

        using StorageContainers = AlignmentsBuckets<MemoryBuckets<ChunkBuckets*>>;

    public:
        ArenaAllocator(IMemoryResource* upstreamResource = DefaultAllocator::Get()) noexcept
            : m_Storage{}
            , m_UpstreamResource{upstreamResource} {}

        ~ArenaAllocator()
        {
            auto& alignmentBuckets = m_Storage;
            for(std::size_t alignmentIndex = 0; alignmentIndex < alignmentBuckets.size(); ++alignmentIndex)
            {
                auto& memoryBuckets = alignmentBuckets[alignmentIndex];
                for(std::size_t memoryIndex = 0; memoryIndex < memoryBuckets.size(); ++memoryIndex) {
                    Deallocate(memoryBuckets[memoryIndex], memoryIndex, alignmentIndex);
                }
            }
        }

        ArenaAllocator(const ArenaAllocator&) = delete;
        ArenaAllocator(ArenaAllocator&&) noexcept = delete;
        ArenaAllocator& operator=(const ArenaAllocator&) = delete;
        ArenaAllocator& operator=(ArenaAllocator&&) noexcept = delete;

        void Clear(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t))
        {
            auto& alignmentBuckets = m_Storage;
            const auto alignmentIndex = Util::Math::Log2((std::max)(alignment, AlignmentBucketsGrowthFactor) / AlignmentBucketsGrowthFactor);
            if(alignmentIndex >= alignmentBuckets.size()) [[unlikely]] {
                return;
            }

            auto& memoryBuckets = alignmentBuckets[alignmentIndex];
            const auto memoryIndex = (bytes - 1) / MemoryBucketsGrowthFactor;
            if(memoryIndex >= memoryBuckets.size()) [[unlikely]] {
                return;
            }

            Deallocate(memoryBuckets[memoryIndex], memoryIndex, alignmentIndex);
        }

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            auto& alignmentBuckets = m_Storage;

            const auto memoryIndex = (bytes - 1) / MemoryBucketsGrowthFactor;
            if(memoryIndex >= (MemoryBucketsMaxSize / MemoryBucketsGrowthFactor)) [[unlikely]] {
                return m_UpstreamResource->AllocateMemory(bytes, alignment);
            }

            const auto alignmentIndex = Util::Math::Log2((std::max)(alignment, AlignmentBucketsGrowthFactor) / AlignmentBucketsGrowthFactor);
            if(alignmentIndex >= alignmentBuckets.size()) [[unlikely]] {
                Reallocate(alignmentBuckets, alignmentIndex);
            }

            auto& memoryBuckets = alignmentBuckets[alignmentIndex];
            if(memoryIndex >= memoryBuckets.size()) [[unlikely]] {
                Reallocate(memoryBuckets, memoryIndex);
            }

            auto& chunkBucket = memoryBuckets[memoryIndex];
            if(!chunkBucket) [[unlikely]] {
                InitChunks(chunkBucket, memoryIndex, alignmentIndex);
            }

            auto& chunks = *chunkBucket;
            const auto begin = std::begin(chunks.m_BitSet);
            const auto end = std::end(chunks.m_BitSet);
            if(const auto it = std::find_if(begin, end, HasSpaceInBucket); it != end)
            {
                const auto distance = std::distance(begin, it);
                const auto offset = std::countr_one(*it);
                const auto index = distance * BitSetSize + offset;
                ++chunks.m_Count;
                *it |= (1uLL << offset);
                return chunks.m_Memory + (index * (MemoryBucketsGrowthFactor * (memoryIndex + 1)));
            }

            return nullptr;
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) override
        {
            auto& alignmentBuckets = m_Storage;

            const auto memoryIndex = (bytes - 1) / MemoryBucketsGrowthFactor;
            if(memoryIndex >= (MemoryBucketsMaxSize / MemoryBucketsGrowthFactor)) [[unlikely]] {
                m_UpstreamResource->FreeMemory(ptr, bytes, alignment);
                return;
            }

            const auto alignmentIndex = Util::Math::Log2((std::max)(alignment, AlignmentBucketsGrowthFactor) / AlignmentBucketsGrowthFactor);
            HELENA_ASSERT(alignmentIndex < alignmentBuckets.size(),
                "alignmentIndex: {} out of bound when container size: {}", alignmentIndex, alignmentBuckets.size());
            if(alignmentIndex >= alignmentBuckets.size()) [[unlikely]] {
                throw std::runtime_error("Alignment index out of pool range!");
            }

            auto& memoryBuckets = alignmentBuckets[alignmentIndex];
            HELENA_ASSERT(memoryIndex < memoryBuckets.size(),
                "ArenaAllocator memoryIndex: {} out of bound when container size: {}", memoryIndex, memoryBuckets.size());
            if(memoryIndex >= memoryBuckets.size()) [[unlikely]] {
                throw std::runtime_error("Memory index out of pool range!");
            }

            auto& chunkBucket = memoryBuckets[memoryIndex];
            HELENA_ASSERT(chunkBucket, "Memory bucket is nullptr, possible double free!");
            if(!chunkBucket) [[unlikely]] {
                throw std::runtime_error("Memory bucket is nullptr, possible double free!");
            }

            auto& chunks = *chunkBucket;
            const auto blockSize = (memoryIndex + 1) * MemoryBucketsGrowthFactor;
            HELENA_ASSERT(ptr >= chunks.m_Memory && ptr <= (chunks.m_Memory + blockSize * ChunkBucketsMaxChunks - MemoryBucketsGrowthFactor),
                "Memory pointer out of range, possible your ptr not allocated using current allocator!");
            if(chunks.m_Memory > ptr || ptr > (chunks.m_Memory + blockSize * ChunkBucketsMaxChunks - MemoryBucketsGrowthFactor)) [[unlikely]] {
                throw std::runtime_error("Memory pointer out of range, possible your ptr not allocated using current allocator!");
            }

            const auto distance = std::distance(chunks.m_Memory, static_cast<std::byte*>(ptr));
            const auto index = (distance / (MemoryBucketsGrowthFactor * (memoryIndex + 1)));
            const auto offset = index / BitSetSize;
            const auto bitOffset = index % BitSetSize;
            HELENA_ASSERT(chunks.m_Memory + blockSize * (offset * BitSetSize + bitOffset) == ptr,
               "Memory pointer address out of range!");
            if((chunks.m_Memory + blockSize * (offset * BitSetSize + bitOffset)) != ptr) [[unlikely]] {
                throw std::runtime_error("Memory pointer address out of range!");
            }

            --chunks.m_Count;
            chunks.m_BitSet[offset] &= ~(1uLL << bitOffset);

            if constexpr(FreeOrphanMemory)
            {
                if(!chunks.m_Count) {
                    Deallocate(chunkBucket, memoryIndex, alignmentIndex);
                }
            }
        }

        bool Equal(const IMemoryResource& other) const override {
            return this == &other;
        }

    private:
        template <typename T>
        HELENA_NOINLINE static void Reallocate(ContainerOf<T>& container, std::size_t index) {
            container.resize(index + 1);
        }

        HELENA_NOINLINE void InitChunks(ChunkBuckets*& storage, std::size_t memoryIndex, std::size_t alignIndex)
        {
            const auto memory = m_UpstreamResource->AllocateMemory(GetSize(memoryIndex), GetAlignment(alignIndex));
            if(!memory) [[unlikely]] {
                throw std::bad_alloc{};
            }

            const auto headerPlace = static_cast<std::byte*>(memory) + GetHeaderOffset(memoryIndex);
            storage = new (headerPlace) ChunkBuckets{
                .m_Count = 0,
                .m_BitSet = {},
                .m_Memory = static_cast<std::byte*>(memory)
            };
        }

        void Deallocate(ChunkBuckets*& chunkBucket, std::size_t memoryIndex, std::size_t alignIndex)
        {
            if(chunkBucket) [[likely]] {
                m_UpstreamResource->FreeMemory(chunkBucket->m_Memory, GetSize(memoryIndex), GetAlignment(alignIndex));
            }
            chunkBucket = nullptr;
        }

        [[nodiscard]] static std::size_t GetHeaderOffset(std::size_t memoryIndex) noexcept {
            return ChunkBucketsMaxChunks * (MemoryBucketsGrowthFactor * (memoryIndex + 1));
        }

        [[nodiscard]] static std::size_t GetSize(std::size_t memoryIndex) noexcept {
            return GetHeaderOffset(memoryIndex) + sizeof(ChunkBuckets);
        }

        [[nodiscard]] static std::size_t GetAlignment(std::size_t alignmentIndex) noexcept {
            return (std::max)(AlignmentBucketsGrowthFactor * (alignmentIndex + 1), alignof(ChunkBuckets));
        }

        [[nodiscard]] static bool HasSpaceInBucket(const BitSetType value) noexcept {
            return value != (std::numeric_limits<BitSetType>::max)();
        }

    private:
        StorageContainers m_Storage;
        IMemoryResource* m_UpstreamResource;
    };


    inline void DefaultAllocator::Set(IMemoryResource* resource) noexcept {
        DefaultAllocator::m_Resource = resource;
    }

    inline void NulledAllocator::Set(IMemoryResource* resource) noexcept {
        NulledAllocator::m_Resource = resource;
    }

    [[nodiscard]] inline IMemoryResource* DefaultAllocator::Get() noexcept
    {
        if(!m_Resource) [[unlikely]] {
            m_Resource = &Default;
        }

        return m_Resource;
    }

    [[nodiscard]] inline IMemoryResource* NulledAllocator::Get() noexcept
    {
        if(!m_Resource) [[unlikely]] {
            m_Resource = &Default;
        }

        return m_Resource;
    }

    DefaultAllocator DefaultAllocator::Default{};
    NulledAllocator NulledAllocator::Default{};
}
#endif // HELENA_TYPES_STACKALLOCATOR_HPP