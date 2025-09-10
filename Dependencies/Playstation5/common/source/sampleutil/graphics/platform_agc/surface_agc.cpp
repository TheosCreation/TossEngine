/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#ifdef _SCE_TARGET_OS_PROSPERO
#include <agc/core/encoder.h>
#include <agc/core/translate.h>
#include <agc/toolkit/toolkit.h>
#include "sampleutil/debug/perf.h"
#include "sampleutil/graphics/render_target.h"
#include "sampleutil/graphics/depth_render_target.h"
#include "sampleutil/graphics/platform_agc/link_libraries_agc.h"

namespace sce { namespace SampleUtil { namespace Graphics {
	void	RenderTargetWrapper::init(const RenderTargetParam	&param, VideoAllocator &videoMemory)
	{
		int ret = SCE_OK; (void)ret;

		fini();

		m_clearResult = {};

		m_resourceName	= param.m_name;
		m_flags			= param.m_flags;
		m_clearColor	= param.m_clearColor;

		Agc::Core::ElementDimensions dims;
		ret = Agc::Core::translate(&dims, param.m_spec.m_format.m_format);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		uint32_t dwords = ((1 << dims.m_bytesPerElementLog2) + 3) / 4;
		SCE_SAMPLE_UTIL_ASSERT(dwords <= 2);

		Agc::Core::Encoder::Encoded	encoded = sce::Agc::Core::Encoder::encode(param.m_spec.m_format, { param.m_clearColor.getX(), param.m_clearColor.getY(), param.m_clearColor.getZ(), param.m_clearColor.getW() }).m_encoded;
		bool useRwTexture = (m_flags & (uint32_t)SurfaceUtil::Flags::kUseRwTexture);

		Agc::Core::RenderTargetSpec rtSpec = param.m_spec;

		const Agc::Core::MetadataCompression fmaskEnabledMask = Agc::Core::MetadataCompression::kUncompressedFmask | Agc::Core::MetadataCompression::kCompressedFmask | Agc::Core::MetadataCompression::kCompressedFmaskTexureCompatible;
		if (useRwTexture)
		{
			rtSpec.m_compression &= ~fmaskEnabledMask; // disable fmask if rw texture is enabled
		}
		SCE_SAMPLE_UTIL_ASSERT(rtSpec.m_numSamples > Agc::CxRenderTarget::NumSamples::k1 || (rtSpec.m_compression & fmaskEnabledMask) == Agc::Core::MetadataCompression::kNone);
		if ((rtSpec.m_compression & fmaskEnabledMask) != Agc::Core::MetadataCompression::kNone || useRwTexture)
		{
			rtSpec.m_compression &= ~Agc::Core::MetadataCompression::kDccBitMask; // disable DCC if fmask or rw texture is enabled
		}
		if ((rtSpec.m_compression & Agc::Core::MetadataCompression::kDccBitMask) != Agc::Core::MetadataCompression::kNone || useRwTexture)
		{
			rtSpec.m_compression &= ~Agc::Core::MetadataCompression::kCmaskFastClear; // disable cmask if dcc or rw texture is enabled
		}

		rtSpec.m_dataAddress = nullptr;
		rtSpec.m_cmaskAddress = nullptr;
		rtSpec.m_fmaskAddress = nullptr;
		rtSpec.m_dccAddress = nullptr;

		if (!param.m_spec.m_allowNullptr)
		{
			auto sizeAlign = Agc::Core::getSize(&rtSpec, Agc::Core::RenderTargetComponent::kData);
			m_colorMemory = Memory::Gpu::make_unique<uint8_t>(sizeAlign.m_size, sizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kRenderTargetBaseAddress, Agc::ResourceRegistration::ResourceType::kTextureBaseAddress }, videoMemory, "ColorBuffer:" + param.m_name);
			rtSpec.m_dataAddress = m_colorMemory.get();
		}

