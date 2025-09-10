/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#include <cstdlib> // malloc
#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <gnm/resourceregistration.h>
#include <gnm/error.h>
#include "sampleutil/memory.h"
#include "sampleutil/sampleutil_common.h"

namespace
{
	const uint32_t kPageSize = 64u*1024u;
#ifdef _DEBUG
	void	*s_pResourceRegistrationMemory	= nullptr;
	size_t	s_resourceRegistrationMemorySizeInBytes	= 0;
#endif
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Graphics {

bool	g_isResourceRegistrationInitialized = false;

VideoAllocator::VideoAllocator(size_t size, SceKernelMemoryType type, int	prot, const char	*pName)
{
	initialize(size, type, pName, prot);
}

void	VideoAllocator::initialize(size_t size, SceKernelMemoryType type, const char	*pName, int	prot)
{
	m_start	= 0;
	m_type	= type;

	void	*mapStart = nullptr;
	auto ret = sceKernelAllocateDirectMemory(0,
		SCE_KERNEL_MAIN_DMEM_SIZE,
		size,
		kPageSize,
		m_type,
		&m_start);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	mapStart = nullptr;
	ret = sceKernelMapNamedDirectMemory(&mapStart, size,
		prot,
		0, m_start, kPageSize, pName);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	setInfo(mapStart, size, pName);
#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		const char *pAttrName = "(deprecated)";
		if (type == SCE_KERNEL_WC_GARLIC)
		{
			pAttrName = "(wc garlic)";
		}
		else if (type == SCE_KERNEL_WB_GARLIC)
		{
			pAttrName = "(wb garlic)";
		}
		else if (type == SCE_KERNEL_WB_ONION)
		{
			pAttrName = "(wb onion)";
		}
		ret = Gnm::registerOwner(&m_owner, (std::string(pName) + pAttrName).c_str());
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
	}
#endif
}

VideoAllocator::~VideoAllocator()
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		ret = Gnm::unregisterOwnerAndResources(m_owner);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
	}
#endif
	ret = sceKernelMunmap(mapStart(), size());
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	ret = sceKernelReleaseDirectMemory(m_start, size());
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

void	initializeResourceRegistration(uint32_t	maxOwnersAndResources, uint32_t	maxNameLength)
{
	int ret = SCE_OK; (void)ret;

	if (!g_isResourceRegistrationInitialized) {
#ifdef _DEBUG
		size_t	resourceRegistrationMemorySizeInBytes;
		ret = Gnm::queryResourceRegistrationUserMemoryRequirements(&resourceRegistrationMemorySizeInBytes, maxOwnersAndResources, maxNameLength);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
		if (ret != SCE_OK) {
			return;
		}
		if (s_pResourceRegistrationMemory != nullptr && s_resourceRegistrationMemorySizeInBytes < resourceRegistrationMemorySizeInBytes) {
			// if memory allocated was not large enough, free it
			sce::SampleUtil::Memory::freeDmem(s_pResourceRegistrationMemory);
			s_pResourceRegistrationMemory	= nullptr;
		}
		if (s_pResourceRegistrationMemory == nullptr) {
			s_resourceRegistrationMemorySizeInBytes	= resourceRegistrationMemorySizeInBytes;
			s_pResourceRegistrationMemory	= sce::SampleUtil::Memory::allocDmem(resourceRegistrationMemorySizeInBytes, 0x10000u, SCE_KERNEL_WB_ONION, SCE_KERNEL_PROT_CPU_RW, "GNM_RESOURCE_REGISTRATION");		// This memory will never be freed. Because 'ResourceRegistration' can't be finalized.
			SCE_SAMPLE_UTIL_ASSERT(s_pResourceRegistrationMemory != nullptr);
			if (s_pResourceRegistrationMemory == nullptr) {
				return;
			}
		}
		ret = Gnm::setResourceRegistrationUserMemory(s_pResourceRegistrationMemory, resourceRegistrationMemorySizeInBytes, maxNameLength);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
		if (ret != SCE_OK)
		{
			return;
		}
#endif
		g_isResourceRegistrationInitialized = true;
	}
}

