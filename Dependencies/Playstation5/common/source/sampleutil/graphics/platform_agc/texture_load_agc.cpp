/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/sizealign.h>
#include <agc/core/texture.h>
#include <agc/core/texturespec.h>
#include <agc/core/translate.h>
#include <agc_gpu_address.h>
#include <agc_texture_tool.h>

#include <sampleutil/sampleutil_common.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/memory.h>
#include <sampleutil/graphics/graphics_memory.h>
#include <sampleutil/graphics/texture_load.h>

namespace sce { namespace SampleUtil { namespace Graphics { namespace Texture { namespace Details {

int	loadGnfFile(sce::Agc::Core::Texture	&outTexture, const char	*pFilename, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory)
{
	int	ret	= SCE_OK;

	FILE	*fp	= fopen(pFilename, "rb");
	SCE_SAMPLE_UTIL_ASSERT(fp != nullptr);
	if (fp == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
	}

	sce::Gnf::Header	gnfHeader;
	fread(&gnfHeader, sizeof(sce::Gnf::Header), 1, fp);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(gnfHeader.m_magicNumber, sce::Gnf::kMagic);
	if (gnfHeader.m_magicNumber != sce::Gnf::kMagic) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]>	gnfFile = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(sizeof(sce::Gnf::Header) + gnfHeader.m_contentsSize, 4, cpuMemory);
	fseek(fp, 0, SEEK_SET);
	fread(gnfFile.get(), sizeof(sce::Gnf::Header) + gnfHeader.m_contentsSize, 1, fp);

	sce::TextureTool::Agc::SizeAlign	texDataSize;
	ret	= sce::Gnf::getTextureDataSize(&texDataSize, gnfFile.get(), 0);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	uint64_t	texelOffset;
	ret	= sce::Gnf::getTextureDataOffset(&texelOffset, gnfFile.get(), 0);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	// read base data
	void	*pPixelsSrc	= videoMemory.allocate(texDataSize.m_size, texDataSize.m_align);
	SCE_SAMPLE_UTIL_ASSERT(pPixelsSrc != nullptr);
	if (pPixelsSrc == nullptr) {
		fclose(fp);
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}
	fseek(fp, texelOffset, SEEK_SET);
	fread(pPixelsSrc, texDataSize.m_size, 1, fp);
	fclose(fp);

	const void	*pConstPixelsSrc = pPixelsSrc;
	const sce::TextureTool::Agc::TSharp	*pPatchedTexture	= sce::Gnf::patchTexturesV5(gnfFile.get(), &pConstPixelsSrc, 0, 1);
	SCE_SAMPLE_UTIL_ASSERT(pPatchedTexture != nullptr);
	if (pPatchedTexture == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	memcpy(&outTexture, pPatchedTexture, sizeof(sce::Agc::Core::Texture));

	return	SCE_OK;
}

int	tileSurface(void	*pTiledData, const void	*pUntiledSurface, const sce::Agc::Core::TextureSpec	&spec, uint32_t	mipLevel, uint32_t	arraySlice)
{
	int	ret = SCE_OK;

	sce::AgcGpuAddress::SurfaceDescription	desc;
	ret	= sce::Agc::Core::translate(&desc, &spec);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	sce::AgcGpuAddress::SurfaceSummary	summary;
	ret	= sce::AgcGpuAddress::computeSurfaceSummary(&summary, &desc);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	uint32_t	totalBitsPerElement;
	uint32_t	texelsPerElement;
	{
		sce::Agc::Core::ElementDimensions	elemDim;
		ret	= sce::Agc::Core::translate(&elemDim, spec.m_format.m_format);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		totalBitsPerElement	= (UINT32_C(1) << elemDim.m_bytesPerElementLog2) << 3;
		texelsPerElement	= elemDim.m_texelsPerElementWide * elemDim.m_texelsPerElementTall;
	}
	
	sce::Agc::SizeAlign	texDataSize	= sce::Agc::Core::getSize(&spec);
	ret	= sce::AgcGpuAddress::tileSurface(
		pTiledData,
		texDataSize.m_size,
		pUntiledSurface,
		spec.getWidth() * spec.getHeight() * (totalBitsPerElement / texelsPerElement) / 8,
		&summary,
		mipLevel,
		arraySlice);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	return	SCE_OK;
}

}}}}} // namespace sce::SampleUtil::Graphics::Texture::Details
#endif // _SCE_TARGET_OS_PROSPERO