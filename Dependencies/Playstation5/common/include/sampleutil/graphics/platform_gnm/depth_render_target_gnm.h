/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <cstdint>
#include <string>
#include <array>
#include <utility>
#include <gnm/depthrendertarget.h>
#include <gnm/texture.h>
#include <gnmx/context.h>
#include "sampleutil/memory.h"
#include "sampleutil/graphics/surface_utility.h"

namespace sce { namespace SampleUtil { namespace Graphics {
	struct DepthRenderTargetParam
	{
		std::string								m_name;
		Gnm::DepthRenderTargetSpec				m_spec;
		int										m_flags;
		float									m_depthClearValue;
		uint8_t									m_stencilClearValue;
		DepthRenderTargetParam() :m_name(""), m_flags(0), m_depthClearValue(1.f), m_stencilClearValue(0) { m_spec.init(); }
		DepthRenderTargetParam(const std::string	&name, const Gnm::DepthRenderTargetSpec	&spec, int	flags, float	depthClearValue, uint8_t	stencilClearValue) :
			m_name(name), m_spec(spec), m_flags(flags), m_depthClearValue(depthClearValue), m_stencilClearValue(stencilClearValue) {}
		DepthRenderTargetParam(const std::string	&name, const std::array<uint32_t, 4>	&dimensions,
			Gnm::ZFormat	zFormat = Gnm::kZFormat32Float,
			Gnm::StencilFormat	sFormat = Gnm::kStencilInvalid,
			Gnm::NumFragments	numFragments = Gnm::kNumFragments1,
			uint32_t	flags = 0, float	depthClearValue = 1.f, uint8_t	stencilClearValue = 0): m_name(name), m_flags(flags), m_depthClearValue(depthClearValue), m_stencilClearValue(stencilClearValue)
		{
			m_spec.init();
			m_spec.m_minGpuMode		= Gnm::getGpuMode();
			m_spec.m_width			= dimensions[0];
			m_spec.m_height			= dimensions[1];
			m_spec.m_numSlices		= dimensions[2];
			m_spec.m_zFormat		= zFormat;
			m_spec.m_stencilFormat	= sFormat;
			m_spec.m_numFragments	= numFragments;
			m_spec.m_tileModeHint	= Gnm::kTileModeDepth_1dThin;
			m_spec.m_flags.enableHtileAcceleration = (flags & (SurfaceUtil::Flags::kUseHtileDepth | SurfaceUtil::Flags::kUseHtileStencil));
			SCE_SAMPLE_UTIL_ASSERT_MSG((flags & SurfaceUtil::Flags::kUseTcDepth) == 0, "TC compatible depth/stencil is not supported");
			m_spec.m_flags.enableTextureWithoutDecompress = (flags & SurfaceUtil::Flags::kUseTcDepth);
		}
		bool operator==(const DepthRenderTargetParam &rhs)
		{
			return (m_spec.m_minGpuMode == rhs.m_spec.m_minGpuMode) &&
				(m_spec.m_width == rhs.m_spec.m_width) &&
				(m_spec.m_height == rhs.m_spec.m_height) &&
				(m_spec.m_numSlices == rhs.m_spec.m_numSlices) &&
				(m_spec.m_zFormat == rhs.m_spec.m_zFormat) &&
				(m_spec.m_stencilFormat == rhs.m_spec.m_stencilFormat) &&
				(m_spec.m_numFragments == rhs.m_spec.m_numFragments) &&
				(m_spec.m_tileModeHint == rhs.m_spec.m_tileModeHint) &&
				(m_flags == rhs.m_flags);
		}
	};
	struct DepthRenderTargetWrapper
	{
		Gnm::DepthRenderTarget				m_cxDepthRenderTarget;
		bool								m_isInitialized;
		Memory::Gpu::unique_ptr<uint8_t[]>	m_depthMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>	m_stencilMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>	m_htileMemory;
		std::string							m_resourceName;
		int									m_flags;
		float								m_depthClearValue;
		uint8_t								m_stencilClearValue;
		Gnm::DepthRenderTargetSpec			m_spec;

