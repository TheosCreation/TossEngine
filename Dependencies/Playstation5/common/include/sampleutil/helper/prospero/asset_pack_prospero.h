/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <istream>
#include <ostream>
#include <sys/dmem.h>
#include "sampleutil/memory/allocator_base.h"
#include "sampleutil/helper/memory_stream.h"
#include "sampleutil/sampleutil_common.h"
#include "async_asset_loader_prospero.h"

namespace sce {	namespace SampleUtil { namespace Helper {

	struct AssetPack
	{

		struct Asset
		{
			struct Memory : public sce::SampleUtil::Memory::AllocatorBase
			{
				std::string					m_filePathName;
				uintptr_t					m_savedVirtualAddress;
				size_t						m_sizeInBytes;
				size_t						m_alignmentInBytes;
				SceKernelMemoryType			m_type;
				int							m_prot;
				int							m_refCount = 0;
				Asset						*m_pParent;
				size_t						m_allocatedSize = 0;
				AsyncAssetLoader::Result	m_loadResult;
				MatPool						m_pool;

				virtual ~Memory() {}

				void	serialize(std::ostream	&os, const std::string	&assetFolderName);
				void	deserialize(std::istream	&is, const std::string	&assetFolderName);

				virtual void		*allocate(size_t	size, size_t	alignment, const std::string	&name = ""); // this is *NOT* thread-safe
				virtual void		free(void	*ptr); // this is thread-safe
				virtual void		registerResource(const void	*ptr, size_t	size, const std::string	&name, const std::vector<sce::Agc::ResourceRegistration::ResourceType>	&resTypes);
				virtual MatGroup	getMatGroup()
				{
					AssetPack::Asset *pAsset = m_pParent;
					AssetPack *pAssetPack = pAsset->m_pParent;
					return	sce::SampleUtil::Memory::getMatGroup(pAssetPack->m_pLoader);
				}
			};
			std::vector<Memory *>	m_contents;
			std::vector<uint8_t>	m_metadata;
			AssetPack				*m_pParent;
			Helper::MemoryStreambuf	m_metadataWriterBuf;
			Helper::MemoryStreambuf	m_metadataReaderBuf;
			Helper::StreamOutToMem	m_metadataWriter;
			Helper::StreamInFromMem	m_metadataReader;

			Asset()
				: m_metadataWriterBuf	(m_metadata)
				, m_metadataReaderBuf	(m_metadata)
				, m_metadataWriter		(m_metadataWriterBuf)
				, m_metadataReader		(m_metadataReaderBuf)
			{}

			~Asset()
			{
				for (auto *pLoadInfo : m_contents) delete pLoadInfo;
			}

			Memory	*createAndAddAssetMemory(const std::string	&filePathName, size_t 	sizeInBytes, size_t	alignment, SceKernelMemoryType	type, int	prot);

			void	waitForLoadCompletion()
			{
				AssetPack *pAssetPack = m_pParent;
				for (auto *pAssetMemory : m_contents)
				{
					pAssetPack->m_pLoader->m_aprListener.waitIf({ pAssetMemory->m_loadResult.m_pLoadState, AsyncAssetLoader::EventListener::kEqual, (uint64_t)AsyncAssetLoader::LoadState::kLoading, nullptr, 0ul });
				}
			}

			void	serialize(std::ostream	&os, const std::string	&assetFolderName);
			void	deserialize(std::istream	&is, const std::string	&assetFolderName);
		};

		AsyncAssetLoader							*m_pLoader;

		std::string									m_assetFolderName;
		std::unordered_map<std::string, Asset *>	m_assets;

		AssetPack(AsyncAssetLoader *pLoader) : m_pLoader(pLoader) {}
		~AssetPack()
		{
			for (auto asset : m_assets) delete asset.second;
		}

		Asset	*createAndAddAsset(const std::string	&assetName);

		// output serialized data to "/devlog/app/"
		// [directory structure]
		// /devlog/app/
		//           +-- asset.pack 
		//           +-- "assetFolderName"
		//                     +-- serialized asset file
		//                     +-- serialized asset file
		//                     +-- serialized asset file
		int	serialize(const std::string	&assetFolderName);
		int	deserialize(std::istream	&assetPackIs, const std::string	&assetFolderName);
	};
} } } // namespace sce::sampleUtil::Helper