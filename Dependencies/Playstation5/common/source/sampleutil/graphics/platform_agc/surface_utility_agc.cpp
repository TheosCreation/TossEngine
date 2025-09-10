/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#ifdef _SCE_TARGET_OS_PROSPERO
#include <agc/core.h>
#include <agc/toolkit/toolkit.h>
#include "sampleutil/memory.h"
#include "sampleutil/graphics/render_target.h"
#include "sampleutil/graphics/depth_render_target.h"
#include "sampleutil/graphics/surface_utility.h"
#include "sampleutil/graphics/platform_agc/link_libraries_agc.h"


#pragma comment(lib,"libSceVideoOut_stub_weak.a")

static sce::SampleUtil::Memory::Gpu::unique_ptr<uint8_t[]>	s_normalMapTexMem;
static sce::Agc::Core::Texture		s_normalMapTexture;
static sce::SampleUtil::Memory::Gpu::unique_ptr<uint8_t[]>	s_passValidation;
static sce::Agc::Core::Buffer		s_passValidationBuffer;
static sce::Agc::Core::Texture		s_passValidationTexture;

namespace sce { namespace SampleUtil { namespace Graphics {
namespace SurfaceUtil
{

	std::string	getName(const sce::Agc::Core::Texture &target) { return getResourceName(target.getDataAddress()); }
	std::string	getName(const sce::Agc::Core::Buffer &target) { return getResourceName(target.getDataAddress()); }
	std::string	getName(const sce::Agc::CxRenderTarget	&target) { return getResourceName(target.getDataAddress()); }
	std::string	getName(const RenderTargetWrapper	&target) { return getResourceName(target.m_cxRenderTarget.getDataAddress()); }
	std::string	getName(const sce::Agc::CxDepthRenderTarget	&target) { return getResourceName(target.getDepthReadAddress()); }
	std::string	getName(const DepthRenderTargetWrapper	&target) { return getResourceName(target.m_cxDepthRenderTarget.getDepthReadAddress()); }

	void	initialize(VideoAllocator	&allocator)
	{
		int ret = SCE_OK; (void)ret;

		// initialize default resource
		s_normalMapTexMem = Memory::Gpu::make_unique<uint8_t>(16, 0x100, { sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress }, allocator, "NORMAL_MAP_TEXTURE");

		sce::Agc::Core::TextureSpec texSpec;
		texSpec.init();
		texSpec.m_type			= sce::Agc::Core::Texture::Type::k2d;
		texSpec.m_width			= 1;
		texSpec.m_height		= 1;
		texSpec.m_format		= { sce::Agc::Core::TypedFormat::k11_11_10UNorm, sce::Agc::Core::Swizzle::kRGB1_R3S34 };
		texSpec.m_dataAddress	= s_normalMapTexMem.get();
		ret = sce::Agc::Core::initialize(&s_normalMapTexture, &texSpec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		sce::Agc::Core::Encoder::Encoded	encoded = sce::Agc::Core::Encoder::encode(texSpec.m_format, { 0.5f, 0.5f, 1.f, 1.f }).m_encoded;
		*reinterpret_cast<uint32_t *>(s_normalMapTexMem.get()) = encoded.m_vals32[0];

		// initialize pass validation texture/buffer
		s_passValidation = Memory::Gpu::make_unique<uint8_t>(0x200, 0x100, allocator);
#ifdef _DEBUG
		allocator.registerResource(s_passValidation.get(), 0x0100, "DUMMY BUFFER JUST FOR PASSING VALIDATION", { sce::Agc::ResourceRegistration::ResourceType::kBufferBaseAddress });
		allocator.registerResource(s_passValidation.get() + 0x100, 0x0100, "DUMMY TEXTURE JUST FOR PASSING VALIDATION", { sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress });
#endif
		sce::Agc::Core::BufferSpec passValidationBufferSpec;
		passValidationBufferSpec.initAsDataBuffer(s_passValidation.get(), { sce::Agc::Core::TypedFormat::k32_32_32_32Float, sce::Agc::Core::Swizzle::kRGBA_R4S4 }, 1);
		ret = sce::Agc::Core::initialize(&s_passValidationBuffer, &passValidationBufferSpec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		texSpec.m_type			= sce::Agc::Core::Texture::Type::k2d;
		texSpec.m_format		= { sce::Agc::Core::TypedFormat::k8UNorm, sce::Agc::Core::Swizzle::kR000_S1 };
		texSpec.m_dataAddress	= s_passValidation.get() + 0x100;
		ret = sce::Agc::Core::initialize(&s_passValidationTexture, &texSpec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	void	finalize()
	{
		s_normalMapTexMem.reset(nullptr);
		s_passValidation.reset(nullptr);
	}

	sce::Agc::Core::Texture	getNormalMapTexture()
	{
		return s_normalMapTexture;
	}

	sce::Agc::Core::Texture	getPassValidationTexture()
	{
		return s_passValidationTexture;
	}

	sce::Agc::Core::Buffer	getPassValidationBuffer()
	{
		return s_passValidationBuffer;
	}
} // namespace SurfaceUtil
}}} // namespace sce::SampleUtil::Graphics

#endif