		const DepthRenderTargetWrapper& operator=(DepthRenderTargetWrapper	&&rhs)
		{
			this->m_cxDepthRenderTarget	= rhs.m_cxDepthRenderTarget;
			this->m_isInitialized		= rhs.m_isInitialized; rhs.m_isInitialized = false;
			this->m_depthMemory			= std::move(rhs.m_depthMemory);
			this->m_stencilMemory		= std::move(rhs.m_stencilMemory);
			this->m_htileMemory			= std::move(rhs.m_htileMemory);
			this->m_resourceName		= std::move(rhs.m_resourceName);
			this->m_flags				= rhs.m_flags; rhs.m_flags = 0;
			this->m_depthClearValue		= rhs.m_depthClearValue;
			this->m_stencilClearValue	= rhs.m_stencilClearValue;
			this->m_spec				= rhs.m_spec; rhs.m_spec.init();

			return *this;
		}

		DepthRenderTargetWrapper() :m_isInitialized(false), m_flags(0) {}
		DepthRenderTargetWrapper(DepthRenderTargetWrapper	&&rhs) { *this = std::move(rhs); }
		DepthRenderTargetWrapper(DepthRenderTargetParam	&param, VideoAllocator &videoMemory)
		{
			this->init(param, videoMemory);
		}
		virtual ~DepthRenderTargetWrapper() {}

		void initAsSiblingOf(const DepthRenderTargetWrapper	&rhs)
		{
			this->m_cxDepthRenderTarget	= rhs.m_cxDepthRenderTarget;
			this->m_isInitialized		= rhs.m_isInitialized;
			this->m_depthMemory.reset(nullptr);
			this->m_stencilMemory.reset(nullptr);
			this->m_htileMemory.reset(nullptr);
			this->m_resourceName		= rhs.m_resourceName;
			this->m_flags				= rhs.m_flags;
			this->m_depthClearValue		= rhs.m_depthClearValue;
			this->m_stencilClearValue	= rhs.m_stencilClearValue;
			this->m_spec				= rhs.m_spec;
		}
		void init(const DepthRenderTargetParam	&param, VideoAllocator &videoMemory);
		void fini();
		DepthRenderTargetParam getParam() const;

		void setStencilAddress(void *pStencilMemory);
		void setHtileAddress(void *pHtileMemory);
		void setArrayView(uint32_t	baseArraySliceIndex, uint32_t	lastArraySliceIndex);

		void clearDepthStencil(Gnmx::GnmxGfxContext	&gfxc, float	depthClearValue, uint8_t	stencilClearValue, uint8_t	stencilMaskValue = 0xff);
		void clearDepth(Gnmx::GnmxGfxContext	&gfxc, float	depthClearValue);
		void clearStencil(Gnmx::GnmxGfxContext	&gfxc, uint8_t	stencilClearValue, uint8_t	stencilMaskValue = 0xff);
		void clear(Gnmx::GnmxGfxContext	&gfxc, ShaderType	shaderType = ShaderType::kGraphics, bool	forceClearSurface = false)
		{
			(void)shaderType;
			(void)forceClearSurface;
			SCE_SAMPLE_UTIL_ASSERT(shaderType == ShaderType::kGraphics);
			if (this->m_cxDepthRenderTarget.getStencilFormat() != Gnm::kStencilInvalid)
			{
				this->clearDepthStencil(gfxc, m_depthClearValue, m_stencilClearValue, 0xff);
			} else {
				this->clearDepth(gfxc, m_depthClearValue);
			}
		}
		void syncClear(Gnmx::GnmxGfxContext	&gfxc)
		{
			(void)gfxc;
		} // no sync is needed

		Gnm::Texture	getDepthTexture() const;
		Gnm::Texture	getStencilTexture() const;
	}; // struct DepthRenderTargetWrapperBase

}}} // namespace sce::SampleUtil::Graphics
#endif