/* SIE CONFIDENTIAL
 PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */

#include <unordered_map>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO

#include <ampr.h>
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/sampleutil_error.h"
#include "sampleutil/memory/dmem_mapper.h"
#include "sampleutil/memory/heap_allocator.h"

#pragma comment(lib, "libSceAmpr.a")

#if _DEBUG
#define PRINTF	printf
#else
#define PRINTF(...)
#endif

#define	PAGE_SIZE_ALIGN(a)	(((a) + (size_t)(SCE_KERNEL_PAGE_SIZE - 1)) & ~((size_t)(SCE_KERNEL_PAGE_SIZE - 1)))

namespace sce { namespace SampleUtil { namespace Memory {

#define DECLARE_AMM_PARAM()	\
	Ampr::AmmCommandBuffer			cb;	\
	SceKernelEqueue					eq;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
	// Auto-area is for Common use
	constexpr size_t kSizeDefaultAutoArea	= 6UL * 1024 * 1024 * 1024;	// 6GiB
	constexpr size_t kAlignAutoArea			= 2Ul * 1024 * 1024;		// 2MiB
	// Direct-area is only for VideoOut
	constexpr size_t kSizeDefaultDirectArea = 2UL * 1024 * 1024 * 1024;	// 2GiB
	constexpr size_t kAlignDirectArea		= 2Ul * 1024 * 1024;		// 2MiB

	constexpr uint32_t kSizeCommandBuffer = SCE_KERNEL_PAGE_SIZE;
	constexpr int32_t kEQ_ID_AMM = 0x123;

	static uint8_t			s_cmdBuffer[kSizeCommandBuffer] __attribute__((aligned(SCE_KERNEL_PAGE_SIZE))) = { 0 };
	static off_t			s_offsDirect = 0;

	static bool				s_initialized = false;
	static HeapAllocator	*s_heapAmm = nullptr;	// This manage VA in Amm all area.
#endif

	off_t	allocateAmmVirtualAddressRange(size_t	sizeInBytes, size_t	alignment)
	{
		off_t result = 0;
#if SCE_SAMPLE_UTIL_DISABLE_AMPR
		(void)sizeInBytes;
		(void)alignment;
#else
		result = (off_t)s_heapAmm->allocate(sizeInBytes, alignment);
#endif
		return result;
	}

	void	freeAmmVirtualAddressRange(off_t	virtualAddr)
	{
#if SCE_SAMPLE_UTIL_DISABLE_AMPR
		(void)virtualAddr;
#else
		s_heapAmm->free((void*)virtualAddr);
#endif
	}

	//------------------------------------------------------------------
	// Committed memory information
	//------------------------------------------------------------------
	struct CommitInfo
	{
		void	*m_start;
		size_t	m_size;
		int		m_type;
		int		m_prot;

		// Is addr is inside of committed area ?
		bool isInside(uint64_t start, size_t len)
		{
			return ((uintptr_t)m_start <= start && start + len <= (uintptr_t)m_start + m_size);
		}

		bool isOverlap(uint64_t start, size_t len)
		{
			return std::max(start, (uintptr_t)m_start) < std::min(start + len, (uintptr_t)m_start + m_size);
		}

		// Is specified area equal to me ?
		bool isEqual(uint64_t addr, size_t len)
		{
			return ((addr == (uint64_t)m_start) && (len == m_size));
		}
	};

	//------------------------------------------------------------------
	// Pooled Memory (This is used like a Pooled Memory on Orbis)
	//------------------------------------------------------------------
	class PooledMemory
	{
	public:
		std::unordered_map<void*, CommitInfo>	m_commitInfo;

		void					*m_mapStart;
		size_t					m_areaSize;

		void init(void *start, size_t sz)
		{
			m_mapStart = start;
			m_areaSize = sz;
		}

		// Is addr is inside of Pooled area ?
		bool isInside(uint64_t start, size_t len)
		{
			return ((uintptr_t)m_mapStart <= start && start + len <= (uintptr_t)m_mapStart + m_areaSize);
		}

		// Is specified area is committable area ?
		bool checkCommit(const void *start, size_t len)
		{
			uint64_t addr = (uint64_t)start;

			// Area [start ～ start + len] is not registered ?
			for (auto data : m_commitInfo)
			{
				// start address is already registered
				if (data.second.isOverlap(addr, len))
				{
					return false;
				}
			}
			// commitable
			return true;
		}

		// Regist commit info after added to command buffer
		void postCommit(void *start, size_t len, int type, int prot)
		{
			// regist
			CommitInfo info = { (void*)start, len, type, prot };
			m_commitInfo[start] = info;
		}

