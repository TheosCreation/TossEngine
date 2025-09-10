/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <fstream>
#include <kernel.h>
#include <sce_atomic.h>
#include "sampleutil/graphics/serialize.h"
#include "sampleutil/graphics/deserialize.h"
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/helper/prospero/asset_pack_prospero.h"
#ifdef _DEBUG
#include "sampleutil/memory/memory_analyzer.h"
#endif

namespace
{
	void	prepareFullpathForWrite(const std::string	&fullPathName)
	{
		for (auto iter = fullPathName.begin(); (iter = std::find(iter, fullPathName.end(), '/')) != fullPathName.end(); iter++)
		{
			const std::string folderName(fullPathName.begin(), iter);
			if (folderName == "" || folderName == "/devlog" || folderName == "/devlog/app")
			{
				continue;
			}
			int ret = sceKernelMkdir(folderName.c_str(), SCE_KERNEL_S_IRWU);
			SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_KERNEL_ERROR_EEXIST);
		}
	}
} // anonymous namespace

namespace sce {	namespace SampleUtil { namespace Helper {
	void	*AssetPack::Asset::Memory::allocate(size_t	size, size_t	alignment, const std::string	&name)
	{
		++m_refCount;
		uintptr_t baseAddress = (uintptr_t)m_loadResult.m_ptr;
		uintptr_t currVa = baseAddress + m_allocatedSize;
		currVa = ((currVa + alignment - 1) / alignment) * alignment;
		size_t newAllocatedSize = currVa + size - baseAddress;
		SCE_SAMPLE_UTIL_ASSERT(newAllocatedSize <= m_sizeInBytes);
		m_allocatedSize = newAllocatedSize;
#ifdef _DEBUG
		ASAN_UNPOISON_MEMORY_REGION(reinterpret_cast<void *>(currVa), size);
		if (SampleUtil::Memory::g_isMatInitialized)
		{
			sceMatAlloc(reinterpret_cast<void *>(currVa), size, 0, SampleUtil::Memory::getMatGroup(this));
		}
#endif
		return reinterpret_cast<void *>(currVa);
	}

	void	AssetPack::Asset::Memory::free(void	*ptr)
	{
#ifdef _DEBUG
		if (Graphics::g_isResourceRegistrationInitialized)
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
				int ret = Agc::ResourceRegistration::unregisterResource(resourceHandle);
				SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
				if (SampleUtil::Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
				{
					sceMatAgcUnregisterResource(resourceHandle);
				}
			}
		}
		if (SampleUtil::Memory::g_isMatInitialized)
		{
			sceMatFree(ptr);
		}
#endif // _DEBUG
		if (sceAtomicDecrement32(&m_refCount) == 1)
		{
			Asset *pAsset = m_pParent;
			AssetPack *pAssetPack = pAsset->m_pParent;
			AsyncAssetLoader *pLoader = pAssetPack->m_pLoader;
#ifdef _DEBUG
			ASAN_POISON_MEMORY_REGION(m_loadResult.m_ptr, m_allocatedSize);
			if (SampleUtil::Memory::g_isMatInitialized)
			{
				sceMatFreePoolMemory(m_pool);
				// allocate loader's memory here. This is rediculous but pLoader->release right after this line will free loader's memory so we allocate here.
				sceMatAlloc(m_loadResult.m_ptr, m_allocatedSize, 0, SampleUtil::Memory::getMatGroup(pLoader));
			}
#endif
			pLoader->release((pAssetPack->m_assetFolderName + "/" + m_filePathName).c_str());
		}
	}

	void	AssetPack::Asset::Memory::registerResource(const void	*ptr, size_t	size, const std::string	&name, const std::vector<sce::Agc::ResourceRegistration::ResourceType>	&resTypes)
	{
		int ret = SCE_OK; (void)ret;

#ifdef _DEBUG
		if (Graphics::g_isResourceRegistrationInitialized)
		{
			Asset *pAsset = m_pParent;
			AssetPack *pAssetPack = pAsset->m_pParent;
			AsyncAssetLoader *pLoader = pAssetPack->m_pLoader;
			for (const auto &resType : resTypes)
			{
				Agc::ResourceRegistration::ResourceHandle rh;
				ret = Agc::ResourceRegistration::registerResource(&rh, pLoader->m_owner, ptr, size, name.c_str(), resType, 0);
				SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG);
				if (SampleUtil::Memory::g_isMatInitialized && (ret == SCE_OK || ret == SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG))
				{
					sceMatAgcRegisterResource(rh, pLoader->m_owner, ptr, size, name.c_str(), (uint32_t)resType, 0);
				}
			}
		}
#endif // _DEBUG
	}

