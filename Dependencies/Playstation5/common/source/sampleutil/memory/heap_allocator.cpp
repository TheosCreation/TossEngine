/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */

#include "sampleutil/memory/heap_allocator.h"
#include "sampleutil/sampleutil_common.h"
#ifdef _DEBUG
#include <sanitizer/asan_interface.h>
#include "sampleutil/memory/memory_analyzer.h"
#endif
#include "sampleutil/debug/perf.h"

namespace sce { namespace SampleUtil { namespace Memory {

struct	HeapAllocator::Chunk
{
	size_t m_size;
	void	*m_ptr;
	Chunk	*m_prev;
	Chunk	*m_next;
	Chunk(size_t	size, void	*ptr = nullptr, Chunk	*prev = nullptr, Chunk	*next =nullptr)
		: m_size(size), m_ptr(ptr), m_prev(prev), m_next(next)
	{
	}
	bool operator<(const Chunk	&rhs) const
	{
		return (m_size < rhs.m_size) || (m_size == rhs.m_size
			&& (!rhs.m_ptr || reinterpret_cast<size_t>(m_ptr) < reinterpret_cast<size_t>(rhs.m_ptr)));
	}
	bool operator==(const Chunk	&rhs) const
	{
		return m_ptr == rhs.m_ptr && m_size == rhs.m_size;
	}
};

HeapAllocator::HeapAllocator(void	*mapStart, size_t	size, const std::string	&name, bool noPhysicalMemory)
	: m_noPhysicalMemory(noPhysicalMemory)
{
	(void)m_noPhysicalMemory;
	(void)m_pool;
	TAG_THIS_CLASS;
	setInfo(mapStart, size, name, noPhysicalMemory);
#ifdef _DEBUG
	if (Memory::g_isMatInitialized && name != "")
	{
		m_pool = sceMatAllocPoolMemory(m_mapStart, m_size, Memory::getMatGroup(this));
		sceMatTagPool(m_pool, name.c_str());
	}
#endif
}

void HeapAllocator::setInfo(void	*mapStart, size_t	size, const std::string	&name, bool	noPhysicalMemory)
{
	for(auto	&chunk : m_chunks)
	{
		delete chunk.second;
	}

	for(auto	&free : m_freeChunks)
	{
		delete free;
	}

	m_size		= size;
	m_mapStart	= mapStart;
#ifdef _DEBUG
	if (Memory::g_isMatInitialized && name != "")
	{
		Memory::registerMatGroup(this, name.c_str());
	}
	if (!noPhysicalMemory) {
		ASAN_POISON_MEMORY_REGION(mapStart, size);
	}
#endif

	if(size > 0 && mapStart)
	{
		m_freeChunks.insert(new Chunk(size, m_mapStart));
	}
}

bool	HeapAllocator::ChunkCompare::operator() (const Chunk	*lhs, const Chunk	*rhs) const
{
	return (*lhs) < (*rhs);
}

HeapAllocator::~HeapAllocator()
{
	UNTAG_THIS_CLASS;
	setInfo(nullptr, 0, "");
#ifdef _DEBUG
	if (m_pool != (MatPool)-1)
	{
		sceMatFreePoolMemory(m_pool);
	}
#endif
}

void	*HeapAllocator::allocate(size_t	size, size_t	align, const std::string	&name)
{
	std::unique_lock<std::mutex> l(m_mutex);
	Chunk c(size);
	auto it = m_freeChunks.lower_bound(&c);
	for(; it != m_freeChunks.end(); ++it)
	{
		size_t ptr = reinterpret_cast<size_t>((*it)->m_ptr);
		size_t offset = (align - (ptr % align)) % align;

		if ((*it)->m_size >= (size + offset))
		{
			Chunk	*cur = *it;

			m_freeChunks.erase(it);

			size_t begPtr = ptr + offset;
			size_t endPtr = ptr + cur->m_size;
			size_t newEnd = begPtr + size;

			void	*newBeg = reinterpret_cast<void*>(begPtr);

			if (offset) // break off back as new chunk
			{
				auto prev = new Chunk(offset, cur->m_ptr, cur->m_prev, cur);
				if (cur->m_prev)
				{
					cur->m_prev->m_next = prev;
				}
				cur->m_prev = prev;
				cur->m_ptr = newBeg;
				cur->m_size -= offset;

				m_freeChunks.insert(prev);
			}

			if (newEnd != endPtr) // break this off as a new chunk
			{
				auto next = cur;
				cur = new Chunk(size, newBeg, next->m_prev, next);
				if (cur->m_prev)
				{
					cur->m_prev->m_next = cur;
				}
				next->m_prev = cur;
				next->m_size = endPtr - newEnd;
				next->m_ptr = reinterpret_cast<void*>(newEnd);

				m_freeChunks.insert(next);
			}

			m_chunks[cur->m_ptr] = cur;
			if (cur->m_ptr)
			{
				allocateHook(cur->m_ptr, cur->m_size, name);
			}
#ifdef _DEBUG
			if (!m_noPhysicalMemory)
			{
				if (Memory::g_isMatInitialized)
				{
					sceMatAlloc(cur->m_ptr, size, 0, getMatGroup());
					sceMatTagAllocation(cur->m_ptr, name.c_str());
				}
				ASAN_UNPOISON_MEMORY_REGION(cur->m_ptr, size);
			}
#endif
			return cur->m_ptr;
		}
	}

	return nullptr;
}

void	HeapAllocator::free(void	*ptr)
{
	if (!ptr)
	{
		return;
	}

	std::unique_lock<std::mutex> l(m_mutex);
	auto it = m_chunks.find(ptr);

	SCE_SAMPLE_UTIL_ASSERT(it != m_chunks.end());

	auto cur = it->second;
	m_chunks.erase(it);

#ifdef _DEBUG
	if (!m_noPhysicalMemory)
	{
		ASAN_POISON_MEMORY_REGION(ptr, cur->m_size);
		if (Memory::g_isMatInitialized)
		{
			sceMatFree(ptr);
		}
		sce::SampleUtil::Debug::Perf::unTagBuffer(ptr);
	}
#endif
	freeHook(ptr, cur->m_size);

	if (cur->m_prev)
	{
		auto prevIt = m_freeChunks.find(cur->m_prev);
		if (prevIt != m_freeChunks.end())
		{
			auto prev = *prevIt;
			m_freeChunks.erase(prevIt);

			if (cur->m_next)
			{
				auto nextIt = m_freeChunks.find(cur->m_next);
				if (nextIt != m_freeChunks.end())
				{
					auto next = *nextIt;

					m_freeChunks.erase(nextIt);

					next->m_size += cur->m_size + prev->m_size;
					next->m_ptr = prev->m_ptr;
					next->m_prev = prev->m_prev;

					if (next->m_prev)
					{
						next->m_prev->m_next = next;
					}

					m_freeChunks.insert(next);

					delete cur;
					delete prev;

					return;
				}
			}

			prev->m_size += cur->m_size;
			prev->m_next = cur->m_next;

			if (prev->m_next)
			{
				prev->m_next->m_prev = prev;
			}

			m_freeChunks.insert(prev);

			delete cur;

			return;
		}
	}

	if (cur->m_next)
	{
		auto nextIt = m_freeChunks.find(cur->m_next);
		if (nextIt != m_freeChunks.end())
		{
			auto next = *nextIt;

			m_freeChunks.erase(nextIt);

			next->m_size += cur->m_size;
			next->m_prev = cur->m_prev;
			next->m_ptr = cur->m_ptr;

			if (next->m_prev)
			{
				next->m_prev->m_next = next;
			}

			m_freeChunks.insert(next);

			delete cur;

			return;
		}
	}

	m_freeChunks.insert(cur);
}

} } } // namespace sce::SampleUtil::Memory
