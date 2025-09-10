/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <gnmx.h>
#include <toolkit/toolkit.h>
#include <toolkit/dataformat_interpreter.h>
#include "sampleutil/memory.h"
#include "sampleutil/debug/perf.h"
#include "sampleutil/graphics/platform_gnm/shader_gnm.h"
#include "sampleutil/graphics/render_target.h"
#include "sampleutil/graphics/depth_render_target.h"
#include "sampleutil/graphics/surface_utility.h"


#pragma comment(lib,"libSceVideoOut_stub_weak.a")
#pragma comment(lib,"libSceGnmDriver_stub_weak.a")
#ifdef _DEBUG
#pragma comment(lib,"libSceGnm_debug.a")
#pragma comment(lib,"libSceGnmx_debug.a")
#else
#pragma comment(lib,"libSceGnm.a")
#pragma comment(lib,"libSceGnmx.a")
#endif

static sce::SampleUtil::Memory::Gpu::unique_ptr<uint8_t[]>	s_normalMapTexMem;
static sce::Gnm::Texture									s_normalMapTexture;
static sce::SampleUtil::Memory::Gpu::unique_ptr<uint8_t[]>	s_passValidation;
static sce::Gnm::Buffer										s_passValidationBuffer;
static sce::Gnm::Texture									s_passValidationTexture;

#if SCE_GNMX_ENABLE_GFX_LCUE
#define CUE(gfxc) (gfxc).m_lwcue
#else
#define CUE(gfxc) (gfxc).m_cue
#endif

namespace sce { namespace SampleUtil { namespace Graphics {
namespace SurfaceUtil
{

	DEFINE_SHADER(sce::Gnmx::CsShader, clear_cmask_c);
	DEFINE_SHADER(sce::Gnmx::CsShader, clear_dcc_c);
	DEFINE_SHADER(sce::Gnmx::CsShader, clear_c);
	DEFINE_SHADER(sce::Gnmx::CsShader, clear128_c);
	DEFINE_SHADER(sce::Gnmx::CsShader, set_uint_fast_c);
	DEFINE_SHADER(sce::Gnmx::PsShader, clear_p);
	DEFINE_SHADER(sce::Gnmx::PsShader, clear32_p);

	std::string	getName(const sce::Gnm::Texture	&target) { return getResourceName(target.getBaseAddress()); }
	std::string	getName(const sce::Gnm::Buffer	&target) { return getResourceName(target.getBaseAddress()); }
	std::string	getName(const sce::Gnm::RenderTarget	&target) { return getResourceName(target.getBaseAddress()); }
	std::string	getName(const RenderTargetWrapper	&target) { return getResourceName(target.m_cxRenderTarget.getBaseAddress()); }
	std::string	getName(const sce::Gnm::DepthRenderTarget	&target) { return getResourceName(target.getZReadAddress()); }
	std::string	getName(const DepthRenderTargetWrapper	&target) { return getResourceName(target.m_cxDepthRenderTarget.getZReadAddress()); }