		// Is specified area is movable area ?
		bool move(void *dst, void *src, size_t len, CommitInfo &srcInfo)
		{
			// check src
			bool included = false;
			for (auto srcIter = m_commitInfo.begin(); srcIter != m_commitInfo.end(); srcIter++)
			{
				if ((*srcIter).second.isInside((uint64_t)src, len))
				{
					srcInfo = srcIter->second;
					m_commitInfo.erase(srcIter); // remove old info

					// add new split info
					if (srcInfo.m_start < src)
					{
						CommitInfo remainInfo = srcInfo;
						remainInfo.m_size = (uintptr_t)src - (uintptr_t)srcInfo.m_start;
						m_commitInfo.insert(std::make_pair(remainInfo.m_start, remainInfo));
					}
					if ((uintptr_t)srcInfo.m_start + srcInfo.m_size > (uintptr_t)src + len)
					{
						CommitInfo remainInfo = srcInfo;
						remainInfo.m_start = (void *)((uintptr_t)src + len);
						remainInfo.m_size = ((uintptr_t)srcInfo.m_start + srcInfo.m_size) - ((uintptr_t)src + len);
						m_commitInfo.insert(std::make_pair(remainInfo.m_start, remainInfo));
					}

					included = true;
					break;
				}
			}
			if (!included)
			{
				PRINTF("Warning : [Pool] SRC area[%lx, %lx] does not exist in this Pool.\n", (uint64_t)src, (uint64_t)src + len);
				return false;
			}
			// check dst
			for (auto dstIter = m_commitInfo.begin(); dstIter != m_commitInfo.end(); dstIter++)
			{
				if ((*dstIter).second.isInside((uint64_t)dst, len))
				{
					// dst is in committed area
					PRINTF("\nWarning : [Pool] DST area[%lx, %lx] exists in this Pool.\n", (uint64_t)dst, (uint64_t)dst + len);
					return false;
				}
			}

			// add destination info
			CommitInfo dstInfo = srcInfo;
			dstInfo.m_start = dst;
			dstInfo.m_size = len;

			m_commitInfo.insert(std::make_pair(dst, dstInfo));

			return true;
		}
	};

