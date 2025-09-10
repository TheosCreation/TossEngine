/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc.
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <gpu_address.h>
#include <toolkit/dataformat_interpreter.h>
#include "sampleutil/debug/perf.h"
#include "sampleutil/graphics/render_target.h"
#include "sampleutil/graphics/depth_render_target.h"

#ifdef _DEBUG
#pragma comment(lib, "libSceGpuAddress_debug.a")
#else
#pragma comment(lib, "libSceGpuAddress.a")
#endif

namespace sce { namespace SampleUtil { namespace Graphics {
	void	RenderTargetWrapper::init(const RenderTargetParam	&param, VideoAllocator &videoMemory)
	{
		fini();

		m_resourceName	= param.m_name;
		m_flags			= param.m_flags;
		m_clearColor	= param.m_clearColor;
		uint32_t encodedClearColor[4] = { 0, 0, 0, 0 }, dwords = 0;
		Gnmx::Toolkit::dataFormatEncoder(encodedClearColor, &dwords, (Gnmx::Toolkit::Reg32*)&param.m_clearColor, param.m_spec.m_colorFormat);
		SCE_SAMPLE_UTIL_ASSERT(dwords <= 2);
		bool useRoTexture = (m_flags & (uint32_t)SurfaceUtil::Flags::kUseRoTexture);
		bool useRwTexture = (m_flags & (uint32_t)SurfaceUtil::Flags::kUseRwTexture);
		bool isRenderToTexture = (m_flags & (uint32_t)SurfaceUtil::Flags::kIsRenderToTexture);

		m_spec = param.m_spec;
		m_spec.m_minGpuMode = Gnm::getGpuMode();

		if (useRwTexture)
		{
			m_spec.m_flags.enableFmaskCompression = 0; // disable fmask if rw texture is enabled
		}
		SCE_SAMPLE_UTIL_ASSERT(m_spec.m_numSamples > Gnm::kNumSamples1 || !m_spec.m_flags.enableFmaskCompression);
		if (m_spec.m_flags.enableFmaskCompression || useRwTexture)
		{
			m_spec.m_flags.enableDccCompression = 0; // disable DCC if fmask or rw texture is enabled
		}
		if (m_spec.m_flags.enableDccCompression || useRwTexture)
		{
			m_spec.m_flags.enableCmaskFastClear = 0; // disable cmask if dcc or rw texture is enabled
		}

		GpuAddress::computeSurfaceTileMode(m_spec.m_minGpuMode, &m_spec.m_colorTileModeHint, useRoTexture ? GpuAddress::kSurfaceTypeTextureFlat : useRwTexture ? GpuAddress::kSurfaceTypeRwTextureFlat : isRenderToTexture ? GpuAddress::kSurfaceTypeColorTarget : GpuAddress::kSurfaceTypeColorTargetDisplayable, m_spec.m_colorFormat, 1 << m_spec.m_numFragments);
		int ret = m_cxRenderTarget.init(&m_spec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		GpuAddress::TilingParameters tp;
		tp.initFromRenderTarget(&m_cxRenderTarget, 0);
		m_spec.m_pitch = tp.m_baseTiledPitch;
		if ((m_spec.m_colorFormat.getBitsPerElement() == 128) && m_spec.m_flags.enableDccCompression)
		{
			m_cxRenderTarget.setDccColorTransform(Gnm::kDccColorTransformNone);
		}
		Gnm::SizeAlign sizeAlign		= m_cxRenderTarget.getColorSizeAlign();
		m_colorMemory = Memory::Gpu::make_unique<uint8_t>(sizeAlign.m_size, sizeAlign.m_align, { Gnm::kResourceTypeRenderTargetBaseAddress, Gnm::kResourceTypeTextureBaseAddress }, videoMemory, "SampleUtil::Graphics::ColorBuffer:" + param.m_name);
		m_cxRenderTarget.setBaseAddress(m_colorMemory.get());

		// cmask
		if (m_spec.m_flags.enableCmaskFastClear)
		{
			Gnm::SizeAlign cmaskSizeAlign = m_cxRenderTarget.getCmaskSizeAlign();
			m_cmaskMemory = Memory::Gpu::make_unique<uint8_t>(cmaskSizeAlign.m_size, cmaskSizeAlign.m_align, { Gnm::kResourceTypeRenderTargetCMaskAddress, Gnm::kResourceTypeBufferBaseAddress }, videoMemory, "SampleUtil::Graphics::CMask:" + param.m_name);
			m_cxRenderTarget.setCmaskAddress(m_cmaskMemory.get());
			m_cxRenderTarget.setCmaskClearColor(encodedClearColor[0], encodedClearColor[1]);
			if (!m_spec.m_flags.enableFmaskCompression)
			{
				m_cxRenderTarget.disableFmaskCompressionForMrtWithCmask();
			}
		}
		// fmask
		if (m_spec.m_flags.enableFmaskCompression)
		{
			Gnm::SizeAlign fmaskSizeAlign = m_cxRenderTarget.getFmaskSizeAlign();
			m_fmaskMemory = Memory::Gpu::make_unique<uint8_t>(fmaskSizeAlign.m_size, fmaskSizeAlign.m_align, { Gnm::kResourceTypeRenderTargetFMaskAddress, Gnm::kResourceTypeTextureBaseAddress }, videoMemory, "SampleUtil::Graphics::FMask:" + param.m_name);
			m_cxRenderTarget.setFmaskAddress(m_fmaskMemory.get());
		}
		// DCC
		if (m_spec.m_flags.enableDccCompression)
		{
			Gnm::SizeAlign dccSizeAlign = m_cxRenderTarget.getDccSizeAlign();
			m_dccMemory = Memory::Gpu::make_unique<uint8_t>(dccSizeAlign.m_size, dccSizeAlign.m_align, { Gnm::kResourceTypeRenderTargetDccAddress, Gnm::kResourceTypeBufferBaseAddress }, videoMemory, "SampleUtil::Graphics::DCC:" + param.m_name);
			memset(m_dccMemory.get(), 0x0, dccSizeAlign.m_size);
			m_cxRenderTarget.setDccAddress(m_dccMemory.get());
			m_cxRenderTarget.setCmaskClearColor(encodedClearColor[0], encodedClearColor[1]);
		}

		m_isInitialized = true;
	}

	Gnm::Texture	RenderTargetWrapper::getColorTexture() const
	{
		Gnm::Texture outTexture;
		outTexture.initFromRenderTarget(&m_cxRenderTarget, false);
		return outTexture;
	}

	Gnm::Texture	RenderTargetWrapper::getFmaskTexture() const
	{
		int ret = SCE_OK; (void)ret;
		Gnm::Texture outTexture;
		ret = outTexture.initAsFmask(&m_cxRenderTarget);
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
		return	{ m_resourceName, m_spec, m_flags, m_clearColor };
	}

	void	RenderTargetWrapper::clear(Gnmx::GnmxGfxContext	&gfxc, ShaderType	shaderType, bool	forceClearColorBuffer)
	{
		Debug::ScopedPerfOf<Gnmx::GnmxGfxContext> perf(&gfxc, "SampleUtil::Graphics::clear rt:" + m_resourceName);
		bool isCleared = false;
		if (m_cxRenderTarget.getCmaskFastClearEnable())
		{
			SCE_SAMPLE_UTIL_ASSERT(shaderType == ShaderType::kGraphics);
			SurfaceUtil::clearCmaskSurface(gfxc, m_cxRenderTarget);
			isCleared = true;
		} else if (m_cxRenderTarget.getDccCompressionEnable())
		{
			SCE_SAMPLE_UTIL_ASSERT(shaderType == ShaderType::kGraphics);
			bool isUncompressed = SurfaceUtil::clearDccSurface(gfxc, m_cxRenderTarget, m_clearColor);
			isCleared = !isUncompressed;
		}

		if (!isCleared || forceClearColorBuffer)
		{
			if (shaderType == ShaderType::kGraphics)
			{
				SurfaceUtil::clearRenderTarget(gfxc, m_cxRenderTarget, m_clearColor);
			} else {
				Gnm::Texture texture;
				texture.initFromRenderTarget(&m_cxRenderTarget, false);
				SurfaceUtil::clearTexture(gfxc, texture, m_clearColor);
			}
			if (forceClearColorBuffer) m_wasForceClearColorBuffer = true;
		}
	}

	void	RenderTargetWrapper::syncClear(Gnmx::GnmxGfxContext	&gfxc)
	{
		(void)gfxc;
	}

	void	DepthRenderTargetWrapper::init(const DepthRenderTargetParam	&param, VideoAllocator &videoMemory)
	{
		int ret = SCE_OK; (void)ret;

		fini();

		m_spec = param.m_spec;
		m_spec.m_minGpuMode	= Gnm::getGpuMode();

		const bool useStencil = (m_spec.m_stencilFormat == Gnm::kStencil8);
		const bool isStencilOnly = useStencil && (m_spec.m_zFormat == Gnm::kZFormatInvalid);
		m_resourceName	= param.m_name;
		m_flags			= param.m_flags;
		const bool useStencilTexture = useStencil && (param.m_flags & SurfaceUtil::Flags::kUseStencilTexture);
		const bool useHtile = (m_flags & (SurfaceUtil::Flags::kUseHtileDepth|SurfaceUtil::Flags::kUseHtileStencil));

		const auto zDataFormat = (m_spec.m_zFormat == Gnm::kZFormat16) ? Gnm::kDataFormatR16Uint : (m_spec.m_zFormat == Gnm::kZFormat32Float) ? Gnm::kDataFormatR32Float : Gnm::kDataFormatInvalid;
		GpuAddress::computeSurfaceTileMode(m_spec.m_minGpuMode, &m_spec.m_tileModeHint, isStencilOnly ? GpuAddress::kSurfaceTypeStencilOnlyTarget : GpuAddress::kSurfaceTypeDepthOnlyTarget, zDataFormat, 1 << m_spec.m_numFragments);
		if (useStencilTexture && (m_spec.m_tileModeHint == Gnm::kTileModeDepth_2dThin_256))
		{
			// technote(https://ps4.develop.playstation.net/technotes/view/677/)
			m_spec.m_tileModeHint = Gnm::kTileModeDepth_2dThin_64;
		}

		ret = m_cxDepthRenderTarget.init(&m_spec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		Gnm::SizeAlign sizeAlign		= m_cxDepthRenderTarget.getZSizeAlign();
		m_depthMemory = Memory::Gpu::make_unique<uint8_t>(sizeAlign.m_size, sizeAlign.m_align, { Gnm::kResourceTypeTextureBaseAddress, Gnm::kResourceTypeDepthRenderTargetBaseAddress }, videoMemory, "SampleUtil::Graphics::Depth:" + param.m_name);
		void *pDepth = isStencilOnly ? nullptr : m_depthMemory.get();
		void *pStencil = nullptr;

		if (useStencil)
		{
			Gnm::SizeAlign stencilSizeAlign = m_cxDepthRenderTarget.getStencilSizeAlign();
			m_stencilMemory = Memory::Gpu::make_unique<uint8_t>(stencilSizeAlign.m_size, stencilSizeAlign.m_align, { Gnm::kResourceTypeTextureBaseAddress, Gnm::kResourceTypeDepthRenderTargetStencilAddress }, videoMemory, "SampleUtil::Graphics::Stencil:" + param.m_name);
			pStencil = m_stencilMemory.get();
		}
		m_cxDepthRenderTarget.setAddresses(pDepth, pStencil);

		if (useHtile)
		{
			Gnm::SizeAlign htileSizeAlign = ((Gnm::DepthRenderTarget*)this)->getHtileSizeAlign();
			htileSizeAlign.m_align = std::max<uint32_t>(htileSizeAlign.m_align, 32 * 1024);
			m_htileMemory = Memory::Gpu::make_unique<uint8_t>(htileSizeAlign.m_size, htileSizeAlign.m_align, { Gnm::kResourceTypeBufferBaseAddress, Gnm::kResourceTypeDepthRenderTargetHTileAddress }, videoMemory, "SampleUtil::Graphics::Htile:" + param.m_name);
			m_cxDepthRenderTarget.setHtileAddress(m_htileMemory.get());
			m_cxDepthRenderTarget.setHtileAccelerationEnable(true);
			m_cxDepthRenderTarget.setZCompareBase(Gnm::kZCompareBaseZMax);
		}

		m_isInitialized = true;
	}

	Gnm::Texture	DepthRenderTargetWrapper::getDepthTexture() const
	{
		Gnm::Texture outTexture;
		outTexture.initFromDepthRenderTarget(&m_cxDepthRenderTarget, false);
		const bool useHtile = (m_flags & (SurfaceUtil::Flags::kUseHtileDepth | SurfaceUtil::Flags::kUseHtileStencil));
		const bool isTcCompatible = useHtile && (m_flags & SurfaceUtil::Flags::kUseTcDepth) && (m_spec.m_minGpuMode == Gnm::kGpuModeNeo);
		if (isTcCompatible)
		{
			outTexture.setMetadataAddress(m_cxDepthRenderTarget.getHtileAddress());
			outTexture.setMetadataCompressionEnable(true);
		}

		return outTexture;
	}

	Gnm::Texture	DepthRenderTargetWrapper::getStencilTexture() const
	{
		Gnm::Texture outTexture;
		SCE_SAMPLE_UTIL_ASSERT(m_spec.m_stencilFormat !=  Gnm::kStencilInvalid);
		outTexture.initFromStencilTarget(&m_cxDepthRenderTarget, Gnm::TextureChannelType::kTextureChannelTypeUInt, false);
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
		return	{ m_resourceName, m_spec, m_flags, m_depthClearValue, m_stencilClearValue };
	}

	void	DepthRenderTargetWrapper::clearDepthStencil(Gnmx::GnmxGfxContext &gfxc, float	depthClearValue, uint8_t	stencilClearValue, uint8_t	stencilMaskValue)
	{
		SurfaceUtil::clearDepthStencilTarget(gfxc, m_cxDepthRenderTarget, depthClearValue, stencilClearValue);
	}

	void	DepthRenderTargetWrapper::clearDepth(Gnmx::GnmxGfxContext &gfxc, float	depthClearValue)
	{
		SurfaceUtil::clearDepthTarget(gfxc, m_cxDepthRenderTarget, depthClearValue);
	}

	void	DepthRenderTargetWrapper::clearStencil(Gnmx::GnmxGfxContext	&gfxc, uint8_t	stencilClearValue, uint8_t	stencilMaskValue)
	{
		SurfaceUtil::clearStencilTarget(gfxc, m_cxDepthRenderTarget, stencilClearValue);
	}
}}} // namespace sce::SampleUtil::Graphics
#endif
