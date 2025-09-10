/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <gnf.h>

#include <sampleutil/sampleutil_common.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/memory.h>
#include <sampleutil/graphics/graphics_memory.h>
#include <sampleutil/graphics/texture_load.h>

namespace sce { namespace SampleUtil { namespace Graphics { namespace Texture { namespace Details {

int	loadGnfFile(sce::Gnm::Texture	&outTexture, const char	*pFilename, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory)
{
	FILE	*fp	= fopen(pFilename, "rb");
	SCE_SAMPLE_UTIL_ASSERT(fp != nullptr);
	if (fp == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
	}

	sce::Gnf::Header	header;
	fread(&header, sizeof(sce::Gnf::Header), 1, fp);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(header.m_magicNumber, sce::Gnf::kMagic);
	if (header.m_magicNumber != sce::Gnf::kMagic) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	sce::Gnf::Contents	*pContents = reinterpret_cast<sce::Gnf::Contents *>(alloca(header.m_contentsSize));
	SCE_SAMPLE_UTIL_ASSERT(pContents != nullptr);
	if (pContents == nullptr) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}
	fread(pContents, header.m_contentsSize, 1, fp);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(pContents->m_version, sce::Gnf::kVersion);
	if (pContents->m_version != sce::Gnf::kVersion) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	auto	texSizeAlign	= sce::Gnf::getTexturePixelsSize(pContents, 0);

	sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]>	contentData	= sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(texSizeAlign.m_size, 4, cpuMemory);
	if (contentData.get() == nullptr) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}
	fread(contentData.get(), texSizeAlign.m_size, 1, fp);
	fclose(fp);

	void	*pTextureMemAddr	= videoMemory.allocate(texSizeAlign.m_size, texSizeAlign.m_align);
	SCE_SAMPLE_UTIL_ASSERT(pTextureMemAddr != nullptr);
	if (pTextureMemAddr == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pTextureMemAddr, reinterpret_cast<const uint8_t*>(contentData.get()), texSizeAlign.m_size);
	outTexture	= *sce::Gnf::patchTextures(pContents, 0, 1, &pTextureMemAddr);

	return	SCE_OK;
}

int	tileSurface(void	*pTiledData, const void	*pUntiledSurface, const sce::Gnm::TextureSpec	&spec, uint32_t	mipLevel, uint32_t	arraySlice)
{
	int	ret = SCE_OK;

	sce::Gnm::Texture	texture;
	ret	= texture.init(&spec);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	sce::GpuAddress::TilingParameters	tileParam;
	ret	= tileParam.initFromTexture(&texture, mipLevel, arraySlice);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	ret	= sce::GpuAddress::tileSurface(pTiledData, pUntiledSurface, &tileParam);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	return	SCE_OK;
} // namespace Details

}}}}} // namespace sce::SampleUtil::Graphics::Texture::Details
#endif // _SCE_TARGET_OS_ORBIS
