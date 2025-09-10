/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <scebase_common.h>
#include "sampleutil/graphics/compat.h"
#include "sampleutil/memory.h"
#include "sampleutil/graphics/graphics_memory.h"
#include "sampleutil/thread/thread.h"
#include "sampleutil/debug/perf.h"
#if _SCE_TARGET_OS_PROSPERO
#include "sampleutil/helper/prospero/async_asset_loader_prospero.h"
#endif

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief Texture type
	 * @~Japanese
	 * @brief テクスチャタイプ
	 */
	enum class TextureType
	{
		/*!
		 * @~English
		 * @brief Emissive map
		 * @~Japanese
		 * @brief エミッシブマップ
		 */
		kEmissive = 0,
		/*!
		 * @~English
		 * @brief Diffuse map
		 * @~Japanese
		 * @brief ディフューズマップ
		 */
		kDiffuse,
		/*!
		 * @~English
		 * @brief Specular map
		 * @~Japanese
		 * @brief スペキュラマップ
		 */
		kSpecular,
		/*!
		 * @~English
		 * @brief Normal map
		 * @~Japanese
		 * @brief ノーマルマップ
		 */
		kNormal,
		/*!
		 * @~English
		 * @brief Mask map
		 * @~Japanese
		 * @brief マスクマップ
		 */
		kMask,
		/*!
		 * @~English
		 * @brief Roughness map
		 * @~Japanese
		 * @brief ラフネスマップ
		 */
		kRoughness,
		/*!
		 * @~English
		 * @brief Metalness map
		 * @~Japanese
		 * @brief メタルネスマップ
		 */
		kMetalness,
		/*!
		 * @~English
		 * @brief Ambient Occlusion map
		 * @~Japanese
		 * @brief アンビエントオクルージョンマップ
		 */
		kAmbientOcclusion,
		/*!
		 * @~English
		 * @brief Shininess map
		 * @~Japanese
		 * @brief シャイニネスマップ
		 */
		 kShininess,
		 /*!
		 * @~English
		 * @brief The number of textures
		 * @~Japanese
		 * @brief テクスチャの数
		 */
		kNum
	};

	/*!
	 * @~English
	 * @brief Texture library
	 * @~Japanese
	 * @brief テクスチャライブラリ
	 */
	class TextureLibrary : public Thread::LockableObject
	{
	public:
		/*!
		 * @~English
		 * @brief Callback called when texture file is not found
		 * @param texLib Loaded texture destination
		 * @param name Texture name
		 * @param type Texture type
		 * @param videoMemory Video memory allocator
		 * @~Japanese
		 * @brief テクスチャファイルが見つからない場合に呼ばれるコールバック
		 * @param texLib テクスチャのロード先
		 * @param name テクスチャ名
		 * @param type テクスチャタイプ
		 * @param videoMemory ビデオメモリアロケータ
		 */
		static bool(*m_pTextureLoadFailureCB)(TextureLibrary &texLib, const std::string	&name, TextureType	type, VideoAllocator	&videoMemory);
#if _SCE_TARGET_OS_ORBIS
		/*!
		 * @~English
		 * @brief Constructor
		 * @param videoMemory Video memory allocator
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param videoMemory ビデオメモリアロケータ
		 */
		TextureLibrary(VideoAllocator	&videoMemory);
#endif
#if _SCE_TARGET_OS_PROSPERO
		/*!
		 * @~English
		 * @brief Constructor
		 * @param asyncAssetLoader AsyncAssetLoader
		 * @param videoMemory Video memory allocator
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param asyncAssetLoader AsyncAssetLoader
		 * @param videoMemory ビデオメモリアロケータ
		 */
		TextureLibrary(VideoAllocator	&videoMemory, Helper::AsyncAssetLoader	*asyncAssetLoader = nullptr);
#endif
		~TextureLibrary()
		{
			UNTAG_THIS_CLASS;
		}
		int getTextureSync(Compat::Texture &outTexture, const std::string &filename, TextureType type);
		int getTextureAsync(Compat::Texture &outTexture, const std::string &filename, TextureType type);
		/*!
		 * @~English
		 * @brief Obtains texture
		 * @details Search for a texture with specified parameter. If not found in library, loads from texture file, and return it.
		 * @param outTexture Texture destination
		 * @param name Tetxure name
		 * @param type Texture type
		 * @retval  SCE_OK Success
		 * @retval SCE_SAMPLE_UTIL_ERROR_FILE_OPEN No texture file found
		 * @retval SCE_SAMPLE_UTIL_ERROR_FILE_LOAD Invalid texture file
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief テクスチャの取得
		 * @details 指定したパラメータに合致するテクスチャを検索する。存在しない場合はファイルからロードする。
		 * @param outTexture 取得したテクスチャの格納先
		 * @param name テクスチャ名
		 * @param type テクスチャタイプ
		 * @retval  SCE_OK 成功。
		 * @retval SCE_SAMPLE_UTIL_ERROR_FILE_OPEN テクスチャファイルが存在しない
		 * @retval SCE_SAMPLE_UTIL_ERROR_FILE_LOAD テクスチャファイルの内容に不整合がある
		 * @retval (<0) エラーコード
		 */
		int getTexture(Compat::Texture &outTexture, const std::string &name, TextureType type = TextureType::kDiffuse);
		/*!
		 * @~English
		 * @brief Resolve loading texture
		 * @details Completes loading of specified texture, Fixes-up Gnf file, and make it usable texture.
		 * @param texture Texture to be resolved loading
		 * @retval  SCE_OK Success.
		 * @retval  SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM The texture to which getTexture() is not called is specified.
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief ロード中テクスチャの解決
		 * @details 指定したテクスチャのロードを完了し、Gnfファイルをフィックスアップし、使用可能なテクスチャとする。
		 * @param texture ロード解決するテクスチャ
		 * @retval  SCE_OK 成功。
		 * @retval  SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM getTexture()でロードを開始していないテクスチャが指定された。
		 * @retval (<0) エラーコード
		 */
		int resolveTexture(Compat::Texture &texture);

		VideoAllocator										*m_pVideoMemory;
		std::unordered_map<std::string, Compat::Texture>	m_textures;
		std::vector<Memory::Gpu::unique_ptr<uint8_t[]>>		m_textureData;
#if _SCE_TARGET_OS_PROSPERO
		Helper::AsyncAssetLoader							*m_pAsyncAssetLoader;
		std::unordered_map<const void *, std::string>		m_loadAddr2name;
#endif
	}; // class TextureLibrary
} } } // namespace sce::SampleUtl::Graphics

