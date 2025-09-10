/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */
#ifdef _DEBUG
#include <sanitizer/asan_interface.h>
#endif
#include "sampleutil/memory/ring_allocator.h"

namespace {
	off_t	alignAddr(off_t	addr, size_t	align)
	{
		return ((addr + align - 1) / align) * align;
	}
} // anonynous namespace

namespace sce { namespace SampleUtil { namespace Memory {

void	*RingAllocator::allocate(size_t	size, size_t	alignment, const std::string	&name)
{
	void *ret = nullptr;

	if (size == 0)
	{
		return ret;
	}

	std::unique_lock<std::mutex> l(m_mutex);

	bool success = false;
	off_t allocOff = 0, new_head = 0;
	while (!success)
	{
		allocOff = alignAddr(m_head, alignment);
		new_head = allocOff + size;
		if (new_head / m_size != m_head / m_size) // allocation is crossing boundary
		{
			allocOff = alignAddr(m_head, m_size);
			new_head = allocOff + size;
		}
		success = (m_head >= m_tail) ? (new_head - m_tail <= (int64_t)m_size) : (m_tail - new_head >= (int64_t)m_size);
		if (!success && !allocateFailedHook())
		{
			break;
		}
	}

	if (success)
	{
		ret = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_mapStart) + (allocOff % m_size));
		m_frameAllocatedMemoriesArray.back().push_back((uint64_t)(allocOff % (2 * m_size)) | ((uint64_t)size << 32));
		m_head = new_head % (2 * m_size);
		allocateHook(ret, size, name);
#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatAlloc(ret, size, 0, Memory::getMatGroup(this));
			sceMatTagAllocation(ret, name.c_str());
		}
		ASAN_UNPOISON_MEMORY_REGION(ret, size);
#endif
	}

	return ret;
}

void	RingAllocator::beginFrame()
{
	m_frameAllocatedMemoriesArray.push_back(std::vector<uint64_t>(0));
}

void	RingAllocator::endFrame()
{
	std::vector<uint64_t> &allocatedMemories = m_frameAllocatedMemoriesArray.front();
	uint64_t sizeAndOffset = 0;
	for (int i = 0; i < allocatedMemories.size(); i++)
	{
		sizeAndOffset = allocatedMemories[i];
		void *ptr = (uint8_t*)m_mapStart + ((sizeAndOffset & 0xffffffffu) % m_size);
		size_t size = sizeAndOffset >> 32;
		freeHook(ptr, size);
#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatFree(ptr);
		}
		ASAN_POISON_MEMORY_REGION(ptr, size);
#endif
	}
	m_tail = (sizeAndOffset & 0xffffffffu) + (sizeAndOffset >> 32);
	m_frameAllocatedMemoriesArray.pop_front();
}

} } } // namespace sce::SampleUtil::Memory
