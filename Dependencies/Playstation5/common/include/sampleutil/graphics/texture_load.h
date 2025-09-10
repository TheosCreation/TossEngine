/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <mspace.h>
#include <sampleutil/graphics/compat.h>
#include <sampleutil/graphics/graphics_memory.h>

namespace sce { namespace SampleUtil { namespace Graphics { namespace Texture {

	/*!
	 * @~English
	 * @brief Create texture from file
	 * @param outTexture created texture will be stored to this variable
	 * @param pFilename texture file name
	 * @param gpuMemory video memory allocator
	 * @param cpuMemory CPU memory allocator
	 * @~Japanese
	 * @brief ファイルからのテクスチャ作成
	 * @param outTexture 作成されたテクスチャの格納先
	 * @param pFilename テクスチャファイル名
	 * @param videoMemory ビデオメモリアロケータ
	 * @param cpuMemory CPUメモリアロケータ
	 */
	int	createFromFile(Compat::Texture	&outTexture, const char	*pFilename, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory);

	/*!
	 * @~English
	 * @brief Create texture from memory
	 * @param outTexture created texture will be stored to this variable
	 * @param pImageData memory which contains texture data
	 * @param gpuMemory video memory allocator
	 * @param cpuMemory CPU memory allocator
	 * @~Japanese
	 * @brief メモリからのテクスチャ作成
	 * @param outTexture 作成されたテクスチャの格納先
	 * @param pImageData テクスチャデータのあるメモリ
	 * @param videoMemory ビデオメモリアロケータ
	 * @param cpuMemory CPUメモリアロケータ
	 */
	int	createFromMemory(Compat::Texture	&outTexture, const void	*pImageData, VideoAllocator	&videoMemory, SceLibcMspace	cpuMemory);

}}}} // namespace sce::SampleUtl::Graphics
