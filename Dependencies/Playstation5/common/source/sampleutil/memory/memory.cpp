/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc.
 * 
 */

#include <cinttypes>
#include <unordered_map>
#include <algorithm>
#include <mat.h>
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/memory.h"

#ifdef _DEBUG
#if _SCE_TARGET_OS_PROSPERO
#pragma comment(lib, "libSceMat_nosubmission_stub_weak.a")
#endif
#if _SCE_TARGET_OS_ORBIS
#pragma comment(lib, "libSceMat_stub_weak.a")
#endif
#endif

namespace
{
	std::unordered_map<const void *, MatGroup>	s_matGroups;
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Memory {

	bool		g_isMatInitialized = false;

	void	registerMatGroup(const void	*pAllocator, const char	*pName)
	{
#ifndef _DEBUG
		(void)pAllocator; (void)pName;
#else
		SCE_SAMPLE_UTIL_ASSERT(s_matGroups.find(pAllocator) == s_matGroups.end());
		// generate new MatGroup and bind it to 'pAllocator'
		MatGroup newMatGroup = s_matGroups.size() + 1;
		s_matGroups.emplace(std::make_pair(pAllocator, newMatGroup));
		sceMatRegisterGroup(newMatGroup, pName, SCEMAT_GROUP_ROOT);
#endif
	}

	MatGroup	getMatGroup(const void	*pAllocator)
	{
		auto iter = s_matGroups.find(pAllocator);
		return (iter == s_matGroups.end()) ? SCEMAT_GROUP_AUTO : iter->second;
	}

	void	initializeMemoryAnalyzer()
	{
		int ret = SCE_OK; (void)ret;

		if (g_isMatInitialized)
		{
			return;
		}
#ifdef _DEBUG
		// initialize memory analyzer
		ret = sceMatInitialize(0);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_MAT_OK);
		if (ret != SCE_MAT_OK)
		{
			return;
		}
#endif
		g_isMatInitialized = true;
	}

	void	finalizeMemoryAnalyzer()
	{
		if (!g_isMatInitialized)
		{
			return;
		}
#ifdef _DEBUG
		// uninitialize memory analyzer
		sceMatUninitialize();
		s_matGroups.clear();
#endif
		g_isMatInitialized = false;
	}

	void *allocDmem(size_t	size, size_t	align, int memType, int prot, const std::string &name)
	{
		int ret = SCE_OK;

		if (!size) {
			return nullptr;
		}

		off_t offsetOut;

		align = (align + 0xffffu) & ~0xffffu;
		size = (size + 0xffffu) & ~0xffffu;

		ret = sceKernelAllocateMainDirectMemory(size, align, memType, &offsetOut);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return nullptr;
		}

		void *ptr = nullptr;
		ret = sceKernelMapNamedDirectMemory(&ptr, size, prot, 0, offsetOut, align, name.c_str());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#ifdef _DEBUG
		if (Memory::g_isMatInitialized) {
			Memory::registerMatGroup(ptr, name.c_str());
			sceMatAlloc(ptr, size, 0, getMatGroup(ptr));
		}
#endif
		return ptr;
	}

	void	freeDmem(void *ptr)
	{
		int ret = SCE_OK; (void)ret;

		SceKernelVirtualQueryInfo info;
		ret = sceKernelVirtualQuery(ptr, 0, &info, sizeof(info));
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return;
		}

		const size_t len = (uintptr_t)info.end - (uintptr_t)info.start;
		ret = sceKernelMunmap(ptr, len);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return;
		}

		ret = sceKernelReleaseDirectMemory(info.offset, len);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return;
		}
#ifdef _DEBUG
		if (Memory::g_isMatInitialized) {
			sceMatFree(ptr);
		}
#endif
	}
}}} // namespace sce::SampleUtil::Memory
