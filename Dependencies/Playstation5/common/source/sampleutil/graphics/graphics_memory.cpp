/* SIE CONFIDENTIAL
 PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#include <sanitizer/asan_interface.h>
#include <mat.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core/sync.h>
#include "sampleutil/graphics/platform_agc/link_libraries_agc.h"
#endif
#include "sampleutil/graphics/compat.h"
#include "sampleutil/memory.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/debug/perf.h"

namespace sce { namespace SampleUtil { namespace Graphics {

void	VideoAllocator::freeHook(void	*ptr, size_t	size)
{
	if (ptr != nullptr && g_isResourceRegistrationInitialized)
	{
		unregisterResource(ptr);
	}
}

void	VideoRingAllocator::beginFrame(Compat::DrawCommandBuffer	&dcb)
{
#if _SCE_TARGET_OS_PROSPERO
	dcb.acquireMem(
		sce::Agc::AcquireMemEngine::kPfp,
		sce::Agc::AcquireMemCbDbOperation::kNone,
		sce::Agc::AcquireMemGcrCntl::kGl0ScalarInvalidate |
		sce::Agc::AcquireMemGcrCntl::kGl1Invalidate);
#endif
#if _SCE_TARGET_OS_ORBIS
	(void)dcb;
#endif
	m_frameInfos.emplace_back();
	RingAllocator::beginFrame();
}

void	VideoRingAllocator::endFrame(Compat::DrawCommandBuffer	&dcb)
{
#if _SCE_TARGET_OS_PROSPERO
	dcb.acquireMem(
		sce::Agc::AcquireMemEngine::kMe, // This needs to be at the end of the CP pipeline
		sce::Agc::AcquireMemCbDbOperation::kNone,
		sce::Agc::AcquireMemGcrCntl::kOrder012 | // Ensure dirty lines have been written from the GL0S to the GL2.
		sce::Agc::AcquireMemGcrCntl::kGl2Writeback);

	const size_t gl2CacheLineSize = 128;
	Agc::Label *pLabel = reinterpret_cast<Agc::Label*>(allocate(gl2CacheLineSize, gl2CacheLineSize));
	SCE_SAMPLE_UTIL_ASSERT(pLabel != nullptr);
	pLabel->m_value = 0ul;

	dcb.queueEndOfPipeAction(
		sce::Agc::EndOfPipeCbDbAction::kNone,
		sce::Agc::GcrCntl::kNone,
		sce::Agc::CachePolicy::kBypass,
		pLabel,
		sce::Agc::EndOfPipeData::kImmediate64b,
		1u);
#endif
#if _SCE_TARGET_OS_ORBIS
	uint64_t *pLabel = reinterpret_cast<uint64_t *>(allocate(sizeof(uint64_t), 8));
	SCE_SAMPLE_UTIL_ASSERT(pLabel != nullptr);
	dcb.writeAtEndOfPipe(Gnm::kEopFlushAndInvalidateCbDbCaches, Gnm::kEventWriteDestMemory, pLabel, Gnm::kEventWriteSource64BitsImmediate, 1ul, Gnm::kCacheActionWriteBackAndInvalidateL1andL2, Gnm::kCachePolicyBypass);
	*pLabel = 0ul;
#endif
	m_frameInfos.back().m_label = pLabel;

#if _SCE_TARGET_OS_PROSPERO
	volatile uint64_t *label = &reinterpret_cast<Agc::Label*>(m_frameInfos.front().m_label)->m_value;
#endif
#if _SCE_TARGET_OS_ORBIS
	uint64_t *label = reinterpret_cast<uint64_t *>(m_frameInfos.front().m_label);
#endif
	if (*label != 0)
	{
#ifdef _DEBUG
		unregisterResources(m_frameInfos.front().m_registeredResources);
#endif
		m_frameInfos.pop_front();
		RingAllocator::endFrame();
	}
}

void	VideoRingAllocator::allocateHook(void	*ptr, size_t	size, const std::string	&name)
{
	if (Debug::Perf::m_isInitialized)
	{
		Debug::Perf::tagBuffer(name, ptr, size, 1);
	}
}

void	VideoRingAllocator::freeHook(void	*ptr, size_t	size)
{
	if (ptr != nullptr && Debug::Perf::m_isInitialized) {
		Debug::Perf::unTagBuffer(ptr);
	}
}

bool	VideoRingAllocator::allocateFailedHook()
{
	if (m_frameInfos.size() == 0)
	{
		return false;
	}
#if _SCE_TARGET_OS_PROSPERO
	volatile uint64_t *label = &reinterpret_cast<Agc::Label*>(m_frameInfos.front().m_label)->m_value;
#endif
#if _SCE_TARGET_OS_ORBIS
	uint64_t *label = reinterpret_cast<uint64_t *>(m_frameInfos.front().m_label);
#endif
	while (*label == 0)
	{
		sceKernelUsleep(100);
	}
#ifdef _DEBUG
	unregisterResources(m_frameInfos.front().m_registeredResources);
#endif
	m_frameInfos.pop_front();
	RingAllocator::endFrame();

	return true;
}

} } } // namespace sce::SampleUtil::Graphics
