/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <string>
#include <scebase_common.h>
#include <vectormath/cpp/vectormath_aos.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core/texture.h>
#include <agc/core/buffer.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnm/texture.h>
#include <gnm/buffer.h>
#endif
#include "graphics_memory.h"

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief Specify whether kGraphics or kCompute
	 * @~Japanese
	 * @brief kGraphics,kComputeのいずれかを指定
	 */
	enum class ShaderType
	{
		/*!
		 * @~English
		 * @brief Graphics
		 * @~Japanese
		 * @brief Graphics
		 */
		kGraphics,
		/*!
		 * @~English
		 * @brief Compute
		 * @~Japanese
		 * @brief Compute
		 */
		kCompute
	};

	// forward declarations
	struct RenderTargetWrapper;
	struct DepthRenderTargetWrapper;

	/*!
	 * @~English
	 * @brief Surface utility
	 * @details Render target and depth render target
	 * @~Japanese
	 * @brief サーフェスユーティリティ
	 * @details レンダーターゲット・デプスレンダーターゲットなど
	 */
	namespace SurfaceUtil
	{
		/*!
		 * @~English
		 * @brief Render target/depth render target initialize flags
		 * @~Japanese
		 * @brief レンダーターゲット・デプスレンダーターゲットの初期化フラグ
		 */
		enum Flags
		{
			/*!
			 * @~English
			 * @brief No flag is specified
			 * @~Japanese
			 * @brief フラグ指定なし
			 */
			kNoFlag				= 0,
			/*!
			 * @~English
			 * @brief Read-only texture
			 * @~Japanese
			 * @brief 読み込み専用テクスチャ
			 */
			kUseRoTexture		= 1,
			/*!
			 * @~English
			 * @brief Read/write texture
			 * @~Japanese
			 * @brief 読み書き可能テクスチャ
			 */
			kUseRwTexture		= 2,
			/*!
			 * @~English
			 * @brief Render-to-texture
			 * @~Japanese
			 * @brief Render-to-texture
			 */
			kIsRenderToTexture	= 4,
			/*!
			 * @~English
			 * @brief Stencil texture enabled
			 * @~Japanese
			 * @brief ステンシルテクスチャ有効
			 */
			kUseStencilTexture	= 8,
			/*!
			 * @~English
			 * @brief CMask enabled
			 * @~Japanese
			 * @brief CMask有効
			 */
			kUseCmask			= 16,
			/*!
			 * @~English
			 * @brief FMask enabled
			 * @~Japanese
			 * @brief FMask有効
			 */
			kUseFmask			= 32,
			/*!
			 * @~English
			 * @brief TC color enabled
			 * @~Japanese
			 * @brief TC color有効
			 */
			kUseTcColor			= 64,
			/*!
			 * @~English
			 * @brief TC fmask enabled
			 * @~Japanese
			 * @brief TC fmask有効
			 */
			kUseTcFmask			= 128,
			/*!
			 * @~English
			 * @brief TC depth enabled
			 * @~Japanese
			 * @brief TC depth有効
			 */
			kUseTcDepth			= 256,
			/*!
			 * @~English
			 * @brief DCC enabled
			 * @~Japanese
			 * @brief DCC有効
			 */
			kUseDcc				= 512,
			/*!
			 * @~English
			 * @brief Video out DCC enabled
			 * @~Japanese
			 * @brief Video out用DCC有効
			 */
			kDccVideoOut		= 1024,
			/*!
			 * @~English
			 * @brief HTILE depth enabled
			 * @~Japanese
			 * @brief HTILE depth有効
			 */
			kUseHtileDepth		= 2048,
			/*!
			 * @~English
			 * @brief HTILE stencil enabled
			 * @~Japanese
			 * @brief HTILE stencil有効
			 */
			kUseHtileStencil	= 4096
		};

		/*!
		 * @~English
		 * @brief Initializes sce::SampleUtil::Graphics::SurfaceUtility
		 * @param allocator Video memory allocator
		 * @~Japanese
		 * @brief sce::SampleUtil::Graphics::SurfaceUtility初期化
		 * @param allocator ビデオメモリアロケータ
		 */
		void	initialize(VideoAllocator	&allocator);
		/*!
		 * @~English
		 * @brief Finalizes sce::SampleUtil::Graphics::SurfaceUtility
		 * @~Japanese
		 * @brief sce::SampleUtil::Graphics::SurfaceUtility終了処理
		 */
		void	finalize();
	}
}}} // namespace sce::SampleUtil::Graphics

#if _SCE_TARGET_OS_PROSPERO
#include "platform_agc/surface_utility_agc.h"
#endif
#if _SCE_TARGET_OS_ORBIS
#include "platform_gnm/surface_utility_gnm.h"
#endif
