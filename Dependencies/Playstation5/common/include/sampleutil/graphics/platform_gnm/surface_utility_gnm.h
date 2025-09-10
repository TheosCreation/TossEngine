/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <cstdint>
#include <gnm/resourceregistration.h>
#include <gnm/rendertarget.h>
#include <gnm/depthRendertarget.h>
#include <gnmx/context.h>
#include "sampleutil/graphics/render_target.h"
#include "sampleutil/graphics/depth_render_target.h"

namespace sce { namespace SampleUtil { namespace Graphics {
namespace SurfaceUtil
{
	/*!
	* @~English
	* @brief Specify resource to be synchronizaed
	* @~Japanese
	* @brief 同期するリソースを指定
	*/
	enum SyncResource
	{
		/*!
		* @~English
		* @brief UAV有効
		* @~Japanese
		* @brief UAV有効
		*/
		kUAV		= 1,
		/*!
		* @~English
		* @brief Color buffer enabled
		* @~Japanese
		* @brief カラーバッファ有効
		*/
		kCb			= 2,
		/*!
		* @~English
		* @brief Depth buffer enabled
		* @~Japanese
		* @brief デプスバッファ有効
		*/
		kDb			= 4,
		/*!
		* @~English
		* @brief CB meta enabled
		* @~Japanese
		* @brief CB metaデータ有効
		*/
		kCbMeta		= 8,
		/*!
		* @~English
		* @brief DB meta enabled
		* @~Japanese
		* @brief DB metaデータ有効
		*/
		kDbMeta		= 16
	};

	std::string	getName(const sce::Gnm::Texture &target);
	std::string	getName(const sce::Gnm::Buffer &target);
	std::string	getName(const sce::Gnm::RenderTarget	&target);
	std::string	getName(const RenderTargetWrapper	&target);
	std::string	getName(const sce::Gnm::DepthRenderTarget	&target);
	std::string	getName(const DepthRenderTargetWrapper	&target);

	static inline sce::Gnm::NumFragments	gnmNumFragments(int	numFragments)
	{
		switch(numFragments)
		{
		case 1: return sce::Gnm::kNumFragments1;
		case 2: return sce::Gnm::kNumFragments2;
		case 4: return sce::Gnm::kNumFragments4;
		case 8: return sce::Gnm::kNumFragments8;
		default:
			SCE_SAMPLE_UTIL_ASSERT(false);
		}
		return	sce::Gnm::kNumFragments1;
	}

	static inline sce::Gnm::NumSamples	gnmNumSamples(int	numSamples)
	{
		switch(numSamples)
		{
		case 1: return sce::Gnm::kNumSamples1;
		case 2: return sce::Gnm::kNumSamples2;
		case 4: return sce::Gnm::kNumSamples4;
		case 8: return sce::Gnm::kNumSamples8;
		case 16: return sce::Gnm::kNumSamples16;
		default:
			SCE_SAMPLE_UTIL_ASSERT(false);
		}
		return sce::Gnm::kNumSamples1;
	}

	static inline void	syncWithResource(sce::Gnmx::GnmxGfxContext	&gfxc, uint32_t	syncResources, bool	enableDestinationCacheFlushAndInvalidate = false)
	{
		sce::Gnm::EndOfPipeEventType eopEventType;
		if ((syncResources & (SyncResource::kCb | SyncResource::kDb | SyncResource::kCbMeta | SyncResource::kDbMeta)) ||
			((syncResources & (SyncResource::kUAV)) && enableDestinationCacheFlushAndInvalidate))
		{
			eopEventType = sce::Gnm::kEopFlushAndInvalidateCbDbCaches;
		} else {
			eopEventType = sce::Gnm::kEopCbDbReadsDone;
		}
		sce::Gnm::CacheAction cacheAction;
		if ((syncResources & (SyncResource::kUAV)) ||
			((syncResources & (SyncResource::kCb | SyncResource::kDb | SyncResource::kCbMeta | SyncResource::kDbMeta)) && enableDestinationCacheFlushAndInvalidate))
		{
			cacheAction = sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2;
		} else {
			cacheAction = sce::Gnm::kCacheActionNone;
		}
		auto pLabel = reinterpret_cast<uint64_t*>(gfxc.allocateFromCommandBuffer(sizeof(uint64_t), sce::Gnm::kEmbeddedDataAlignment16));
		*pLabel = 0;
		gfxc.writeImmediateAtEndOfPipe(eopEventType, pLabel, 1, cacheAction);
		gfxc.waitOnAddress(pLabel, 0xffffffffu, sce::Gnm::kWaitCompareFuncEqual, 1);
		if (syncResources & SyncResource::kCb)
		{
			gfxc.triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateCbPixelData);
		}
		if (syncResources & SyncResource::kCbMeta)
		{
			gfxc.triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateCbMeta);
		}
		if (syncResources & SyncResource::kDbMeta)
		{
			gfxc.triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateDbMeta);
		}
	}

	// Assign to any buffers that are unused to prevent validation from
	// reporting an error about a buffer with an invalid base address
	sce::Gnm::Texture	getPassValidationTexture();
	sce::Gnm::Buffer	getPassValidationBuffer();

	sce::Gnm::Texture	getNormalMapTexture();

	static inline std::string	getResourceName(void	*ptr)
	{
		(void)ptr;
		char name[256] = "";
#ifdef _DEBUG
		sce::Gnm::ResourceHandle resourceHandle;
		sce::Gnm::findResources(ptr, 16, [](sce::Gnm::ResourceHandle resourceHandle, sce::Gnm::OwnerHandle ownerHandle, uint64_t callbackdata)
		{
			(void)ownerHandle;
			sce::Gnm::ResourceHandle *pResourceHandles = reinterpret_cast<sce::Gnm::ResourceHandle *>(callbackdata);
			*pResourceHandles = resourceHandle;
			return SCE_OK;
		}, reinterpret_cast<uintptr_t>(&resourceHandle));

		sce::Gnm::getResourceName(resourceHandle, name, 256);

#endif // _DEBUG
		return std::string(name);
	}

	void	clearCmaskSurface(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::RenderTarget &renderTarget);

	bool	clearDccSurface(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::RenderTarget &renderTarget, sce::Vectormath::Simd::Aos::Vector4_arg clearColor);

	void	clearTexture(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::Texture &dstTexture, sce::Vectormath::Simd::Aos::Vector4_arg color);

	void	clearRenderTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::RenderTarget &dstRenderTarget, sce::Vectormath::Simd::Aos::Vector4_arg color);

	void	clearDepthStencilTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::DepthRenderTarget &dstDepthRenderTarget, float	depth, uint8_t	stencil);

	void	clearDepthTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::DepthRenderTarget &dstDepthRenderTarget, float	depth);

	void	clearStencilTarget(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::DepthRenderTarget &dstDepthRenderTarget, uint8_t	stencil);

	void	clearHtileSurface(sce::Gnmx::GnmxGfxContext &gfxc, const sce::Gnm::DepthRenderTarget &dstDepthRenderTarget, const sce::Gnm::Htile htile);

} // namespace SurafceUtil
}}} // namespace SampleUtil::Graphics
#endif