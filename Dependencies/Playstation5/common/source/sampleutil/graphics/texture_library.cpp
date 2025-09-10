/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */
#include <fstream>
#include <regex>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core/translate.h>
#include "sampleutil/graphics/platform_agc/link_libraries_agc.h"
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnf.h>
#include <gnm/texture.h>
#include <gpu_address.h>
#endif
#include "sampleutil/sampleutil_error.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/graphics/texture_library.h"

namespace
{
	bool	handleDefaultTextureLoadFailure(sce::SampleUtil::Graphics::TextureLibrary	&texLib, const std::string	&name, sce::SampleUtil::Graphics::TextureType	type, sce::SampleUtil::Graphics::VideoAllocator	&videoMemory)
	{
		int ret = SCE_OK;
#if _SCE_TARGET_OS_PROSPERO
		sce::Agc::Core::TextureSpec spec;
		spec.init();
		spec.m_type = sce::Agc::Core::Texture::Type::k2d;
		spec.m_width = spec.m_height = spec.m_depth = spec.m_numMips = spec.m_numSlices = 1;
		if (type == sce::SampleUtil::Graphics::TextureType::kRoughness || type == sce::SampleUtil::Graphics::TextureType::kMetalness || type == sce::SampleUtil::Graphics::TextureType::kMask)
		{
			spec.m_format = { sce::Agc::Core::TypedFormat::k32Float, sce::Agc::Core::Swizzle::kR000_S1 };
		} else {
			spec.m_format = { sce::Agc::Core::TypedFormat::k32_32_32Float, sce::Agc::Core::Swizzle::kRGB1_R3S34 };
		}
		sce::Agc::SizeAlign sizeAlign = sce::Agc::Core::getSize(&spec);
		std::vector<sce::Agc::ResourceRegistration::ResourceType> resTypes = { sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress };
		texLib.m_textureData.emplace_back(sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(sizeAlign.m_size, sizeAlign.m_align, resTypes, videoMemory, name));
		float *pData = reinterpret_cast<float*>(texLib.m_textureData.back().get());
		spec.m_dataAddress = pData;
		sce::Agc::Core::Texture tex;
		ret = sce::Agc::Core::initialize(&tex, &spec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return false;
		}
#endif
#if _SCE_TARGET_OS_ORBIS
		sce::Gnm::TextureSpec spec;
		spec.init();
		spec.m_minGpuMode	= sce::Gnm::getGpuMode();
		spec.m_textureType	= sce::Gnm::kTextureType2d;
		spec.m_width = spec.m_height = 8;  spec.m_pitch = 32; spec.m_depth = 1;
		if (type == sce::SampleUtil::Graphics::TextureType::kRoughness || type == sce::SampleUtil::Graphics::TextureType::kMetalness || type == sce::SampleUtil::Graphics::TextureType::kMask)
		{
			spec.m_format = sce::Gnm::kDataFormatR32Float;
		} else {
			spec.m_format = sce::Gnm::kDataFormatR32G32B32A32Float;
		}
		ret = sce::GpuAddress::computeSurfaceTileMode(spec.m_minGpuMode, &spec.m_tileModeHint, sce::GpuAddress::kSurfaceTypeTextureFlat, spec.m_format, 1);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return false;
		}
		sce::Gnm::Texture tex;
		ret = tex.init(&spec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return false;
		}
		sce::Gnm::SizeAlign sizeAlign = tex.getSizeAlign();
		std::vector<sce::Gnm::ResourceType> resTypes = { sce::Gnm::kResourceTypeTextureBaseAddress };
		texLib.m_textureData.emplace_back(sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(sizeAlign.m_size, sizeAlign.m_align, resTypes, videoMemory, name));
		float *pData = reinterpret_cast<float*>(texLib.m_textureData.back().get());
		tex.setBaseAddress(pData);
#endif

