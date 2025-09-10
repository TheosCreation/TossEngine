/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#include <mspace.h>
#include <kernel.h>
#include <png_dec.h>
#include <jpeg_dec.h>
#if _SCE_TARGET_OS_ORBIS
#include <gnf.h>
#endif
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/graphics/compat.h>
#include <sampleutil/memory.h>
#include <sampleutil/graphics/graphics_memory.h>

#pragma comment(lib, "libScePngDec_stub_weak.a")
#pragma comment(lib, "libSceJpegDec_stub_weak.a")

namespace sce { namespace SampleUtil { namespace Graphics { namespace Texture {
namespace Details {

extern int	loadGnfFile(Compat::Texture	&outTexture, const char	*pFilename, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory);

extern int	tileSurface(void	*pTiledData, const void	*pUntiledSurface, const Compat::TextureSpec	&spec, uint32_t	mipLevel, uint32_t	arraySlice);

struct CloseOnExit
{
	~CloseOnExit()
	{
		sceKernelClose(m_fileHanle);
	}
	int	m_fileHanle	= -1;
};

enum class TextureKind
{
	kUnsupported = -1,
	kGnf,
	kPng,
	kJpeg,
};

struct FileBlob
{
	FileBlob(char const	*pFilename, SceLibcMspace	cpuMemory)
	{
		const int	handle = sceKernelOpen(pFilename, SCE_KERNEL_O_RDONLY, 0);
		if (handle < 0) {
			m_errorCode = SCE_SAMPLE_UTIL_ERROR_FILE_LOAD;
			return;
		}
		const CloseOnExit	coe{ handle };

		SceKernelStat	fileStats{};
		if (sceKernelFstat(handle, &fileStats) != SCE_OK) {
			m_errorCode = SCE_SAMPLE_UTIL_ERROR_FILE_LOAD;
			return;
		}
		size_t	fileSize = static_cast<size_t>(fileStats.st_size);
		m_data = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(fileSize, 4, cpuMemory);

		size_t	toRead = fileSize;
		size_t	offset = 0;
		uint8_t* ptr = m_data.get();
		while (toRead > 0)
		{
			ssize_t	read = sceKernelPread(handle, ptr, toRead, offset);
			if (read <= 0) {
				m_errorCode = SCE_SAMPLE_UTIL_ERROR_FILE_LOAD;
				return;
			}
			ptr += read;
			offset += read;
			toRead -= read;
		}

		m_size = fileSize;
	}

	bool isOk() const
	{
		return	m_errorCode == SCE_OK;
	}