	// Pooled memory array
	static std::vector<PooledMemory> s_pool;

	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	// Initialize AMM mapper
	int initializeMapper(const MemoryOption	*pOption)
	{
		(void)pOption;
		// Initialize once
#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		if (s_initialized) return SCE_OK;

		// Get the range of virtual address space managed by the AMM explicitly.
		Ampr::AmmVirtualAddressRanges vr;
		Ampr::Amm::getVirtualAddressRanges(vr);
		PRINTF("------- SampleUtil::Memory::%s ---------------\n", __FUNCTION__);
		PRINTF("[AMM] All VA range :   [%lx-%lx] / multimap[%lx-%lx]\n", vr.vaStart, vr.vaEnd, vr.multimapVaStart, vr.multimapVaEnd);

		int ret = SCE_OK;

		MemoryOption option;
		if (pOption != nullptr)
		{
			// Apply option from Applicaiont
			option = *pOption;

			// Check range
			SCE_SAMPLE_UTIL_ASSERT(option.m_vaStart >= vr.vaStart);
			if (option.m_vaStart < vr.vaStart)
			{
				// vaStart is out of range
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			SCE_SAMPLE_UTIL_ASSERT(option.m_vaEnd <= vr.vaEnd);
			if (option.m_vaEnd > vr.vaEnd)
			{
				// vaEnd is out of range
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			SCE_SAMPLE_UTIL_ASSERT(option.m_autoDirectMemorySize + option.m_directDirectMemorySize <= vr.vaEnd - vr.vaStart);
			if (option.m_autoDirectMemorySize + option.m_directDirectMemorySize > vr.vaEnd - vr.vaStart)
			{
				// Total size is over the size limit
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			SCE_SAMPLE_UTIL_ASSERT(option.m_vaStart + option.m_autoDirectMemorySize + option.m_directDirectMemorySize <= vr.vaEnd);
			if (option.m_vaStart + option.m_autoDirectMemorySize + option.m_directDirectMemorySize > vr.vaEnd)
			{
				// Calculated va end is out of range, so vaStart or total size is invalid
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
		}
		else
		{
			// Apply default range
			option.m_vaStart = vr.vaStart;
			option.m_vaEnd = vr.vaEnd;
			option.m_autoDirectMemorySize = kSizeDefaultAutoArea;
			option.m_directDirectMemorySize = kSizeDefaultDirectArea;
		}
		off_t dmemOffset;

		// Make DMEM pages managed by the AMM.
		// Here we create 2 regions. One for Auto mapping, Another for Direct mapping.
		ret = Ampr::Amm::giveDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, option.m_autoDirectMemorySize, kAlignAutoArea, Ampr::Amm::Usage::kAuto, &dmemOffset);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			if (ret == SCE_KERNEL_ERROR_EFAULT) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		ret = Ampr::Amm::giveDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, option.m_directDirectMemorySize, kAlignDirectArea, Ampr::Amm::Usage::kDirect, &s_offsDirect);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			if (ret == SCE_KERNEL_ERROR_EFAULT) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		// Manage VA in Auto-area
		s_heapAmm = new HeapAllocator((void*)option.m_vaStart, option.m_vaEnd - option.m_vaStart, "AMM reserved virtual space", true);
		SCE_SAMPLE_UTIL_ASSERT(s_heapAmm != nullptr);
		if (s_heapAmm == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		PRINTF("[AMM] SampleUtil uses: [%lx-%lx]\n", option.m_vaStart, option.m_vaEnd);
		PRINTF("------------------------------------------------------------\n");

		s_initialized = true;
#endif
		return SCE_OK;
	}
#if !SCE_SAMPLE_UTIL_DISABLE_AMPR

	// Preapare AMM command buffer.
	// Call this before issue commands.
	static int prepareAmm(Ampr::AmmCommandBuffer &cb, SceKernelEqueue &eq)
	{
		int ret; (void)ret;

		// Creating equeue receiving events from the AMM,
		// and add the pair of filter and id to used by AMM to the queue.
		ret = sceKernelCreateEqueue(&eq, "sce::SampleUtil::Memory::Amm");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		ret = sceKernelAddAmprEvent(eq, kEQ_ID_AMM, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		// clear buffer
		memset(s_cmdBuffer, 0, sizeof(s_cmdBuffer));

		// Construct a command buffer object
		// Command buffer must be placed on the memory with AMPR_READ flag.
		ret = sceKernelMprotect(s_cmdBuffer, kSizeCommandBuffer, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_AMPR_READ);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		ret = cb.setBuffer(s_cmdBuffer, kSizeCommandBuffer);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		// erase commands left in the buffer.
		ret = cb.reset();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			if (ret == SCE_KERNEL_ERROR_EBUSY) return SCE_SAMPLE_UTIL_ERROR_BUSY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		return SCE_OK;
	}

	// Submit AMM command and wait done.
	// Call this after issued commands.
	static int submitAndWait(Ampr::AmmCommandBuffer &cb, SceKernelEqueue &eq)
	{
		int ret = SCE_OK; (void)ret;

		// Command 
		ret = cb.writeKernelEventQueueOnCompletion(eq, kEQ_ID_AMM, 0x0);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EBUSY) return SCE_SAMPLE_UTIL_ERROR_BUSY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		// submit the command.
		ret = Ampr::Amm::submitCommandBuffer(&cb, Ampr::Amm::Priority::kHigh);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		// Wait
		SceKernelEvent ev;
		int num;
		ret = sceKernelWaitEqueue(eq, &ev, 1, &num, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			if (ret == SCE_KERNEL_ERROR_ETIMEDOUT) return SCE_SAMPLE_UTIL_ERROR_TIMEDOUT;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		// erase commands left in the buffer.
		ret = cb.reset();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EBUSY) return SCE_SAMPLE_UTIL_ERROR_BUSY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		return SCE_OK;
	}

	// Map AMM auto memory (internal use)
	static int	mapDirectMemoryInternal(void **outVa, size_t len, int memoryType, int prot, int flags, size_t alignment, Ampr::AmmCommandBuffer &cb)
	{
		int ret; (void)ret;

		SCE_SAMPLE_UTIL_ASSERT(outVa != nullptr);
		// Search VA
		void *va = nullptr;
		if (*outVa == nullptr)
		{
			va = s_heapAmm->allocate(PAGE_SIZE_ALIGN(len), PAGE_SIZE_ALIGN(alignment));
			SCE_SAMPLE_UTIL_ASSERT(va != nullptr);
			if (va == nullptr)
			{
				return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			}
		} else {
			va = *outVa;
		}

		// map to Auto-area
		ret = cb.map((uint64_t)va, PAGE_SIZE_ALIGN(len), memoryType, prot);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EINVAL)  return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		PRINTF("[AMM] map       : [0x%016lx - 0x%016zx] Type=%d, Prot=0x%08x\n", (uint64_t)va, (uint64_t)va + PAGE_SIZE_ALIGN(len) - 1, memoryType, prot);

		*outVa = va;

		return SCE_OK;
	}
#else
	static int s_memCounter = 0;
#endif

	int	reserveVirtualRange(void	**addr, size_t	len, int	flags, size_t	alignment)
	{
		int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		void *va = s_heapAmm->allocate(PAGE_SIZE_ALIGN(len), PAGE_SIZE_ALIGN(alignment));
		SCE_SAMPLE_UTIL_ASSERT(va != nullptr);
		if (va == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		if (addr != nullptr)
		{
			*addr = va;
		}
#else
		ret = sceKernelReserveVirtualRange(addr, len, flags, alignment);
#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatReserveVirtualRange(addr, len, flags, alignment);
		}
#endif
#endif

		return ret;
	}

	// Mapping Auto-area (Caution: Different from FunctionName "direct")
	int	mapDirectMemory(void **addr, size_t len, int memoryType, int prot, int flags, size_t alignment)
	{
		int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		// declaration amm param
		DECLARE_AMM_PARAM();

		// prepare for auto
		ret = prepareAmm(cb, eq);
		if (ret != SCE_OK)
		{
			return ret;
		}

		// map main
		void *va = nullptr;
		ret = mapDirectMemoryInternal(&va, len, memoryType, prot, flags, alignment, cb);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			return ret;
		}

		// submit command and wait finish
		ret = submitAndWait(cb, eq);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			return ret;
		}
		ret = sceKernelDeleteEqueue(eq);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		// return mapped address
		if (addr) {
			*addr = va;
		}
#else
		off_t pa;
		ret = sceKernelAllocateMainDirectMemory(len, alignment, memoryType, &pa);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EAGAIN) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		ret = sceKernelMapDirectMemory2(addr, len, memoryType, prot, flags, pa, alignment);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EBUSY) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		ret = sceKernelSetVirtualRangeName(*addr, len, ("sce::SampleUtil::Memory#" + std::to_string(s_memCounter++)).c_str());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatAllocPhysicalMemory(pa, len, alignment, memoryType);
			sceMatMapDirectMemory(*addr, len, prot, flags, pa, alignment, memoryType);
			sceMatTagVirtualMemory(*addr, len, ("sce::SampleUtil::Memory#" + std::to_string(s_memCounter++)).c_str());
		}
#endif
#endif