	void	AssetPack::Asset::Memory::serialize(std::ostream	&os, const std::string	&assetFolderName)
	{
		Graphics::Offline::writeString(os, m_filePathName);
		Graphics::Offline::writeValue(os, (uintptr_t)m_loadResult.m_ptr);
		Graphics::Offline::writeValue(os, m_sizeInBytes);
		Graphics::Offline::writeValue(os, m_alignmentInBytes);
		Graphics::Offline::writeValue(os, m_type);
		Graphics::Offline::writeValue(os, m_prot);

		const std::string fileFullPathName = "/devlog/app/" + assetFolderName + "/" + m_filePathName;
		prepareFullpathForWrite(fileFullPathName);
		std::ofstream ofs(fileFullPathName, std::ios::out | std::ios::binary);
		SCE_SAMPLE_UTIL_ASSERT(ofs.is_open());
		printf("writing AssetMemory to \"%s\"...\n", fileFullPathName.c_str());

		ofs.write(reinterpret_cast<const char *>(m_loadResult.m_ptr), m_sizeInBytes);
		printf("done.\n");
	}

	void	AssetPack::Asset::Memory::deserialize(std::istream	&is, const std::string	&assetFolderName)
	{
		Graphics::Offline::readString(is, m_filePathName);
		Graphics::Offline::readValue(is, m_savedVirtualAddress);
		Graphics::Offline::readValue(is, m_sizeInBytes);
		Graphics::Offline::readValue(is, m_alignmentInBytes);
		Graphics::Offline::readValue(is, m_type);
		Graphics::Offline::readValue(is, m_prot);
		m_refCount = 0;
		m_allocatedSize = 0;

		const std::string assetPathName = assetFolderName + "/" + m_filePathName;

		Asset *pAsset = m_pParent;
		AssetPack *pAssetPack = pAsset->m_pParent;
		AsyncAssetLoader *pLoader = pAssetPack->m_pLoader;
		m_loadResult = pLoader->acquire(assetPathName.c_str(), m_alignmentInBytes, m_type, m_prot);
#ifdef _DEBUG
		if (SampleUtil::Memory::g_isMatInitialized)
		{
			// free acquired memory here and change it to Pool Memory so that those are can be managed by AssetPack::Asset::Memory
			sceMatFree(m_loadResult.m_ptr);
			m_pool = sceMatAllocPoolMemory(m_loadResult.m_ptr, m_sizeInBytes, SampleUtil::Memory::getMatGroup(pLoader));
			sceMatTagPool(m_pool, assetPathName.c_str());
			SampleUtil::Memory::registerMatGroup(this, assetPathName.c_str());
		}
#endif
	}