	Memory::Cpu::unique_ptr<uint8_t[]>	m_data;
	size_t								m_size = 0;
	int									m_errorCode = SCE_OK;
};


// PNG loading
static uint32_t	getPngSize(const void	*pImageData)
{
#define ADVANCE_PTR_AND_ADD_SIZE(size) pData += (size); retSize += (size);

	const uint8_t	*pData	= (uint8_t *)pImageData;
	uint32_t		retSize	= 0;

	// Skip Signature
	ADVANCE_PTR_AND_ADD_SIZE(8);

	// Read chunk
	while (1)
	{
		// Chunk Data (4 + 4 + X + 4 bytes)
		// 4 byte: Data part length = X
		// 4 byte: Type
		// X byte: Data
		// 4 byte: CRC

		// Check chunk type
		uint32_t type = (*(pData + 4) << 24) | (*(pData + 5) << 16) | (*(pData + 6) << 8) | *(pData + 7);
		if (type == 'IEND') {
			// End of Chunk
			retSize	+= 12;
			break;
		}

		// Invalid data or failed to parse file
		if (type == 0) {
			SCE_SAMPLE_UTIL_ASSERT(false);
			retSize	= 0;
			break;
		}

		// Chunk length
		uint32_t	length	= (*pData << 24) | (*(pData + 1) << 16) | (*(pData + 2) << 8) | *(pData + 3);
		// Next chunk
		ADVANCE_PTR_AND_ADD_SIZE(12 + length);
	}
#undef ADVANCE_PTR_AND_ADD_SIZE

	return	retSize;
}

static int	decodePng(Compat::Texture	&outTexture, const void	*pImageData, uint32_t	imageSize, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory)
{
	int	ret = SCE_OK;

	// Decode png file
	ScePngDecParseParam		parseParam;
	ScePngDecImageInfo		imageInfo;
	ScePngDecCreateParam	createParam;
	ScePngDecHandle			handle;
	ScePngDecDecodeParam	decodeParam;

	// get image info.
	parseParam.pngMemAddr	= pImageData;
	parseParam.pngMemSize	= imageSize;
	parseParam.reserved0	= 0;
	ret	= scePngDecParseHeader(&parseParam, &imageInfo);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if ((ret == SCE_PNG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_SIZE)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_PARAM)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// setup texture
	void* pTextureDataAddress = nullptr;
	Compat::TextureSpec	textureSpec;
	textureSpec.init();
	textureSpec.m_width			= imageInfo.imageWidth;
	textureSpec.m_height		= imageInfo.imageHeight;
#if _SCE_TARGET_OS_ORBIS
	textureSpec.m_textureType	= sce::Gnm::kTextureType2d;
	textureSpec.m_format		= sce::Gnm::kDataFormatR8G8B8A8UnormSrgb;
	textureSpec.m_tileModeHint	= sce::Gnm::kTileModeDisplay_LinearAligned;
	textureSpec.m_minGpuMode	= sce::Gnm::getGpuMode();
	// initialize texture
	ret = outTexture.init(&textureSpec);
	const sce::Gnm::SizeAlign textureSizeAlign = outTexture.getSizeAlign();
#endif
#if _SCE_TARGET_OS_PROSPERO
	textureSpec.m_type			= sce::Agc::Core::Texture::Type::k2d;
	textureSpec.m_format		= sce::Agc::Core::DataFormat({ sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 });
	textureSpec.m_tileMode		= sce::Agc::Core::Texture::TileMode::kLinear;
	textureSpec.m_allowNullptr	= true;
	// initialize texture
	ret = sce::Agc::Core::initialize(&outTexture, &textureSpec);
	const sce::Agc::SizeAlign	textureSizeAlign	= sce::Agc::Core::getSize(&textureSpec);
#endif
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// allocate memory for output image
	pTextureDataAddress = videoMemory.allocate(textureSizeAlign.m_size, textureSizeAlign.m_align);
	SCE_SAMPLE_UTIL_ASSERT(pTextureDataAddress != nullptr);
	if (pTextureDataAddress == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}
#if _SCE_TARGET_OS_ORBIS
	outTexture.setBaseAddress(pTextureDataAddress);
#endif
#if _SCE_TARGET_OS_PROSPERO
	outTexture.setDataAddress(pTextureDataAddress);
#endif

	// query memory size for PNG decoder
	createParam.thisSize		= sizeof(createParam);
	createParam.attribute		= imageInfo.bitDepth >> 4;
	createParam.maxImageWidth	= imageInfo.imageWidth;
	ret	= scePngDecQueryMemorySize(&createParam);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if ((ret == SCE_PNG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_PARAM)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_SIZE)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// allocate memory for PNG decoder
	int32_t	decSize	= ret;
	sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]>	decoderMemory = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(decSize, 4, cpuMemory);
	if (decoderMemory.get() == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}

	// create PNG decoder
	ret	= scePngDecCreate(&createParam, decoderMemory.get(), decSize, &handle);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if ((ret == SCE_PNG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_PARAM)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_SIZE)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// allocate for untiled surface
	sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]>	untiledSurface = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(textureSizeAlign.m_size, textureSizeAlign.m_align, cpuMemory);
	if (untiledSurface.get() == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}

