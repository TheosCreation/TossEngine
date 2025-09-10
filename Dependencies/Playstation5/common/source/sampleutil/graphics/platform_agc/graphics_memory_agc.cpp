/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2024 Sony Interactive Entertainment Inc.
* 
*/

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/resourceregistration.h>
#include <agc/error.h>
#include "sampleutil/memory.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/memory/dmem_mapper.h"
#include "sampleutil/debug/perf.h"

namespace
{
	const uint32_t kPageSize = 64u*1024u;
#ifdef _DEBUG
	void	*s_pResourceRegistrationMemory	= nullptr;
	size_t	s_resourceRegistrationMemorySizeInBytes = 0;
#endif
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Graphics {

bool	g_isResourceRegistrationInitialized = false;

VideoAllocator::VideoAllocator(size_t size, SceKernelMemoryType type, bool amm, int	prot, const char	*pName)
{
	TAG_THIS_DERIVED_CLASS;
	initialize(size, type, pName, amm, prot);
}

void	VideoAllocator::initialize(size_t size, SceKernelMemoryType type, const char	*pName, bool ammDirect, int	prot)
{
	int ret; (void)ret;

	m_start	= 0;
	m_type	= type;

	void	*mapStart = nullptr;
	// AMM Direct (use this for only Scanout)
	if (ammDirect)
	{
		ret = SampleUtil::Memory::mapVideoOutMemory(&mapStart, 0, size, type, prot, 0, kPageSize);
	}
	// AMM Auto (general use)
	else
	{
		ret = SampleUtil::Memory::mapDirectMemory(&mapStart, size, type, prot, 0, kPageSize);
	}

	setInfo(mapStart, size, pName);
#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		const char *pAttrName = "(deprecated)";
		if (type == SCE_KERNEL_MTYPE_C)
		{
			pAttrName = "(cached)";
		}
		else if (type == SCE_KERNEL_MTYPE_C_SHARED)
		{
			pAttrName = "(cached shared)";
		}
		ret = Agc::ResourceRegistration::registerOwner(&m_owner, (std::string(pName) + pAttrName).c_str());
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
		if (Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
		{
			sceMatAgcRegisterOwner(m_owner, (std::string(pName) + pAttrName).c_str());
		}
	}
#endif
}

VideoAllocator::~VideoAllocator()
{
	int ret = SCE_OK; (void)ret;

	UNTAG_THIS_DERIVED_CLASS;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		ret = Agc::ResourceRegistration::unregisterOwnerAndResources(m_owner);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
		if (Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
		{
			sceMatAgcUnregisterOwnerAndResources(m_owner);
		}
	}
#endif
	ret = SampleUtil::Memory::munmap(mapStart(), size());
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

void	initializeResourceRegistration(uint32_t	maxOwnersAndResources, uint32_t	maxNameLength)
{
	int ret = SCE_OK; (void)ret;

	if (!g_isResourceRegistrationInitialized)
	{
#ifdef _DEBUG
		static bool s_hasInitializedResourceRegistration = false;
		if (!s_hasInitializedResourceRegistration)
		{
			size_t	resourceRegistrationMemorySizeInBytes;
			ret = Agc::ResourceRegistration::queryMemoryRequirements(&resourceRegistrationMemorySizeInBytes, maxOwnersAndResources, maxNameLength);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (ret != SCE_OK)
			{
				return;
			}
			if (s_pResourceRegistrationMemory != nullptr && s_resourceRegistrationMemorySizeInBytes < resourceRegistrationMemorySizeInBytes) {
				// if memory allocated was not large enough, free it
				sce::SampleUtil::Memory::freeDmem(s_pResourceRegistrationMemory);
				s_pResourceRegistrationMemory	= nullptr;
			}
			if (s_pResourceRegistrationMemory == nullptr) {
				s_resourceRegistrationMemorySizeInBytes	= resourceRegistrationMemorySizeInBytes;
				s_pResourceRegistrationMemory	= sce::SampleUtil::Memory::allocDmem(resourceRegistrationMemorySizeInBytes, 0x10000u, SCE_KERNEL_MTYPE_C, SCE_KERNEL_PROT_CPU_RW, "AGC_RESOURCE_REGISTRATION");		// This memory will never be freed. Because 'ResourceRegistration' can't be finalized.
				SCE_SAMPLE_UTIL_ASSERT(s_pResourceRegistrationMemory != nullptr);
				if (s_pResourceRegistrationMemory == nullptr) {
					return;
				}
			}
			ret = Agc::ResourceRegistration::init(s_pResourceRegistrationMemory, resourceRegistrationMemorySizeInBytes, maxNameLength);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (ret != SCE_OK)
			{
				return;
			}
			s_hasInitializedResourceRegistration = true;
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

void	VideoAllocator::registerResource(const void	*ptr, size_t size, const std::string &name, const std::vector<Agc::ResourceRegistration::ResourceType> &resTypes)
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		for (const auto &resType : resTypes)
		{
			Agc::ResourceRegistration::ResourceHandle rh;
			ret = Agc::ResourceRegistration::registerResource(&rh, m_owner, ptr, size, name.c_str(), resType, 0);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
			{
				sceMatAgcRegisterResource(rh, m_owner, ptr, size, name.c_str(), (uint32_t)resType, 0);
			}
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
		std::vector<Agc::ResourceRegistration::ResourceHandle> resourceHandles;
		Agc::ResourceRegistration::findResources(ptr, 16, [](Agc::ResourceRegistration::ResourceHandle resourceHandle, Agc::ResourceRegistration::OwnerHandle ownerHandle, uint64_t callbackdata)
		{
			std::vector<Agc::ResourceRegistration::ResourceHandle> *pResourceHandles = reinterpret_cast<std::vector<Agc::ResourceRegistration::ResourceHandle> *>(callbackdata);
			pResourceHandles->push_back(resourceHandle);
			return SCE_OK;
		}, reinterpret_cast<uintptr_t>(&resourceHandles));
		for (auto resourceHandle : resourceHandles)
		{
			ret = Agc::ResourceRegistration::unregisterResource(resourceHandle);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
			{
				sceMatAgcUnregisterResource(resourceHandle);
			}
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
	int ret; (void)ret;
	ret = SampleUtil::Memory::mapDirectMemory(&m_mapStart, size, type, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, 0, kPageSize);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

#ifdef _DEBUG
	if (Memory::g_isMatInitialized)
	{
		m_pool = sceMatAllocPoolMemory(m_mapStart, size, Memory::getMatGroup(this));
		sceMatTagPool(m_pool, pName);
	}
	if (g_isResourceRegistrationInitialized)
	{
		const char *pAttrName = "(deprecated)";
		if (type == SCE_KERNEL_MTYPE_C)
		{
			pAttrName = "(cached)";
		}
		else if (type == SCE_KERNEL_MTYPE_C_SHARED)
		{
			pAttrName = "(cached shared)";
		}
		ret = Agc::ResourceRegistration::registerOwner(&m_owner, (std::string(pName) + pAttrName).c_str());
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
		if (Memory::g_isMatInitialized)
		{
			sceMatAgcRegisterOwner(m_owner, pName);
		}
	}
#endif
}

VideoRingAllocator::~VideoRingAllocator()
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (Memory::g_isMatInitialized)
	{
		sceMatFreePoolMemory(m_pool);
	}
	if (g_isResourceRegistrationInitialized)
	{
		ret = Agc::ResourceRegistration::unregisterOwnerAndResources(m_owner);
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
		if (Memory::g_isMatInitialized)
		{
			sceMatAgcUnregisterOwnerAndResources(m_owner);
		}
	}
#endif
	ret = SampleUtil::Memory::munmap(m_mapStart, m_size);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

void	VideoRingAllocator::registerResource(const void	*ptr, size_t size, const std::string &name, const std::vector<Agc::ResourceRegistration::ResourceType> &resTypes)
{
	int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
	if (g_isResourceRegistrationInitialized)
	{
		for (const auto &resType : resTypes)
		{
			Agc::ResourceRegistration::ResourceHandle rh;
			ret = Agc::ResourceRegistration::registerResource(&rh, m_owner, ptr, size, name.c_str(), resType, 0);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (ret == SCE_OK) {
				m_frameInfos.back().m_registeredResources.push_back(rh);
				if (Memory::g_isMatInitialized) {
					sceMatAgcRegisterResource(rh, m_owner, ptr, size, name.c_str(), (uint32_t)resType, 0);
				}
			}
		}
	}
#endif // _DEBUG
}

#ifdef _DEBUG
void	VideoRingAllocator::unregisterResources(const std::vector<sce::Agc::ResourceRegistration::ResourceHandle>	&resourceHandles)
{
	int ret = SCE_OK; (void)ret;

	if (g_isResourceRegistrationInitialized) {
		for (sce::Agc::ResourceRegistration::ResourceHandle	resourceHandle : resourceHandles) {
			ret = Agc::ResourceRegistration::unregisterResource(resourceHandle);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
			if (Memory::g_isMatInitialized && ret == SCE_OK) {
				sceMatAgcUnregisterResource(resourceHandle);
			}
		}
	}
}
#endif // _DEBUG

}}} // namespace sce::SampleUtil::Graphics

#endif