/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */
#include <scebase_common.h>
#include <kernel.h>
#include <razorcpu.h>
#include <scebase_common.h>
#include "sampleutil/memory.h"
#include "sampleutil/debug/perf.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/sampleutil_error.h"

#ifdef _DEBUG
#if _SCE_TARGET_OS_PROSPERO
#pragma comment(lib, "libSceRazorCpu_debug_nosubmission_stub_weak.a")
#endif
#if _SCE_TARGET_OS_ORBIS
#pragma comment(lib, "libSceRazorCpu_debug_stub_weak.a")
#endif
#endif
#pragma comment(lib, "libSceRazorCpu_stub_weak.a")

namespace
{
#ifdef _DEBUG
	int	getThreadInfo(char pThreadName[32], void *&pStackTop, size_t &stackSize)
	{
		int ret = SCE_OK;

		ScePthread thread = scePthreadSelf();
		ScePthreadAttr threadAttr;
		ret = scePthreadAttrInit(&threadAttr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		ret = scePthreadAttrGet(thread, &threadAttr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		ret = scePthreadAttrGetstack(&threadAttr, &pStackTop, &stackSize);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		ret = scePthreadAttrDestroy(&threadAttr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		ret = scePthreadGetname(thread, pThreadName);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		return SCE_OK;
	}

	void	*pRazorCpuDataTagBuffer = nullptr;
#endif
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Debug {
bool																			Perf::m_isInitialized = false;
std::vector<uint32_t>															Perf::m_markerFlags;
bool																			Perf::m_isDataTagEnabled = false;
#if _SCE_TARGET_OS_PROSPERO
std::unordered_map<const Agc::CommandBuffer *, Perf::Context *>					Perf::m_gpuCtxs;
#endif
#if _SCE_TARGET_OS_ORBIS
std::unordered_map<const Gnmx::GnmxGfxContext *, Perf::Context *>				Perf::m_gpuCtxs;
#endif
std::unordered_map<ScePthread, Perf::Context *>									Perf::m_cpuCtxs;

void	Perf::initialize(PerfOption	*option)
{
	int ret = SCE_OK; (void)ret;

	if (m_isInitialized)
	{
		return;
	}

#ifdef _DEBUG
	if (option->useDataTag)
	{
		// initialize Razor CPU memory tags
		size_t tagBufferSize = sceRazorCpuGetDataTagStorageSize(option->maxNumDataTags);
#if _SCE_TARGET_OS_PROSPERO
		pRazorCpuDataTagBuffer = sce::SampleUtil::Memory::allocDmem(tagBufferSize, 0x10000u, SCE_KERNEL_MTYPE_C, SCE_KERNEL_PROT_CPU_RW, "RAZORCPU_MEMORYTAGS");
#endif
#if _SCE_TARGET_OS_ORBIS
		pRazorCpuDataTagBuffer = sce::SampleUtil::Memory::allocDmem(tagBufferSize, 0x10000u, SCE_KERNEL_WB_ONION, SCE_KERNEL_PROT_CPU_RW, "RAZORCPU_MEMORYTAGS");
#endif
		SCE_SAMPLE_UTIL_ASSERT(pRazorCpuDataTagBuffer != nullptr);
		ret = sceRazorCpuInitDataTags(pRazorCpuDataTagBuffer, tagBufferSize);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		// tag thread's stack area
		char pThreadName[32];
		void *pStackTop;
		size_t stackSize;
		ret = getThreadInfo(pThreadName, pStackTop, stackSize);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceRazorCpuTagBuffer(("stack of " + std::string(pThreadName)).c_str(), 0, pStackTop, stackSize);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		m_isDataTagEnabled = true;
	}
#endif

	if (option->useGpuMarker)
	{
		for (const auto &ctx : m_gpuCtxs)
		{
			ctx.second->markerHead.lev = 0;
			ctx.second->markerHead.isPopped = false;
		}
		m_markerFlags.push_back(kGpuMarker);
	}
	if (option->useCpuMarker)
	{
		for (const auto &ctx : m_cpuCtxs)
		{
			ctx.second->markerHead.lev = 0;
			ctx.second->markerHead.isPopped = false;
		}
		m_markerFlags.push_back(kCpuMarker);
	}

	m_isInitialized = true;
}

void	Perf::finalize()
{
	int ret = SCE_OK; (void)ret;

	if (!m_isInitialized)
	{
		return;
	}

#ifdef _DEBUG
	if (m_isDataTagEnabled)
	{
		ret = sceRazorCpuShutdownDataTags();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		sce::SampleUtil::Memory::freeDmem(pRazorCpuDataTagBuffer);
		m_isDataTagEnabled = false;
	}
#endif

	m_isInitialized = false;
}

#if _SCE_TARGET_OS_PROSPERO
Perf::Context	*Perf::getCtx(const Agc::CommandBuffer	*cb)
#endif
#if _SCE_TARGET_OS_ORBIS
Perf::Context	*Perf::getCtx(const Gnmx::GnmxGfxContext	*cb)
#endif
{
	thread_local static struct CtxCache
	{
#if _SCE_TARGET_OS_PROSPERO
		const sce::Agc::CommandBuffer	*m_cb;
#endif
#if _SCE_TARGET_OS_ORBIS
		const sce::Gnmx::GnmxGfxContext	*m_cb;
#endif
		Perf::Context					*m_ctx;
	} ctxCache = { nullptr, nullptr };

	if (ctxCache.m_cb != cb) {
		if (m_gpuCtxs.find(cb) == m_gpuCtxs.end()) {
			m_gpuCtxs.insert(std::make_pair(cb, new Context()));
		}
		ctxCache.m_cb	= cb;
		ctxCache.m_ctx	= m_gpuCtxs[cb];
	}
	return ctxCache.m_ctx;
}

Perf::Context	*Perf::getCtx(ScePthread	self)
{
	thread_local static struct CtxCache
	{
		ScePthread		m_thread;
		Perf::Context	*m_ctx;
	} ctxCache = { nullptr, nullptr };
	if (ctxCache.m_thread != self) {
		if (m_cpuCtxs.find(self) == m_cpuCtxs.end()) {
			m_cpuCtxs.insert(std::make_pair(self, new Context()));
		}
		ctxCache.m_thread	= self;
		ctxCache.m_ctx		= m_cpuCtxs[self];
	}
	return ctxCache.m_ctx;
}

void	Perf::beginTagBuffer(const std::string	&name, void	*pBuffer, size_t	sizeOfBuffer, size_t	numElements)
{
#ifdef _DEBUG
	if (m_isDataTagEnabled)
	{
		int ret = SCE_OK; (void)ret;
		if (numElements == 1)
		{
			ret = sceRazorCpuTagBuffer(name.c_str(), 0, pBuffer, sizeOfBuffer);
		} else {
			ret = sceRazorCpuTagArray(name.c_str(), 0, pBuffer, sizeOfBuffer * numElements, sizeOfBuffer);
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}
#endif // _DEBUG
	++Perf::getCtx(scePthreadSelf())->markerHead.lev;
}

void	Perf::endTagBuffer()
{
	--Perf::getCtx(scePthreadSelf())->markerHead.lev;
}

void	Perf::tagBuffer(const std::string	&name, void	*pBuffer, size_t	sizeOfBuffer, size_t	numElements)
{
	beginTagBuffer(name, pBuffer, sizeOfBuffer, numElements);
	endTagBuffer();
}

void	Perf::beginUnTagBuffer(void	*pBuffer)
{
#ifdef _DEBUG
	if (m_isDataTagEnabled)
	{
		int ret = SCE_OK; (void)ret;
		ret = sceRazorCpuUnTagBuffer(0, pBuffer);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_RAZOR_CPU_ERROR_TAG_NOT_FOUND);
	}
#endif // _DEBUG
	++Perf::getCtx(scePthreadSelf())->markerHead.lev;
}

void	Perf::endUnTagBuffer()
{
	--Perf::getCtx(scePthreadSelf())->markerHead.lev;
}

void	Perf::unTagBuffer(void	*pBuffer)
{
	beginUnTagBuffer(pBuffer);
	endUnTagBuffer();
}

}}} // namespace sce::SampleUtil::Debug