void	finalizeResourceRegistration()
{
	int ret = SCE_OK; (void)ret;

	g_isResourceRegistrationInitialized = false;
}

void	VideoAllocator::registerResource(const void	*ptr, size_t size, const std::string &name, const std::vector<Gnm::ResourceType> &resTypes)
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		for (const auto &resType : resTypes)
		{
			Gnm::ResourceHandle rh;
			ret = Gnm::registerResource(&rh, m_owner, ptr, size, name.c_str(), resType, 0);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
		}
	}
#endif // _DEBUG
}

void	VideoAllocator::unregisterResource(const void	*ptr)
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		std::vector<Gnm::ResourceHandle> resourceHandles;
		Gnm::findResources(ptr, 16, [](Gnm::ResourceHandle resourceHandle, Gnm::OwnerHandle ownerHandle, uint64_t callbackdata)
		{
			std::vector<Gnm::ResourceHandle> *pResourceHandles = reinterpret_cast<std::vector<Gnm::ResourceHandle> *>(callbackdata);
			pResourceHandles->push_back(resourceHandle);
			return SCE_OK;
		}, reinterpret_cast<uintptr_t>(&resourceHandles));
		for (auto resourceHandle : resourceHandles)
		{
			ret = Gnm::unregisterResource(resourceHandle);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
		}
	}
#endif // _DEBUG
}

VideoRingAllocator::VideoRingAllocator(size_t size, SceKernelMemoryType type, const char	*pName):RingAllocator(pName)
{
	initialize(size, type, pName);
}

void	VideoRingAllocator::initialize(size_t size, SceKernelMemoryType type, const char	*pName)
{
	m_size = size;
	m_head = 0;
	m_tail = 0;
	m_type = type;

	m_mapStart = nullptr;
	auto ret = sceKernelAllocateDirectMemory(0,
		SCE_KERNEL_MAIN_DMEM_SIZE,
		size,
		kPageSize,
		m_type,
		&m_start);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	m_mapStart = nullptr;
	ret = sceKernelMapNamedDirectMemory(&m_mapStart, size,
		SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW,
		0, m_start, kPageSize, pName);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		const char *pAttrName = "(deprecated)";
		if (type == SCE_KERNEL_WC_GARLIC)
		{
			pAttrName = "(wc garlic)";
		}
		else if (type == SCE_KERNEL_WB_GARLIC)
		{
			pAttrName = "(wb garlic)";
		}
		else if (type == SCE_KERNEL_WB_ONION)
		{
			pAttrName = "(wb onion)";
		}
		ret = Gnm::registerOwner(&m_owner, (std::string(pName) + pAttrName).c_str());
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
	}
#endif
}

VideoRingAllocator::~VideoRingAllocator()
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		ret = Gnm::unregisterOwnerAndResources(m_owner);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
	}
#endif
	ret = sceKernelMunmap(m_mapStart, m_size);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	ret = sceKernelReleaseDirectMemory(m_start, m_size);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

void	VideoRingAllocator::registerResource(const void	*ptr, size_t size, const std::string &name, const std::vector<Gnm::ResourceType> &resTypes)
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		for (const auto &resType : resTypes)
		{
			Gnm::ResourceHandle rh;
			ret = Gnm::registerResource(&rh, m_owner, ptr, size, name.c_str(), resType, 0);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
			if (ret == SCE_OK) {
				m_frameInfos.back().m_registeredResources.push_back(rh);
			}
		}
	}
#endif // _DEBUG
}

#ifdef _DEBUG
void	VideoRingAllocator::unregisterResources(const std::vector<sce::Gnm::ResourceHandle>	&resourceHandles)
{
	int ret = SCE_OK; (void)ret;

	if (g_isResourceRegistrationInitialized)
	{
		for (auto resourceHandle : resourceHandles)
		{
			ret = Gnm::unregisterResource(resourceHandle);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_GNM_ERROR_FAILURE);
		}
	}
}
#endif // _DEBUG

}}} // namespace sce::SampleUtil::Graphics

#endif