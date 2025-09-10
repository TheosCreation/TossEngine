/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <utility>
#include <vectormath/cpp/vectormath_aos.h>
#include <agc/registerstructs.h>
#include <agc/commandbuffer.h>
#include <agc/core/rendertarget.h>
#include <agc/core/texture.h>
#include "sampleutil/memory.h"
#include "sampleutil/graphics/surface_utility.h"

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 *
	 * @brief Render target attribute
	 * @details This is passed as parameter when creating render target
	 * @~Japanese
	 *
	 * @brief レンダーターゲットの属性
	 * @details レンダーターゲットを作成する際にパラメータとして渡します
	 */
	struct RenderTargetParam
	{
		/*!
		 * @~English
		 * @brief name
		 * @~Japanese
		 * @brief 名前
		 */
		std::string								m_name;
		/*!
		 * @~English
		 * @brief The spec to initialize Agc::CxRenderTarget
		 * @~Japanese
		 * @brief Agc::CxRenderTargetを初期化する際の仕様
		 */
		Agc::Core::RenderTargetSpec				m_spec;
		/*!
		 * @~English
		 * @brief Render target attributes
		 * @details Specify logical-or of sce::SampleUtil::Graphics::SurfaceUtil::Flags enum values
		 * @~Japanese
		 * @brief レンダーターゲットの属性
		 * @details sce::SampleUtil::Graphics::SurfaceUtil::Flags enum値の論理和で指定
		 */
		int										m_flags;
		/*!
		 * @~English
		 * @brief Color clear value
		 * @~Japanese
		 * @brief カラークリア値
		 */
		Vectormath::Simd::Aos::Vector4			m_clearColor;
		RenderTargetParam() :m_name(""), m_flags(0), m_clearColor(Vectormath::Simd::Aos::Vector4(0.f)) { m_spec.init(); }
		/*!
		 * @~English
		 * @brief Constructor
		 * @param name Name
		 * @param spec The spec to initialize sce::Agc::CxRenderTarget
		 * @param flags Logical-or of sce::SampleUtil::Graphics::SurfaceUtil::Flags enum values
		 * @param clearColor Color clear value
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param name 名前
		 * @param spec sce::Agc::CxRenderTargetを初期化する際の仕様
		 * @param flags sce::SampleUtil::Graphics::SurfaceUtil::Flags enum値の論理和で指定
		 * @param clearColor カラークリア値
		 */
		RenderTargetParam(const std::string	&name, const Agc::Core::RenderTargetSpec	&spec, int	flags, const Vectormath::Simd::Aos::Vector4_arg	&clearColor) :
			m_name(name), m_spec(spec), m_flags(flags), m_clearColor(clearColor) {}
		/*!
		 * @~English
		 * @brief Constructor
		 * @param name Name
		 * @param dimensions Array of width, height, num of slices, and num of mips of color render target
		 * @param format Color format
		 * @param numSamples The number of samples
		 * @param numFragments The number of fragments
		 * @param tileMode Tile mode
		 * @param dimension Either one dimension, two dimensions or three dimensions can be specified
		 * @param flags Logical-or of sce::SampleUtil::Graphics::SurfaceUtil::Flags enum values
		 * @param clearColor Color clear value
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param name 名前
		 * @param dimensions レンダーターゲットの幅、高さ、スライス数、ミップ数を指定する配列
		 * @param format カラーフォーマット
		 * @param numSamples サンプル数
		 * @param numFragments フラグメント数
		 * @param tileMode タイルモード
		 * @param dimension 1次元、2次元、3次元のいずれかを指定
		 * @param flags sce::SampleUtil::Graphics::SurfaceUtil::Flags enum値の論理和で指定
		 * @param clearColor カラークリア値
		 */
		RenderTargetParam(const std::string	&name, const std::array<uint32_t, 4>	&dimensions, const Agc::Core::DataFormat	&format,
			Agc::CxRenderTarget::NumSamples	numSamples = Agc::CxRenderTarget::NumSamples::k1,
			Agc::CxRenderTarget::NumFragments	numFragments = Agc::CxRenderTarget::NumFragments::k1, Agc::CxRenderTarget::TileMode tileMode = Agc::CxRenderTarget::TileMode::kRenderTarget, Agc::CxRenderTarget::Dimension dimension = Agc::CxRenderTarget::Dimension::k2d,uint32_t	flags = 0,
			const Vectormath::Simd::Aos::Vector4_arg	&clearColor = Vectormath::Simd::Aos::Vector4(0.f)) :m_name(name), m_flags(flags), m_clearColor(clearColor)
		{
			m_spec.init();
			m_spec.m_width			= dimensions[0];
			m_spec.m_height			= dimensions[1];
			m_spec.m_numSlices		= dimensions[2];
			m_spec.m_numMips		= dimensions[3];
			m_spec.m_format			= format;
			m_spec.m_numSamples		= numSamples;
			m_spec.m_numFragments	= numFragments;
			m_spec.m_tileMode		= tileMode;
			m_spec.m_dimension		= dimension;
			if (flags & SurfaceUtil::Flags::kUseCmask)
			{
				m_spec.m_compression |= Agc::Core::MetadataCompression::kCmaskFastClear;
			}
			if (flags & (SurfaceUtil::Flags::kUseDcc | SurfaceUtil::Flags::kDccVideoOut))
			{
				Agc::Core::DccCompatibility comp = Agc::Core::DccCompatibility::kColorBuffer;
				if (flags & SurfaceUtil::Flags::kUseTcColor)
				{
					comp |= Agc::Core::DccCompatibility::kTextureReadOnly;
				}
				if (flags & SurfaceUtil::Flags::kDccVideoOut)
				{
					comp |= Agc::Core::DccCompatibility::kVideoOut;
				}
				int ret = SCE_OK; (void)ret;
				ret = Agc::Core::translate(&m_spec.m_compression, m_spec.m_width, m_spec.m_format.m_format, comp);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}
			if (numSamples > Agc::CxRenderTarget::NumSamples::k1)
			{
				if (flags & SurfaceUtil::Flags::kUseFmask)
				{
					if (flags & SurfaceUtil::Flags::kUseTcFmask)
					{
						m_spec.m_compression |= Agc::Core::MetadataCompression::kCompressedFmaskTexureCompatible;
					} else {
						m_spec.m_compression |= Agc::Core::MetadataCompression::kCompressedFmask;
					}
				}
			}
		}
		bool operator==(const RenderTargetParam &rhs)
		{
			return (m_spec.m_width == rhs.m_spec.m_width) && (m_spec.m_height == rhs.m_spec.m_height) &&
				(m_spec.m_depth == rhs.m_spec.m_depth) && (m_spec.m_numSlices == rhs.m_spec.m_numSlices) &&
				(m_spec.m_numMips == rhs.m_spec.m_numMips) &&
				(memcmp(&m_spec.m_format, &rhs.m_spec.m_format, sizeof(Agc::Core::DataFormat)) == 0) &&
				(m_spec.m_numSamples == rhs.m_spec.m_numSamples) &&
				(m_spec.m_numFragments == rhs.m_spec.m_numFragments) &&
				(m_spec.m_compression == rhs.m_spec.m_compression) &&
				(m_flags == rhs.m_flags);
		}
	}; // struct RenderTargetParam

	/*!
	 * @~English
	 *
	 * @brief Render target
	 * @details This is wrapper for sce::Agc::CxRenderTarget
	 * @~Japanese
	 *
	 * @brief レンダーターゲット
	 * @details sce::Agc::CxRenderTargetのラッパーです。
	 */
	struct RenderTargetWrapper
	{
		Agc::CxRenderTarget						m_cxRenderTarget;
		bool									m_isInitialized = false;
		bool									m_wasForceClearColorBuffer = false;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_colorMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_cmaskMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_fmaskMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_dccMemory;
		Vectormath::Simd::Aos::Vector4			m_clearColor;
		bool									m_needToWaitCsClear = false;
		bool									m_needToWaitPsClear = false;
		std::string								m_resourceName;
		int										m_flags = 0;
		Agc::Core::GpuSyncPostProducerResult	m_clearResult = {};
		Memory::Gpu::unique_ptr<Agc::Label>		m_clearSync;

		const RenderTargetWrapper& operator=(RenderTargetWrapper	&&rhs)
		{
			this->m_cxRenderTarget				= rhs.m_cxRenderTarget; rhs.m_cxRenderTarget.init();
			this->m_isInitialized				= rhs.m_isInitialized; rhs.m_isInitialized = false;
			this->m_wasForceClearColorBuffer	= rhs.m_wasForceClearColorBuffer; rhs.m_wasForceClearColorBuffer = false;
			this->m_colorMemory					= std::move(rhs.m_colorMemory);
			this->m_cmaskMemory					= std::move(rhs.m_cmaskMemory);
			this->m_fmaskMemory					= std::move(rhs.m_fmaskMemory);
			this->m_dccMemory					= std::move(rhs.m_dccMemory);
			this->m_clearColor					= rhs.m_clearColor;
			this->m_needToWaitCsClear			= rhs.m_needToWaitCsClear; rhs.m_needToWaitCsClear = false;
			this->m_needToWaitPsClear			= rhs.m_needToWaitPsClear; rhs.m_needToWaitPsClear = false;
			this->m_resourceName				= std::move(rhs.m_resourceName);
			this->m_flags						= rhs.m_flags; rhs.m_flags = 0;
			this->m_clearSync					= std::move(rhs.m_clearSync);
			this->m_clearResult					= rhs.m_clearResult; rhs.m_clearResult = {};

			return *this;
		}

		RenderTargetWrapper(RenderTargetWrapper &&rhs) { *this = std::move(rhs); }
		RenderTargetWrapper() { m_cxRenderTarget.init(); }
		/*!
		 * @~English
		 * @brief Constructor
		 * @param param Parameter to be used to initialize render target
		 * @param videoMemory Video memory allocartor used to initalize render target
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param param レンダーターゲットを初期化するパラメータ
		 * @param videoMemory レンダーターゲット初期化に使用するビデオメモリアロケータ
		 */
		RenderTargetWrapper(const RenderTargetParam	&param, VideoAllocator &videoMemory)
		{
			init(param, videoMemory);
		}
		virtual ~RenderTargetWrapper() {}

		/*!
		 * @~English
		 * @brief Initializes render target
		 * @param param Attribute to be used to initialize render target
		 * @param videoMemory Video memory allocartor used to initalize render target
		 * @~Japanese
		 * @brief レンダーターゲットを初期化
		 * @param param レンダーターゲットの初期化属性
		 * @param videoMemory レンダーターゲット初期化に使用するビデオメモリアロケータ
		 */
		void	init(const RenderTargetParam	&param, VideoAllocator	&videoMemory);
		/*!
		 * @~English
		 * @brief Finalizes render target
		 * @~Japanese
		 * @brief レンダーターゲットを解放
		 */
		void	fini();
		/*!
		 * @~English
		 * @brief Obtains render target attribute
		 * @return レンダーターゲットの初期化属性
		 * @~Japanese
		 * @brief レンダーターゲットの初期化属性を取得
		 * @return レンダーターゲットの初期化属性
		 */
		RenderTargetParam	getParam() const;

		/*!
		 * @~English
		 * @brief Starts clear
		 * @param dcb DrawCommandBuffer
		 * @param shaderType sce::SampleUtil::Graphics::ShaderType::kGraphics or ce::SampleUtil::Graphics::ShaderType::kCompute can be specified
		 * @param forceClearSurface Sepcify if depth surface is to be cleared
		 * @~Japanese
		 * @brief クリアを開始
		 * @param dcb DrawCommandBuffer
		 * @param shaderType sce::SampleUtil::Graphics::ShaderType::kGraphicsもしくはce::SampleUtil::Graphics::ShaderType::kComputeを指定
		 * @param forceClearSurface 常にデプスサーフェスをクリアするかどうかを指定
		 */
		void	clear(Agc::DrawCommandBuffer	*dcb, ShaderType	shaderType = ShaderType::kGraphics, bool	forceClearSurface = false);

		/*!
		 * @~English
		 * @brief Waits for clear completion
		 * @param dcb DrawCommandBuffer
		 * @~Japanese
		 * @brief クリアの完了待ち
		 * @param dcb DrawCommandBuffer
		 */
		void	syncClear(Agc::DrawCommandBuffer	*dcb);

		/*!
		 * @~English
		 * @brief Obtains color texture
		 * @return Color texture
		 * @~Japanese
		 * @brief カラーテクスチャを取得
		 * @return カラーテクスチャ
		 */
		Agc::Core::Texture	getColorTexture() const;
		/*!
		 * @~English
		 * @brief Obtains fmask texture
		 * @return Fmask texture
		 * @~Japanese
		 * @brief Fmaskテクスチャを取得
		 * @return Fmaskテクスチャ
		 */
		Agc::Core::Texture	getFmaskTexture() const;
	}; // struct RenderTargetWrapper

}}} // namespace sce::SampleUtil::Graphics