	void	initialize(VideoAllocator	&allocator)
	{
		int ret = SCE_OK; (void)ret;

		// initialize shaders
		CREATE_SHADER(sce::Gnmx::CsShader, clear_cmask_c, &allocator);
		CREATE_SHADER(sce::Gnmx::CsShader, clear_dcc_c, &allocator);
		CREATE_SHADER(sce::Gnmx::CsShader, clear_c, &allocator);
		CREATE_SHADER(sce::Gnmx::CsShader, set_uint_fast_c, &allocator);
		CREATE_SHADER(sce::Gnmx::CsShader, clear128_c, &allocator);
		CREATE_SHADER(sce::Gnmx::PsShader, clear_p, &allocator);
		CREATE_SHADER(sce::Gnmx::PsShader, clear32_p, &allocator);

		// initialize default resource
		s_normalMapTexMem	= Memory::Gpu::make_unique<uint8_t>(16, 0x100, { sce::Gnm::kResourceTypeTextureBaseAddress }, allocator, "NORMAL_MAP_TEXTURE");

		sce::Gnm::TextureSpec texSpec;
		texSpec.init();
		texSpec.m_textureType	= sce::Gnm::kTextureType2d;
		texSpec.m_width			= 1;
		texSpec.m_height		= 1;
		texSpec.m_depth			= 1;
		texSpec.m_numMipLevels	= 1;
		texSpec.m_numSlices		= 1;
		texSpec.m_format		= sce::Gnm::kDataFormatR10G10B10A2Unorm;
		texSpec.m_tileModeHint	= sce::Gnm::kTileModeThin_1dThin;
		texSpec.m_minGpuMode	= sce::Gnm::getGpuMode();

		s_normalMapTexture.init(&texSpec);

		s_normalMapTexture.setBaseAddress(s_normalMapTexMem.get());
		const Vector4	defaultNormal(0.5f, 0.5f, 1.f, 0.f);
		uint32_t	dwords = 0;
		sce::Gnmx::Toolkit::dataFormatEncoder((uint32_t *)s_normalMapTexMem.get(), &dwords, (sce::Gnmx::Toolkit::Reg32 *)&defaultNormal, texSpec.m_format);
		SCE_SAMPLE_UTIL_ASSERT(dwords == 1);

		// initialize pass validation texture/buffer
		s_passValidation = Memory::Gpu::make_unique<uint8_t>(0x200, 0x100, allocator);
#ifdef _DEBUG
		allocator.registerResource(s_passValidation.get(), 0x0100, "DUMMY BUFFER JUST FOR PASSING VALIDATION", { sce::Gnm::kResourceTypeBufferBaseAddress });
		allocator.registerResource(s_passValidation.get() + 0x100, 0x0100, "DUMMY TEXTURE JUST FOR PASSING VALIDATION", { sce::Gnm::kResourceTypeTextureBaseAddress });
#endif
		s_passValidationBuffer.initAsDataBuffer(s_passValidation.get(), sce::Gnm::kDataFormatR32G32B32A32Float, 1);
		texSpec.init();
		texSpec.m_textureType	= sce::Gnm::kTextureType2d;
		texSpec.m_minGpuMode	= sce::Gnm::getGpuMode();
		texSpec.m_width			= 1;
		texSpec.m_height		= 1;
		texSpec.m_format		= sce::Gnm::kDataFormatA8Unorm;
		texSpec.m_tileModeHint	= sce::Gnm::kTileModeThin_1dThin;
		s_passValidationTexture.init(&texSpec);
		s_passValidationTexture.setBaseAddress(s_passValidation.get() + 0x100);
	}

	void	finalize()
	{
		DESTROY_SHADER(clear_cmask_c);
		DESTROY_SHADER(clear_dcc_c);
		DESTROY_SHADER(clear_c);
		DESTROY_SHADER(set_uint_fast_c);
		DESTROY_SHADER(clear128_c);
		DESTROY_SHADER(clear_p);
		DESTROY_SHADER(clear32_p);

		s_normalMapTexMem.reset(nullptr);
		s_passValidation.reset(nullptr);
	}

	sce::Gnm::Texture	getNormalMapTexture()
	{
		return s_normalMapTexture;
	}

	sce::Gnm::Texture	getPassValidationTexture()
	{
		return s_passValidationTexture;
	}

	sce::Gnm::Buffer	getPassValidationBuffer()
	{
		return s_passValidationBuffer;
	}

	void	clearCmaskSurface(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::RenderTarget &renderTarget)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, std::string(__FUNCTION__) + ":" + getName(renderTarget));

		sce::Gnmx::ResourceBarrier	barrier;
		barrier.init(&renderTarget, sce::Gnmx::ResourceBarrier::kUsageRenderTarget, sce::Gnmx::ResourceBarrier::kUsageRwTexture);
		barrier.enableDestinationCacheFlushAndInvalidate(true);
		gfxc.writeResourceBarrier(&barrier);