	// decode PNG image
	decodeParam.pngMemAddr		= pImageData;
	decodeParam.pngMemSize		= imageSize;
	decodeParam.imageMemAddr	= untiledSurface.get();
	decodeParam.imageMemSize	= textureSizeAlign.m_size;
	decodeParam.imagePitch		= outTexture.getWidth() * 4;
	decodeParam.pixelFormat		= SCE_PNG_DEC_PIXEL_FORMAT_R8G8B8A8;
	decodeParam.alphaValue		= 255;
	ret	= scePngDecDecode(handle, &decodeParam, nullptr);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		scePngDecDelete(handle);
		if ((ret == SCE_PNG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_PARAM)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_SIZE)
			|| (ret == SCE_PNG_DEC_ERROR_INVALID_HANDLE)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// delete PNG decoder
	ret	= scePngDecDelete(handle);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if (ret == SCE_PNG_DEC_ERROR_INVALID_HANDLE) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	ret	= tileSurface(pTextureDataAddress, untiledSurface.get(), textureSpec, 0, 0);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	return	SCE_OK;
}

// JPEG loading implementation
static uint32_t	getJpegSize(const void	*pImageData)
{
#define ADVANCE_PTR_AND_ADD_SIZE(size) pData += (size); retSize += (size);

	uint32_t		retSize	= 0;
	const uint8_t	*pData	= (uint8_t *)pImageData;

	// Skip SOI
	ADVANCE_PTR_AND_ADD_SIZE(2);

	// Read segment
	while (1)
	{
		// Segment (2 + N byte)
		// 2 byte   : Marker
		// 2 byte   : Length = N
		// N-2 byte : Segment parameter

		// Invalid jpeg or failed to parse image
		SCE_SAMPLE_UTIL_ASSERT(*pData == 0xff);
		if (*pData != 0xff) {
			retSize	= 0;
			break;
		}

		// Check marker
		if (*pData == 0xff && *(pData + 1) == 0xd9) {
			// EOI
			retSize	+= 2;
			break;
		}
		// Keep SOS or not
		bool	isSOS = ((*pData == 0xff) && (*(pData + 1) == 0xda));

		// skip marker
		ADVANCE_PTR_AND_ADD_SIZE(2);
		// segment size
		uint32_t	length = (*pData << 8) | *(pData + 1);
		// next segment
		ADVANCE_PTR_AND_ADD_SIZE(length);

		// Compressed data exists after SOS segment
		if (isSOS) {
			// Skip archived data and find EOI
			while (1) {
				uint8_t	addVal = 1;
				if (*pData == 0xff) {
					if (*(pData + 1) == 0xd9) {
						retSize	+= 2;
						break;
					}
					addVal = 2;
				}
				ADVANCE_PTR_AND_ADD_SIZE(addVal);
			}
			break;
		}
	}
#undef ADVANCE_PTR_AND_ADD_SIZE

	return	retSize;
}

static TextureKind	checkTextureFormat(const void	*pFileData)
{
	// Is this the Gnf ?
	const sce::Gnf::GnfFile	*pGnf = reinterpret_cast<const sce::Gnf::GnfFile *>(pFileData);
	if (pGnf->header.m_magicNumber == sce::Gnf::kMagic) {
		return	TextureKind::kGnf;
	}
	// Is this the Png ?
	const uint64_t	kPngMagic	= 0x0a1a0a0d474e5089;
	if (*(reinterpret_cast<const uint64_t*>(pFileData)) == kPngMagic) {
		return	TextureKind::kPng;
	}
	// Is this the Jpeg ?
	const uint16_t	kJpegSoi	= 0xd8ff;
	if (*(reinterpret_cast<const uint16_t*>(pFileData)) == kJpegSoi) {
		return	TextureKind::kJpeg;
	}
	// Unsupported format
	return	TextureKind::kUnsupported;
}

static int	decodeJpeg(Compat::Texture	&outTexture, const void	*pImageData, uint32_t	imageSize, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory, bool	isYuyv)
{
	int	ret = SCE_OK;

	// Decode jpeg file
	SceJpegDecParseParam	parseParam;
	SceJpegDecImageInfo		imageInfo;
	SceJpegDecCreateParam	createParam;
	SceJpegDecHandle		handle;
	SceJpegDecDecodeParam	decodeParam;

	// get image info.
	parseParam.jpegMemAddr	= pImageData;
	parseParam.jpegMemSize	= imageSize;
	parseParam.decodeMode	= SCE_JPEG_DEC_DECODE_MODE_NORMAL;
	parseParam.downScale	= 1;
	ret	= sceJpegDecParseHeader(&parseParam, &imageInfo);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if ((ret == SCE_JPEG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_SIZE)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_PARAM)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}
	SCE_SAMPLE_UTIL_ASSERT(imageInfo.colorSpace != SCE_JPEG_DEC_COLOR_SPACE_UNKNOWN);
	if (imageInfo.colorSpace == SCE_JPEG_DEC_COLOR_SPACE_UNKNOWN) {
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	// setup texture
	void	*pTextureDataAddress = nullptr;
	Compat::TextureSpec textureSpec;
	textureSpec.init();
	textureSpec.m_width			= isYuyv ? ((imageInfo.outputImageWidth + (imageInfo.outputImageWidth & 1u)) / 2) : imageInfo.outputImageWidth;
	textureSpec.m_height		= imageInfo.outputImageHeight;
#if _SCE_TARGET_OS_ORBIS
	textureSpec.m_textureType	= sce::Gnm::kTextureType2d;
	textureSpec.m_format		= sce::Gnm::kDataFormatR8G8B8A8UnormSrgb;
	textureSpec.m_tileModeHint	= sce::Gnm::kTileModeDisplay_LinearAligned;
	textureSpec.m_minGpuMode	= sce::Gnm::getGpuMode();
	// initialize texture
	ret = outTexture.init(&textureSpec);

	const sce::Gnm::SizeAlign textureSizeAlign		= outTexture.getSizeAlign();
#endif
#if _SCE_TARGET_OS_PROSPERO
	textureSpec.m_type			= sce::Agc::Core::Texture::Type::k2d;
	textureSpec.m_format		= sce::Agc::Core::DataFormat({ sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 });
	textureSpec.m_allowNullptr	= true;
	// initialize texture
	ret = sce::Agc::Core::initialize(&outTexture, &textureSpec);

	const sce::Agc::SizeAlign	textureSizeAlign	= sce::Agc::Core::getSize(&textureSpec);
#endif
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// allocate memory for output image
	pTextureDataAddress = videoMemory.allocate(textureSizeAlign.m_size, textureSizeAlign.m_align);
	SCE_SAMPLE_UTIL_ASSERT(pTextureDataAddress != nullptr);
	if (pTextureDataAddress == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}
#if _SCE_TARGET_OS_ORBIS
	outTexture.setBaseAddress(pTextureDataAddress);
#endif
#if _SCE_TARGET_OS_PROSPERO
	outTexture.setDataAddress(pTextureDataAddress);
#endif

	// allocate coefficient memory to be used for decoding progressive JPEG, if needed
	sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]>	coefficientMemory;
	if (imageInfo.coefficientMemSize > 0) {
		coefficientMemory = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(imageInfo.coefficientMemSize, 4, cpuMemory);
		SCE_SAMPLE_UTIL_ASSERT(coefficientMemory.get() != nullptr);
		if (coefficientMemory.get() == nullptr) {
			return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
	}

	// query memory size for JPEG decoder
	createParam.thisSize		= sizeof(createParam);
	createParam.attribute		= imageInfo.suitableCscAttribute;
	createParam.maxImageWidth	= imageInfo.outputImageWidth;
	ret	= sceJpegDecQueryMemorySize(&createParam);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if ((ret == SCE_JPEG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_SIZE)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_PARAM)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// allocate memory for JPEG decoder
	size_t	decoderSize	= ret;
	sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]>	decoderMemory = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(decoderSize, 4, cpuMemory);
	if (decoderMemory.get() == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}