		return SCE_OK;
	}

	// Mapping Direct-area (only for VideoOut)
	int	mapVideoOutMemory(void **addr, off_t offset, size_t len, int memoryType, int prot, int flags, size_t alignment)
	{
		int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		// declaration amm param
		DECLARE_AMM_PARAM();

		// prepare for auto
		ret = prepareAmm(cb, eq);
		if (ret != SCE_OK)
		{
			return ret;
		}

		// Search VA
		void *va = s_heapAmm->allocate(PAGE_SIZE_ALIGN(len), PAGE_SIZE_ALIGN(alignment));
		SCE_SAMPLE_UTIL_ASSERT(va != nullptr);
		if (va == nullptr)
		{
			ret = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}

		// map to Direct-area
		ret = cb.mapDirect((uint64_t)va, s_offsDirect + offset, len, memoryType, prot);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			if (ret == SCE_KERNEL_ERROR_EINVAL)  return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		PRINTF("[AMM] mapDirect : [0x%016lx - 0x%016zx] Type=%d, Prot=0x%08x, Offs=0x%016lx\n", (uint64_t)va, (uint64_t)va + PAGE_SIZE_ALIGN(len) - 1, memoryType, prot, s_offsDirect + offset);

		// submit command and wait finish
		ret = submitAndWait(cb, eq);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			return ret;
		}
		ret = sceKernelDeleteEqueue(eq);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		// return mapped address
		if (addr) {
			*addr = va;
		}
#else
		off_t pa;
		ret = sceKernelAllocateMainDirectMemory(len, alignment, memoryType, &pa);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EAGAIN) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		ret = sceKernelMapDirectMemory2(addr, len, memoryType, prot, flags, pa, alignment);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EBUSY) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		ret = sceKernelSetVirtualRangeName(*addr, len, ("sce::SampleUtil::Memory#" + std::to_string(s_memCounter++)).c_str());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatAllocPhysicalMemory(pa, len, alignment, memoryType);
			sceMatMapDirectMemory(*addr, len, prot, flags, pa, alignment, memoryType);
			sceMatTagVirtualMemory(*addr, len, ("sce::SampleUtil::Memory#" + std::to_string(s_memCounter++)).c_str());
		}
#endif
#endif

		return SCE_OK;
	}
#if !SCE_SAMPLE_UTIL_DISABLE_AMPR

	// Unmap AMM memory (internal use)
	static int munmapInternal(void *addr, size_t len, Ampr::AmmCommandBuffer &cb)
	{
		int ret; (void)ret;

		// unmap addr
		ret = cb.unmap((uint64_t)addr, PAGE_SIZE_ALIGN(len));
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EINVAL)  return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		PRINTF("[AMM] ummap     : [0x%016lx - 0x%016zx]\n", (uint64_t)addr, (uint64_t)addr + PAGE_SIZE_ALIGN(len) - 1);

		// release area
		s_heapAmm->free(addr);

		return SCE_OK;
	}
#endif
	// Unmap AMM memory
	int	munmap(void *addr, size_t len)
	{
		int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		// declaration amm param
		DECLARE_AMM_PARAM();

		// prepare for auto
		ret = prepareAmm(cb, eq);
		if (ret != SCE_OK)
		{
			return ret;
		}

		// unmap main
		ret = munmapInternal(addr, len, cb);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			return ret;
		}

		// submit command and wait finish
		ret = submitAndWait(cb, eq);
		if (ret != SCE_OK)
		{
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			return ret;
		}

		ret = sceKernelDeleteEqueue(eq);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#else
		SceKernelVirtualQueryInfo info;
		ret = sceKernelVirtualQuery(addr, 0, &info, sizeof(info));
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EACCES) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		ret = sceKernelMunmap(addr, len);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		ret = sceKernelReleaseDirectMemory(info.offset, len);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatUnmapMemory(addr, len);
			sceMatReleasePhysicalMemory(info.offset, len);
		}