		gfxc.triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateCbMeta);
		gfxc.setCsShader(SHADER(clear_cmask_c), OFFSETS_TABLE(clear_cmask_c));
		sce::Gnm::Buffer	dstDataBuffer;
		uint32_t	baseSlice	= renderTarget.getBaseArraySliceIndex();
		uint32_t	lastSlice	= renderTarget.getLastArraySliceIndex();
		uint64_t	sliceSize	= renderTarget.getCmaskSliceSizeInBytes();
		dstDataBuffer.initAsDataBuffer(reinterpret_cast<uint8_t*>(renderTarget.getCmaskAddress()) + baseSlice * sliceSize, sce::Gnm::kDataFormatR32Uint, (sliceSize * (lastSlice - baseSlice + 1)) / sce::Gnm::kDataFormatR32Uint.getBytesPerElement());
		dstDataBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeGC);
		struct UserData
		{
			sce::Gnm::Buffer	*pDstDataBuffer;
		} userData;
		auto	pBuffers	= reinterpret_cast<sce::Gnm::Buffer*>(gfxc.allocateFromCommandBuffer(sizeof(sce::Gnm::Buffer) * 1, sce::Gnm::kEmbeddedDataAlignment16));
		userData.pDstDataBuffer		= &pBuffers[0];
		*userData.pDstDataBuffer	= dstDataBuffer;
		CUE(gfxc).setUserSrtBuffer(sce::Gnm::kShaderStageCs, &userData, sizeof(userData) / 4);

		gfxc.dispatch(dstDataBuffer.getNumElements() >> 7, 1, 1); // process 128 pixels per wavefront = 2 pixels per thread.