		// cmask
		if (Agc::Core::isCmaskEnabled(rtSpec.m_compression))
		{
			Agc::SizeAlign cmaskSizeAlign = Agc::Core::getSize(&rtSpec, Agc::Core::RenderTargetComponent::kCmask);
			m_cmaskMemory = Memory::Gpu::make_unique<uint8_t>(cmaskSizeAlign.m_size, cmaskSizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kRenderTargetCMaskAddress, Agc::ResourceRegistration::ResourceType::kBufferBaseAddress }, videoMemory, "CMask:" + param.m_name);
			rtSpec.m_cmaskAddress = m_cmaskMemory.get();
		}
		// fmask
		if (Agc::Core::isFmaskEnabled(rtSpec.m_compression))
		{
			Agc::SizeAlign fmaskSizeAlign = Agc::Core::getSize(&rtSpec, Agc::Core::RenderTargetComponent::kFmask);
			m_fmaskMemory = Memory::Gpu::make_unique<uint8_t>(fmaskSizeAlign.m_size, fmaskSizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kRenderTargetFMaskAddress, Agc::ResourceRegistration::ResourceType::kTextureBaseAddress }, videoMemory, "FMask:" + param.m_name);
			rtSpec.m_fmaskAddress = m_fmaskMemory.get();
		}
		// DCC
		if (Agc::Core::isDccEnabled(rtSpec.m_compression))
		{
			Agc::SizeAlign dccSizeAlign = Agc::Core::getSize(&rtSpec, Agc::Core::RenderTargetComponent::kDcc);
			m_dccMemory = Memory::Gpu::make_unique<uint8_t>(dccSizeAlign.m_size, dccSizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kTextureMetadataAddress, Agc::ResourceRegistration::ResourceType::kRenderTargetDccAddress, Agc::ResourceRegistration::ResourceType::kBufferBaseAddress }, videoMemory, "DCC:" + param.m_name);
			memset(m_dccMemory.get(), 0x0, dccSizeAlign.m_size);
			rtSpec.m_dccAddress = m_dccMemory.get();
		}
		ret = Agc::Core::initialize(&m_cxRenderTarget, &rtSpec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		if (Agc::Core::isCmaskEnabled(rtSpec.m_compression) || Agc::Core::isDccEnabled(rtSpec.m_compression))
		{
			m_cxRenderTarget.setClearColorWord0(encoded.m_vals32[0]);
			m_cxRenderTarget.setClearColorWord1(encoded.m_vals32[1]);
		}

		m_clearSync = Memory::Gpu::make_unique<Agc::Label>(Agc::Alignment::kLabel, { Agc::ResourceRegistration::ResourceType::kLabel }, videoMemory, "clearSync:" + param.m_name);

		m_isInitialized = true;
	}

	Agc::Core::Texture	RenderTargetWrapper::getColorTexture() const
	{
		int ret = SCE_OK; (void)ret;
		Agc::Core::Texture outTexture;

		Agc::Core::RenderTargetSpec spec;
		Agc::Core::translate(&spec, &m_cxRenderTarget);
		ret = Agc::Core::translate(&outTexture, &m_cxRenderTarget, Agc::Core::RenderTargetComponent::kData,
			Agc::Core::isDccEnabled(spec.m_compression)	? Agc::Core::MaintainCompression::kEnable : Agc::Core::MaintainCompression::kDisable);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return outTexture;
	}

	Agc::Core::Texture	RenderTargetWrapper::getFmaskTexture() const
	{
		int ret = SCE_OK; (void)ret;
		Agc::Core::Texture outTexture;

		Agc::Core::RenderTargetSpec spec;
		Agc::Core::translate(&spec, &m_cxRenderTarget);
		ret = Agc::Core::translate(&outTexture, &m_cxRenderTarget, Agc::Core::RenderTargetComponent::kFmask,
			(spec.m_compression & Agc::Core::MetadataCompression::kCompressedFmaskTexureCompatible) != Agc::Core::MetadataCompression::kNone
			? Agc::Core::MaintainCompression::kEnable : Agc::Core::MaintainCompression::kDisable);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return outTexture;
	}

	void	RenderTargetWrapper::fini()
	{
		m_colorMemory.reset(nullptr);
		m_cmaskMemory.reset(nullptr);
		m_fmaskMemory.reset(nullptr);
		m_dccMemory.reset(nullptr);
	}

	RenderTargetParam	RenderTargetWrapper::getParam() const
	{
		int ret = SCE_OK; (void)ret;

		Agc::Core::RenderTargetSpec spec;
		ret = Agc::Core::translate(&spec, &m_cxRenderTarget);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		return	{ m_resourceName,	spec, m_flags, m_clearColor };
	}

	void	RenderTargetWrapper::clear(Agc::DrawCommandBuffer	*dcb, ShaderType	shaderType, bool	forceClearColorBuffer)
	{
		int ret = SCE_OK; (void)ret;

		Debug::ScopedPerfOf<Agc::DrawCommandBuffer> perf(dcb, "clear rt:" + m_resourceName);

		Agc::Core::Encoder::EncoderValue clearValue = Agc::Core::Encoder::encode({ m_clearColor.getX(), m_clearColor.getY(), m_clearColor.getZ(), m_clearColor.getW() });
		Agc::Toolkit::Result result;
		if (shaderType == ShaderType::kCompute)
		{
			result = Agc::Toolkit::clearRenderTargetCs(dcb, &m_cxRenderTarget, clearValue);
		} else {
			Agc::Core::StateBuffer sb;
			sb.init(512, dcb, dcb);
			result = Agc::Toolkit::clearRenderTargetPs(dcb, &sb, &m_cxRenderTarget, clearValue);
			sb.postDraw();
		}

		const uint64_t labelClearValue = 0ul;
		dcb->writeData(Agc::WriteDataDst::kGl2Me, Agc::CachePolicy::kLru, (uint64_t)m_clearSync.get(), &labelClearValue, 2);
		m_clearResult = Agc::Core::gpuSyncPostProducer(dcb, result.getSyncCacheOp(Agc::Toolkit::Result::Caches::kGl2), m_clearSync.get());
	}

	void	RenderTargetWrapper::syncClear(Agc::DrawCommandBuffer *dcb)
	{
		int ret = SCE_OK; (void)ret;

		ret = Agc::Core::gpuSyncPreConsumer(dcb, m_clearResult.m_deferredCacheOps, m_clearSync.get());
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		m_wasForceClearColorBuffer = false;
	}

	void	DepthRenderTargetWrapper::init(const DepthRenderTargetParam	&param, VideoAllocator &videoMemory)
	{
		int ret = SCE_OK; (void)ret;

		fini();

		m_clearResult = {};

		Agc::Core::DepthRenderTargetSpec drtSpec = param.m_spec;

		const bool useStencil = (drtSpec.m_stencilFormat == Agc::CxDepthRenderTarget::StencilFormat::k8UInt);
		const bool isStencilOnly = useStencil && (drtSpec.m_depthFormat == Agc::CxDepthRenderTarget::DepthFormat::kInvalid);
		m_resourceName		= param.m_name;
		m_flags				= param.m_flags;

		Agc::SizeAlign sizeAlign		= Agc::Core::getSize(&drtSpec, Agc::Core::DepthRenderTargetComponent::kDepth);
		m_depthMemory = Memory::Gpu::make_unique<uint8_t>(sizeAlign.m_size, sizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kTextureBaseAddress, Agc::ResourceRegistration::ResourceType::kDepthRenderTargetBaseAddress }, videoMemory, "Depth:" + param.m_name);
		void *pDepth = isStencilOnly ? nullptr : m_depthMemory.get();
		void *pStencil = nullptr;

		if (useStencil)
		{
			Agc::SizeAlign stencilSizeAlign = Agc::Core::getSize(&drtSpec, Agc::Core::DepthRenderTargetComponent::kStencil);
			m_stencilMemory = Memory::Gpu::make_unique<uint8_t>(stencilSizeAlign.m_size, stencilSizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kTextureBaseAddress, Agc::ResourceRegistration::ResourceType::kDepthRenderTargetStencilAddress }, videoMemory, "Stencil:" + param.m_name);
			pStencil = m_stencilMemory.get();
		}
		drtSpec.m_depthReadAddress = drtSpec.m_depthWriteAddress = pDepth;
		drtSpec.m_stencilReadAddress = drtSpec.m_stencilWriteAddress = pStencil;

		if (Agc::Core::isHtileEnabled(drtSpec.m_compression))
		{
			Agc::SizeAlign htileSizeAlign = Agc::Core::getSize(&drtSpec, Agc::Core::DepthRenderTargetComponent::kHtile);
			m_htileMemory = Memory::Gpu::make_unique<uint8_t>(htileSizeAlign.m_size, htileSizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kBufferBaseAddress, Agc::ResourceRegistration::ResourceType::kDepthRenderTargetHTileAddress }, videoMemory, "Htile:" + param.m_name);
			drtSpec.m_htileAddress = m_htileMemory.get();
		}
		ret = Agc::Core::initialize(&m_cxDepthRenderTarget, &drtSpec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		m_cxDepthRenderTarget.setDepthClearValue(param.m_depthClearValue);
		m_cxDepthRenderTarget.setStencilClearValue(param.m_stencilClearValue);
		if ((drtSpec.m_compression & (Agc::Core::MetadataCompression::kHtileDepth | Agc::Core::MetadataCompression::kHtileDepthTextureCompatible)) != Agc::Core::MetadataCompression::kNone)
		{
			m_cxDepthRenderTarget.setZCompareBase(Agc::CxDepthRenderTarget::ZCompareBase::kZMax);
		}

		m_clearSync = Memory::Gpu::make_unique<Agc::Label>(Agc::Alignment::kLabel, { Agc::ResourceRegistration::ResourceType::kLabel }, videoMemory, "clearSync:" + param.m_name);

		m_isInitialized = true;
	}

	sce::Agc::Core::Texture	DepthRenderTargetWrapper::getDepthTexture() const
	{
		int ret = SCE_OK; (void)ret;
		Agc::Core::Texture outTexture;

		sce::Agc::Core::DepthRenderTargetSpec drtSpec;
		sce::Agc::Core::translate(&drtSpec, &m_cxDepthRenderTarget);
		bool isTcCompatible = sce::Agc::Core::isHtileEnabled(drtSpec.m_compression) &&
			((drtSpec.m_compression & sce::Agc::Core::MetadataCompression::kHtileDepthTextureCompatible) != sce::Agc::Core::MetadataCompression::kNone);
		ret = Agc::Core::translate(&outTexture, &m_cxDepthRenderTarget, Agc::Core::DepthRenderTargetComponent::kDepth, isTcCompatible ? Agc::Core::MaintainCompression::kEnable : Agc::Core::MaintainCompression::kDisable);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return outTexture;
	}

	sce::Agc::Core::Texture	DepthRenderTargetWrapper::getStencilTexture() const
	{
		int ret = SCE_OK; (void)ret;
		Agc::Core::Texture outTexture;

		Agc::Core::DepthRenderTargetSpec drtSpec;
		ret = Agc::Core::translate(&drtSpec, &m_cxDepthRenderTarget);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		SCE_SAMPLE_UTIL_ASSERT(drtSpec.m_stencilFormat ==  Agc::CxDepthRenderTarget::StencilFormat::k8UInt);
		bool isTcCompatible = sce::Agc::Core::isHtileEnabled(drtSpec.m_compression) &&
			((drtSpec.m_compression & sce::Agc::Core::MetadataCompression::kHtileStencilTextureCompatible) != sce::Agc::Core::MetadataCompression::kNone);
		ret = Agc::Core::translate(&outTexture, &m_cxDepthRenderTarget, Agc::Core::DepthRenderTargetComponent::kStencil, isTcCompatible ? Agc::Core::MaintainCompression::kEnable : Agc::Core::MaintainCompression::kDisable);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return outTexture;
	}

	void	DepthRenderTargetWrapper::fini()
	{
		m_depthMemory.reset(nullptr);
		m_stencilMemory.reset(nullptr);
		m_htileMemory.reset(nullptr);
	}

	DepthRenderTargetParam	DepthRenderTargetWrapper::getParam() const
	{
		int ret = SCE_OK; (void)ret;

		Agc::Core::DepthRenderTargetSpec spec;
		ret = Agc::Core::translate(&spec, &m_cxDepthRenderTarget);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return	{ m_resourceName, spec, m_flags, m_cxDepthRenderTarget.getDepthClearValue(), (uint8_t)m_cxDepthRenderTarget.getStencilClearValue() };
	}
}}} // namespace sce::SampleUtil::Graphics
#endif