#endif
#endif

		return SCE_OK;
	}

	// Batching AMM commands
	int	batchMap(BatchMapEntry *entries, int numberOfEntries, int *numberOfEntriesOut, int flags)
	{
		int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		// declaration amm param
		DECLARE_AMM_PARAM();

		// prepare
		ret = prepareAmm(cb, eq);
		if (ret != SCE_OK)
		{
			return ret;
		}

		// for keeping address which will be mapped by batch

		PRINTF("[AMM] batch start\n");

		int count = 0;
		for (int i = 0; i < numberOfEntries; i++)
		{
			PRINTF("[AMM] batch %02d >> ", i);

			BatchMapEntry batch = entries[i];
			const auto op = (SampleUtil::Memory::BatchMapEntry::Operation)batch.m_operation;
			switch (op)
			{
			case SampleUtil::Memory::BatchMapEntry::Operation::kMapDirect:
				{
					SCE_SAMPLE_UTIL_ASSERT(batch.m_start != nullptr);
					void *va = batch.m_start;
					ret = mapDirectMemoryInternal(&va, batch.m_length, batch.m_type, batch.m_protection, 0, SCE_KERNEL_PAGE_SIZE, cb);
					if (ret != SCE_OK)
					{
						int ret2 = sceKernelDeleteEqueue(eq);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
						return ret;
					}
					count++;
				}
				break;

			case SampleUtil::Memory::BatchMapEntry::Operation::kUnamp:
				{
					SCE_SAMPLE_UTIL_ASSERT(batch.m_start != nullptr);
					if (batch.m_start != nullptr)
					{
						// if specified address is null, use address mapped by previous batch
						ret = munmapInternal(batch.m_start, batch.m_length, cb);
						if (ret != SCE_OK)
						{
							int ret2 = sceKernelDeleteEqueue(eq);
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
							return ret;
						}
						count++;
					}
				}
				break;

			case SampleUtil::Memory::BatchMapEntry::Operation::kProtect:
				{
					SCE_SAMPLE_UTIL_ASSERT(batch.m_start != nullptr);
					if (batch.m_start != nullptr)
					{
						// if specified address is null, use address mapped by previous batch
						ret = cb.modifyProtect((uint64_t)batch.m_start, PAGE_SIZE_ALIGN(batch.m_length), batch.m_protection, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
						if (ret != SCE_OK)
						{
							int ret2 = sceKernelDeleteEqueue(eq);
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
							if (ret == SCE_KERNEL_ERROR_EINVAL)  return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
							return SCE_SAMPLE_UTIL_ERROR_FATAL;
						}
						PRINTF("[AMM] modifyPro : (0x%016lx - 0x%016lx) :: Changed Protect 0x%08x\n", (uint64_t)batch.m_start, (uint64_t)batch.m_start + PAGE_SIZE_ALIGN(batch.m_length) - 1, batch.m_protection);
						count++;
					}
				}
				break;

			case SampleUtil::Memory::BatchMapEntry::Operation::kTypeProtect:
				{
					SCE_SAMPLE_UTIL_ASSERT(batch.m_start != nullptr);
					if (batch.m_start != nullptr)
					{
						// if specified address is null, use address mapped by previous batch
						ret = cb.modifyMtypeProtect((uint64_t)batch.m_start, PAGE_SIZE_ALIGN(batch.m_length), batch.m_type, batch.m_protection, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
						if (ret != SCE_OK)
						{
							int ret2 = sceKernelDeleteEqueue(eq);
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
							if (ret == SCE_KERNEL_ERROR_EINVAL)  return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
							return SCE_SAMPLE_UTIL_ERROR_FATAL;
						}
						PRINTF("[AMM] modifyMtP : (0x%016lx - 0x%016lx) :: Changed Type %d, Prot 0x%08x\n", (uint64_t)batch.m_start, (uint64_t)batch.m_start + PAGE_SIZE_ALIGN(batch.m_length) - 1, batch.m_type, batch.m_protection);
						count++;
					}
				}
				break;

			default:
				SCE_SAMPLE_UTIL_ASSERT(false);
				sceKernelDeleteEqueue(eq);
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
		}

		PRINTF("[AMM] batch end\n");

		// submit command and wait finish
		ret = submitAndWait(cb, eq);
		int ret2 = sceKernelDeleteEqueue(eq);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
		if (ret != SCE_OK)
		{
			return ret;
		}

		// numbers of executed commands
		if (numberOfEntriesOut)
		{
			*numberOfEntriesOut = count;
		}
#else
		auto *_entries = (SceKernelBatchMapEntry *)entries;
		for (int i = 0; i < numberOfEntries; i++)
		{
			if (entries[i].m_operation == (int)BatchMapEntry::Operation::kMapDirect)
			{
				ret = sceKernelAllocateMainDirectMemory(entries[i].m_length, 2 * 1024 * 1024, entries[i].m_type, &_entries[i].offset);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK)
				{
					if (ret == SCE_KERNEL_ERROR_EAGAIN) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
					return SCE_SAMPLE_UTIL_ERROR_FATAL;
				}
#ifdef _DEBUG
				if (Memory::g_isMatInitialized)
				{
					sceMatAllocPhysicalMemory(_entries[i].offset, entries[i].m_length, 2 * 1024 * 1024, entries[i].m_type);
				}
#endif
			}
		}
		ret = sceKernelBatchMap2(_entries, numberOfEntries, numberOfEntriesOut, flags);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatBatchMap(_entries, numberOfEntries, flags);
		}
#endif
#endif

		return SCE_OK;
	}

	// Pooled memory
	namespace Pool
	{
		// Reserve AMM auto area as "Pooled Memory"
		int	reserve(void *addrIn, size_t len, size_t alignment, int flags, void **addrOut)
		{
#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
			(void)addrIn;	// unused
			(void)flags;

			// reserve = allocate from amm heap but do not map
			const size_t kPageSize2MB = 1u << 21;
			void *va = s_heapAmm->allocate(len, (alignment != 0) ? alignment : kPageSize2MB);

			// regist pool
			PooledMemory pool;
			pool.init(va, len);
			s_pool.push_back(pool);

			if (addrOut != nullptr)
			{
				*addrOut = va;
			}

			PRINTF("[Pool] reserve  : [0x%016lx - 0x%016lx] (0x%016zx)\n", (uint64_t)va, (uint64_t)va + len - 1, len);
#else
			int ret = SCE_OK;
			ret = sceKernelMemoryPoolReserve(addrIn, len, alignment, flags, addrOut);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatMemoryPoolReserve(*addrOut, len, alignment, flags);
			}
#endif
#endif

			return SCE_OK;
		}

		// Ummap reserved Pooled Memory area
		int munmap(void *addrIn, size_t len)
		{
#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
			for(auto it = s_pool.begin(); it != s_pool.end(); it++)
			{
				// Search target pool
				if (addrIn == it->m_mapStart)
				{
					int ret; (void)ret;

					// declaration amm param
					DECLARE_AMM_PARAM();

					// prepare for auto
					ret = prepareAmm(cb, eq);
					if (ret != SCE_OK)
					{
						return ret;
					}

					// unmap all nodes in pool
					for (auto node : it->m_commitInfo)
					{
						munmapInternal(node.second.m_start, node.second.m_size, cb);
					}
					it->m_commitInfo.clear();

					// submit command and wait finish
					ret = submitAndWait(cb, eq);
					int ret2 = sceKernelDeleteEqueue(eq);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
					if (ret != SCE_OK)
					{
						return ret;
					}

					// remove pool
					s_pool.erase(it);
					break;
				}
			}
#else
			int ret = SCE_OK;

			SceKernelVirtualQueryInfo info;
			ret = sceKernelVirtualQuery(addrIn, 0, &info, sizeof(info));
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				if (ret == SCE_KERNEL_ERROR_EACCES) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}

			ret = sceKernelMunmap(addrIn, len);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}

			ret = sceKernelReleaseDirectMemory(info.offset, len);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatUnmapMemory(addrIn, len);
				sceMatReleasePhysicalMemory(info.offset, len);
			}
