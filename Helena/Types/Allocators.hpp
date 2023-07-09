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
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            return Allocate(bytes, alignment);
        }

        void FreeMemory(void* ptr, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) noexcept {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment: {} must be a power of two.", alignment);
            Free(ptr, bytes, alignment);
        }

        void CopyableAllocator(IMemoryResource* resource) noexcept {
            m_CopyableAllocator = resource;
        }

        [[nodiscard]] IMemoryResource* CopyableAllocator() const noexcept {
            return m_CopyableAllocator;
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

        [[nodiscard]] static constexpr std::size_t PowerOf2(std::size_t alignment) noexcept
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

        [[nodiscard]] static constexpr void* Align(void* ptr, std::size_t& space, std::size_t size, std::size_t alignment) noexcept {
            const auto distance = AlignDistance(ptr, alignment);
            const auto success = space >= (distance + size);
            const auto result = std::bit_cast<void*>(std::bit_cast<std::uintptr_t>(ptr) * success + distance * success);
            return space -= distance * success, result;
        }

        [[nodiscard]] static constexpr void* AlignForward(void* ptr, std::size_t alignment) noexcept {
            HELENA_ASSERT(IsPowerOf2(alignment), "Alignment requirements are not met.");
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

    private:
        IMemoryResource* m_CopyableAllocator{};
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
        static inline IMemoryResource* m_Resource{};

    public:
        DefaultAllocator() = default;
        ~DefaultAllocator() noexcept = default;
        DefaultAllocator(const DefaultAllocator&) = delete;
        DefaultAllocator(DefaultAllocator&&) noexcept = default;
        DefaultAllocator& operator=(const DefaultAllocator&) = delete;
        DefaultAllocator& operator=(DefaultAllocator&&) noexcept = default;

        static void Set(IMemoryResource* resource) noexcept;
        [[nodiscard]] static IMemoryResource* Get() noexcept;

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override
        {
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                return ::operator new(bytes, std::align_val_t{alignment});
            }

            return ::operator new(bytes);
        }

        void Free(void* ptr, [[maybe_unused]] std::size_t bytes, std::size_t alignment) override
        {
            if(alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                return ::operator delete(ptr, bytes, std::align_val_t{alignment});
            }

            ::operator delete(ptr, bytes);
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
        static inline IMemoryResource* m_Resource{};

    public:
        NulledAllocator() = default;
        ~NulledAllocator() noexcept = default;
        NulledAllocator(const NulledAllocator&) = delete;
        NulledAllocator(NulledAllocator&&) noexcept = default;
        NulledAllocator& operator=(const NulledAllocator&) = delete;
        NulledAllocator& operator=(NulledAllocator&&) noexcept = default;

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
        MonotonicAllocator(MonotonicAllocator&&) noexcept = default;
        MonotonicAllocator& operator=(const MonotonicAllocator&) = delete;
        MonotonicAllocator& operator=(MonotonicAllocator&&) noexcept = default;

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
        StackAllocator(StackAllocator&&) noexcept = default;
        StackAllocator& operator=(const StackAllocator&) = delete;
        StackAllocator& operator=(StackAllocator&&) noexcept = default;

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
    *
    * @code{.cpp}
    * Types::LoggingAllocator<"Test Allocator", Types::DefaultAllocator> allocator;
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
        static_assert(!std::is_final_v<Allocator>, "Allocator type does not meet requirements!");
        static_assert(!std::is_same_v<Allocator, IMemoryResource>, "IMemoryResource is not allocator!");

    public:
        template <typename... Args>
        requires std::constructible_from<Allocator, Args...>
        LoggingAllocator(Args&&... args) : Allocator(std::forward<Args>(args)...) {}
        ~LoggingAllocator() = default;
        LoggingAllocator(const LoggingAllocator&) = delete;
        LoggingAllocator(LoggingAllocator&&) noexcept = default;
        LoggingAllocator& operator=(const LoggingAllocator&) = delete;
        LoggingAllocator& operator=(LoggingAllocator&&) noexcept = default;

        [[nodiscard]] static constexpr const char* Name() noexcept {
            return NameIdentifier;
        }

    protected:
        void* Allocate(std::size_t bytes, std::size_t alignment) override {
            HELENA_MSG_MEMORY("[ID: {} | {}] Alloc bytes: {} | alignment: {}", NameIdentifier, IMemoryResource::NameOf<Allocator>, bytes, alignment);
            return Allocator::Allocate(bytes, alignment);
        }

        void Free(void* ptr, std::size_t bytes, std::size_t alignment) override {
            HELENA_MSG_MEMORY("[ID: {} | {}] Free bytes: {} | alignment: {}", NameIdentifier, IMemoryResource::NameOf<Allocator>, bytes, alignment);
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
        DebuggingAllocator(DebuggingAllocator&&) noexcept = default;
        DebuggingAllocator& operator=(const DebuggingAllocator&) = delete;
        DebuggingAllocator& operator=(DebuggingAllocator&&) noexcept = default;

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
            std::uninitialized_construct_using_allocator(ptr, *this, std::forward<Args>(args)...);
        }

        template <typename U>
        void DestroyObject(U* const ptr) noexcept {
            std::allocator_traits<MemoryAllocator>::destroy(*this, ptr);
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
            std::allocator_traits<MemoryAllocator>::destroy(*this, ptr);
            deallocate_object(ptr);
        }

        template <typename U, typename... Args>
        void construct(U* const ptr, Args&&... args) {
            std::uninitialized_construct_using_allocator(ptr, *this, std::forward<Args>(args)...);
        }

        [[nodiscard]] MemoryAllocator select_on_container_copy_construction() const noexcept
        {
            if(m_Resource->CopyableAllocator()) {
                return MemoryAllocator(m_Resource->CopyableAllocator());
            }

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

    inline void DefaultAllocator::Set(IMemoryResource* resource) noexcept {
        DefaultAllocator::m_Resource = resource;
    }

    inline void NulledAllocator::Set(IMemoryResource* resource) noexcept {
        NulledAllocator::m_Resource = resource;
    }

    [[nodiscard]] inline IMemoryResource* DefaultAllocator::Get() noexcept
    {
        if(!DefaultAllocator::m_Resource) [[unlikely]] {
            static DefaultAllocator resource{};
            DefaultAllocator::m_Resource = &resource;
        }

        return DefaultAllocator::m_Resource;
    }

    [[nodiscard]] inline IMemoryResource* NulledAllocator::Get() noexcept
    {
        if(!NulledAllocator::m_Resource) [[unlikely]] {
            static NulledAllocator resource{};
            NulledAllocator::m_Resource = &resource;
        }

        return NulledAllocator::m_Resource;
    }
}
#endif // HELENA_TYPES_STACKALLOCATOR_HPP