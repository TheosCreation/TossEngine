/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <utility>
#include <agc/registerstructs.h>
#include <agc/core/depthrendertarget.h>
#include <agc/core/texture.h>
#include <agc/core/sync.h>
#include <agc/toolkit/toolkit.h>
#include "sampleutil/memory.h"
#include "sampleutil/graphics/surface_utility.h"
#include "sampleutil/debug/perf.h"

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 *
	 * @brief Depth render target attribute
	 * @details This is passed as parameter when creating depth render target
	 * @~Japanese
	 *
	 * @brief デプスレンダーターゲットの属性
	 * @details デプスレンダーターゲットを作成する際にパラメータとして渡します
	 */
	struct DepthRenderTargetParam
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
		 * @brief The spec to initialize sce::Agc::CxDepthRenderTarget
		 * @~Japanese
		 * @brief sce::Agc::CxDepthRenderTargetを初期化する際の仕様
		 */
		Agc::Core::DepthRenderTargetSpec		m_spec;
		/*!
		 * @~English
		 * @brief Depth render target attributes
		 * @details Specify logical-or of sce::SampleUtil::Graphics::SurfaceUtil::Flags enum values
		 * @~Japanese
		 * @brief デプスレンダーターゲットの属性
		 * @details sce::SampleUtil::Graphics::SurfaceUtil::Flags enum値の論理和で指定
		 */
		int										m_flags;
		/*!
		 * @~English
		 * @brief Depth clear value
		 * @~Japanese
		 * @brief デプスクリア値
		 */
		float									m_depthClearValue;
		/*!
		 * @~English
		 * @brief Stencil clear value
		 * @~Japanese
		 * @brief ステンシルクリア値
		 */
		uint8_t									m_stencilClearValue;
		DepthRenderTargetParam() :m_name(""), m_flags(0), m_depthClearValue(1.f), m_stencilClearValue(0) { m_spec.init(); }
		/*!
		 * @~English
		 * @brief Constructor
		 * @param name Name
		 * @param spec The spec to initialize sce::Agc::CxDepthRenderTarget
		 * @param flags Logical-or of sce::SampleUtil::Graphics::SurfaceUtil::Flags enum values
		 * @param depthClearValue Depth clear value
		 * @param stencilClearValue Stencil clear value
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param name 名前
		 * @param spec sce::Agc::CxDepthRenderTargetを初期化する際の仕様
		 * @param flags sce::SampleUtil::Graphics::SurfaceUtil::Flags enum値の論理和で指定
		 * @param depthClearValue デプスクリア値
		 * @param stencilClearValue ステンシルクリア値
		 */
		DepthRenderTargetParam(const std::string	&name, const Agc::Core::DepthRenderTargetSpec	&spec, int	flags, float	depthClearValue, uint8_t	stencilClearValue) :
			m_name(name), m_spec(spec), m_flags(flags), m_depthClearValue(depthClearValue), m_stencilClearValue(stencilClearValue) {}
		/*!
		 * @~English
		 * @brief Constructor
		 * @param name Name
		 * @param dimensions Array of width, height, num of slices, and num of mips of depth render target
		 * @param zFormat Depth format
		 * @param sFormat Stencil format
		 * @param numFragments The number of fragments
		 * @param flags Depth render target attributes
		 * @param depthClearValue Depth clear value
		 * @param stencilClearValue Stencil clear value
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param name 名前
		 * @param dimensions デプスレンダーターゲットの幅、高さ、スライス数、ミップ数を指定する配列
		 * @param zFormat デプスフォーマット
		 * @param sFormat ステンシルフォーマット
		 * @param numFragments フラグメント数
		 * @param flags sce::SampleUtil::Graphics::SurfaceUtil::Flags enum値の論理和で指定
		 * @param depthClearValue デプスクリア値
		 * @param stencilClearValue ステンシルクリア値
		 */
		DepthRenderTargetParam(const std::string	&name, const std::array<uint32_t, 4>	&dimensions,
			Agc::CxDepthRenderTarget::DepthFormat	zFormat = Agc::CxDepthRenderTarget::DepthFormat::k32Float,
			Agc::CxDepthRenderTarget::StencilFormat	sFormat = Agc::CxDepthRenderTarget::StencilFormat::kInvalid,
			Agc::CxDepthRenderTarget::NumFragments	numFragments = Agc::CxDepthRenderTarget::NumFragments::k1,
			uint32_t	flags = 0, float	depthClearValue = 1.f, uint8_t	stencilClearValue = 0): m_name(name), m_flags(flags), m_depthClearValue(depthClearValue), m_stencilClearValue(stencilClearValue)
		{
			m_spec.init();
			m_spec.m_width			= dimensions[0];
			m_spec.m_height			= dimensions[1];
			m_spec.m_numSlices		= dimensions[2];
			m_spec.m_numMips		= dimensions[3];
			m_spec.m_depthFormat	= zFormat;
			m_spec.m_stencilFormat	= sFormat;
			m_spec.m_numFragments	= numFragments;
			if (flags & (SurfaceUtil::Flags::kUseHtileDepth | SurfaceUtil::Flags::kUseHtileStencil))
			{
				if (flags & SurfaceUtil::Flags::kUseTcDepth)
				{
					if (flags & SurfaceUtil::Flags::kUseHtileDepth)
					{
						m_spec.m_compression |= Agc::Core::MetadataCompression::kHtileDepthTextureCompatible;
					}
					if (flags & SurfaceUtil::Flags::kUseHtileStencil)
					{
						m_spec.m_compression |= Agc::Core::MetadataCompression::kHtileStencilTextureCompatible;
					}
				} else {
					if (flags & SurfaceUtil::Flags::kUseHtileDepth)
					{
						m_spec.m_compression |= Agc::Core::MetadataCompression::kHtileDepth;
					}
					if (flags & SurfaceUtil::Flags::kUseHtileStencil)
					{
						m_spec.m_compression |= Agc::Core::MetadataCompression::kHtileStencil;
					}
				}
			}
		}
		bool operator==(const DepthRenderTargetParam &rhs)
		{
			return (m_spec.m_width == rhs.m_spec.m_width) &&
				(m_spec.m_height == rhs.m_spec.m_height) &&
				(m_spec.m_numMips == rhs.m_spec.m_numMips) &&
				(m_spec.m_numSlices == rhs.m_spec.m_numSlices) &&
				(m_spec.m_depthFormat == rhs.m_spec.m_depthFormat) &&
				(m_spec.m_stencilFormat == rhs.m_spec.m_stencilFormat) &&
				(m_spec.m_numFragments == rhs.m_spec.m_numFragments) &&
				(m_spec.m_compression == rhs.m_spec.m_compression) &&
				(m_flags == rhs.m_flags);
		}
	};

	/*!
	 * @~English
	 *
	 * @brief Depth render target
	 * @details This is wrapper for sce::Agc::CxDepthRenderTarget
	 * @~Japanese
	 *
	 * @brief デプスレンダーターゲット
	 * @details sce::Agc::CxDepthRenderTargetのラッパーです。
	 */
	struct DepthRenderTargetWrapper
	{
		Agc::CxDepthRenderTarget			m_cxDepthRenderTarget;
		bool								m_isInitialized;
		Memory::Gpu::unique_ptr<uint8_t[]>	m_depthMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>	m_stencilMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>	m_htileMemory;
		std::string							m_resourceName;
		int									m_flags;
		Agc::Core::GpuSyncPostProducerResult	m_clearResult;
		Memory::Gpu::unique_ptr<Agc::Label>	m_clearSync;

		const DepthRenderTargetWrapper& operator=(DepthRenderTargetWrapper	&&rhs)
		{
			this->m_cxDepthRenderTarget	= rhs.m_cxDepthRenderTarget; rhs.m_cxDepthRenderTarget.init();
			this->m_isInitialized		= rhs.m_isInitialized; rhs.m_isInitialized = false;
			this->m_depthMemory			= std::move(rhs.m_depthMemory);
			this->m_stencilMemory		= std::move(rhs.m_stencilMemory);
			this->m_htileMemory			= std::move(rhs.m_htileMemory);
			this->m_resourceName		= std::move(rhs.m_resourceName);
			this->m_flags				= rhs.m_flags; rhs.m_flags = 0;
			this->m_clearResult			= rhs.m_clearResult; rhs.m_clearResult = {};
			this->m_clearSync			= std::move(rhs.m_clearSync);

			return *this;
		}

		DepthRenderTargetWrapper() :m_isInitialized(false), m_flags(0), m_clearResult{} { TAG_THIS_CLASS;  m_cxDepthRenderTarget.init(); }
		DepthRenderTargetWrapper(DepthRenderTargetWrapper	&&rhs) { *this = std::move(rhs); }
		/*!
		 * @~English
		 * @brief Constructor
		 * @param param Parameter to be used to initialize depth render target
		 * @param videoMemory Video memory allocartor used to initalize depth render target
		 * @~Japanese
		 * @brief コンストラクタ
		 * @param param デプスレンダーターゲットを初期化するパラメータ
		 * @param videoMemory デプスレンダーターゲット初期化に使用するビデオメモリアロケータ
		 */
		DepthRenderTargetWrapper(DepthRenderTargetParam	&param, VideoAllocator &videoMemory)
		{
			this->init(param, videoMemory);
		}
		virtual ~DepthRenderTargetWrapper() { UNTAG_THIS_CLASS; }

		/*!
		 * @~English
		 * @brief Initializes depth render target with the settings of specified depth render target
		 * @param rhs The depth render target whose attribute is used to initialize this
		 * @~Japanese
		 * @brief 指定したデプスレンダーターゲットと同一の属性で初期化
		 * @param rhs 元になるデプスレンダーターゲット
		 */
		void initAsSiblingOf(const DepthRenderTargetWrapper	&rhs)
		{
			this->m_cxDepthRenderTarget	= rhs.m_cxDepthRenderTarget;
			this->m_isInitialized		= rhs.m_isInitialized;
			this->m_depthMemory.reset(nullptr);
			this->m_stencilMemory.reset(nullptr);
			this->m_htileMemory.reset(nullptr);
			this->m_resourceName		= rhs.m_resourceName;
			this->m_flags				= rhs.m_flags;
		}
		/*!
		 * @~English
		 * @brief Initializes depth render target
		 * @param param Attribute to be used to initialize depth render target
		 * @param videoMemory Video memory allocartor used to initalize depth render target
		 * @~Japanese
		 * @brief デプスレンダーターゲットを初期化
		 * @param param デプスレンダーターゲットの初期化属性
		 * @param videoMemory デプスレンダーターゲット初期化に使用するビデオメモリアロケータ
		 */
		void init(const DepthRenderTargetParam	&param, VideoAllocator &videoMemory);
		/*!
		 * @~English
		 * @brief Finalizes depth render target
		 * @~Japanese
		 * @brief デプスレンダーターゲットを解放
		 */
		void fini();
		/*!
		 * @~English
		 * @brief Obtains depth render target attribute
		 * @return デプスレンダーターゲットの初期化属性
		 * @~Japanese
		 * @brief デプスレンダーターゲットの初期化属性を取得
		 * @return デプスレンダーターゲットの初期化属性
		 */
		DepthRenderTargetParam getParam() const;

		/*!
		 * @~English
		 * @brief Sets stencil memory
		 * @param pStencilMemory Stencil memory address to be set
		 * @~Japanese
		 * @brief ステンシルメモリを設定
		 * @param pStencilMemory 設定するステンシルメモリのアドレス
		 */
		void setStencilAddress(void *pStencilMemory);
		/*!
		 * @~English
		 * @brief Sets HTILE memory
		 * @param pHtileMemory HTILE memory address to be set
		 * @~Japanese
		 * @brief HTILEメモリを設定
		 * @param pHtileMemory 設定するHTILEメモリのアドレス
		 */
		void setHtileAddress(void *pHtileMemory);
		/*!
		 * @~English
		 * @brief Sets start/end slice indices
		 * @param baseArraySliceIndex Start slice index
		 * @param lastArraySliceIndex End slice index
		 * @~Japanese
		 * @brief スライスの開始・終了インデックスを指定
		 * @param baseArraySliceIndex スライスの開始インデックス
		 * @param lastArraySliceIndex スライスの終了インデックス
		 */
		void setArrayView(uint32_t	baseArraySliceIndex, uint32_t	lastArraySliceIndex);

		/*!
		 * @~English
		 * @brief Depth/stencil clear
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param depthClearValue Depth clear value
		 * @param stencilClearValue Stencil clear valur
		 * @~Japanese
		 * @brief デプス・ステンシルクリア
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param depthClearValue デプスクリア値
		 * @param stencilClearValue ステンシルクリア値
		 */
		void clearDepthStencil(Agc::DrawCommandBuffer	*dcb, Agc::Core::StateBuffer *sb, float	depthClearValue, uint8_t	stencilClearValue)
		{
			Debug::ScopedPerfOf<Agc::DrawCommandBuffer> perf(dcb, std::string(__FUNCTION__) + ": " + m_resourceName);

			m_cxDepthRenderTarget.setDepthClearValue(depthClearValue);
			m_cxDepthRenderTarget.setStencilClearValue(stencilClearValue);
			Agc::Toolkit::Result result = Agc::Toolkit::clearDepthRenderTargetPs(dcb, sb, &m_cxDepthRenderTarget);
			const uint64_t labelClearValue = 0ul;
			dcb->writeData(Agc::WriteDataDst::kGl2Me, Agc::CachePolicy::kLru, (uint64_t)m_clearSync.get(), &labelClearValue, 2);
			m_clearResult = Agc::Core::gpuSyncPostProducer(dcb, result.getSyncCacheOp(Agc::Toolkit::Result::Caches::kGl2), m_clearSync.get());
		}
		/*!
		 * @~English
		 * @brief Depth clear
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param depthClearValue Depth clear value
		 * @~Japanese
		 * @brief デプスクリア
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param depthClearValue デプスクリア値
		 */
		void clearDepth(Agc::DrawCommandBuffer	*dcb, Agc::Core::StateBuffer *sb, float	depthClearValue)
		{
			Debug::ScopedPerfOf<Agc::DrawCommandBuffer> perf(dcb, std::string(__FUNCTION__) + ": " + m_resourceName);

			m_cxDepthRenderTarget.setDepthClearValue(depthClearValue);
			Agc::Toolkit::Result result = Agc::Toolkit::clearDepthRenderTargetPs(dcb, sb, &m_cxDepthRenderTarget, Agc::Toolkit::DepthRenderTargetClearBuffer::kDepth);
			const uint64_t labelClearValue = 0ul;
			dcb->writeData(Agc::WriteDataDst::kGl2Me, Agc::CachePolicy::kLru, (uint64_t)m_clearSync.get(), &labelClearValue, 2);
			m_clearResult = Agc::Core::gpuSyncPostProducer(dcb, result.getSyncCacheOp(Agc::Toolkit::Result::Caches::kGl2), m_clearSync.get());
		}
		/*!
		 * @~English
		 * @brief Stencil clear
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param stencilClearValue Stencil clear valur
		 * @~Japanese
		 * @brief ステンシルクリア
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param stencilClearValue ステンシルクリア値
		 */
		void clearStencil(Agc::DrawCommandBuffer	*dcb, Agc::Core::StateBuffer *sb, uint8_t	stencilClearValue)
		{
			Debug::ScopedPerfOf<Agc::DrawCommandBuffer> perf(dcb, std::string(__FUNCTION__) + ": " + m_resourceName);

			m_cxDepthRenderTarget.setStencilClearValue(stencilClearValue);
			Agc::Toolkit::Result result = Agc::Toolkit::clearDepthRenderTargetPs(dcb, sb, &m_cxDepthRenderTarget, Agc::Toolkit::DepthRenderTargetClearBuffer::kStencil);
			const uint64_t labelClearValue = 0ul;
			dcb->writeData(Agc::WriteDataDst::kGl2Me, Agc::CachePolicy::kLru, (uint64_t)m_clearSync.get(), &labelClearValue, 2);
			m_clearResult = Agc::Core::gpuSyncPostProducer(dcb, result.getSyncCacheOp(Agc::Toolkit::Result::Caches::kGl2), m_clearSync.get());
		}
		/*!
		 * @~English
		 * @brief Starts clear
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param shaderType sce::SampleUtil::Graphics::ShaderType::kGraphics or ce::SampleUtil::Graphics::ShaderType::kCompute can be specified
		 * @param forceClearSurface Sepcify if depth surface is to be cleared
		 * @~Japanese
		 * @brief クリアを開始
		 * @param dcb DrawCommandBuffer
		 * @param sb StateBuffer
		 * @param shaderType sce::SampleUtil::Graphics::ShaderType::kGraphicsもしくはce::SampleUtil::Graphics::ShaderType::kComputeを指定
		 * @param forceClearSurface 常にデプスサーフェスをクリアするかどうかを指定
		 */
		void clear(Agc::DrawCommandBuffer	*dcb, Agc::Core::StateBuffer *sb, ShaderType	shaderType = ShaderType::kGraphics, bool	forceClearSurface = false)
		{
			(void)forceClearSurface;

			Debug::ScopedPerfOf<Agc::DrawCommandBuffer> perf(dcb, std::string(__FUNCTION__) + "(" + (shaderType == ShaderType::kGraphics ? "Graphics" : "Compute") + ") : " + m_resourceName);

			Agc::Toolkit::Result result;
			if (shaderType == ShaderType::kGraphics)
			{
				result = Agc::Toolkit::clearDepthRenderTargetPs(dcb, sb, &m_cxDepthRenderTarget);
			} else {
				result = Agc::Toolkit::clearDepthRenderTargetCs(dcb, &m_cxDepthRenderTarget);
			}
			const uint64_t labelClearValue = 0ul;
			dcb->writeData(Agc::WriteDataDst::kGl2Me, Agc::CachePolicy::kLru, (uint64_t)m_clearSync.get(), &labelClearValue, 2);
			m_clearResult = Agc::Core::gpuSyncPostProducer(dcb, result.getSyncCacheOp(Agc::Toolkit::Result::Caches::kGl2), m_clearSync.get());
		}
		/*!
		 * @~English
		 * @brief Waits for clear completion
		 * @param dcb DrawCommandBuffer
		 * @~Japanese
		 * @brief クリアの完了待ち
		 * @param dcb DrawCommandBuffer
		 */
		void syncClear(Agc::DrawCommandBuffer	*dcb)
		{
			int ret = SCE_OK; (void)ret;
			ret = Agc::Core::gpuSyncPreConsumer(dcb, m_clearResult.m_deferredCacheOps, m_clearSync.get());
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		/*!
		 * @~English
		 * @brief Obtains depth texture
		 * @return Depth texture
		 * @~Japanese
		 * @brief デプステクスチャを取得
		 * @return デプステクスチャ
		 */
		Agc::Core::Texture	getDepthTexture() const;
		/*!
		 * @~English
		 * @brief Obtains stencil texture
		 * @return Stencil texture
		 * @~Japanese
		 * @brief ステンシルテクスチャを取得
		 * @return ステンシルテクスチャ
		 */
		Agc::Core::Texture	getStencilTexture() const;
	}; // struct DepthRenderTargetWrapper

}}} // namespace sce::SampleUtil::Graphics