	// create JPEG decoder
	ret	= sceJpegDecCreate(&createParam, decoderMemory.get(), decoderSize, &handle);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if ((ret == SCE_JPEG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_SIZE)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_PARAM)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// allocate for untiled surface
	sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]>	untiledSurface = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(textureSizeAlign.m_size, textureSizeAlign.m_align, cpuMemory);
	if (untiledSurface.get() == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}

	// decode JPEG image
	decodeParam.jpegMemAddr			= pImageData;
	decodeParam.jpegMemSize			= imageSize;
	decodeParam.imageMemAddr		= untiledSurface.get();
	decodeParam.imageMemSize		= textureSizeAlign.m_size;
	decodeParam.decodeMode			= parseParam.decodeMode;
	decodeParam.downScale			= parseParam.downScale;
	decodeParam.pixelFormat			= isYuyv ? SCE_JPEG_DEC_PIXEL_FORMAT_Y8U8Y8V8 : SCE_JPEG_DEC_PIXEL_FORMAT_R8G8B8A8;
	decodeParam.imagePitch			= outTexture.getWidth() * 4;
	decodeParam.alphaValue			= 255;
	decodeParam.coefficientMemAddr	= (imageInfo.coefficientMemSize > 0) ? coefficientMemory.get() : nullptr;
	decodeParam.coefficientMemSize	= imageInfo.coefficientMemSize;
	ret	= sceJpegDecDecode(handle, &decodeParam, nullptr);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		sceJpegDecDelete(handle);
		if ((ret == SCE_JPEG_DEC_ERROR_INVALID_ADDR)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_SIZE)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_PARAM)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_HANDLE)
			|| (ret == SCE_JPEG_DEC_ERROR_INVALID_COEF_MEMORY)
			) {
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// delete JPEG decoder
	ret	= sceJpegDecDelete(handle);
	SCE_SAMPLE_UTIL_ASSERT(ret >= 0);
	if (ret < 0) {
		if (ret == SCE_JPEG_DEC_ERROR_INVALID_HANDLE) {
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	// tileSurface
	ret	= tileSurface(pTextureDataAddress, untiledSurface.get(), textureSpec, 0, 0);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return	SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	return	SCE_OK;
}

static	inline int loadPngFile(Compat::Texture& outTexture, const char* pFilename, VideoAllocator& videoMemory, SceLibcMspace	cpuMemory)
{
	const FileBlob	file(pFilename, cpuMemory);
	if (!file.isOk()) {
		return	file.m_errorCode;
	}
	return	decodePng(outTexture, file.m_data.get(), file.m_size, videoMemory, cpuMemory);
}

static inline int	loadJpegFile(Compat::Texture& outTexture, const char* pFilename, VideoAllocator& videoMemory, SceLibcMspace	cpuMemory, bool	isYuyv = false)
{
	const FileBlob file(pFilename, cpuMemory);
	if (!file.isOk()) {
		return file.m_errorCode;
	}

	return	decodeJpeg(outTexture, file.m_data.get(), file.m_size, videoMemory, cpuMemory, isYuyv);
}

} // namespace Details

int	createFromFile(Compat::Texture	&outTexture, const char	*pFilename, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory)
{
	std::string	path = pFilename;
	size_t	dotIndex = path.find_last_of(".");
	SCE_SAMPLE_UTIL_ASSERT(dotIndex != path.npos);
	if (dotIndex == path.npos) {
		// Extension is not included in the path.
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}
	++dotIndex;

	// This supports 3 types of file format. (gnf, png, jpeg)
	std::string	ext = path.substr(dotIndex, path.size() - dotIndex);
	int	ret = SCE_OK; (void)ret;
	// Gnf
	if ((ext == "GNF") || (ext == "gnf")) {
		ret = Details::loadGnfFile(outTexture, pFilename, videoMemory, cpuMemory);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}
	// PNG
	else if ((ext == "PNG") || (ext == "png")) {
		ret = Details::loadPngFile(outTexture, pFilename, videoMemory, cpuMemory);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}
	// Jpeg
	else if ((ext == "jpg") || (ext == "JPG") || (ext == "jpeg") || (ext == "JPEG")) {
		ret = Details::loadJpegFile(outTexture, pFilename, videoMemory, cpuMemory);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}
	else {
		// File format is not supported.
		SCE_SAMPLE_UTIL_ASSERT(false);
		return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	if (ret != SCE_OK) {
		return	ret;
	}

#if _SCE_TARGET_OS_ORBIS
	std::vector<sce::Gnm::ResourceType> resTypes = { sce::Gnm::kResourceTypeTextureBaseAddress };
	videoMemory.registerResource(outTexture.getBaseAddress(), outTexture.getSizeAlign().m_size, pFilename, resTypes);
#endif
#if _SCE_TARGET_OS_PROSPERO
	std::vector<sce::Agc::ResourceRegistration::ResourceType> resTypes = { sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress };
	videoMemory.registerResource(outTexture.getDataAddress(), sce::Agc::Core::getSize(&outTexture).m_size, pFilename, resTypes);
#endif

	return	SCE_OK;
}

int	createFromMemory(Compat::Texture	&outTexture, const void	*pImageData, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory)
{
	SCE_SAMPLE_UTIL_ASSERT(pImageData != nullptr);
	if (pImageData == nullptr) {
		return	SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	// Check texture format
	const Details::TextureKind	texType = Details::checkTextureFormat(pImageData);
	int	ret = SCE_OK;
	switch (texType)
	{
	case Details::TextureKind::kGnf:
#if _SCE_TARGET_OS_ORBIS
	{
		auto	*pHeader = reinterpret_cast<const sce::Gnf::Header *>(pImageData);

		sce::SampleUtil::Memory::Cpu::unique_ptr<uint8_t[]> contentData = sce::SampleUtil::Memory::Cpu::make_unique<uint8_t>(pHeader->m_contentsSize, 4, cpuMemory);
		sce::Gnf::Contents	*pContents = reinterpret_cast<sce::Gnf::Contents *>(contentData.get());
		if (pContents == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pContents, reinterpret_cast<const uint8_t*>(pImageData) + sizeof(sce::Gnf::Header), pHeader->m_contentsSize);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(pContents->m_version, sce::Gnf::kVersion);
		if (pContents->m_version != sce::Gnf::kVersion) {
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		auto texSizeAlign = sce::Gnf::getTexturePixelsSize(pContents, 0);
		void	*pTextureMemAddr = videoMemory.allocate(texSizeAlign.m_size, texSizeAlign.m_align);
		SCE_SAMPLE_UTIL_ASSERT(pTextureMemAddr != nullptr);
		if (pTextureMemAddr == nullptr) {
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}
		memcpy(pTextureMemAddr, (uint8_t*)pImageData + sizeof(sce::Gnf::Header) + pHeader->m_contentsSize, texSizeAlign.m_size);
		outTexture = *sce::Gnf::patchTextures(pContents, 0, 1, &pTextureMemAddr);
	}
#endif
#if _SCE_TARGET_OS_PROSPERO
		if (sce::Gnf::getVersion(pImageData) == 4) {
			ret = sce::Agc::Core::translate(&outTexture, (sce::Gnf::GnfFile *)pImageData);
		} else if (sce::Gnf::getVersion(pImageData) == 5) {
			ret = sce::Agc::Core::translate(&outTexture, (sce::Gnf::GnfFileV5 *)pImageData);
		} else {
			ret = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
#endif
		break;
	case Details::TextureKind::kPng:
	{
		const uint32_t	imageSize = Details::getPngSize(pImageData);
		ret = Details::decodePng(outTexture, pImageData, imageSize, videoMemory, cpuMemory);
	}
	break;
	case Details::TextureKind::kJpeg:
	{
		const uint32_t	imageSize = Details::getJpegSize(pImageData);
		ret = Details::decodeJpeg(outTexture, pImageData, imageSize, videoMemory, cpuMemory, false);
	}
	break;
	default:
		ret = SCE_SAMPLE_UTIL_ERROR_FATAL;
		break;
	}
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	return	ret;
}


}}}} // namespace sce::SampleUtil::Graphics::Texture
