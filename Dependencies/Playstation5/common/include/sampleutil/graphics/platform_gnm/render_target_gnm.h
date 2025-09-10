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
#include <vectormath/cpp/vectormath_aos.h>
#include <gnmx/context.h>
#include <gnm/rendertarget.h>
#include <gnm/texture.h>
#include "sampleutil/memory.h"
#include "sampleutil/graphics/surface_utility.h"

namespace sce { namespace SampleUtil { namespace Graphics {
	struct RenderTargetParam
	{
		std::string							m_name;
		Gnm::RenderTargetSpec				m_spec;
		int									m_flags;
		Vectormath::Simd::Aos::Vector4		m_clearColor;
		RenderTargetParam() :m_name(""), m_flags(0), m_clearColor(Vectormath::Simd::Aos::Vector4(0.f)) { m_spec.init(); }
		RenderTargetParam(const std::string	&name, const Gnm::RenderTargetSpec	&spec, int	flags, const Vectormath::Simd::Aos::Vector4_arg	&clearColor) :
			m_name(name), m_spec(spec), m_flags(flags), m_clearColor(clearColor) {}
		RenderTargetParam(const std::string	&name, const std::array<uint32_t, 4>	&dimensions, const Gnm::DataFormat	&format,
			Gnm::NumSamples	numSamples = Gnm::kNumSamples1,
			Gnm::NumFragments	numFragments = Gnm::kNumFragments1, Gnm::TileMode tileMode = Gnm::kTileModeThin_2dThin, uint32_t	flags = 0,
			const Vectormath::Simd::Aos::Vector4_arg	&clearColor = Vectormath::Simd::Aos::Vector4(0.f)) :m_name(name), m_flags(flags), m_clearColor(clearColor)
		{
			m_spec.init();
			m_spec.m_minGpuMode		= Gnm::getGpuMode();
			m_spec.m_width			= dimensions[0];
			m_spec.m_height			= dimensions[1];
			m_spec.m_numSlices		= dimensions[2];
			m_spec.m_colorFormat	= format;
			m_spec.m_numSamples		= numSamples;
			m_spec.m_numFragments	= numFragments;
			m_spec.m_colorTileModeHint	= tileMode;
			m_spec.m_flags.enableCmaskFastClear = (flags & SurfaceUtil::Flags::kUseCmask);
			m_spec.m_flags.enableFmaskCompression = (flags & SurfaceUtil::Flags::kUseFmask);
			m_spec.m_flags.enableDccCompression = (flags & SurfaceUtil::Flags::kUseDcc);
			m_spec.m_flags.enableColorTextureWithoutDecompress = (flags & SurfaceUtil::Flags::kUseDcc) && (flags & SurfaceUtil::Flags::kUseTcColor);
		}
		bool operator==(const RenderTargetParam &rhs)
		{
			return (m_spec.m_minGpuMode == rhs.m_spec.m_minGpuMode) &&
				(m_spec.m_width == rhs.m_spec.m_width) && (m_spec.m_height == rhs.m_spec.m_height) &&
				(m_spec.m_numSlices == rhs.m_spec.m_numSlices) &&
				(memcmp(&m_spec.m_colorFormat, &rhs.m_spec.m_colorFormat, sizeof(Gnm::DataFormat)) == 0) &&
				(m_spec.m_numSamples == rhs.m_spec.m_numSamples) &&
				(m_spec.m_numFragments == rhs.m_spec.m_numFragments) &&
				(m_spec.m_colorTileModeHint == rhs.m_spec.m_colorTileModeHint) &&
				(m_flags == rhs.m_flags);
		}
	}; // struct RenderTargetParam
	struct RenderTargetWrapper
	{
		Gnm::RenderTarget						m_cxRenderTarget;
		bool									m_isInitialized;
		bool									m_wasForceClearColorBuffer;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_colorMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_cmaskMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_fmaskMemory;
		Memory::Gpu::unique_ptr<uint8_t[]>		m_dccMemory;
		Vectormath::Simd::Aos::Vector4			m_clearColor;
		bool									m_needToWaitCsClear;
		bool									m_needToWaitPsClear;
		std::string								m_resourceName;
		int										m_flags;
		Gnm::RenderTargetSpec					m_spec;
		uint64_t								*m_pClearSync;

		const RenderTargetWrapper& operator=(RenderTargetWrapper	&&rhs)
		{
			this->m_cxRenderTarget				= rhs.m_cxRenderTarget;
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
			this->m_spec						= rhs.m_spec; rhs.m_spec.init();
			this->m_pClearSync					= rhs.m_pClearSync;

			return *this;
		}

		RenderTargetWrapper(RenderTargetWrapper &&rhs) { *this = std::move(rhs); }
		RenderTargetWrapper() :m_isInitialized(false), m_needToWaitCsClear(false), m_flags(0) { }
		RenderTargetWrapper(const RenderTargetParam	&param, VideoAllocator &videoMemory)
		{
			init(param, videoMemory);
		}
		virtual ~RenderTargetWrapper() {}

		void	init(const RenderTargetParam	&param, VideoAllocator	&videoMemory);
		void	fini();
		RenderTargetParam	getParam() const;

		void	clear(Gnmx::GnmxGfxContext	&gfxc, ShaderType	shaderType = ShaderType::kGraphics, bool	forceClearSurface = false);

		void	syncClear(Gnmx::GnmxGfxContext	&gfxc);

		Gnm::Texture	getColorTexture() const;
		Gnm::Texture	getFmaskTexture() const;
	}; // struct RenderTargetWrapper

}}} // namespace sce::SampleUtil::Graphics
#endif