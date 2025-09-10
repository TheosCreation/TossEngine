/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/drawcommandbuffer.h>
#include <agc/resourceregistration.h>
#include <agc/sizealign.h>
#include <agc/core/buffer.h>
#include <agc/core/texture.h>
#include <agc/core/sampler.h>
#include <agc/core/rendertarget.h>
#include <agc/core/depthrendertarget.h>
#include <agc/core/contexts.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnm/gpustructs.h>
#include <gnm/drawcommandbuffer.h>
#include <gnm/resourceregistration.h>
#include <gnm/gpumem.h> /* Gnm::SizeAlign */
#include <gnm/buffer.h>
#include <gnm/texture.h>
#include <gnm/sampler.h>
#include <gnm/rendertarget.h>
#include <gnm/depthrendertarget.h>
#include <gnm/drawcommandbuffer.h>
#endif

namespace sce { namespace SampleUtil { namespace Graphics {
	/*!
	 * @~English
	 * @brief Cross platform type definitions
	 * @~Japanese
	 * @brief クロスプラットフォームの型定義
	 */
	namespace Compat
	{
#if _SCE_TARGET_OS_PROSPERO
		using CommandBuffer = Agc::CommandBuffer;
		using DrawCommandBuffer = Agc::DrawCommandBuffer;
		using ResourceType = Agc::ResourceRegistration::ResourceType;
		using OwnerHandle = Agc::ResourceRegistration::OwnerHandle;
		using ResourceHandle = Agc::ResourceRegistration::ResourceHandle;
		using SizeAlign = Agc::SizeAlign;
		using Buffer = Agc::Core::Buffer;
		using Texture = Agc::Core::Texture;
		using TextureSpec = Agc::Core::TextureSpec;
		using Sampler = Agc::Core::Sampler;
		using RenderTarget = Agc::CxRenderTarget;
		using DepthRenderTarget = Agc::CxDepthRenderTarget;
		using OcclusionQueryResults = Agc::OcclusionQueryResults;
		using PrimitiveType = Agc::UcPrimitiveType::Type;

		enum class Alignment : uint32_t
		{
			kOcclusionQueryResult = Agc::Alignment::kOcclusionQueryResults
		};
#endif
#if _SCE_TARGET_OS_ORBIS
		using CommandBuffer = Gnm::CommandBuffer;
		using DrawCommandBuffer = Gnm::DrawCommandBuffer;
		using ResourceType = Gnm::ResourceType;
		using OwnerHandle = Gnm::OwnerHandle;
		using ResourceHandle = Gnm::ResourceHandle;
		using SizeAlign = Gnm::SizeAlign;
		using Buffer = Gnm::Buffer;
		using Texture = Gnm::Texture;
		using TextureSpec = Gnm::TextureSpec;
		using Sampler = Gnm::Sampler;
		using RenderTarget = Gnm::RenderTarget;
		using DepthRenderTarget = Gnm::DepthRenderTarget;
		using OcclusionQueryResults = Gnm::OcclusionQueryResults;
		using PrimitiveType = Gnm::PrimitiveType;

		enum class Alignment : uint32_t
		{
			kOcclusionQueryResult = Gnm::kAlignmentOfOcclusionQueryInBytes
		};
#endif
	}
} } } // namespace sce::SampleUtil::Graphics

