#ifndef HELENA_MEMORY_CACHEALLOCATOR_HPP
#define HELENA_MEMORY_CACHEALLOCATOR_HPP

#include <Helena/Debug/Assert.hpp>

#include <memory>
#include <type_traits>

namespace Helena::Memory
{
	template <typename T, std::size_t Capacity, std::size_t Align = alignof(std::max_align_t)>
	class CacheAllocator
	{
	public:
		using value_type = T;
		using alloc = std::allocator<T>;

		CacheAllocator() noexcept : m_MemoryPtr(m_CachedMemory) {}
		~CacheAllocator() {
			m_MemoryPtr = nullptr;
		}
		
	private:
		std::aligned_storage_t<Capacity, Align> m_CachedMemory;
		char* m_MemoryPtr;
	};
}

#endif // HELENA_MEMORY_CACHEALLOCATOR_HPP