		for (float *pCurr = pData; pCurr < pData + sizeAlign.m_size / 4;)
		{
			switch (type)
			{
			case sce::SampleUtil::Graphics::TextureType::kDiffuse:
			{
				const float grayTone = 0.8f;
				pCurr[0] = grayTone;
				pCurr[1] = grayTone;
				pCurr[2] = grayTone;
				pCurr += 4;
				break;
			}
			case sce::SampleUtil::Graphics::TextureType::kEmissive:
			{
				pCurr[0] = 0.f;
				pCurr[1] = 0.f;
				pCurr[2] = 0.f;
				pCurr += 4;
				break;
			}
			case sce::SampleUtil::Graphics::TextureType::kSpecular:
			{
				pCurr[0] = 0.5f;
				pCurr[1] = 0.5f;
				pCurr[2] = 0.5f;
				pCurr += 4;
				break;
			}
			case sce::SampleUtil::Graphics::TextureType::kNormal:
				pCurr[0] = 0.5f;
				pCurr[1] = 0.5f;
				pCurr[2] = 1.f;
				pCurr += 4;
				break;
			case sce::SampleUtil::Graphics::TextureType::kMask:
				pCurr[0] = 1.f;
				pCurr += 1;
				break;
			case sce::SampleUtil::Graphics::TextureType::kRoughness:
				pCurr[0] = 0.5f;
				pCurr += 1;
				break;
			case sce::SampleUtil::Graphics::TextureType::kMetalness:
				pCurr[0] = 0.f;
				pCurr += 1;
				break;
			case sce::SampleUtil::Graphics::TextureType::kAmbientOcclusion:
				pCurr[0] = 0.2f;
				pCurr[1] = 0.2f;
				pCurr[2] = 0.2f;
				pCurr += 4;
				break;
			case sce::SampleUtil::Graphics::TextureType::kShininess:
				pCurr[0] = 0.3f;
				pCurr[1] = 0.3f;
				pCurr[2] = 0.3f;
				pCurr += 4;
				break;
			case sce::SampleUtil::Graphics::TextureType::kNum:
				SCE_SAMPLE_UTIL_ASSERT(false);
			}
		}

		texLib.m_textures[name] = tex;

		return true;
	}

} // anonympus namespace