	AssetPack::Asset::Memory	*AssetPack::Asset::createAndAddAssetMemory(const std::string	&filePathName, size_t 	sizeInBytes, size_t 	alignmentInBytes, SceKernelMemoryType	type, int	prot)
	{
		SCE_SAMPLE_UTIL_ASSERT(std::find_if(m_contents.begin(), m_contents.end(), [&](Memory *mem) { return mem->m_filePathName == filePathName; }) == m_contents.end());
		Memory *pAssetMemory = new Memory();

		pAssetMemory->m_filePathName		= filePathName;
		pAssetMemory->m_savedVirtualAddress = 0ul;
		pAssetMemory->m_sizeInBytes			= sizeInBytes;
		pAssetMemory->m_alignmentInBytes	= alignmentInBytes;
		pAssetMemory->m_type				= type;
		pAssetMemory->m_prot				= prot;
		pAssetMemory->m_refCount			= 0;
		pAssetMemory->m_allocatedSize		= 0ul;
		pAssetMemory->m_pParent				= this;

		AssetPack *pAssetPack = m_pParent;
		AsyncAssetLoader *pLoader = pAssetPack->m_pLoader;
		pAssetMemory->m_loadResult = pLoader->acquireNoLoad(filePathName.c_str(), alignmentInBytes, type, prot);
#ifdef _DEBUG
		if (SampleUtil::Memory::g_isMatInitialized)
		{
			// free acquired memory here and change it to Pool Memory so that those can be managed by AssetPack::Asset::Memory
			sceMatFree(pAssetMemory->m_loadResult.m_ptr);
			pAssetMemory->m_pool = sceMatAllocPoolMemory(pAssetMemory->m_loadResult.m_ptr, sizeInBytes, SampleUtil::Memory::getMatGroup(pLoader));
			sceMatTagPool(pAssetMemory->m_pool, filePathName.c_str());
			SampleUtil::Memory::registerMatGroup(pAssetMemory, filePathName.c_str());
		}
#endif

		m_contents.push_back(pAssetMemory);

		return pAssetMemory;
	}

	void	AssetPack::Asset::serialize(std::ostream	&os, const std::string	&assetFolderName)
	{
		Graphics::Offline::writeValue(os, (uint32_t)m_contents.size());
		for (Asset::Memory *pAssetMemory : m_contents)
		{
			pAssetMemory->serialize(os, assetFolderName);
			pAssetMemory->m_pParent = this;
		}
		Graphics::Offline::writeVector(os, m_metadata);
	}

	void	AssetPack::Asset::deserialize(std::istream	&is, const std::string	&assetFolderName)
	{
		uint32_t numContents;
		Graphics::Offline::readValue(is, numContents);
		for (int i = 0; i < numContents; i++)
		{
			Memory *pAssetMemory = new Memory();
			pAssetMemory->m_pParent = this;
			pAssetMemory->deserialize(is, assetFolderName);
			m_contents.push_back(pAssetMemory);
		}
		Graphics::Offline::readVector(is, m_metadata);
	}

	AssetPack::Asset	*AssetPack::createAndAddAsset(const std::string	&assetName)
	{
		SCE_SAMPLE_UTIL_ASSERT(m_assets.find(assetName) == m_assets.end());

		Asset *pAsset = new Asset();
		pAsset->m_pParent = this;

		m_assets.emplace(std::make_pair(assetName, pAsset));

		return pAsset;
	}

	int	AssetPack::serialize(const std::string	&assetFolderName)
	{
		const std::string fullPathName("/devlog/app/asset.apk");
		prepareFullpathForWrite(fullPathName);
		std::ofstream packOfs(fullPathName, std::ios::out | std::ios::binary);
		SCE_SAMPLE_UTIL_ASSERT(packOfs.is_open());
		printf("writing AssetPack to \"%s\"...\n", fullPathName.c_str());

		const uint32_t numAssets = m_assets.size();
		Graphics::Offline::writeValue(packOfs, numAssets);
		for (auto asset : m_assets)
		{
			const std::string assetName = asset.first;
			Asset *pAsset = asset.second;

			Graphics::Offline::writeString(packOfs, assetName);
			pAsset->serialize(packOfs, assetFolderName);
		}
		printf("write AssetPack to \"%s\" done.\n", fullPathName.c_str());

		return SCE_OK;
	}

	int	AssetPack::deserialize(std::istream	&is, const std::string	&assetFolderName)
	{
		m_assetFolderName = assetFolderName;
		SCE_SAMPLE_UTIL_ASSERT(is.good());
		uint32_t numAssets;
		Graphics::Offline::readValue(is, numAssets);
		for (int i = 0; i < numAssets; i++)
		{
			std::string assetName;
			Graphics::Offline::readString(is, assetName);
			Asset *pAsset = createAndAddAsset(assetName);
			pAsset->deserialize(is, assetFolderName);
			m_assets.emplace(std::make_pair(assetName, pAsset));
		}

		return SCE_OK;
	}

} } } // namespace sce::sampleUtil::Helper
#endif // _SCE_TARGET_OS_PROSPERO