#endif
#endif

			return SCE_OK;
		}

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		// Commit virtual address area (internal use)
		static int commitInternal(void *vaAddr, size_t len, int type, int prot, int flags, Ampr::AmmCommandBuffer &cb, SceKernelEqueue &eq)
		{
			(void)flags;

			for (auto &pool : s_pool)
			{
				if (pool.isInside((uint64_t)vaAddr, len))
				{
					int ret; (void)ret;

					if (!pool.checkCommit(vaAddr, len))
					{
						// already commited, overlapped area ...
						SCE_SAMPLE_UTIL_ASSERT(false);
						return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
					}

					// map to Auto-area
					ret = cb.map((uint64_t)vaAddr, len, type, prot);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					if (ret != SCE_OK)
					{
						if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
						return SCE_SAMPLE_UTIL_ERROR_FATAL;
					}

					// Reflect commit to PooledMemorty
					pool.postCommit(vaAddr, len, type, prot);

					PRINTF("[Pool] commit   : [0x%016lx - 0x%016zx] Type=%d, Prot=0x%08x\n", (uint64_t)vaAddr, (uint64_t)vaAddr + len - 1, type, prot);

					// finish commit
					return SCE_OK;
				}
			}
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

#endif

		// Commit virtual address area
		int	commit(void *vaAddr, size_t len, int type, int prot, int flags)
		{
			int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
			// declaration amm param
			DECLARE_AMM_PARAM();

			// prepare for auto
			ret = prepareAmm(cb, eq);
			if (ret != SCE_OK)
			{
				return ret;
			}

			// check in each pool
			ret = commitInternal(vaAddr, len, type, prot, flags, cb, eq);
			if (ret != SCE_OK)
			{
				int ret2 = sceKernelDeleteEqueue(eq);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
				return ret;
			}

			// submit command and wait finish
			ret = submitAndWait(cb, eq);
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
#else
			ret = sceKernelMemoryPoolCommit(vaAddr, len, type, prot, flags);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				if (ret == SCE_KERNEL_ERROR_EBUSY) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatMemoryPoolCommit(vaAddr, len, type, prot, flags);
			}
#endif
#endif

			return SCE_OK;
		}

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		// Decommit virtual address area (internal use)
		static int decomitInternal(void *addr, size_t len, int flags, Ampr::AmmCommandBuffer &cb, SceKernelEqueue &eq)
		{
			(void)flags;

			for (auto &pool : s_pool)
			{
				if (pool.isInside((uint64_t)addr, len))
				{
					int ret; (void)ret;

					for (auto iter = pool.m_commitInfo.begin(); iter != pool.m_commitInfo.end();)
					{
						CommitInfo info = (*iter).second;
						if (info.isOverlap((uintptr_t)addr, len))
						{
							// erase old commit info
							iter = pool.m_commitInfo.erase(iter);
							// unmap addr
							uintptr_t unmapMemStart = std::max((uintptr_t)addr, (uintptr_t)info.m_start);
							uintptr_t unmapMemEnd = std::min((uintptr_t)addr + len, (uintptr_t)info.m_start + info.m_size);

							ret = cb.unmap(unmapMemStart, unmapMemEnd - unmapMemStart);
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
							if (ret != SCE_OK)
							{
								if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
								return SCE_SAMPLE_UTIL_ERROR_FATAL;
							}

							// add updated commit Info if needed
							if ((uintptr_t)info.m_start < unmapMemStart)
							{
								CommitInfo newInfo = info;
								newInfo.m_size = unmapMemStart - (uintptr_t)info.m_start;
								pool.m_commitInfo.insert(std::make_pair(newInfo.m_start, newInfo));
							}
							if (unmapMemEnd < (uintptr_t)info.m_start + info.m_size)
							{
								CommitInfo newInfo = info;
								newInfo.m_start = (void *)unmapMemEnd;
								newInfo.m_size = (uintptr_t)info.m_start + info.m_size - unmapMemEnd;
								pool.m_commitInfo.insert(std::make_pair(newInfo.m_start, newInfo));
							}
						} else {
							iter++;
						}
					}

					PRINTF("[Pool] decommit : [0x%016lx - 0x%016zx]\n", (uint64_t)addr, (uint64_t)addr + len - 1);

					// finish decommit
					return SCE_OK;
				}
			}
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
#endif

		// Decommit virtual address area
		int	decommit(void *addr, size_t len, int flags)
		{
			int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
			// declaration amm param
			DECLARE_AMM_PARAM();

			// prepare for auto
			ret = prepareAmm(cb, eq);
			if (ret != SCE_OK)
			{
				return ret;
			}

			ret = decomitInternal(addr, len, flags, cb, eq);
			if (ret != SCE_OK)
			{
				int ret2 = sceKernelDeleteEqueue(eq);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
				return ret;
			}

			// submit command and wait finish
			ret = submitAndWait(cb, eq);
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
#else
			ret = sceKernelMemoryPoolDecommit(addr, len, flags);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatMemoryPoolDecommit(addr, len, flags);
			}
#endif
#endif

			return SCE_OK;
		}

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
		// Move data on virtual area (internal use)
		static int moveInternal(void *dst, void *src, size_t len, int flags, Ampr::AmmCommandBuffer &cb, SceKernelEqueue &eq)
		{
			for (auto &pool : s_pool)
			{
				if (pool.isInside((uint64_t)dst, len))
				{
					int ret; (void)ret;

					CommitInfo	srcInfo;
					if (!pool.move(dst, src, len, srcInfo))
					{
						SCE_SAMPLE_UTIL_ASSERT(false);
						return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
					}

					// remap VA
					ret = cb.remap((uint64_t)dst, (uint64_t)src, len, srcInfo.m_prot);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					if (ret != SCE_OK)
					{
						if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
						return SCE_SAMPLE_UTIL_ERROR_FATAL;
					}

					return SCE_OK;
				}
			}
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
#endif
		// Move specified virtual area data
		int	move(void *dst, void *src, size_t len, int flags)
		{
			int ret = SCE_OK; (void)ret;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
			// declaration amm param
			DECLARE_AMM_PARAM();

			// prepare for auto
			ret = prepareAmm(cb, eq);
			if (ret != SCE_OK)
			{
				return ret;
			}

			ret = moveInternal(dst, src, len, flags, cb, eq);
			if (ret != SCE_OK)
			{
				int ret2 = sceKernelDeleteEqueue(eq);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
				return ret;
			}

			// submit command and wait finish
			ret = submitAndWait(cb, eq);
			int ret2 = sceKernelDeleteEqueue(eq);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
#else
			ret = sceKernelMemoryPoolMove(dst, src, len, flags);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				if (ret == SCE_KERNEL_ERROR_EBUSY) return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				if (ret == SCE_KERNEL_ERROR_ENOMEM) return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatMemoryPoolMove(dst, src, len, flags);
			}
#endif
#endif

			return SCE_OK;
		}

		// Batching commands for Pooled memory
		int	batch(const BatchEntry *entries, int n, int *indexOut, int flags)
		{
			int ret = SCE_OK;

#if !SCE_SAMPLE_UTIL_DISABLE_AMPR
			// declaration amm param
			DECLARE_AMM_PARAM();

			// prepare for auto
			ret = prepareAmm(cb, eq);
			if (ret != SCE_OK)
			{
				return ret;
			}

			PRINTF("[Pool] batch start\n");

			bool needToSubmit = false;
			for (int i = 0; i < n; i++)
			{
				PRINTF("[Pool] batch %02d >> ", i);

				BatchEntry batch = entries[i];
				const auto op = (SampleUtil::Memory::Pool::BatchEntry::Operation)batch.m_op;
				switch (op)
				{
				case SampleUtil::Memory::Pool::BatchEntry::Operation::kCommit:
					{
						ret = commitInternal(batch.m_commit.m_addr, batch.m_commit.m_len, batch.m_commit.m_type, batch.m_commit.m_prot, batch.m_flags, cb, eq);
						if (ret == SCE_OK)
						{
							needToSubmit = true;
						}
					}
					break;
				case SampleUtil::Memory::Pool::BatchEntry::Operation::kDecommit:
					{
						ret = decomitInternal(batch.m_decommit.m_addr, batch.m_decommit.m_len, batch.m_flags, cb, eq);
						if (ret == SCE_OK)
						{
							needToSubmit = true;
						}
					}
					break;
				case SampleUtil::Memory::Pool::BatchEntry::Operation::kProtect:
					{
						ret = cb.modifyProtect((uint64_t)batch.m_protect.m_addr, batch.m_protect.m_len, batch.m_protect.m_prot, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
						if (ret != SCE_OK)
						{
							int ret2 = sceKernelDeleteEqueue(eq);
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
							if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
							return SCE_SAMPLE_UTIL_ERROR_FATAL;
						}
						PRINTF("[Pool] modifyPr : (0x%016lx - 0x%016lx) Changed Protect 0x%08x\n", (uint64_t)batch.m_protect.m_addr, (uint64_t)batch.m_protect.m_addr + batch.m_protect.m_len - 1, batch.m_protect.m_prot);
						needToSubmit = true;
					}
					break;
				case SampleUtil::Memory::Pool::BatchEntry::Operation::kTypeProtect:
					{
						ret = cb.modifyMtypeProtect((uint64_t)batch.m_typeProtect.m_addr, batch.m_typeProtect.m_len, batch.m_typeProtect.m_type, batch.m_typeProtect.m_prot, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
						if (ret != SCE_OK)
						{
							int ret2 = sceKernelDeleteEqueue(eq);
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
							if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
							return SCE_SAMPLE_UTIL_ERROR_FATAL;
						}
						PRINTF("[Pool] modifyMP : (0x%016lx - 0x%016lx) Changed Type %d, Prot 0x%08x\n", (uint64_t)batch.m_typeProtect.m_addr, (uint64_t)batch.m_typeProtect.m_addr + batch.m_typeProtect.m_len - 1, batch.m_typeProtect.m_type, batch.m_typeProtect.m_prot);
						needToSubmit = true;
					}
					break;
				case SampleUtil::Memory::Pool::BatchEntry::Operation::kMove:
					{
						ret = moveInternal(batch.m_move.m_dst, batch.m_move.m_src, batch.m_move.m_len, batch.m_flags, cb, eq);
					}
					break;
				default:
					sceKernelDeleteEqueue(eq);
					SCE_SAMPLE_UTIL_ASSERT(false);
					return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				}
			}
			PRINTF("[Pool] batch end\n");

			// submit command and wait finish
			if (needToSubmit)
			{
				ret = submitAndWait(cb, eq);
				int ret2 = sceKernelDeleteEqueue(eq);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret2, SCE_OK);
				if (ret != SCE_OK)
				{
					return ret;
				}
			}
#else
			ret = sceKernelMemoryPoolBatch((SceKernelMemoryPoolBatchEntry*)entries, n, indexOut, flags);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				if (ret == SCE_KERNEL_ERROR_EINVAL) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				if (ret == SCE_KERNEL_ERROR_EOPNOTSUPP) return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatMemoryPoolBatch((SceKernelMemoryPoolBatchEntry*)entries, *indexOut, flags);
			}
#endif
#endif

			return SCE_OK;
		}

	}	// namespace Pool
}}} // namespace sce::SampleUtil::Memory

#endif // _SCE_TARGET_OS_PROSPERO