namespace sce { namespace SampleUtil { namespace Graphics {
	bool	(*TextureLibrary::m_pTextureLoadFailureCB)(TextureLibrary	&texLib, const std::string	&name, TextureType	type, VideoAllocator	&videoMemory);
#if _SCE_TARGET_OS_ORBIS
	TextureLibrary::TextureLibrary(VideoAllocator	&videoMemory)
		: m_pVideoMemory(&videoMemory)
	{
		TAG_THIS_CLASS;
		m_pTextureLoadFailureCB = handleDefaultTextureLoadFailure; // default texture callback
	}
#endif
#if _SCE_TARGET_OS_PROSPERO
	TextureLibrary::TextureLibrary(VideoAllocator	&videoMemory, Helper::AsyncAssetLoader	*asyncAssetLoader)
		: m_pVideoMemory(&videoMemory)
		, m_pAsyncAssetLoader(asyncAssetLoader)
	{
		TAG_THIS_CLASS;
		m_pTextureLoadFailureCB = handleDefaultTextureLoadFailure; // default texture callback
	}
#endif
	int	TextureLibrary::getTextureSync(Compat::Texture	&outTexture, const std::string	&filename, TextureType	type)
	{
		int ret = SCE_OK; (void)ret;

		std::string _filename = filename;
#if _SCE_TARGET_OS_ORBIS
		bool	is2ndTry = false;
	retry:
#endif
		ret = sceKernelCheckReachability(_filename.c_str());
		SCE_SAMPLE_UTIL_ASSERT(ret != SCE_KERNEL_ERROR_ENOMEM);
		if (ret == SCE_KERNEL_ERROR_ENOMEM)
		{
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
		}

		printf("loading texture %s...\n", _filename.c_str());

		std::ifstream ifs(_filename, std::ios::binary | std::ios::ate);
		SCE_SAMPLE_UTIL_ASSERT(ifs);
		if (!ifs)
		{
			return SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
		}
		size_t resourceSize = 0;
#if _SCE_TARGET_OS_PROSPERO
		auto gnfFileSize = ifs.tellg();
		char *pGnfBinary = reinterpret_cast<char *>(m_pVideoMemory->allocate(gnfFileSize, 64 * 1024, filename));
		ifs.seekg(0);
		if (!ifs.read(pGnfBinary, gnfFileSize))
		{
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_FILE_LOAD;
		}
		if (sce::Gnf::getVersion(pGnfBinary) == 4)
		{
			ret = Agc::Core::translate(&outTexture, reinterpret_cast<Gnf::GnfFile*>(pGnfBinary));
		} else if (sce::Gnf::getVersion(pGnfBinary) == 5)
		{
			ret = Agc::Core::translate(&outTexture, reinterpret_cast<Gnf::GnfFileV5*>(pGnfBinary));
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		m_textureData.emplace_back(reinterpret_cast<uint8_t*>(pGnfBinary), VideoAllocator::Dealloc<uint8_t[]>{ m_pVideoMemory });
		resourceSize = gnfFileSize;
#endif
#if _SCE_TARGET_OS_ORBIS
		Gnf::Header header;
		ifs.seekg(0);
		ifs.read(reinterpret_cast<char*>(&header), sizeof(Gnf::Header));
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(header.m_magicNumber, Gnf::kMagic);
		if (header.m_magicNumber != Gnf::kMagic)
		{
			ifs.close();
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		sce::Gnf::Contents *pContents = reinterpret_cast<sce::Gnf::Contents*>(malloc(header.m_contentsSize));
		SCE_SAMPLE_UTIL_ASSERT(pContents != nullptr);
		if (pContents == nullptr)
		{
			ifs.close();
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		ifs.read(reinterpret_cast<char*>(pContents), header.m_contentsSize);
		if (pContents->m_version != Gnf::kVersion)
		{
			free(pContents);
			ifs.close();
			if (!is2ndTry)
			{
				_filename = _filename.substr(0, _filename.size() - 4) + "_ps4.gnf";
				is2ndTry = true;
				goto retry;
			}
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		auto sa = Gnf::getTexturePixelsSize(pContents, 0);

		void *pTextureMemAddr = m_pVideoMemory->allocate(sa.m_size, sa.m_align, filename);
		SCE_SAMPLE_UTIL_ASSERT(pTextureMemAddr != nullptr);
		if (pTextureMemAddr == nullptr)
		{
			free(pContents);
			ifs.close();
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		ifs.read(reinterpret_cast<char*>(pTextureMemAddr), sa.m_size);
		outTexture = *Gnf::patchTextures(pContents, 0, 1, &pTextureMemAddr);
		free(pContents);
		ifs.close();
		m_textureData.emplace_back(reinterpret_cast<uint8_t*>(pTextureMemAddr), VideoAllocator::Dealloc<uint8_t[]>{m_pVideoMemory});
		resourceSize = sa.m_size;
#endif
		if (filename != "")
		{
			sce::SampleUtil::Graphics::Compat::ResourceType type;
#if _SCE_TARGET_OS_PROSPERO
			type = sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress;
#endif
#if _SCE_TARGET_OS_ORBIS
			type = sce::Gnm::kResourceTypeTextureBaseAddress;
#endif
			m_pVideoMemory->registerResource(m_textureData.back().get(), resourceSize, filename, { type });
		}

		return SCE_OK;
	}

	int	TextureLibrary::getTextureAsync(Compat::Texture	&outTexture, const std::string	&filename, TextureType	type)
	{
#if _SCE_TARGET_OS_ORBIS
		(void)outTexture;
		(void)filename;
		(void)type;
#endif
#if _SCE_TARGET_OS_PROSPERO
		int ret = SCE_OK; (void)ret;

		Helper::AsyncAssetLoader::Result result;
		result = m_pAsyncAssetLoader->acquire(filename.c_str(), 64 * 1024/* max texture alignment*/,
											SCE_KERNEL_MTYPE_C_SHARED,
											SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_AMPR_RW,
											{ sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress });
		if (result.m_ptr == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_FILE_LOAD;
		}

		printf("loading texture %s...\n", filename.c_str());

		// Create dummy texture(format=kInvalid) just for place holder.
		// Dummy texture has load target address as texture base address which is later used as hash for querying texture name.
		outTexture.init().setDataAddress(result.m_ptr).setFormat(sce::Agc::Core::TypedFormat::kInvalid);
#endif

		return SCE_OK;
	}

	int	TextureLibrary::getTexture(Compat::Texture	&outTexture, const std::string	&name, TextureType	type)
	{
		int ret = SCE_OK; (void)ret;

		Thread::ScopedLock criticalSection(this, Thread::LockableObjectAccessAttr::kWrite);

		auto it = m_textures.find(name);
		if (it != m_textures.end())
		{
			outTexture = it->second;
			return SCE_OK;
		}

		std::string filename = name;
		std::smatch texFilename;
		std::regex_search(filename, texFilename, std::regex("\\.(tga|tif|dds|png|jpg|bmp)$"));
		if (!texFilename.empty())
		{
			filename = texFilename.prefix().str() + ".gnf";
		}
		std::string::size_type i = 0;
		while ((i = filename.find('\\', i)) != std::string::npos)
		{
			filename[i] = '/';
		}

		filename = "/app0/" + filename;

		const bool	validGnfFilename	= (filename.rfind(".gnf") == filename.length() - 4);
		if (validGnfFilename) {
#if _SCE_TARGET_OS_PROSPERO
			if (m_pAsyncAssetLoader != nullptr)
			{
				ret = getTextureAsync(outTexture, filename, type);
			} else
#endif
			{
				ret = getTextureSync(outTexture, filename, type);
			}
		}

		if (ret == SCE_SAMPLE_UTIL_ERROR_FILE_OPEN || ret == SCE_SAMPLE_UTIL_ERROR_FILE_LOAD || !validGnfFilename)
		{
			printf("No gnf version of %s found.\n", name.c_str());

			if (m_pTextureLoadFailureCB != nullptr && (*m_pTextureLoadFailureCB)(*this, name, type, *m_pVideoMemory)) // call texgen callback if exists
			{
				outTexture = m_textures[name];

				return SCE_OK;
			}

			return SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
		}

		m_textures[name] = outTexture;
#if _SCE_TARGET_OS_PROSPERO
		if (m_pAsyncAssetLoader != nullptr)
		{
			m_loadAddr2name[outTexture.getDataAddress()] = name;
		}
#endif

		return SCE_OK;
	}

	int TextureLibrary::resolveTexture(Compat::Texture	&texture)
	{
#if _SCE_TARGET_OS_ORBIS
		(void)texture;
#endif
#if _SCE_TARGET_OS_PROSPERO
		if (/* isLoading*/texture.getFormat() == Agc::Core::TypedFormat::kInvalid)
		{
			SCE_SAMPLE_UTIL_ASSERT(m_pAsyncAssetLoader != nullptr);

			Thread::ScopedLock criticalSection(this, Thread::LockableObjectAccessAttr::kWrite);

			void *ptr = texture.getDataAddress();
			auto nameIter = m_loadAddr2name.find(ptr);
			if (nameIter == m_loadAddr2name.end())
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			const std::string name = nameIter->second;
			auto textureIter = m_textures.find(name);
			SCE_SAMPLE_UTIL_ASSERT(textureIter != m_textures.end());
			if (/* loading already finished */textureIter->second.getFormat() != Agc::Core::TypedFormat::kInvalid)
			{
				texture = textureIter->second;

				return SCE_OK;
			}
			Agc::Label *pLoadState = nullptr;
			{
				Thread::ScopedLock criticalSection(m_pAsyncAssetLoader, Thread::LockableObjectAccessAttr::kRead);

				auto loadStateIter = m_pAsyncAssetLoader->m_loadStates.find(ptr);
				SCE_SAMPLE_UTIL_ASSERT(loadStateIter != m_pAsyncAssetLoader->m_loadStates.end());
				pLoadState = loadStateIter->second;
			}

			m_pAsyncAssetLoader->m_aprListener.waitIf({ pLoadState, Helper::AsyncAssetLoader::EventListener::Condition::kEqual, (uint64_t)Helper::AsyncAssetLoader::LoadState::kLoading, nullptr, 0ul });

			if (sce::Gnf::getVersion(ptr) == 4)
			{
				Agc::Core::translate(&texture, reinterpret_cast<Gnf::GnfFile*>(ptr));
			} else if (sce::Gnf::getVersion(ptr) == 5)
			{
				Agc::Core::translate(&texture, reinterpret_cast<Gnf::GnfFileV5*>(ptr));
			} else {
				SCE_SAMPLE_UTIL_ASSERT(false);
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}

			textureIter->second = texture;
		}
#endif

		return SCE_OK;
	}
}}} // namespace sce::SampleUtil::Graphics