		barrier.init(&renderTarget, sce::Gnmx::ResourceBarrier::kUsageRwBuffer, sce::Gnmx::ResourceBarrier::kUsageRenderTarget);
		barrier.enableDestinationCacheFlushAndInvalidate(true);
		gfxc.writeResourceBarrier(&barrier);
	}

	bool	clearDccSurface(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::RenderTarget	&renderTarget,  sce::Vectormath::Simd::Aos::Vector4_arg	clearColor)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, std::string(__FUNCTION__) + ":" + getName(renderTarget));

		sce::Gnmx::ResourceBarrier	barrier;
		barrier.init(&renderTarget, sce::Gnmx::ResourceBarrier::kUsageRenderTarget, sce::Gnmx::ResourceBarrier::kUsageRwTexture);
		barrier.enableDestinationCacheFlushAndInvalidate(true);
		gfxc.writeResourceBarrier(&barrier);

		uint32_t dccInitialValue = sce::Gnm::kDccClearValueUncompressed;
		if (clearColor.getX() == 0.f && clearColor.getY() == 0.f && clearColor.getZ() == 0.f && clearColor.getW() == 0.f)
		{
			dccInitialValue = sce::Gnm::kDccClearValueRgb0A0;
		} else if (clearColor.getX() == 0.f && clearColor.getY() == 0.f && clearColor.getZ() == 0.f && clearColor.getW() == 1.f)
		{
			dccInitialValue = sce::Gnm::kDccClearValueRgb0A1;
		} else if (clearColor.getX() == 1.f && clearColor.getY() == 1.f && clearColor.getZ() == 1.f && clearColor.getW() == 0.f)
		{
			dccInitialValue = sce::Gnm::kDccClearValueRgb1A0;
		} else if (clearColor.getX() == 1.f && clearColor.getY() == 1.f && clearColor.getZ() == 1.f && clearColor.getW() == 1.f)
		{
			dccInitialValue = sce::Gnm::kDccClearValueRgb1A1;
		}
		gfxc.setCsShader(SHADER(clear_dcc_c), OFFSETS_TABLE(clear_dcc_c));
		sce::Gnm::Buffer dstDataBuffer;
		uint64_t sliceSize = renderTarget.getDccSliceSizeInBytes();
		uint32_t startSlice = renderTarget.getBaseArraySliceIndex();
		uint32_t endSlice = renderTarget.getLastArraySliceIndex();
		dstDataBuffer.initAsDataBuffer(reinterpret_cast<uint8_t *>(renderTarget.getDccAddress()) + startSlice * sliceSize, sce::Gnm::kDataFormatR32Uint, (sliceSize * (endSlice - startSlice + 1)) / sce::Gnm::kDataFormatR32Uint.getBytesPerElement());
		dstDataBuffer.setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
		struct UserData
		{
			sce::Gnm::Buffer	*pDstDataBuffer;
			uint32_t			dccInitialValue;
		}	userData;
		auto pBuffers = reinterpret_cast<sce::Gnm::Buffer*>(gfxc.allocateFromCommandBuffer(sizeof(Gnm::Buffer) * 1, sce::Gnm::kEmbeddedDataAlignment16));
		userData.pDstDataBuffer		= &pBuffers[0];
		*userData.pDstDataBuffer	= dstDataBuffer;
		userData.dccInitialValue	= dccInitialValue;
		CUE(gfxc).setUserSrtBuffer(sce::Gnm::kShaderStageCs, &userData, sizeof(userData) / 4);

		gfxc.dispatch(dstDataBuffer.getNumElements() >> 7, 1, 1); // process 128 pixels per wavefront = 2 pixels per thread.

		barrier.init(&renderTarget, sce::Gnmx::ResourceBarrier::kUsageRwBuffer, sce::Gnmx::ResourceBarrier::kUsageRenderTarget);
		barrier.enableDestinationCacheFlushAndInvalidate(true);
		gfxc.writeResourceBarrier(&barrier);

		return	(dccInitialValue == sce::Gnm::kDccClearValueUncompressed);
	}

	void	clearTexture(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::Texture	&dstTexture, sce::Vectormath::Simd::Aos::Vector4_arg	color)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, std::string(__FUNCTION__) + ":" + getName(dstTexture));

		uint32_t	colorVal[4]	= { 0,0,0,0 };
		uint32_t	dwords		= 0;
		sce::Gnmx::Toolkit::dataFormatEncoder(colorVal, &dwords, (sce::Gnmx::Toolkit::Reg32 *)&color, dstTexture.getDataFormat());
		if (dwords == 1)	// w, z are optional: for SHADER(clear128_c), y is always necessary.
		{
			colorVal[3] = colorVal[2] = colorVal[1] = colorVal[0];
		}
		if (dwords == 2)	// optional: for SHADER(clear128_c)
		{
			colorVal[2] = colorVal[0];
			colorVal[3] = colorVal[1];
		}
		SCE_SAMPLE_UTIL_ASSERT(dwords != 3);	// no support

		if (dwords <= 2)
		{
			gfxc.setCsShader(SHADER(clear_c), OFFSETS_TABLE(clear_c));
		} else {
			gfxc.setCsShader(SHADER(clear128_c), OFFSETS_TABLE(clear128_c));
		}

		sce::Gnm::Buffer	dstDataBuffer;
		uint64_t	dstSize;

		const sce::Gnm::DataFormat	elemType = sce::Gnm::kDataFormatR32Uint;
		uint64_t	surfaceOffset = 0;

		gfxc.m_dcb.setComputeShaderControl(75, 0, 0);
		for (uint32_t mip = dstTexture.getBaseMipLevel(); mip <= dstTexture.getLastMipLevel(); mip++)
		{
			for (uint32_t slice = dstTexture.getBaseArraySliceIndex(); slice <= dstTexture.getLastArraySliceIndex(); slice++)
			{
				GpuAddress::computeTextureSurfaceOffsetAndSize(&surfaceOffset, &dstSize, &dstTexture, mip, slice);
				dstDataBuffer.initAsDataBuffer(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(dstTexture.getBaseAddress()) + surfaceOffset), elemType, dstSize / elemType.getBytesPerElement());
				dstDataBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypePV);
				struct UserData
				{
					sce::Gnm::Buffer	m_dstDataBuffer;
					uint32_t			m_value[4];
				} userData;
				userData.m_dstDataBuffer = dstDataBuffer;
				memcpy(userData.m_value, colorVal, sizeof(uint32_t) * 4);
				CUE(gfxc).setUserSrtBuffer(Gnm::kShaderStageCs, &userData, sizeof(userData) / 4);
				gfxc.dispatch((dstDataBuffer.getNumElements() + 127) >> 7, 1, 1);
			}
		}
		gfxc.m_dcb.setComputeShaderControl(0,0,0);

		sce::Gnmx::ResourceBarrier	barrier;
		barrier.init(&dstTexture, sce::Gnmx::ResourceBarrier::kUsageRwBuffer, sce::Gnmx::ResourceBarrier::kUsageRoTexture);
		barrier.enableDestinationCacheFlushAndInvalidate(true);
		gfxc.writeResourceBarrier(&barrier);
	}

	void	clearRenderTarget(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::RenderTarget	&dstRenderTarget, sce::Vectormath::Simd::Aos::Vector4_arg	color)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, std::string(__FUNCTION__) + ":" + getName(dstRenderTarget));

		// copy texture using ShaderTypeGraphics
		gfxc.setRenderTarget(0, &dstRenderTarget);
		gfxc.setRenderTargetMask(0xf);
		auto rtFormat = dstRenderTarget.getDataFormat().getRenderTargetFormat();
		if (rtFormat == sce::Gnm::kRenderTargetFormat32 || rtFormat == sce::Gnm::kRenderTargetFormat32_32 || rtFormat == sce::Gnm::kRenderTargetFormat32_32_32_32)
		{
			gfxc.setPsShader(SHADER(clear32_p), OFFSETS_TABLE(clear32_p));
		} else {
			gfxc.setPsShader(SHADER(clear_p), OFFSETS_TABLE(clear_p));
		}
		struct ClearColor
		{
			Vector4Unaligned	m_clearColor;
		} userData;
		userData.m_clearColor.x = color.getX();
		userData.m_clearColor.y = color.getY();
		userData.m_clearColor.z = color.getZ();
		userData.m_clearColor.w = color.getW();
		CUE(gfxc).setUserSrtBuffer(sce::Gnm::kShaderStagePs, &userData, sizeof(userData) / 4);
		uint32_t baseSlice = dstRenderTarget.getBaseArraySliceIndex();
		uint32_t lastSlice = dstRenderTarget.getLastArraySliceIndex();
		gfxc.setRenderTargetMask(0xf);
		gfxc.setDepthRenderTarget(nullptr);
		gfxc.setupScreenViewport(0, 0, dstRenderTarget.getWidth(), dstRenderTarget.getHeight(), 0.5f, 0.5f);
		for (uint32_t slice = baseSlice; slice <= lastSlice; slice++)
		{
			sce::Gnm::RenderTarget target = dstRenderTarget;
			target.setArrayView(slice, slice);
			gfxc.setRenderTarget(0, &target);
			Gnmx::renderFullScreenQuad(&gfxc);
		}
	}

	static void	clearDepthStencil(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::DepthRenderTarget	*depthTarget)
	{
		gfxc.setRenderTargetMask(0x0);

		gfxc.setPsShader(nullptr);

		const uint32_t width	= depthTarget->getWidth();
		const uint32_t height	= depthTarget->getHeight();
		gfxc.setupScreenViewport(0, 0, width, height, 0.5f, 0.5f);
		const uint32_t firstSlice = depthTarget->getBaseArraySliceIndex();
		const uint32_t lastSlice  = depthTarget->getLastArraySliceIndex();
		sce::Gnm::DepthRenderTarget	dtCopy = *depthTarget;
		for(uint32_t iSlice=firstSlice; iSlice<=lastSlice; ++iSlice)
		{
			dtCopy.setArrayView(iSlice, iSlice);
			gfxc.setDepthRenderTarget(&dtCopy);
			sce::Gnmx::renderFullScreenQuad(&gfxc);
		}

		gfxc.setRenderTargetMask(0xF);

		Gnm::DbRenderControl dbRenderControl;
		dbRenderControl.init();
		gfxc.setDbRenderControl(dbRenderControl);
	}

	void	clearDepthStencilTarget(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::DepthRenderTarget	&depthTarget, float depthValue, uint8_t	stencilValue)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, __FUNCTION__ + getName(depthTarget));

		sce::Gnm::DbRenderControl	dbRenderControl;
		dbRenderControl.init();
		dbRenderControl.setDepthClearEnable(true);
		dbRenderControl.setStencilClearEnable(true);
		gfxc.setDbRenderControl(dbRenderControl);

		sce::Gnm::DepthStencilControl	depthControl;
		depthControl.init();
		depthControl.setDepthControl(sce::Gnm::kDepthControlZWriteEnable, sce::Gnm::kCompareFuncAlways);
		depthControl.setStencilFunction(sce::Gnm::kCompareFuncAlways);
		depthControl.setDepthEnable(true);
		depthControl.setStencilEnable(true);
		gfxc.setDepthStencilControl(depthControl);

		sce::Gnm::StencilOpControl	stencilOpControl;
		stencilOpControl.init();
		stencilOpControl.setStencilOps(sce::Gnm::kStencilOpReplaceTest, sce::Gnm::kStencilOpReplaceTest, sce::Gnm::kStencilOpReplaceTest);
		gfxc.setStencilOpControl(stencilOpControl);
		const sce::Gnm::StencilControl stencilControl = {0xff, 0xff, 0xff, 0xff};
		gfxc.setStencil(stencilControl);

		gfxc.setDepthClearValue(depthValue);
		gfxc.setStencilClearValue(stencilValue);

		clearDepthStencil(gfxc, &depthTarget);
	}

	void	clearStencilTarget(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::DepthRenderTarget	&depthTarget, const uint8_t	stencilValue)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, __FUNCTION__ + getName(depthTarget));

		sce::Gnm::DbRenderControl	dbRenderControl;
		dbRenderControl.init();
		dbRenderControl.setStencilClearEnable(true);
		gfxc.setDbRenderControl(dbRenderControl);

		sce::Gnm::DepthStencilControl	depthControl;
		depthControl.init();
		depthControl.setDepthControl(sce::Gnm::kDepthControlZWriteDisable, sce::Gnm::kCompareFuncAlways);
		depthControl.setStencilFunction(sce::Gnm::kCompareFuncAlways);
		depthControl.setDepthEnable(true);
		depthControl.setStencilEnable(true);
		gfxc.setDepthStencilControl(depthControl);

		sce::Gnm::StencilOpControl	stencilOpControl;
		stencilOpControl.init();
		stencilOpControl.setStencilOps(sce::Gnm::kStencilOpReplaceTest, sce::Gnm::kStencilOpReplaceTest, sce::Gnm::kStencilOpReplaceTest);
		gfxc.setStencilOpControl(stencilOpControl);
		const sce::Gnm::StencilControl stencilControl = {0xff, 0xff, 0xff, 0xff};
		gfxc.setStencil(stencilControl);

		gfxc.setStencilClearValue(stencilValue);

		clearDepthStencil(gfxc, &depthTarget);
	}

	void	clearDepthTarget(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::DepthRenderTarget	&depthTarget, const float	depthValue)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, __FUNCTION__ + getName(depthTarget));

		sce::Gnm::DbRenderControl dbRenderControl;
		dbRenderControl.init();
		dbRenderControl.setDepthClearEnable(true);
		gfxc.setDbRenderControl(dbRenderControl);

		sce::Gnm::DepthStencilControl depthControl;
		depthControl.init();
		depthControl.setDepthControl(sce::Gnm::kDepthControlZWriteEnable, sce::Gnm::kCompareFuncAlways);
		depthControl.setStencilFunction(sce::Gnm::kCompareFuncNever);
		depthControl.setDepthEnable(true);
		gfxc.setDepthStencilControl(depthControl);

		gfxc.setDepthClearValue(depthValue);

		clearDepthStencil(gfxc, &depthTarget);
	}

	static void	fillDwordsWithCompute(sce::Gnmx::GnmxGfxContext	&gfxc, void	*address, uint32_t	dwords, uint32_t	val)
	{
		uint32_t	*source = static_cast<uint32_t*>(gfxc.allocateFromCommandBuffer(sizeof(uint32_t), sce::Gnm::kEmbeddedDataAlignment4));
		*source	= val;

		gfxc.setCsShader(SHADER(set_uint_fast_c), OFFSETS_TABLE(set_uint_fast_c));

		sce::Gnm::Buffer *destinationBuffer = reinterpret_cast<sce::Gnm::Buffer *>(gfxc.allocateFromCommandBuffer(sizeof(sce::Gnm::Buffer), sce::Gnm::kEmbeddedDataAlignment16));
		destinationBuffer->initAsDataBuffer(address, sce::Gnm::kDataFormatR32Uint, dwords);
		destinationBuffer->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);

		sce::Gnm::Buffer *sourceBuffer = reinterpret_cast<sce::Gnm::Buffer *>(gfxc.allocateFromCommandBuffer(sizeof(sce::Gnm::Buffer), sce::Gnm::kEmbeddedDataAlignment16));
		sourceBuffer->initAsDataBuffer(const_cast<uint32_t*>(source), sce::Gnm::kDataFormatR32Uint, 1);
		sourceBuffer->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeRO);

		struct UserData
		{
			sce::Gnm::Buffer	*m_destination;
			sce::Gnm::Buffer	*m_source;
			uint				m_destUints;
			uint				m_srcUintsMinusOne;
		} userData =
		{
			destinationBuffer,
			sourceBuffer,
			dwords,
			0
		};
		CUE(gfxc).setUserSrtBuffer(sce::Gnm::kShaderStageCs, &userData, sizeof(userData) / sizeof(uint32_t));

		gfxc.dispatch((dwords + sce::Gnm::kThreadsPerWavefront - 1) / sce::Gnm::kThreadsPerWavefront, 1, 1);
	}

	void	clearHtileSurface(sce::Gnmx::GnmxGfxContext	&gfxc, const sce::Gnm::DepthRenderTarget &depthTarget, const sce::Gnm::Htile htile)
	{
		Debug::ScopedPerfOf<sce::Gnmx::GnmxGfxContext>	perf(&gfxc, __FUNCTION__ + getName(depthTarget));

		SCE_SAMPLE_UTIL_ASSERT_MSG(depthTarget.getHtileAddress() != nullptr, "depthTarget (%s) has no HTILE surface.", getName(depthTarget).c_str());
		gfxc.triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateDbMeta);
		const auto baseSlice     = depthTarget.getBaseArraySliceIndex();
		const auto lastSlice     = depthTarget.getLastArraySliceIndex();
		const auto bytesPerSlice = depthTarget.getHtileSliceSizeInBytes();
		const auto baseAddress   = static_cast<uint8_t *>(depthTarget.getHtileAddress());
		const auto slices          = lastSlice - baseSlice + 1;
		const auto dwordsPerSlice  = bytesPerSlice / sizeof(uint32_t);
		const auto offsetInBytes   = baseSlice * bytesPerSlice;
		const auto dwordsToClear   = slices * dwordsPerSlice;
		fillDwordsWithCompute(gfxc, baseAddress + offsetInBytes, dwordsToClear, htile.m_asInt);

		sce::Gnmx::ResourceBarrier barrier;
		barrier.init(&depthTarget, sce::Gnmx::ResourceBarrier::kUsageRwBuffer, sce::Gnmx::ResourceBarrier::kUsageDepthSurface);
		barrier.enableDestinationCacheFlushAndInvalidate(true);
		gfxc.writeResourceBarrier(&barrier);
	}
} // namespace SurfaceUtil
}}} // namespace sce::SampleUtil::Graphics
#endif