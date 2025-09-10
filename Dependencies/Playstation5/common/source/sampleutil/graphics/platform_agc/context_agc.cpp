/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */

#include <cstring> // memset/memcpy
#include <string>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <kernel.h>
#include <sys/dmem.h>
#include <video_out.h>
#include <agc/core.h>
#include <mat.h>
#include <razorcpu.h>
#include <vectormath/cpp/vectormath_aos.h>
#include <libsysmodule.h>
#include <agc/toolkit/toolkit.h>
#include <hmd2/fsr_curves.h>

#include <sampleutil/memory.h>
#include <sampleutil/graphics.h>
#include <sampleutil/graphics/render_target.h>
#include <sampleutil/graphics/texture_load.h>
#include <sampleutil/debug/perf.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/graphics/context.h>
#include <sampleutil/memory/dmem_mapper.h>
#include <sampleutil/graphics/platform_agc/link_libraries_agc.h>
#include "../source/imgui/imgui_internal.h"
#include "../source/imgui/imgui.h"

#pragma comment(lib, "SceVideoOut_stub_weak")

#ifdef _DEBUG
#pragma comment(lib, "libSceFrToolkit_debug_nosubmission.a")
#else
#pragma comment(lib, "libSceFrToolkit.a")
#endif

using namespace sce::Vectormath::Simd::Aos;

namespace
{
	int createScanoutBuffers(int videoHandle, const sce::SampleUtil::Graphics::RenderTargetWrapper	*rts, sce::SampleUtil::Graphics::GraphicsContext::DisplayMode	mode, bool	noGamma, SceVideoOutBusType port, uint32_t	count)
	{
		// Next we need to inform scan-out about the format of our buffers. This can be done by directly talking to VideoOut, or
		// by letting Agc::Core do the translation. To do so, we first need to get a RenderTargetSpec, which we can extract from
		// the list of CxRenderTargets passed into the function.
		sce::Agc::Core::RenderTargetSpec spec;
		SceError error = sce::Agc::Core::translate(&spec, &rts[0].m_cxRenderTarget);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(error, SCE_OK);
		if (error != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		if (port == SCE_VIDEO_OUT_BUS_TYPE_MAIN) {
			uint64_t outputMode = SCE_VIDEO_OUT_OUTPUT_MODE_DEFAULT;
			if (spec.getWidth() > 3840 && spec.getHeight() > 2160) {
				SCE_SAMPLE_UTIL_ASSERT_MSG(
					(spec.getWidth() == 7680 && spec.getHeight() == 4320) ||
					(spec.getWidth() == 6400 && spec.getHeight() == 3600) ||
					(spec.getWidth() == 5120 && spec.getHeight() == 2880), "Invalid 8K resolution");
				outputMode = SCE_VIDEO_OUT_OUTPUT_MODE_4320P_59_94HZ;
			}

			error = sceVideoOutConfigureOutput(videoHandle, outputMode, nullptr, nullptr, 0);
			SCE_SAMPLE_UTIL_ASSERT(error == SCE_OK || (outputMode == SCE_VIDEO_OUT_OUTPUT_MODE_4320P_59_94HZ && error == SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE));
			if (outputMode == SCE_VIDEO_OUT_OUTPUT_MODE_4320P_59_94HZ && error == SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE) {
				return	error;
			}
		}

		sce::Agc::Core::Colorimetry inColorimetry = sce::Agc::Core::Colorimetry::kSrgb;
		sce::Agc::Core::Colorimetry outColorimetry = sce::Agc::Core::Colorimetry::kBt709;
		switch (mode)
		{
		case sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal:
			inColorimetry = sce::Agc::Core::Colorimetry::kSrgb;
			outColorimetry = sce::Agc::Core::Colorimetry::kBt709;
			if (noGamma)
			{
				spec.m_format = { sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
			}
			break;
		case sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kHdrView:
			inColorimetry = sce::Agc::Core::Colorimetry::kSt2084;
			outColorimetry = sce::Agc::Core::Colorimetry::kSt2084;
			break;
		case sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kHdrLinearView:
			inColorimetry = sce::Agc::Core::Colorimetry::kLinear;
			outColorimetry = sce::Agc::Core::Colorimetry::kSt2084;
			break;
		default:
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			break;
		}
		// Next, we use this RenderTargetSpec to create a SceVideoOutBufferAttribute2 which tells VideoOut how it should interpret
		// our buffers. VideoOut needs to know how the color data in the target should be interpreted, and since our pixel shader has
		// been writing linear values into an sRGB RenderTarget, the data VideoOut will find in memory are sRGB encoded.
		SceVideoOutBufferAttribute2 attribute;
		error = sce::Agc::Core::translate(&attribute, &spec, inColorimetry, outColorimetry);
		if (error != SCE_OK)
		{
			// Depending on the RT format, we may not be able to get sRGB support from scan-out.
			error = sce::Agc::Core::translate(&attribute, &spec, sce::Agc::Core::Colorimetry::kLinear, sce::Agc::Core::Colorimetry::kBt709);
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(error, SCE_OK);
		if (error != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		// Ideally, all buffers should be registered with VideoOut in a single call to sceVideoOutRegisterBuffers2.
		// The reason for this is that the buffers provided in each call get associated with one attribute slot in the API.
		// Even if consecutive calls pass the same SceVideoOutBufferAttribute2 into the function, they still get assigned
		// new attribute slots. When processing a flip, there is significant extra cost associated with switching attribute
		// slots, which should be avoided.
		SceVideoOutBuffers* addresses = (SceVideoOutBuffers*)calloc(count, sizeof(SceVideoOutBuffers));
		bool compressed = false; // Determine if we are scanning out DCC compressed buffers. This determines the 
		for (uint32_t i = 0; i < count; ++i)
		{
			// We could manually call into VideoOut to set up the scan-out buffers, but Agc::Core provides a helper for this.
			addresses[i].data = rts[i].m_cxRenderTarget.getDataAddress();
			addresses[i].metadata = rts[i].m_cxRenderTarget.getDccAddress();
			if (addresses[i].metadata) // We are assuming here that all scan-out buffers have the same basic configuration.
			{
				compressed = true;
			}
		}

		// VideoOut internally groups scan-out buffers in sets. Every buffer in a set has the same attributes and switching (flipping) between
		// buffers of the same set is a light-weight operation. Switching to a buffer from a different set is significantly more expensive
		// and should be avoided. If an application wants to change the attributes of a scan-out buffer, or wants to unregister buffers,
		// these operations are done on whole sets and affect every buffer in the set. This sample only registers one set of buffers and never
		// modified the set.
		const int32_t setIndex = (int32_t)port;
		error = sceVideoOutRegisterBuffers2(videoHandle, setIndex, 0, addresses, count, &attribute, compressed ? SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_CATEGORY_COMPRESSED : SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_CATEGORY_UNCOMPRESSED, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(error, SCE_OK);
		free(addresses);
		if (error != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

	int	createScanoutBuffers(int videoHandle, const SceHmd2ReprojectionBuffer	*params, uint32_t	count)
	{
		int ret = SCE_OK; (void)ret;

		SceVideoOutBufferAttribute2 attribute = {};
		std::vector<SceVideoOutBuffers>	addresses(count);
		for (uint32_t i = 0; i < count; ++i)
		{
			addresses[i].data = (void *)&params[i];
		}
		const int32_t setIndex = 2; // fixme: SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN cannot be used for set index since it's larger than index max(=2)
		ret = sceVideoOutRegisterBuffers2(videoHandle, setIndex, 0, addresses.data(), count, &attribute, SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_CATEGORY_HMD2, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		
		return	ret;
	}

    template<typename T>
	uint32_t getRecursionFromBinnerControl(T sizeEnum)
	{
		// Since there are only a fixed set of bin sizes, we can precalculate the recursion values.
		switch (sizeEnum)
		{
		case T::k32Pixels:
			return 0;
		case T::k64Pixels:
			return 1;
		case T::k128Pixels:
			return 2;
		case T::k256Pixels:
			return 3;
		default:
			return 0;
		}
	}

	struct FsrRenderState
	{
		// Identify this as being composed of context registers only.
		static const sce::Agc::RegisterType m_type = sce::Agc::RegisterType::kCxVerifiable;

		sce::Agc::CxPaScBinnerControl  m_binnerControl;
		sce::Agc::CxPaScFsrEnable      m_fsrEnable;
		sce::Agc::CxFsrRecursions      m_fsrRecursions;
		sce::Agc::CxDrawPayloadControl m_drawPayloadControl;

		// Like all AGC registers, these need to first be initialized.
		FsrRenderState& init()
		{
			m_binnerControl.init()
				.setMode(sce::Agc::CxPaScBinnerControl::Mode::kForceOn)
				.setBinSizeX(sce::Agc::CxPaScBinnerControl::BinSizeX::k32Pixels)
				.setBinSizeY(sce::Agc::CxPaScBinnerControl::BinSizeY::k32Pixels);
			m_fsrEnable.init();
			m_fsrRecursions.init()
				.setFbwRecursionsX(3)
				.setFbwRecursionsY(3);
			m_drawPayloadControl.init(); 
			return *this;
		}

		// These functions modify the bin sizes, as well as the recursion values.
		FsrRenderState& setBinSizeX(sce::Agc::CxPaScBinnerControl::BinSizeX binSizeX)
		{
			m_binnerControl.setBinSizeX(binSizeX);
			// The recursion value needs to be updated accordingly.
			uint32_t fbw_recursions_x = getRecursionFromBinnerControl(binSizeX);
			m_fsrRecursions.setFbwRecursionsX(fbw_recursions_x);
			return *this;
		}

		FsrRenderState& setBinSizeY(sce::Agc::CxPaScBinnerControl::BinSizeY binSizeY)
		{
			m_binnerControl.setBinSizeY(binSizeY);
			// The recursion value needs to be updated accordingly.
			uint32_t fbw_recursions_y = getRecursionFromBinnerControl(binSizeY);
			m_fsrRecursions.setFbwRecursionsY(fbw_recursions_y);
			return *this;
		}

		FsrRenderState& enableFsr(bool enable, uint16_t viewportMasks = 0x01)
		{
			if (enable)
			{
				m_binnerControl.setMode(sce::Agc::CxPaScBinnerControl::Mode::kForceOn);
				m_fsrEnable
					.setEnableViewportMask(viewportMasks)
					.setFsrDistortionNumEnabledViewports(sce::Agc::CxPaScFsrEnable::FsrDistortionNumEnabledViewports::kNonzero);
				m_drawPayloadControl.setFsr(sce::Agc::CxDrawPayloadControl::Fsr::kEnable);
			} else {
				m_binnerControl.setMode(sce::Agc::CxPaScBinnerControl::Mode::kAutomatic);
				m_fsrEnable
					.setEnableViewportMask(0)
					.setFsrDistortionNumEnabledViewports(sce::Agc::CxPaScFsrEnable::FsrDistortionNumEnabledViewports::kZero);
				m_drawPayloadControl.setFsr(sce::Agc::CxDrawPayloadControl::Fsr::kDisable);
			}
			return *this;
		}

		bool getIsFsrEnabled() const
		{
			return m_drawPayloadControl.getFsr() == sce::Agc::CxDrawPayloadControl::Fsr::kEnable;
		}

		FsrRenderState const& assertValid() const
		{
			m_binnerControl.assertValid();
			m_fsrEnable.assertValid();
			m_fsrRecursions.assertValid();
			m_drawPayloadControl.assertValid();
			return *this;
		}
	};
} // anonymous namespace

namespace sce { namespace SampleUtil { namespace Graphics {
	int	GraphicsContext::setupVideoOut(const GraphicsContextOption	*prevOption)
	{
		int ret = SCE_OK;

		if (m_option.enableHdrViewMain)
		{
			if (m_option.enableLinearHdrMain)
			{
				m_displayModeMain = DisplayMode::kHdrLinearView; // todo: param.json check
			} else {
				m_displayModeMain = DisplayMode::kHdrView; // todo: param.json check
			}
		} else {
			m_displayModeMain = DisplayMode::kNormal;
		}
		if (m_option.enableHdrViewOverlay)
		{
			if (m_option.enableDccOverlay)
			{
				m_displayModeOverlay = DisplayMode::kHdrLinearView; // todo: param.json check
			} else {
				m_displayModeOverlay = DisplayMode::kHdrView; // todo: param.json check
			}
		} else {
			m_displayModeOverlay = DisplayMode::kNormal;
		}
		m_enableDccScanoutMain = m_option.enableDccMain;
		m_enableDccScanoutOverlay = m_option.enableDccOverlay;

		std::vector<int> masterPortHandles, slavePortHandles;
		for (uint64_t remainMask = m_option.videoOutBusUseBitmap; remainMask != 0;)
		{
			int port = SCE_VIDEO_OUT_BUS_TYPE_MAIN;
			SceVideoOutFlipMode flipMode = SCE_VIDEO_OUT_FLIP_MODE_VSYNC;
			DisplayMode displayMode;
			uint32_t numFrameBuffer = 0;
			uint32_t frameBufferWidth = 0, frameBufferHeight = 0;
			bool enableDcc = false;
			bool noGamma = false;
			int voutHandle = -1;
			Memory::Gpu::unique_ptr<uint8_t[]> *scanoutBufferMem = &m_scanoutBuffersMemMain;
			if (remainMask & (1ul << SCE_VIDEO_OUT_BUS_TYPE_MAIN))
			{
				port = SCE_VIDEO_OUT_BUS_TYPE_MAIN;
				displayMode = m_displayModeMain;
				numFrameBuffer = m_option.numFrameBufferMain;
				frameBufferWidth = m_width;
				frameBufferHeight = m_height;
				noGamma = m_option.noGammaMain;
				enableDcc = m_enableDccScanoutMain;
				scanoutBufferMem = &m_scanoutBuffersMemMain;
				flipMode = m_option.flipModeMain;
				voutHandle = m_videoHandleMain;
			} else if (remainMask & (1ul << SCE_VIDEO_OUT_BUS_TYPE_OVERLAY))
			{
				port = SCE_VIDEO_OUT_BUS_TYPE_OVERLAY;
				displayMode = m_displayModeOverlay;
				numFrameBuffer = m_option.numFrameBufferOverlay;
				frameBufferWidth = m_option.frameBufferWidthOverlay;
				frameBufferHeight = m_option.frameBufferHeightOverlay;
				noGamma = m_option.noGammaOverlay;
				enableDcc = m_enableDccScanoutOverlay;
				scanoutBufferMem = &m_scanoutBuffersMemOverlay;
				flipMode = m_option.flipModeOverlay;
				voutHandle = m_videoHandleOverlay;
			} else if (remainMask & (1ul << SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN))
			{
				SCE_SAMPLE_UTIL_ASSERT(!m_option.enableHmdReprojectionMirroring || !(m_option.videoOutBusUseBitmap & (1ul << SCE_VIDEO_OUT_BUS_TYPE_MAIN)));
				port = SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN;
				numFrameBuffer = 0;
				if (m_option.enableHmdVr3d) numFrameBuffer += m_option.numFrameBufferHmdMain * (m_option.enableHmdVr3dSideBySide ? 1 : 2);
				if (m_option.enableHmdUi2d) numFrameBuffer += m_option.numFrameBufferHmdMain;
				if (m_option.enableHmdVr2d) numFrameBuffer += m_option.numFrameBufferHmdMain;
				flipMode = m_option.flipModeHmdMain;
				voutHandle = m_videoHandleHmd;
			} else {
				SCE_SAMPLE_UTIL_ASSERT(false);
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			remainMask &= ~(1ul << port);

			sce::Agc::SizeAlign dccSizeAlign;
			// Set up the RenderTarget spec.
			RenderTargetWrapper *pFrameBufferRenderTargets = new RenderTargetWrapper[numFrameBuffer];
			if (port != SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
			{
				Agc::SizeAlign totalSizeAlign = { 0,0 };
				uint32_t dccOffset = 0;
				for (int i = 0; i < numFrameBuffer; i++)
				{
					Agc::Core::DataFormat format = { Agc::Core::TypedFormat::k8_8_8_8Srgb, Agc::Core::Swizzle::kRGBA_R4S4 };
					switch (displayMode)
					{
					case DisplayMode::kNormal:
						if (!noGamma)
						{
							format = { Agc::Core::TypedFormat::k8_8_8_8Srgb, Agc::Core::Swizzle::kRGBA_R4S4 };
						} else {
							format = { Agc::Core::TypedFormat::k8_8_8_8UNorm, Agc::Core::Swizzle::kRGBA_R4S4 };
						}
						break;
					case DisplayMode::kHdrView:
						format = { Agc::Core::TypedFormat::k10_10_10_2UNorm, Agc::Core::Swizzle::kRGBA_R4S4 };
						break;
					case DisplayMode::kHdrLinearView:
						format = { Agc::Core::TypedFormat::k16_16_16_16Float, Agc::Core::Swizzle::kRGBA_R4S4 };
						break;
					default:
						SCE_SAMPLE_UTIL_ASSERT(false);
						break;
					}
					RenderTargetParam rtParam("sce::SampleUtil::Graphics::GraphicsContext::ScanOut#" + std::to_string(i), { { frameBufferWidth, frameBufferHeight, 1, 1 } }, format,
						Agc::CxRenderTarget::NumSamples::k1, Agc::CxRenderTarget::NumFragments::k1, Agc::CxRenderTarget::TileMode::kRenderTarget, Agc::CxRenderTarget::Dimension::k2d,
						enableDcc ? SurfaceUtil::Flags::kDccVideoOut : 0);
					rtParam.m_spec.m_allowNullptr = true;
					pFrameBufferRenderTargets[i].init(rtParam, m_phyAddrContiguousVideoMemory/* this is not used because m_allocNullptr is true*/);
					auto dataSizeAlign = Agc::Core::getSize(&rtParam.m_spec, Agc::Core::RenderTargetComponent::kData);
					dccSizeAlign = Agc::Core::getSize(&rtParam.m_spec, Agc::Core::RenderTargetComponent::kDcc);
					if (enableDcc)
					{
						totalSizeAlign.m_size += ((dataSizeAlign.m_size + dccSizeAlign.m_align - 1) / dccSizeAlign.m_align) * dccSizeAlign.m_align;
						if (dccOffset == 0)
						{
							dccOffset = totalSizeAlign.m_size;
						}
						totalSizeAlign.m_size += ((dccSizeAlign.m_size * 2 + dataSizeAlign.m_align - 1) / dataSizeAlign.m_align) * dataSizeAlign.m_align;
						totalSizeAlign.m_align = std::max(dataSizeAlign.m_align, dccSizeAlign.m_align);
					} else {
						totalSizeAlign.m_size += ((dataSizeAlign.m_size + dataSizeAlign.m_align - 1) / dataSizeAlign.m_align) * dataSizeAlign.m_align;
						totalSizeAlign.m_align = dataSizeAlign.m_align;
					}
				}
				// allocate physical address contiguous memory here
				scanoutBufferMem->reset(nullptr);
				*scanoutBufferMem = Memory::Gpu::make_unique<uint8_t>(totalSizeAlign.m_size, totalSizeAlign.m_align, { Agc::ResourceRegistration::ResourceType::kRenderTargetBaseAddress, Agc::ResourceRegistration::ResourceType::kTextureBaseAddress }, m_phyAddrContiguousVideoMemory, "ColorBuffer:sce::SampleUtil::Graphics::GraphicsContext::ScanOut");
				for (int i = 0; i < numFrameBuffer; i++)
				{
					pFrameBufferRenderTargets[i].m_cxRenderTarget.setDataAddress(scanoutBufferMem->get() + i * (totalSizeAlign.m_size / numFrameBuffer));
					m_busTypeTable.insert({pFrameBufferRenderTargets[i].m_cxRenderTarget.getDataAddress(), port});
					if (enableDcc)
					{
						pFrameBufferRenderTargets[i].m_cxRenderTarget.setDccAddress(scanoutBufferMem->get() + i * (totalSizeAlign.m_size / numFrameBuffer) + dccOffset);
					}
				}
			} else {
				// HMD2 scanout buffer setup
				int rt_i = 0;
				if (m_option.enableHmdVr3d)
				{
					for (int n = m_option.numFrameBufferHmdMain * (m_option.enableHmdVr3dSideBySide ? 1 : 2); n-- > 0; rt_i++)
					{
						HmdEye eye = (rt_i < m_option.numFrameBufferHmdMain) ? HmdEye::kLeft : HmdEye::kRight;
						RenderTargetParam rtParam(std::string("sce::SampleUtil::Graphics::GraphicsContext::ScanOutVr3d") + (m_option.enableHmdVr3dSideBySide ? "SideBySide#" : (eye == HmdEye::kLeft) ? "Left#" : "Right#") + std::to_string(rt_i % m_option.numFrameBufferHmdMain),
							m_option.frameBufferHmdMainVr3d, 0, Vector4(0, 0, 0, 1));
						pFrameBufferRenderTargets[rt_i].init(rtParam, m_videoMemory);
						m_busTypeTable.insert({ pFrameBufferRenderTargets[rt_i].m_cxRenderTarget.getDataAddress(), port });
					}
				}
				if (m_option.enableHmdUi2d)
				{
					for (int n = m_option.numFrameBufferHmdMain; n-- > 0; rt_i++)
					{
						RenderTargetParam rtParam("sce::SampleUtil::Graphics::GraphicsContext::ScanOutUi2d" + std::to_string(rt_i % m_option.numFrameBufferHmdMain),
							m_option.frameBufferHmdMainUi2d, 0, Vector4(0, 0, 0, 1));
						pFrameBufferRenderTargets[rt_i].init(rtParam, m_videoMemory);
						m_busTypeTable.insert({ pFrameBufferRenderTargets[rt_i].m_cxRenderTarget.getDataAddress(), port });
					}
				}
				if (m_option.enableHmdVr2d)
				{
					for (int n = m_option.numFrameBufferHmdMain; n-- > 0; rt_i++)
					{
						RenderTargetParam rtParam("sce::SampleUtil::Graphics::GraphicsContext::ScanOutVr2d" + std::to_string(rt_i % m_option.numFrameBufferHmdMain),
							m_option.frameBufferHmdMainVr2d, 0, Vector4(0, 0, 0, 1));
						pFrameBufferRenderTargets[rt_i].init(rtParam, m_videoMemory);
						m_busTypeTable.insert({ pFrameBufferRenderTargets[rt_i].m_cxRenderTarget.getDataAddress(), port });
					}
				}
				if (prevOption == nullptr || (prevOption->videoOutBusUseBitmap & (1ul << SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)) == 0ul || m_option.hmdReprojectionSyncMode != prevOption->hmdReprojectionSyncMode)
				{
					const uint64_t flipFreq = (m_option.hmdReprojectionSyncMode == HmdRefreshRate::kRender90Reprojection90) ? SCE_VIDEO_OUT_OUTPUT_MODE_HMD2_89_91HZ : SCE_VIDEO_OUT_OUTPUT_MODE_HMD2_119_88HZ;
					ret = sceHmd2ReprojectionEnableVrMode(flipFreq);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				}
			}
			
			if (prevOption == nullptr || (prevOption->videoOutBusUseBitmap & (1ul << port)) == 0ul || port == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
			{
				voutHandle = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, port, 0, nullptr);
				SCE_SAMPLE_UTIL_ASSERT(voutHandle >= 0);
				if (voutHandle < 0)
				{
					return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				}
			}
			if (flipMode == SCE_VIDEO_OUT_FLIP_MODE_MASTER)
			{
				masterPortHandles.push_back(voutHandle);
			} else if (flipMode == SCE_VIDEO_OUT_FLIP_MODE_SLAVE)
			{
				slavePortHandles.push_back(voutHandle);
			}

			// We need the videoout handle to flip.
			if (port == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
			{
				m_pFrameBufferRenderTargetsMain = pFrameBufferRenderTargets;
				if (voutHandle >= 0)
				{
					m_videoHandleMain = voutHandle;
					// Add flip event
					ret = sceVideoOutAddFlipEvent(m_flipEventQueueMain, m_videoHandleMain, nullptr);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					if (ret != SCE_OK)
					{
						return SCE_SAMPLE_UTIL_ERROR_FATAL;
					}
				}
				ret = createScanoutBuffers(m_videoHandleMain, m_pFrameBufferRenderTargetsMain, displayMode, m_option.noGammaMain, (SceVideoOutBusType)port, numFrameBuffer);
				if (ret == SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE) {
					if (prevOption == nullptr || (prevOption->videoOutBusUseBitmap & (1ul << port)) == 0ul) {
						ret = sceVideoOutClose(voutHandle);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					}
					scanoutBufferMem->reset(nullptr);
					return	SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE;
				}
			} else if (port == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
			{
				m_pFrameBufferRenderTargetsOverlay = pFrameBufferRenderTargets;
				if (voutHandle >= 0)
				{
					m_videoHandleOverlay = voutHandle;
					// Add flip event
					ret = sceVideoOutAddFlipEvent(m_flipEventQueueOverlay, m_videoHandleOverlay, nullptr);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					if (ret != SCE_OK)
					{
						return SCE_SAMPLE_UTIL_ERROR_FATAL;
					}
				}
				ret = createScanoutBuffers(m_videoHandleOverlay, m_pFrameBufferRenderTargetsOverlay, displayMode, m_option.noGammaOverlay, (SceVideoOutBusType)port, numFrameBuffer);
			} else if (port == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
			{
				uint32_t srcCount = 0;
				int rtBaseIndex = 0;
				if (m_option.enableHmdVr3d)
				{
					m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kLeft]		= &pFrameBufferRenderTargets[rtBaseIndex]; rtBaseIndex += m_option.numFrameBufferHmdMain;
					if (!m_option.enableHmdVr3dSideBySide) {
						m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kRight] = &pFrameBufferRenderTargets[rtBaseIndex]; rtBaseIndex += m_option.numFrameBufferHmdMain;
					}
					++srcCount;
				}
				if (m_option.enableHmdUi2d)
				{
					m_pFrameBufferRenderTargetsHmdUi2d							= &pFrameBufferRenderTargets[rtBaseIndex]; rtBaseIndex += m_option.numFrameBufferHmdMain;
					++srcCount;
				}
				if (m_option.enableHmdVr2d)
				{
					m_pFrameBufferRenderTargetsHmdVr2d							= &pFrameBufferRenderTargets[rtBaseIndex]; rtBaseIndex += m_option.numFrameBufferHmdMain;
					++srcCount;
				}
				std::vector<SceHmd2ReprojectionBuffer> reprojectionBuffer(m_option.numFrameBufferHmdMain);
				std::vector<std::array<SceHmd2ReprojectionSource,3>> reprojectionSrc(m_option.numFrameBufferHmdMain);

				std::vector<sce::FrToolkit::FsrLUTs> vr3dFsrLUTs;
				m_hmdVr3dFsrLUTsMemory.reset(nullptr);
				if (m_option.enableHmdVr3d && m_option.enableHmdVr3dFsr)
				{
					const uint32_t lutWidth		= m_option.frameBufferHmdMainVr3d.getWidth() / (m_option.enableHmdVr3dSideBySide? 2 : 1);
					const uint32_t lutHeight	= m_option.frameBufferHmdMainVr3d.getHeight();
					const uint32_t numLUTs		= m_option.numFrameBufferHmdMain * 2;
					/* Our LUTs will be half the total width of the VR display, because that's how wide
					*  the viewport for each individual eye will be.
					*/
					sce::Agc::SizeAlign lutSa;
					ret = sce::FrToolkit::getLUTsMemoryRequirement(&lutSa, lutWidth, lutHeight, numLUTs);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					// Allocate memory for all of the LUTs
					m_hmdVr3dFsrLUTsMemory = sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(lutSa.m_size, lutSa.m_align, { sce::Agc::ResourceRegistration::ResourceType::kTextureBaseAddress, sce::Agc::ResourceRegistration::ResourceType::kBufferBaseAddress }, m_videoMemory, "Vr3d FSR LUTs");

					/* The reprojection API uses 1D textures instead of buffers, so we can't just use
					*  the FrToolkit::initLUTs function to initialize them. However, we can use it to
					*  quickly prepare the required data, and split up the memory buffer as needed. 
					*/
					vr3dFsrLUTs.resize(numLUTs);
					ret = sce::FrToolkit::initLUTs(vr3dFsrLUTs.data(), m_hmdVr3dFsrLUTsMemory.get(), lutWidth, lutHeight, numLUTs);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

					/* Now we can just translate our FSR curves to the FSR Uc register. Note that
					*  you need to correctly set the viewports depending on how each eye will be
					*  rendered. In this sample we'll render each eye to a separate render target
					*  so both viewports are identical.
					*/
					setLensOptimizedFsrView();
				}

				m_hmdVr3dFsrLuts.clear();
				for (int fr_i = 0; fr_i < m_option.numFrameBufferHmdMain; ++fr_i)
				{
					reprojectionBuffer[fr_i].src = reprojectionSrc[fr_i].data();
					reprojectionBuffer[fr_i].srcCount = srcCount;

					int src_i = 0;
					if (m_option.enableHmdVr3d)
					{
						reprojectionBuffer[fr_i].src[src_i].layerType = SCE_HMD2_REPROJECTION_LAYER_TYPE_VR3D;
						reprojectionBuffer[fr_i].src[src_i].data.src3d.enableFsrReprojection = m_option.enableHmdVr3dFsr;
						reprojectionBuffer[fr_i].src[src_i].data.src3d.srcL = m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kLeft][fr_i].getColorTexture();
						reprojectionBuffer[fr_i].src[src_i].data.src3d.srcR = m_ppFrameBufferRenderTargetsHmdVr3d[(int)(m_option.enableHmdVr3dSideBySide ? HmdEye::kLeft : HmdEye::kRight)][fr_i].getColorTexture();
						if (m_option.enableHmdVr3dFsr)
						{
							uint32_t lutWidth = m_option.frameBufferHmdMainVr3d.getWidth() / (m_option.enableHmdVr3dSideBySide ? 2 : 1);
							uint32_t lutHeight = m_option.frameBufferHmdMainVr3d.getHeight();
							sce::Agc::Core::TextureSpec texSpec;
							texSpec.init().setType(sce::Agc::Core::Texture::Type::k1d).setFormat(vr3dFsrLUTs[0].m_horizontal.getDataFormat());

							m_hmdVr3dFsrLuts.emplace_back();
							ret = sce::Agc::Core::initialize(&m_hmdVr3dFsrLuts.back().horizontal, &texSpec.setWidth(lutWidth).setDataAddress(vr3dFsrLUTs[fr_i * 2].m_horizontal.getDataAddress()));
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
							ret = sce::Agc::Core::initialize(&m_hmdVr3dFsrLuts.back().vertical, &texSpec.setWidth(lutHeight).setDataAddress(vr3dFsrLUTs[fr_i * 2].m_vertical.getDataAddress()));
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
							reprojectionBuffer[fr_i].src[src_i].data.src3d.fsrLutL = m_hmdVr3dFsrLuts.back();
							m_hmdVr3dFsrLuts.emplace_back();
							ret = sce::Agc::Core::initialize(&m_hmdVr3dFsrLuts.back().horizontal, &texSpec.setWidth(lutWidth).setDataAddress(vr3dFsrLUTs[fr_i * 2 + 1].m_horizontal.getDataAddress()));
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
							ret = sce::Agc::Core::initialize(&m_hmdVr3dFsrLuts.back().vertical, &texSpec.setWidth(lutHeight).setDataAddress(vr3dFsrLUTs[fr_i * 2 + 1].m_vertical.getDataAddress()));
							SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
							reprojectionBuffer[fr_i].src[src_i].data.src3d.fsrLutR = m_hmdVr3dFsrLuts.back();
						}
						++src_i;
					}
					if (m_option.enableHmdUi2d)
					{
						reprojectionBuffer[fr_i].src[src_i].layerType = SCE_HMD2_REPROJECTION_LAYER_TYPE_UI2D;
						reprojectionBuffer[fr_i].src[src_i].data.src2d.src = m_pFrameBufferRenderTargetsHmdUi2d[fr_i].getColorTexture();
						++src_i;
					}
					if (m_option.enableHmdVr2d)
					{
						reprojectionBuffer[fr_i].src[src_i].layerType = SCE_HMD2_REPROJECTION_LAYER_TYPE_VR2D;
						reprojectionBuffer[fr_i].src[src_i].data.src2d.src = m_pFrameBufferRenderTargetsHmdVr2d[fr_i].getColorTexture();
					}
				}

				if (voutHandle >= 0)
				{
					m_videoHandleHmd = voutHandle;
					// Add flip event
					ret = sceVideoOutAddFlipEvent(m_flipEventQueueHmd, m_videoHandleHmd, nullptr);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					if (ret != SCE_OK)
					{
						return SCE_SAMPLE_UTIL_ERROR_FATAL;
					}
				}
				ret = createScanoutBuffers(m_videoHandleHmd, reprojectionBuffer.data(), m_option.numFrameBufferHmdMain);

				switch (m_option.hmdReprojectionTiming)
				{
				case HmdReprojectionTiming::k2000usec:
					ret = sceHmd2ReprojectionSetTiming(SCE_HMD2_REPROJECTION_TIMING_2000USEC);
					break;
				case HmdReprojectionTiming::k3000usec:
					ret = sceHmd2ReprojectionSetTiming(SCE_HMD2_REPROJECTION_TIMING_3000USEC);
					break;
				case HmdReprojectionTiming::k4000usec:
					ret = sceHmd2ReprojectionSetTiming(SCE_HMD2_REPROJECTION_TIMING_4000USEC);
					break;
				case HmdReprojectionTiming::k5000usec:
					ret = sceHmd2ReprojectionSetTiming(SCE_HMD2_REPROJECTION_TIMING_5000USEC);
					break;
				case HmdReprojectionTiming::k6000usec:
					ret = sceHmd2ReprojectionSetTiming(SCE_HMD2_REPROJECTION_TIMING_6000USEC);
					break;
				default:
					SCE_SAMPLE_UTIL_ASSERT(false);
					break;
				}
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (m_option.hmdReprojectionSyncMode == HmdRefreshRate::kRender90Reprojection90)
				{
					ret = sceHmd2ReprojectionSetRenderConfig({ SCE_HMD2_REPROJECTION_SYNC_MODE_RENDER120_AT_REPROJECTION120, SCE_HMD2_REPROJECTION_SYNC_PHASE_HDMI_FORMER_HALF }, nullptr);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					ret = sceVideoOutConfigureOutput(m_videoHandleHmd, SCE_VIDEO_OUT_OUTPUT_MODE_HMD2_89_91HZ, nullptr, nullptr, 0);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				} else if (m_option.hmdReprojectionSyncMode == HmdRefreshRate::kRender120Reprojection120)
				{
					ret = sceHmd2ReprojectionSetRenderConfig({ SCE_HMD2_REPROJECTION_SYNC_MODE_RENDER120_AT_REPROJECTION120, SCE_HMD2_REPROJECTION_SYNC_PHASE_HDMI_FORMER_HALF }, nullptr);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					ret = sceVideoOutConfigureOutput(m_videoHandleHmd, SCE_VIDEO_OUT_OUTPUT_MODE_HMD2_119_88HZ, nullptr, nullptr, 0);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				} else if (m_option.hmdReprojectionSyncMode == HmdRefreshRate::kRender60Reprojection120SyncPhaseHdmiFormerHalf)
				{
					ret = sceHmd2ReprojectionSetRenderConfig({ SCE_HMD2_REPROJECTION_SYNC_MODE_RENDER60_AT_REPROJECTION120, SCE_HMD2_REPROJECTION_SYNC_PHASE_HDMI_FORMER_HALF }, nullptr);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					ret = sceVideoOutConfigureOutput(m_videoHandleHmd, SCE_VIDEO_OUT_OUTPUT_MODE_HMD2_119_88HZ, nullptr, nullptr, 0);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				} else if (m_option.hmdReprojectionSyncMode == HmdRefreshRate::kRender60Reprojection120SyncPhaseHdmiLatterHalf)
				{
					ret = sceHmd2ReprojectionSetRenderConfig({ SCE_HMD2_REPROJECTION_SYNC_MODE_RENDER60_AT_REPROJECTION120, SCE_HMD2_REPROJECTION_SYNC_PHASE_HDMI_LATTER_HALF }, nullptr);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					ret = sceVideoOutConfigureOutput(m_videoHandleHmd, SCE_VIDEO_OUT_OUTPUT_MODE_HMD2_119_88HZ, nullptr, nullptr, 0);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				}
			} else {
				SCE_SAMPLE_UTIL_ASSERT(false);
			}
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}

			if (enableDcc)
			{
				for (int i = 0; i < numFrameBuffer; i++)
				{
					auto &frameBufferTarget = pFrameBufferRenderTargets[i].m_cxRenderTarget;
					// set shadow DCC buffer for rendering
					frameBufferTarget.setDccAddress(frameBufferTarget.getDccAddress() + dccSizeAlign.m_size);
				}
			}

			// Setup DepthRenderTarget and flip labels
			{
				Agc::Core::DepthRenderTargetSpec spec;
				spec.init();
				spec.m_width = frameBufferWidth;
				spec.m_height = frameBufferHeight;
				Agc::Label *flipLabels = nullptr;
				if (port == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
				{
					DepthRenderTargetParam param("sce::SampleUtil::Graphics::GraphicsContext::depthMain", spec, 0, 1.f, 0);
					m_frameBufferDepthRenderTargetMain.init(param, m_videoMemory);
					m_flipLabelsMain = Memory::Gpu::make_unique<Agc::Label>(numFrameBuffer, Agc::Alignment::kLabel, { Agc::ResourceRegistration::ResourceType::kLabel }, m_videoMemory, "sce::SampleUtil::Graphics::GraphicsContext::flipLabelsMain");
					flipLabels = m_flipLabelsMain.get();
				} else if (port == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
				{
					DepthRenderTargetParam param("sce::SampleUtil::Graphics::GraphicsContext::depthOverlay", spec, 0, 1.f, 0);
					m_frameBufferDepthRenderTargetOverlay.init(param, m_videoMemory);
					m_flipLabelsOverlay = Memory::Gpu::make_unique<Agc::Label>(numFrameBuffer, Agc::Alignment::kLabel, { Agc::ResourceRegistration::ResourceType::kLabel }, m_videoMemory, "sce::SampleUtil::Graphics::GraphicsContext::flipLabelsOverlay");
					flipLabels = m_flipLabelsOverlay.get();
				} else if (port == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
				{
					if (m_option.enableHmdVr3d)
					{
						spec.m_width = m_option.frameBufferHmdMainVr3d.getWidth();
						spec.m_height = m_option.frameBufferHmdMainVr3d.getHeight();
						DepthRenderTargetParam param("sce::SampleUtil::Graphics::GraphicsContext::depthHmdVr3d", spec, 0, 1.f, 0);
						m_frameBufferDepthRenderTargetsHmdVr3d[(int)HmdEye::kLeft].init(param, m_videoMemory);
						if (!m_option.enableHmdVr3dSideBySide) {
							m_frameBufferDepthRenderTargetsHmdVr3d[(int)HmdEye::kRight].init(param, m_videoMemory);
						}
					}
					if (m_option.enableHmdUi2d)
					{
						spec.m_width = m_option.frameBufferHmdMainUi2d.getWidth();
						spec.m_height = m_option.frameBufferHmdMainUi2d.getHeight();
						DepthRenderTargetParam param("sce::SampleUtil::Graphics::GraphicsContext::depthHmdUi2d", spec, 0, 1.f, 0);
						m_frameBufferDepthRenderTargetHmdUi2d.init(param, m_videoMemory);
					}
					if (m_option.enableHmdVr2d)
					{
						spec.m_width = m_option.frameBufferHmdMainVr2d.getWidth();
						spec.m_height = m_option.frameBufferHmdMainVr2d.getHeight();
						DepthRenderTargetParam param("sce::SampleUtil::Graphics::GraphicsContext::depthHmdVr2d", spec, 0, 1.f, 0);
						m_frameBufferDepthRenderTargetHmdVr2d.init(param, m_videoMemory);
					}
					m_flipLabelsHmd = Memory::Gpu::make_unique<Agc::Label>(numFrameBuffer, Agc::Alignment::kLabel, { Agc::ResourceRegistration::ResourceType::kLabel }, m_videoMemory, "sce::SampleUtil::Graphics::GraphicsContext::flipLabelsHmd");
					flipLabels = m_flipLabelsHmd.get();
				} else {
					SCE_SAMPLE_UTIL_ASSERT(false);
				}
				for (int i = 0; i < numFrameBuffer; i++)
				{
					flipLabels[i].m_value = 1;
				}
			}
		}

		SCE_SAMPLE_UTIL_ASSERT_MSG((masterPortHandles.size() == 0 && slavePortHandles.size() == 0) || (masterPortHandles.size() == 1 && slavePortHandles.size() > 0), "Invalid master/slave flip mode is specified");

		for (auto slavePortHandle : slavePortHandles)
		{
			do
			{
				if (ret == SCE_VIDEO_OUT_ERROR_RESOURCE_BUSY)
				{
					sceKernelUsleep(100);
				}
				ret = sceVideoOutSetFlipMaster(slavePortHandle, masterPortHandles[0]);
			} while (ret == SCE_VIDEO_OUT_ERROR_RESOURCE_BUSY);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
		}

		if (m_option.enableHmdReprojectionMirroring && (m_option.videoOutBusUseBitmap & (1ul << SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)))
		{
			if (m_hmdReprojectionMirroringWorkMemory.get() == nullptr && m_hmdReprojectionMirroringDisplayMemory.get() == nullptr)
			{
				sce::Agc::SizeAlign mirrorWorkSizeAlign = sceHmd2ReprojectionGetMirroringWorkMemorySizeAlign();
				sce::Agc::SizeAlign mirrorDisplaySizeAlign = sceHmd2ReprojectionGetMirroringDisplayBufferSizeAlign();
				m_hmdReprojectionMirroringWorkMemory = sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(mirrorWorkSizeAlign.m_size, mirrorWorkSizeAlign.m_align, m_videoMemory, "hmd2 reprojection mirror work");
				m_hmdReprojectionMirroringDisplayMemory = sce::SampleUtil::Memory::Gpu::make_unique<uint8_t>(mirrorDisplaySizeAlign.m_size, mirrorDisplaySizeAlign.m_align, m_phyAddrContiguousVideoMemory, "hmd2 reprojection mirror display");
			}
			SceHmd2ReprojectionMirroringParam mirrorParam = { m_hmdReprojectionMirroringWorkMemory.get(), m_hmdReprojectionMirroringDisplayMemory.get() };
			ret = sceHmd2ReprojectionEnableMirroring(&mirrorParam);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		m_isCleared	= false;

		return SCE_OK;
	}

	int	GraphicsContext::clearVideoOut(const GraphicsContextOption	*nextOption)
	{
		int ret = SCE_OK;

		if (m_isCleared) {
			return SCE_OK;
		}

		if (m_option.enableHmdReprojectionMirroring && (m_option.videoOutBusUseBitmap & (1ul << SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)))
		{
			ret = sceHmd2ReprojectionDisableMirroring();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		m_busTypeTable.clear();

		uint32_t frameBufferCounts[] =		{ m_option.numFrameBufferMain,	m_option.numFrameBufferOverlay,	m_option.numFrameBufferHmdMain};
		SceVideoOutFlipMode flipModes[] =	{ m_option.flipModeMain,		m_option.flipModeOverlay,		m_option.flipModeHmdMain };
		int videoOutHandle[] =				{ m_videoHandleMain,			m_videoHandleOverlay,			m_videoHandleHmd };
		SceKernelEqueue flipEqueues[] =		{ m_flipEventQueueMain,			m_flipEventQueueOverlay,		m_flipEventQueueHmd };

		uint64_t buses = m_option.videoOutBusUseBitmap;

		// Find the maixmum buffer count on the active buses.
		uint32_t numFrameBuffers = 0;
		for (int i = 0; i < sizeof(frameBufferCounts) / sizeof(frameBufferCounts[0]); i++)
		{
			uint32_t bus = (i < 2) ? i : SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN;
			if ((buses & (1ul << bus)) != 0ul)
			{
				numFrameBuffers = std::max(numFrameBuffers, frameBufferCounts[i]);
				// Delete flip event
				ret = sceVideoOutDeleteFlipEvent(flipEqueues[i], videoOutHandle[i]);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK)
				{
					return SCE_SAMPLE_UTIL_ERROR_FATAL;
				}
			}
		}

		if (nextOption != nullptr)
		{
			// Flip for the maximum buffer count.
			for (uint32_t flip = 0; flip < numFrameBuffers; flip++)
			{
				// Flip slaves first.
				for (int i = 0; i < sizeof(flipModes) / sizeof(flipModes[0]); i++)
				{
					uint32_t bus = (i < 2) ? i : SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN;
					if ((buses & (1ul << bus)) != 0ul && (flipModes[i] == SCE_VIDEO_OUT_FLIP_MODE_SLAVE))
					{
						while (sceVideoOutIsFlipPending(videoOutHandle[i]))
						{
							sceKernelUsleep(100);
						}
						do
						{
							ret = sceVideoOutSubmitFlip(videoOutHandle[i], SCE_VIDEO_OUT_BUFFER_INDEX_BLANK, flipModes[i], 0);
						} while (ret == SCE_VIDEO_OUT_ERROR_FLIP_QUEUE_FULL);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
						if (ret != SCE_OK)
						{
							return SCE_SAMPLE_UTIL_ERROR_FATAL;
						}
					}
				}

				// Then non slaves.
				for (int i = 0; i < sizeof(flipModes) / sizeof(flipModes[0]); i++)
				{
					uint32_t bus = (i < 2) ? i : SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN;
					if ((buses & (1ul << bus)) != 0ul && (flipModes[i] != SCE_VIDEO_OUT_FLIP_MODE_SLAVE))
					{
						while (sceVideoOutIsFlipPending(videoOutHandle[i]))
						{
							sceKernelUsleep(100);
						}
						do
						{
							ret = sceVideoOutSubmitFlip(videoOutHandle[i], SCE_VIDEO_OUT_BUFFER_INDEX_BLANK, flipModes[i], 0);
						} while (ret == SCE_VIDEO_OUT_ERROR_FLIP_QUEUE_FULL);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
						if (ret != SCE_OK)
						{
							return SCE_SAMPLE_UTIL_ERROR_FATAL;
						}
					}
				}
			}

			// Then free up all the buffers.
			for (int i = 0; i < sizeof(flipModes) / sizeof(flipModes[0]); i++)
			{
				uint32_t bus = (i < 2) ? i : SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN;
				if ((buses & (1ul << bus)) != 0ul)
				{
					while (sceVideoOutIsFlipPending(videoOutHandle[i]))
					{
						sceKernelUsleep(100);
					}
					ret = sceVideoOutUnregisterBuffers(videoOutHandle[i], i);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					if (ret != SCE_OK)
					{
						if (ret == SCE_VIDEO_OUT_ERROR_RESOURCE_BUSY)
						{
							return SCE_SAMPLE_UTIL_ERROR_BUSY;
						}
						return SCE_SAMPLE_UTIL_ERROR_FATAL;
					}
				}
			}
		}

		RenderTargetWrapper *rts[] = { m_pFrameBufferRenderTargetsMain, m_pFrameBufferRenderTargetsOverlay };
		DepthRenderTargetWrapper *drts[] = { &m_frameBufferDepthRenderTargetMain, &m_frameBufferDepthRenderTargetOverlay };

		// Shutdown the slave video out first.
		for (int i = 0; i < sizeof(flipModes) / sizeof(flipModes[0]); i++)
		{
			uint32_t bus = (i < 2) ? i : SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN;
			if ((buses & (1ul << bus)) != 0ul && (flipModes[i] == SCE_VIDEO_OUT_FLIP_MODE_SLAVE))
			{
				int ret = sceVideoOutClearFlipMaster(videoOutHandle[i]);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (nextOption == nullptr || (nextOption->videoOutBusUseBitmap & (1ul << bus)) == 0ul || bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
				{
					int ret = sceVideoOutClose(videoOutHandle[i]);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				}
				if (i < 2)
				{
					RenderTargetWrapper *pFrameBufferRenderTargets = rts[i];
					DepthRenderTargetWrapper *pFrameBufferDepthRenderTarget = drts[i];

					for (int j = 0; j < frameBufferCounts[i]; j++)
					{
						pFrameBufferRenderTargets[j].fini();
					}
					delete[] pFrameBufferRenderTargets;
					pFrameBufferDepthRenderTarget->fini();
				} else {
					for (int j = 0; j < frameBufferCounts[i]; j++)
					{
						if (m_ppFrameBufferRenderTargetsHmdVr3d[0] != nullptr)
						{
							m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kLeft][j].fini();
							if (!m_option.enableHmdVr3dSideBySide) {
								m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kRight][j].fini();
							}
						}
						if (m_pFrameBufferRenderTargetsHmdUi2d != nullptr)
						{
							m_pFrameBufferRenderTargetsHmdUi2d[j].fini();
						}
						if (m_pFrameBufferRenderTargetsHmdVr2d != nullptr)
						{
							m_pFrameBufferRenderTargetsHmdVr2d[j].fini();
						}
					}
					if (m_ppFrameBufferRenderTargetsHmdVr3d[0] != nullptr)
					{
						delete[] m_ppFrameBufferRenderTargetsHmdVr3d[0];
					} else if (m_pFrameBufferRenderTargetsHmdUi2d != nullptr)
					{
						delete[] m_pFrameBufferRenderTargetsHmdUi2d;
					} else if (m_pFrameBufferRenderTargetsHmdVr2d != nullptr)
					{
						delete[] m_pFrameBufferRenderTargetsHmdVr2d;
					}
					m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kLeft] = m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kRight] = m_pFrameBufferRenderTargetsHmdUi2d = m_pFrameBufferRenderTargetsHmdVr2d = nullptr;
					m_frameBufferDepthRenderTargetsHmdVr3d[(int)HmdEye::kLeft].fini();
					if (!m_option.enableHmdVr3dSideBySide) {
						m_frameBufferDepthRenderTargetsHmdVr3d[(int)HmdEye::kRight].fini();
					}
				}
			}
		}

		// Then non-slaves.
		for (int i = 0; i < sizeof(flipModes) / sizeof(flipModes[0]); i++)
		{
			uint32_t bus = (i < 2) ? i : SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN;
			if ((buses & (1ul << bus)) != 0ul && (flipModes[i] != SCE_VIDEO_OUT_FLIP_MODE_SLAVE))
			{
				if (nextOption == nullptr || (nextOption->videoOutBusUseBitmap & (1ul << bus)) == 0ul || bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
				{
					int ret = sceVideoOutClose(videoOutHandle[i]);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				}
				if (i < 2)
				{
					RenderTargetWrapper *pFrameBufferRenderTargets = rts[i];
					DepthRenderTargetWrapper *pFrameBufferDepthRenderTarget = drts[i];

					for (int j = 0; j < frameBufferCounts[i]; j++)
					{
						pFrameBufferRenderTargets[j].fini();
					}
					delete[] pFrameBufferRenderTargets;
					pFrameBufferDepthRenderTarget->fini();
				} else {
					for (int j = 0; j < frameBufferCounts[i]; j++)
					{
						if (m_ppFrameBufferRenderTargetsHmdVr3d[0] != nullptr)
						{
							m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kLeft][j].fini();
							if (!m_option.enableHmdVr3dSideBySide) {
								m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kRight][j].fini();
							}
						}
						if (m_pFrameBufferRenderTargetsHmdUi2d != nullptr)
						{
							m_pFrameBufferRenderTargetsHmdUi2d[j].fini();
						}
						if (m_pFrameBufferRenderTargetsHmdVr2d != nullptr)
						{
							m_pFrameBufferRenderTargetsHmdVr2d[j].fini();
						}
					}
					if (m_ppFrameBufferRenderTargetsHmdVr3d[0] != nullptr)
					{
						delete[] m_ppFrameBufferRenderTargetsHmdVr3d[0]; 
					} else if (m_pFrameBufferRenderTargetsHmdUi2d != nullptr)
					{
						delete[] m_pFrameBufferRenderTargetsHmdUi2d;
					} else if (m_pFrameBufferRenderTargetsHmdVr2d != nullptr)
					{
						delete[] m_pFrameBufferRenderTargetsHmdVr2d;
					}
					m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kLeft] = m_ppFrameBufferRenderTargetsHmdVr3d[(int)HmdEye::kRight] = m_pFrameBufferRenderTargetsHmdUi2d = m_pFrameBufferRenderTargetsHmdVr2d = nullptr;
					m_frameBufferDepthRenderTargetsHmdVr3d[(int)HmdEye::kLeft].fini();
					if (!m_option.enableHmdVr3dSideBySide) {
						m_frameBufferDepthRenderTargetsHmdVr3d[(int)HmdEye::kRight].fini();
					}
				}
			}
		}

		if ((buses & (1ul << SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)) != 0ul &&
			(nextOption == nullptr || (nextOption->videoOutBusUseBitmap & (1ul << SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)) == 0ul || m_option.hmdReprojectionSyncMode != nextOption->hmdReprojectionSyncMode))
		{
			ret = sceHmd2ReprojectionDisableVrMode();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		m_flipLabelsMain.reset(nullptr);
		m_flipLabelsOverlay.reset(nullptr);
		m_flipLabelsHmd.reset(nullptr);

		m_isCleared = true;

		return SCE_OK;
	}

	GraphicsContext::GraphicsContext(uint32_t	width, uint32_t	height, Helper::AsyncAssetLoader	*asyncAssetLoader, GraphicsContextOption	*option)
		: m_errorCode					(SCE_OK)
		, m_isCleared					(false)
		, m_option						(option)
		, m_videoMemory					(m_option.videoHeapSize, SCE_KERNEL_MTYPE_C_SHARED, false, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, "sce::SampleUtil::Graphics::GraphicsContext::m_videoMemory")	// Auto
		, m_phyAddrContiguousVideoMemory(m_option.paContVideoHeapSize, SCE_KERNEL_MTYPE_C_SHARED, true, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, "sce::SampleUtil::Graphics::GraphicsContext::m_phyAddrContiguousVideoMemory")	// Direct
		, m_videoRingMemory				(m_option.videoRingSize, SCE_KERNEL_MTYPE_C_SHARED, "sce::SampleUtil::Graphics::GraphicsContext::m_videoRingMemory")
		, m_isFlipRequestPendingMain	(false)
		, m_isFlipRequestPendingOverlay	(false)
		, m_isFlipRequestPendingHmd		(false)
		, m_flipCountMain				(0)
		, m_flipCountOverlay			(0)
		, m_width						(width)
		, m_height						(height)
		, m_texLib						(m_videoMemory, asyncAssetLoader)
	{
		int ret = SCE_OK; (void)ret;

		SCE_SAMPLE_UTIL_ASSERT(m_option.videoOutBusUseBitmap & (1ul << m_option.baseFrameBus));

		TAG_THIS_CLASS;
		// create initial frame deferred free queue
		DeferredGpuMemoryRelease::m_pAlloc = &m_videoMemory;
		m_deferredGpuMemoryRelease.emplace_back();
		m_deferredGpuMemoryRelease.back().m_pReleaseLabel = reinterpret_cast<Agc::Label*>(DeferredGpuMemoryRelease::m_pAlloc->allocate(sizeof(Agc::Label), Agc::Alignment::kLabel));
		m_deferredGpuMemoryRelease.back().m_pReleaseLabel->m_value = 0ull;
		m_deferredGpuMemoryRelease.back().m_isClosed = false;

		// Load modules for "createTextureFromFile()"
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_PNG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_JPEG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		SampleUtil::Graphics::SurfaceUtil::initialize(m_videoMemory);

		// Init Toolkit which is required for clearing targets
		static bool hasInializedToolkit = false;
		if (!hasInializedToolkit)
		{
			ret = Agc::Toolkit::init();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			hasInializedToolkit = true;
		}
		static bool hasInitializedFsrToolkit = false;
		if (!hasInitializedFsrToolkit)
		{
			ret = sce::FrToolkit::init();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			hasInitializedFsrToolkit = true;
		}

		if (option != nullptr)
		{
			if (m_option.frameBufferWidthOverlay == 0)
			{
				m_option.frameBufferWidthOverlay = m_width;
			}
			if (m_option.frameBufferHeightOverlay == 0)
			{
				m_option.frameBufferHeightOverlay = m_height;
			}
		}

		// Create equeue
		ret = sceKernelCreateEqueue(&m_flipEventQueueMain, "flip event queue main");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelCreateEqueue(&m_flipEventQueueOverlay, "flip event queue overlay");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelCreateEqueue(&m_flipEventQueueHmd, "flip event queue HMD");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = Agc::registerWorkloadStream(m_option.workloadStreamId, "SampleUtil Workload");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = setupVideoOut();
		SCE_SAMPLE_UTIL_ASSERT(ret == SCE_OK || ret == SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE);
		if (ret == SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE) {

			m_errorCode	= SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE;
			return;
		}

		{
			const size_t align = 128 * 1024 * 1024;
			m_cpuMemoryMappedAddr = nullptr;
			ret = SampleUtil::Memory::mapDirectMemory(&m_cpuMemoryMappedAddr, m_option.cpuHeapSize, SCE_KERNEL_MTYPE_C, SCE_KERNEL_PROT_CPU_RW, 0, align);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			m_cpuMemory = sceLibcMspaceCreate("cpu memory", m_cpuMemoryMappedAddr, m_option.cpuHeapSize, 0);
			SCE_SAMPLE_UTIL_ASSERT(m_cpuMemory != nullptr);
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				Memory::registerMatGroup(m_cpuMemory, "sce::SampleUtil::Graphics::GraphicsContext::m_cpuMemory");
				sceMatMspaceCreate(m_cpuMemory, "cpu memory", m_cpuMemoryMappedAddr, m_option.cpuHeapSize, 0, Memory::getMatGroup(m_cpuMemory));
				m_cpuMemoryPool = sceMatAllocPoolMemory(m_cpuMemoryMappedAddr, m_option.cpuHeapSize, Memory::getMatGroup(m_cpuMemory));
				sceMatTagPool(m_cpuMemoryPool, "sce::SampleUtil::Graphics::GraphicsContext::m_cpuMemory");
			}
#endif
		}

		for (int mrt_i = 0; mrt_i < 8; mrt_i++)
		{
			m_pCurrentRenderTargets[mrt_i].init();
		}
		m_numRenderTargets = 0;
		m_currentDepthRenderTarget.init();

		const uint32_t segmentSizeInDwords = (option ? option->ringBufferSegmentSize / 4 : 1024 * 1024);
		SCE_SAMPLE_UTIL_ASSERT(segmentSizeInDwords > sizeof(Agc::Core::RingBuffer::SegmentHeader) / 4 + Agc::Core::RingBuffer::kReservedSpaceSizeInDword);

		const uint32_t numSegments = option ? option->numRingBufferSegments : 8;

		m_rbSemgemtMem = Memory::Gpu::make_unique<uint32_t>(segmentSizeInDwords * numSegments, Agc::Alignment::kCommandBuffer, { Agc::ResourceRegistration::ResourceType::kDrawCommandBufferBaseAddress }, m_videoMemory, "sce::SampleUtil::Graphics::RingBufferSegment");
		m_rb.init()
			.addSegments(segmentSizeInDwords * sizeof(uint32_t), numSegments, m_rbSemgemtMem.get()) // We could add segments later if we wanted to, not just at initialization.
			.setAutoSubmitMode(Agc::Core::RingBuffer::AutoSubmit::kThreshold, 1); // The AutoSubmitMode can be changed at any time.

		// The DCB itself is the only TwoSidedAllocator used in this sample.
		// It is initialized without memory backing and will immediately request memory from the RingBuffer.
		m_ctx.m_dcb.init(
			nullptr,
			0,
			// This is the OutOfMemoryCallback, which is called by the CommandBuffer whenever it runs out
			// of space in the segment. It encodes the queue that this command buffer is submitted to.
			Agc::Core::RingBuffer::callback(Agc::GraphicsQueue::kNormal),
			&m_rb);

		// The StateBuffer uses the DCB as its storage.
		m_ctx.m_sb.init(128, &m_ctx.m_dcb, &m_ctx.m_dcb);

		// Initialise the binder. We set scratch to nullptr as we are going to set this separately per stage.
		m_ctx.m_bdr.init(&m_ctx.m_dcb, nullptr);

		// The payload for the OutOfMemoryCallback are pointers to the RingBuffer and the parent dcb that references this data segment.
		m_dataOomPayload.m_rb		= &m_rb;
		m_dataOomPayload.m_parentCb	= &m_ctx.m_dcb;

		// Set the per-stage scratch allocators.
		for(uint32_t stage_i = 0; stage_i < (uint32_t)sce::Agc::ShaderType::kRealCount; ++stage_i)
		{

			// Initialise the data allocator which will be managed by the RingBuffer.
			m_pDataAllocators[stage_i].init(
				nullptr,
				0,
				// Similar to when initialising the dcb, we must provide an OutOfMemoryCallback.
				// In this case it is a dataCallback which encodes whether the parent cb is graphics or async compute.
				sce::Agc::Core::RingBuffer::dataCallback(sce::Agc::Core::RingBuffer::CbType::kGraphics),
				&m_dataOomPayload);

			// Set the scratch allocator for this StageBinder.
			m_ctx.m_bdr.getStage((sce::Agc::ShaderType)stage_i).setScratch(&m_pDataAllocators[stage_i]);
		}

		// The callbacks are set up so that each all contexts are nofied whenever a segment is closed.
		m_rbSegmentCallback	= Agc::Core::BasicContext::segmentLostCallback;
		m_rbSegmentPayload	= &m_ctx;

		// Register the callbacks.
		m_rb.setSegmentChangeCallbacks(&m_rbSegmentCallback, &m_rbSegmentPayload, 1);
		
		m_currentCtx = &m_ctx;

		m_videoRingMemory.beginFrame(m_ctx.m_dcb);

		m_hmdReprojectionMirroringWorkMemory.reset(nullptr);
		m_hmdReprojectionMirroringDisplayMemory.reset(nullptr);
	}

	GraphicsContext::~GraphicsContext()
	{
		int ret = SCE_OK; (void)ret;

		UNTAG_THIS_CLASS;
		ret = Agc::unregisterWorkloadStream(m_option.workloadStreamId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		if (getErrorCode() != SCE_VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE) {
			clearVideoOut();

			m_rbSemgemtMem.reset(nullptr);

#ifdef _DEBUG
			if (Memory::g_isMatInitialized) {
				sceMatMspaceDestroy(m_cpuMemory);
				sceMatFreePoolMemory(m_cpuMemoryPool);
			}
#endif
			ret = sceLibcMspaceDestroy(m_cpuMemory);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			ret = Memory::munmap(m_cpuMemoryMappedAddr, m_option.cpuHeapSize);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		// Delete equeue
		ret = sceKernelDeleteEqueue(m_flipEventQueueMain);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelDeleteEqueue(m_flipEventQueueOverlay);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelDeleteEqueue(m_flipEventQueueHmd);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		SampleUtil::Graphics::SurfaceUtil::finalize();

		// Unload modules
		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_JPEG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_PNG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		for(auto	&releaseLabel : m_deferredGpuMemoryRelease) {
			DeferredGpuMemoryRelease::m_pAlloc->free(releaseLabel.m_pReleaseLabel);
		}
	}

	Agc::CxRenderTarget	*GraphicsContext::getCurrentRenderTarget(int	mrt_i)
	{
		return (mrt_i < m_numRenderTargets) ? &m_pCurrentRenderTargets[mrt_i] : nullptr;
	}

	Agc::CxDepthRenderTarget	*GraphicsContext::getCurrentDepthStencilSurface()
	{
		return	(m_currentDepthRenderTarget.getDepthWriteAddress() != nullptr || m_currentDepthRenderTarget.getStencilWriteAddress() != nullptr) ? &m_currentDepthRenderTarget : nullptr;
	}

	int GraphicsContext::changeOption(const GraphicsContextOption	&option, uint32_t	width, uint32_t	height)
	{
		int	ret = SCE_OK;

		if (option.videoHeapSize != m_option.videoHeapSize ||
			option.videoRingSize != m_option.videoRingSize ||
			option.cpuHeapSize != m_option.cpuHeapSize ||
			option.dcbSize != m_option.dcbSize)
		{
			SCE_SAMPLE_UTIL_ASSERT(false);
			return	SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		ret = clearVideoOut(&option);
		if (ret != SCE_OK)
		{
			return	ret;
		}
		const auto prevOption = m_option;
		m_option = option;
		if (width != 0 && height != 0) {
			m_width		= width;
			m_height	= height;
		}
		ret = setupVideoOut(&prevOption);
		if (ret != SCE_OK) {
			m_option	= prevOption;
		}

		return	ret;
	}

	int	GraphicsContext::setRenderTarget(sce::Agc::Core::BasicContext	*currentCtx)
	{
		int ret = SCE_OK;

		uint32_t width = 0, height = 0;
		for (int mrt_i = 0; mrt_i < m_numRenderTargets; mrt_i++)
		{
			if (m_pCurrentRenderTargets[mrt_i].getDataAddress() != nullptr)
			{
				uint32_t mip = m_pCurrentRenderTargets[mrt_i].getCurrentMipLevel();
				sce::Agc::Core::RenderTargetSpec spec;
				ret = Agc::Core::translate(&spec, &m_pCurrentRenderTargets[mrt_i]);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK)
				{
					return ret;
				}
				sce::AgcGpuAddress::SurfaceSummary summary;
				ret = Agc::Core::translate(&summary, &spec);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK)
				{
					return ret;
				}
				width = summary.m_mips[mip].m_width;
				height = summary.m_mips[mip].m_height;
				SCE_SAMPLE_UTIL_ASSERT(width > 0 && height > 0);
				if (!(width > 0 && height > 0))
				{
					return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				}
				break;
			}
		}
		if (width == 0 && height == 0)
		{
			uint32_t mip = m_currentDepthRenderTarget.getCurrentMipLevel();
			sce::Agc::Core::DepthRenderTargetSpec spec;
			ret = Agc::Core::translate(&spec, &m_currentDepthRenderTarget);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
			sce::AgcGpuAddress::SurfaceSummary summary;
			ret = Agc::Core::translate(&summary, &spec, Agc::Core::DepthRenderTargetComponent::kDepth);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
			width = summary.m_mips[mip].m_width;
			height = summary.m_mips[mip].m_height;
			SCE_SAMPLE_UTIL_ASSERT(width > 0 && height > 0);
			if (!(width > 0 && height > 0))
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
		}
		Agc::CxDccControl dccCtl;
		dccCtl.init();
		if (m_numRenderTargets > 0)
		{
			ret = Agc::Core::setOverwriteCombiner(&dccCtl, m_pCurrentRenderTargets, m_numRenderTargets);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
		}
		for (int mrt_i = 0; mrt_i < 8; mrt_i++)
		{
			currentCtx->m_sb.setState(((mrt_i < m_numRenderTargets) ? m_pCurrentRenderTargets[mrt_i] : Agc::CxRenderTarget().init()).setSlot(mrt_i));
		}
		currentCtx->m_sb
			.setState(Agc::CxRenderTargetMask().init()
				.setMask(0, (0 < m_numRenderTargets) ? 0xf : 0)
				.setMask(1, (1 < m_numRenderTargets) ? 0xf : 0)
				.setMask(2, (2 < m_numRenderTargets) ? 0xf : 0)
				.setMask(3, (3 < m_numRenderTargets) ? 0xf : 0)
				.setMask(4, (4 < m_numRenderTargets) ? 0xf : 0)
				.setMask(5, (5 < m_numRenderTargets) ? 0xf : 0)
				.setMask(6, (6 < m_numRenderTargets) ? 0xf : 0)
				.setMask(7, (7 < m_numRenderTargets) ? 0xf : 0))
			.setState(m_currentDepthRenderTarget)
			.setState(dccCtl);

		if (m_currentFsrTarget == HmdFsrTarget::kNone)
		{
			if (m_option.enableHmdVr3dSideBySide && m_currentBus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN) {
				Agc::Core::CxScreenViewport	viewports[2];
				sce::Agc::Core::setupScreenViewport(&viewports[0], 0, 0, width, height, 0.5f, 0.5f);
				viewports[1] = viewports[0];
				sce::Agc::Core::setViewport(&viewports[0].m_viewport, width / 2, height, 0, 0, 0.f, 1.f);
				sce::Agc::Core::setViewport(&viewports[1].m_viewport, width / 2, height, width / 2, 0, 0.f, 1.f);
				viewports[0].m_viewport.setSlot(0);
				viewports[1].m_viewport.setSlot(1);
				currentCtx->m_sb.setState(viewports[0]).setState(viewports[1]);
			} else {
				Agc::Core::CxScreenViewport viewport;
				currentCtx->m_sb
					.setState(Agc::Core::setupScreenViewport(&viewport, 0, 0, width, height, 0.5f, 0.5f))
					.setState(FsrRenderState().init().enableFsr(false));
			}
		} else {
			// FSR render target size needs to be identical to one for HMD target
			Agc::Core::CxScreenViewport viewports[2];
			Agc::Core::GuardbandScale	gbScales[2];
			sce::FrToolkit::UcFsrView	tmpFsrViews[2] = { m_hmdVr3dFsrViews[0], m_hmdVr3dFsrViews[1] };
			switch (m_currentFsrTarget)
			{
			case HmdFsrTarget::kLeft:
				ret = sce::FrToolkit::computeGuardbandScale(&gbScales[0].x, &gbScales[0].y, &m_hmdVr3dFsrViews[(int)HmdEye::kLeft]);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK) {
					return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				}
				Agc::Core::setupScreenViewport(&viewports[0], 0, 0, (m_option.enableHmdVr3dSideBySide && m_currentBus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN) ? width / 2 : width, height, 0.5f, 0.5f, gbScales[0]);

				currentCtx->m_sb
					.setState(FsrRenderState().init().enableFsr(true, 0x1))
					.setState(tmpFsrViews[(int)HmdEye::kLeft].setSlot(sce::FrToolkit::UcFsrView::Slot::kLeft))
					.setState(viewports[0]);
				break;
			case HmdFsrTarget::kRight:
				ret = sce::FrToolkit::computeGuardbandScale(&gbScales[0].x, &gbScales[0].y, &m_hmdVr3dFsrViews[(int)HmdEye::kRight]);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK) {
					return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				}
				Agc::Core::setupScreenViewport(&viewports[0], (m_option.enableHmdVr3dSideBySide && m_currentBus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN) ? width / 2 : 0, 0, width, height, 0.5f, 0.5f, gbScales[0]);

				currentCtx->m_sb
					.setState(FsrRenderState().init().enableFsr(true, 0x1))
					/* When each eye is rendered separately, the FSR slot for both eyes is set to 'Left'.
					*  When using stereo expansion, the correct slot needs to be set.
					*/
					.setState(tmpFsrViews[(int)HmdEye::kRight].setSlot(sce::FrToolkit::UcFsrView::Slot::kLeft))
					.setState(viewports[0]);
				break;
			case HmdFsrTarget::kStereoExpansion:
				tmpFsrViews[(int)HmdEye::kLeft].setSlot(sce::FrToolkit::UcFsrView::Slot::kLeft).m_window.setTopLeftX(0).setBottomRightX(width / 2);
				tmpFsrViews[(int)HmdEye::kRight].setSlot(sce::FrToolkit::UcFsrView::Slot::kRight).m_window.setTopLeftX(width / 2).setBottomRightX(width);

				ret = sce::FrToolkit::computeGuardbandScale(&gbScales[(int)HmdEye::kLeft].x, &gbScales[(int)HmdEye::kLeft].y, &tmpFsrViews[(int)HmdEye::kLeft]);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK) {
					return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				}
				ret = sce::FrToolkit::computeGuardbandScale(&gbScales[(int)HmdEye::kRight].x, &gbScales[(int)HmdEye::kRight].y, &tmpFsrViews[(int)HmdEye::kRight]);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				if (ret != SCE_OK) {
					return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				}
				gbScales[0].x = std::max(gbScales[0].x, gbScales[1].x);
				gbScales[0].y = std::max(gbScales[0].y, gbScales[1].y);

				Agc::Core::setupScreenViewport(&viewports[(int)HmdEye::kLeft], 0, 0, width, height, 0.5f, 0.5f, gbScales[0]);
				Agc::Core::setupScreenViewport(&viewports[(int)HmdEye::kRight], 0, 0, width, height, 0.5f, 0.5f, gbScales[0]);
				Agc::Core::setViewport(&viewports[(int)HmdEye::kLeft].m_viewport, width / 2, height, 0, 0, 0.f, 1.f);
				Agc::Core::setViewport(&viewports[(int)HmdEye::kRight].m_viewport, width / 2, height, width / 2, 0, 0.f, 1.f);

				viewports[(int)HmdEye::kRight].m_viewport.setSlot(1);

				currentCtx->m_sb
					.setState(FsrRenderState().init().enableFsr(true, 0x3))
					.setState(tmpFsrViews[(int)HmdEye::kLeft])
					.setState(viewports[(int)HmdEye::kLeft])
					.setState(tmpFsrViews[(int)HmdEye::kRight])
					.setState(viewports[(int)HmdEye::kRight]);
				break;
			default:
				break;
			}
		}

		currentCtx->m_sb.closeCx();

		return SCE_OK;
	}

	void	GraphicsContext::clearRenderTarget(uint32_t	color, sce::Agc::Core::BasicContext	*currentCtx)
	{
		int ret = SCE_OK; (void)ret;

		if (currentCtx == nullptr) currentCtx = m_currentCtx; // set default(m_currentCtx) if nullptr is set

		sce::SampleUtil::Debug::ScopedPerfOf<sce::Agc::DrawCommandBuffer> perf(&currentCtx->m_dcb, "GraphicsContext::clearRenderTarget");

		Agc::Toolkit::Result result;

		// Clear RenderTarget
		for (int mrt_i = 0; mrt_i < m_numRenderTargets; mrt_i++)
		{
			if (m_pCurrentRenderTargets[mrt_i].getDataAddress() != nullptr)
			{
				sce::SampleUtil::Debug::ScopedPerfOf<sce::Agc::DrawCommandBuffer> perf(&currentCtx->m_dcb, "Toolkit::clearRenderTargetCs");
				Agc::Core::Encoder::EncoderValue clearValue = Agc::Core::Encoder::encode({ (float)(color & 0xff) / 255.f, (float)((color >> 8) & 0xff) / 255.f, (float)((color >> 16) & 0xff) / 255.f, (float)((color >> 24) & 0xff) / 255.f });
				result = Agc::Toolkit::clearRenderTargetCs(&currentCtx->m_dcb, &m_pCurrentRenderTargets[mrt_i], clearValue);
				currentCtx->resetToolkitChangesAndSyncToGl2(result);
			}
		}

		if (m_currentDepthRenderTarget.getDepthWriteAddress() != nullptr || m_currentDepthRenderTarget.getStencilWriteAddress() != nullptr)
		{
			// Clear Depth
			sce::SampleUtil::Debug::ScopedPerfOf<sce::Agc::DrawCommandBuffer> perf(&currentCtx->m_dcb, "Toolkit::clearDepthTargetPs");
			m_currentDepthRenderTarget.setDepthClearValue(1.0f);
			result = Agc::Toolkit::clearDepthRenderTargetPs(&currentCtx->m_dcb, &currentCtx->m_sb, &m_currentDepthRenderTarget);
			currentCtx->resetToolkitChangesAndSyncToGl2(result);
		}

		currentCtx->m_sb.postDraw();

		// revert render target/depth render target settings to ones set by beginScene()
		ret = setRenderTarget(currentCtx);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	bool	GraphicsContext::isDisplaymodeSupported(DisplayMode	mode) const
	{
		return true;
	}

	int	GraphicsContext::setDisplaymode(DisplayMode	mode, int	bus)
	{
		auto newOption = m_option;
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			switch (mode)
			{
			case DisplayMode::kNormal:
				newOption.enableHdrViewMain = false;
				break;
			case DisplayMode::kHdrView:
				newOption.enableHdrViewMain = true;
				newOption.enableLinearHdrMain = false;
				break;
			case DisplayMode::kHdrLinearView:
				newOption.enableHdrViewMain = true;
				newOption.enableLinearHdrMain = true;
				break;
			default:
				break;
			}
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			switch (mode)
			{
			case DisplayMode::kNormal:
				newOption.enableHdrViewOverlay = false;
				break;
			case DisplayMode::kHdrView:
				newOption.enableHdrViewOverlay = true;
				newOption.enableLinearHdrOverlay = false;
				break;
			case DisplayMode::kHdrLinearView:
				newOption.enableHdrViewOverlay = true;
				newOption.enableLinearHdrOverlay = true;
				break;
			default:
				break;
			}
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return changeOption(newOption);
	}

	int	GraphicsContext::setFsrView(HmdEye	eye, const sce::FrToolkit::FoveatedViewSpec	&spec)
	{
		int ret = SCE_OK; (void)ret;

		ret = sce::FrToolkit::translate(&m_hmdVr3dFsrViews[(int)eye], &spec);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

	int	GraphicsContext::setLensOptimizedFsrView(uint32_t	width, uint32_t	height, bool	isSideBySide)
	{
		int ret = SCE_OK;

		if (width == 0 || height == 0) {
			width			= m_option.frameBufferHmdMainVr3d.getWidth();
			height			= m_option.frameBufferHmdMainVr3d.getHeight();
			isSideBySide	= m_option.enableHmdVr3dSideBySide;
		}

		sce::FrToolkit::FsrCurves curves;
		const uint32_t halfViewWidth = isSideBySide ? width / 2 : width;
		const uint32_t halfViewHeight = height;
		memcpy(&curves.m_horizontal, &SCE_HMD2_FSR_CURVE_LEFT.m_horizontal, sizeof(sce::FrToolkit::FsrAxis));
		memcpy(&curves.m_vertical, &SCE_HMD2_FSR_CURVE_LEFT.m_vertical, sizeof(sce::FrToolkit::FsrAxis));
		m_hmdVr3dFsrViews[(int)HmdEye::kLeft].init().setSlot(sce::FrToolkit::UcFsrView::Slot::kLeft);
		ret = sce::FrToolkit::translate(&m_hmdVr3dFsrViews[(int)HmdEye::kLeft], &curves, 0, 0, halfViewWidth, halfViewHeight);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		memcpy(&curves.m_horizontal, &SCE_HMD2_FSR_CURVE_RIGHT.m_horizontal, sizeof(sce::FrToolkit::FsrAxis));
		memcpy(&curves.m_vertical, &SCE_HMD2_FSR_CURVE_RIGHT.m_vertical, sizeof(sce::FrToolkit::FsrAxis));
		/* When each eye is rendered separately, the FSR slot for both eyes is set to 'Left'.
		*  When using stereo expansion, the correct slot needs to be set.
		*/
		m_hmdVr3dFsrViews[(int)HmdEye::kRight].init().setSlot(isSideBySide ? sce::FrToolkit::UcFsrView::Slot::kRight : sce::FrToolkit::UcFsrView::Slot::kLeft);
		ret = sce::FrToolkit::translate(&m_hmdVr3dFsrViews[(int)HmdEye::kRight], &curves, isSideBySide ? halfViewWidth : 0, 0, isSideBySide ? halfViewWidth * 2 : halfViewWidth, halfViewHeight);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}

		return SCE_OK;
	}

	GraphicsContext::DisplayMode	GraphicsContext::getDisplaymode(int	bus) const
	{
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			return m_displayModeMain;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			return m_displayModeOverlay;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			return DisplayMode::kNormal;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return DisplayMode::kNormal;
		}
	}

	int	GraphicsContext::setFrameBufferSize(uint32_t	width, uint32_t	height, int	bus, HmdLayer layer)
	{
		int ret = SCE_OK; (void)ret;

		SCE_SAMPLE_UTIL_ASSERT((m_option.videoOutBusUseBitmap & (1ul << (int)bus)) != 0ul);

		uint32_t					oldWidth			= 0u;
		uint32_t					oldHeight			= 0u;
		int							videoHandle			= -1;
		bool						noGamma				= false;
		DisplayMode					displayMode			= DisplayMode::kNormal;
		RenderTargetWrapper			**ppRenderTargets	= nullptr;
		uint32_t					numRenderTargets	= 0;
		DepthRenderTargetWrapper	*pDepthRenderTargets	= nullptr;

		switch (bus)
		{
		case SCE_VIDEO_OUT_BUS_TYPE_MAIN:
			oldWidth							= m_width;
			oldHeight							= m_height;
			m_width								= width;
			m_height							= height;
			videoHandle							= m_videoHandleMain;
			noGamma								= m_option.noGammaMain;
			displayMode							= m_displayModeMain;
			ppRenderTargets						= &m_pFrameBufferRenderTargetsMain;
			numRenderTargets					= m_option.numFrameBufferMain;
			pDepthRenderTargets					= &m_frameBufferDepthRenderTargetMain;
			break;
		case SCE_VIDEO_OUT_BUS_TYPE_OVERLAY:
			oldWidth							= m_option.frameBufferWidthOverlay;
			oldHeight							= m_option.frameBufferHeightOverlay;
			m_option.frameBufferWidthOverlay	= width;
			m_option.frameBufferHeightOverlay	= height;
			videoHandle							= m_videoHandleOverlay;
			noGamma								= m_option.noGammaOverlay;
			displayMode							= m_displayModeOverlay;
			ppRenderTargets						= &m_pFrameBufferRenderTargetsOverlay;
			pDepthRenderTargets					= &m_frameBufferDepthRenderTargetOverlay;
			numRenderTargets					= m_option.numFrameBufferOverlay;
			break;
		case SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN:
			if (layer == HmdLayer::kVr3d)
			{
				oldWidth						= m_option.frameBufferHmdMainVr3d.getWidth();
				oldHeight						= m_option.frameBufferHmdMainVr3d.getHeight();
				ppRenderTargets					= m_ppFrameBufferRenderTargetsHmdVr3d;
				pDepthRenderTargets				= m_frameBufferDepthRenderTargetsHmdVr3d;
				m_option.frameBufferHmdMainVr3d.setWidth(width);
				m_option.frameBufferHmdMainVr3d.setHeight(height);
			} else if (layer == HmdLayer::kUi2d)
			{
				oldWidth						= m_option.frameBufferHmdMainUi2d.getWidth();
				oldHeight						= m_option.frameBufferHmdMainUi2d.getHeight();
				ppRenderTargets					= &m_pFrameBufferRenderTargetsHmdUi2d;
				pDepthRenderTargets				= &m_frameBufferDepthRenderTargetHmdUi2d;
				m_option.frameBufferHmdMainUi2d.setWidth(width);
				m_option.frameBufferHmdMainUi2d.setHeight(height);
			} else {
				oldWidth						= m_option.frameBufferHmdMainVr2d.getWidth();
				oldHeight						= m_option.frameBufferHmdMainVr2d.getHeight();
				ppRenderTargets					= &m_pFrameBufferRenderTargetsHmdVr2d;
				pDepthRenderTargets				= &m_frameBufferDepthRenderTargetHmdVr2d;
				m_option.frameBufferHmdMainVr2d.setWidth(width);
				m_option.frameBufferHmdMainVr2d.setHeight(height);
			}
			videoHandle							= m_videoHandleHmd;
			noGamma								= false;
			displayMode							= DisplayMode::kNormal;
			numRenderTargets					= m_option.numFrameBufferHmdMain;
			break;
		default: // never reach here
			break;
		}
		if (oldWidth == width && oldHeight == height) // no size change
		{
			return SCE_OK;
		}

		if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			while (sceVideoOutIsFlipPending(videoHandle)) {
				sceKernelUsleep(100);
			}
			const int32_t setIndex = 3; // fixme: SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN cannot be used for set index since it's larger than index max(=3)
			ret = sceVideoOutUnregisterBuffers(videoHandle, setIndex);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		std::vector<SceHmd2ReprojectionBuffer> reprojectionBuffer(numRenderTargets);
		std::vector<std::array<SceHmd2ReprojectionSource, 2>> reprojectionSrc(numRenderTargets);
		memset(reprojectionSrc.data(), 0, sizeof(SceHmd2ReprojectionSource) * 2);

		// change render target size
		SCE_SAMPLE_UTIL_ASSERT(numRenderTargets == 0 || ppRenderTargets[0] != nullptr);
		sce::Agc::Core::RenderTargetSpec rtSpec;

		int numEyes = (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN && layer == HmdLayer::kVr3d) ? 2 : 1;
		uint32_t srcCount = 0;
		if (m_option.enableHmdVr3d) ++srcCount;
		if (m_option.enableHmdUi2d) ++srcCount;
		if (m_option.enableHmdVr2d) ++srcCount;

		for (uint32_t rt_i = 0; rt_i < numRenderTargets; ++rt_i)
		{
			for (int eye = 0; eye < numEyes; eye++)
			{
				if (bus != SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
				{
					sce::Agc::CxRenderTarget &cxRenderTarget = ppRenderTargets[eye][rt_i].m_cxRenderTarget;

					// Get current state and update res.
					ret = sce::Agc::Core::translate(&rtSpec, &cxRenderTarget);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

					// Find the current pair of buffers for DCC and adjust the render target DCC buffer.
					// +-------------+
					// | Scanout DCC |
					// +-------------+ <---- CxRenderTarget DCC address.
					// | Render DCC  |
					// +-------------+
					// Stepping from one to the other is achieved by stepping back DCC size, so if the DCC size changes then the scanout DCC
					// used as the retile target will be in the wrong place for scanout.

					uint8_t	*pOldDccAddress = cxRenderTarget.getDccAddress();
					sce::Agc::SizeAlign oldDccSizeAlign = sce::Agc::Core::getSize(&rtSpec, sce::Agc::Core::RenderTargetComponent::kDcc);
					uint8_t	*pScanoutDccAddress = pOldDccAddress - oldDccSizeAlign.m_size;

					rtSpec.setWidth(width);
					rtSpec.setHeight(height);

					// Generate new compression type for the new size.
					if (rtSpec.getCompression() != sce::Agc::Core::MetadataCompression::kNone)
					{
						sce::Agc::Core::MetadataCompression newCompression = sce::Agc::Core::MetadataCompression::kNone;
						ret = sce::Agc::Core::translate(&newCompression, width, rtSpec.getFormat().m_format,
							sce::Agc::Core::DccCompatibility::kColorBuffer | sce::Agc::Core::DccCompatibility::kVideoOut);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

						if (rtSpec.getCompression() != newCompression)
						{
							rtSpec.setCompression(newCompression);
						}

						sce::Agc::SizeAlign newDccSizeAlign = sce::Agc::Core::getSize(&rtSpec, sce::Agc::Core::RenderTargetComponent::kDcc);

						uint8_t *pNewDccAddress = pScanoutDccAddress + newDccSizeAlign.m_size;
						rtSpec.setDccAddress(pNewDccAddress);
					}

					if (rt_i == 0)
					{
						// change video out attribute
						SCE_SAMPLE_UTIL_ASSERT(videoHandle > 0);
						sce::Agc::Core::Colorimetry inColorimetry = sce::Agc::Core::Colorimetry::kSrgb;
						sce::Agc::Core::Colorimetry outColorimetry = sce::Agc::Core::Colorimetry::kBt709;
						switch (displayMode)
						{
						case DisplayMode::kNormal:
							inColorimetry = sce::Agc::Core::Colorimetry::kSrgb;
							outColorimetry = sce::Agc::Core::Colorimetry::kBt709;
							if (noGamma)
							{
								rtSpec.m_format = { sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
							}
							break;
						case DisplayMode::kHdrView:
							inColorimetry = sce::Agc::Core::Colorimetry::kSt2084;
							outColorimetry = sce::Agc::Core::Colorimetry::kSt2084;
							break;
						case DisplayMode::kHdrLinearView:
							inColorimetry = sce::Agc::Core::Colorimetry::kLinear;
							outColorimetry = sce::Agc::Core::Colorimetry::kSt2084;
							break;
						default:
							SCE_SAMPLE_UTIL_ASSERT(false);
							break;
						}

						SceVideoOutBufferAttribute2 voutAttr;
						ret = sce::Agc::Core::translate(&voutAttr, &rtSpec, inColorimetry, outColorimetry);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

						ret = sceVideoOutSubmitChangeBufferAttribute2(videoHandle, bus, &voutAttr, nullptr);
						if (ret == SCE_VIDEO_OUT_ERROR_RESOURCE_BUSY)
						{
							// revert size
							switch (bus)
							{
							case SCE_VIDEO_OUT_BUS_TYPE_MAIN:
								m_width = oldWidth;
								m_height = oldHeight;
								break;
							case SCE_VIDEO_OUT_BUS_TYPE_OVERLAY:
								m_option.frameBufferWidthOverlay = oldWidth;
								m_option.frameBufferHeightOverlay = oldHeight;
								break;
							default: // never reach here
								break;
							}
							return SCE_SAMPLE_UTIL_ERROR_BUSY; // there is pending change buffer attribute request.
						}
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					}

					ret = sce::Agc::Core::initialize(&cxRenderTarget, &rtSpec);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				} else {
					// HMD scanout render target size change
					if (!m_option.enableHmdVr3dSideBySide || eye == 0) {
						RenderTargetParam newParam = ppRenderTargets[m_option.enableHmdVr3dSideBySide ? 0 : eye][rt_i].getParam();
						newParam.m_spec = (layer == HmdLayer::kVr3d) ? m_option.frameBufferHmdMainVr3d : ((layer == HmdLayer::kUi2d) ? m_option.frameBufferHmdMainUi2d : m_option.frameBufferHmdMainVr2d);
						ppRenderTargets[eye][rt_i].init(newParam, m_videoMemory);
					}

					reprojectionBuffer[rt_i].src = reprojectionSrc[rt_i].data();
					reprojectionBuffer[rt_i].srcCount = srcCount;

					int src_i = 0;

					if (m_option.enableHmdVr3d)
					{
						reprojectionBuffer[rt_i].src[src_i].layerType = SCE_HMD2_REPROJECTION_LAYER_TYPE_VR3D;
						reprojectionBuffer[rt_i].src[src_i].data.src3d.lumaScaleFromSdrToHdr = 1.f;
						reprojectionBuffer[rt_i].src[src_i].data.src3d.enableFsrReprojection = false;
						if (eye == (int)HmdEye::kLeft)
						{
							reprojectionBuffer[rt_i].src[src_i].data.src3d.srcL = m_ppFrameBufferRenderTargetsHmdVr3d[eye][rt_i].getColorTexture();
						} else {
							reprojectionBuffer[rt_i].src[src_i].data.src3d.srcR = m_ppFrameBufferRenderTargetsHmdVr3d[m_option.enableHmdVr3dSideBySide ? 0 : eye][rt_i].getColorTexture();
						}
						++src_i;
					}
					if (m_option.enableHmdUi2d)
					{
						reprojectionBuffer[rt_i].src[src_i].layerType = SCE_HMD2_REPROJECTION_LAYER_TYPE_UI2D;
						reprojectionBuffer[rt_i].src[src_i].data.src2d.lumaScaleFromSdrToHdr = 1.f;
						reprojectionBuffer[rt_i].src[src_i].data.src2d.src = m_pFrameBufferRenderTargetsHmdUi2d[rt_i].getColorTexture();
						++src_i;
					}
					if (m_option.enableHmdVr2d)
					{
						reprojectionBuffer[rt_i].src[src_i].layerType = SCE_HMD2_REPROJECTION_LAYER_TYPE_VR2D;
						reprojectionBuffer[rt_i].src[src_i].data.src2d.src = m_pFrameBufferRenderTargetsHmdVr2d[rt_i].getColorTexture();
						++src_i;
					}
				}
			}
		}

		if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			ret = createScanoutBuffers(m_videoHandleHmd, reprojectionBuffer.data(), numRenderTargets);
		}

		// change depth render target size
		for (int eye = 0; eye < (m_option.enableHmdVr3dSideBySide ? 1 : numEyes); eye++)
		{
			sce::Agc::Core::DepthRenderTargetSpec drtSpec;
			ret = sce::Agc::Core::translate(&drtSpec, &pDepthRenderTargets[eye].m_cxDepthRenderTarget);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			drtSpec.m_width		= width;
			drtSpec.m_height	= height;
			ret = sce::Agc::Core::initialize(&pDepthRenderTargets[eye].m_cxDepthRenderTarget, &drtSpec);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		}

		return SCE_OK;
	}

	Agc::CxRenderTarget	*GraphicsContext::getNextRenderTarget(int	bus, HmdLayer	layer, HmdEye	eye)
	{
		RenderTargetWrapper *pRenderTargetWrapper = getNextRenderTargetWrapper(bus, layer, eye);
		return (pRenderTargetWrapper != nullptr) ? &pRenderTargetWrapper->m_cxRenderTarget : nullptr;
	}

	RenderTargetWrapper	*GraphicsContext::getNextRenderTargetWrapper(int	bus, HmdLayer	layer, HmdEye	eye)
	{
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			return &m_pFrameBufferRenderTargetsMain[m_flipCountMain % m_option.numFrameBufferMain];
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			return &m_pFrameBufferRenderTargetsOverlay[m_flipCountOverlay % m_option.numFrameBufferOverlay];
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			if (layer == HmdLayer::kVr3d)
			{
				return &m_ppFrameBufferRenderTargetsHmdVr3d[(int)eye][m_flipCountHmd % m_option.numFrameBufferHmdMain];
			} else if (layer == HmdLayer::kUi2d)
			{
				return &m_pFrameBufferRenderTargetsHmdUi2d[m_flipCountHmd % m_option.numFrameBufferHmdMain];
			} else {
				return &m_pFrameBufferRenderTargetsHmdVr2d[m_flipCountHmd % m_option.numFrameBufferHmdMain];
			}
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return nullptr;
		}
	}

	Agc::CxDepthRenderTarget	*GraphicsContext::getDepthStencilSurface(int	bus, HmdLayer	layer, HmdEye	eye)
	{
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			return &m_frameBufferDepthRenderTargetMain.m_cxDepthRenderTarget;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			return &m_frameBufferDepthRenderTargetOverlay.m_cxDepthRenderTarget;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			if (layer == HmdLayer::kVr3d)
			{
				return &m_frameBufferDepthRenderTargetsHmdVr3d[(int)eye].m_cxDepthRenderTarget;
			} else if (layer == HmdLayer::kUi2d)
			{
				return &m_frameBufferDepthRenderTargetHmdUi2d.m_cxDepthRenderTarget;
			} else {
				return &m_frameBufferDepthRenderTargetHmdVr2d.m_cxDepthRenderTarget;
			}
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return nullptr;
		}
	}

	int	GraphicsContext::beginScene(Agc::CxRenderTarget	*renderTarget, Agc::CxDepthRenderTarget	*depthStencilSurface, int	numRenderTargets, HmdFsrTarget	fsr, Agc::Core::BasicContext	*currentCtx)
	{
		int ret = SCE_OK;

		SCE_SAMPLE_UTIL_ASSERT(renderTarget != nullptr || depthStencilSurface != nullptr);
		if (renderTarget == nullptr && depthStencilSurface == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		m_currentFsrTarget = fsr;
		if (currentCtx == nullptr) currentCtx = m_currentCtx; // set default(m_currentCtx) if nullptr is set

		m_numRenderTargets = renderTarget ? numRenderTargets : 0;
		for (int mrt_i = 0; mrt_i < m_numRenderTargets; mrt_i++)
		{
			m_pCurrentRenderTargets[mrt_i] = renderTarget[mrt_i];
		}
		m_currentDepthRenderTarget = depthStencilSurface ? *depthStencilSurface : Agc::CxDepthRenderTarget().init();

		m_currentBus = -1;
		if (m_numRenderTargets > 0) {
			auto busSearch = m_busTypeTable.find(m_pCurrentRenderTargets[0].getDataAddress());
			if (busSearch != m_busTypeTable.end()) {
				m_currentBus = busSearch->second;
			}
		}

		ret = setRenderTarget(currentCtx);
		if (ret != SCE_OK) {
			return ret;
		}

		HdrFormat hdr = HdrFormat::kBT709;

		// do waitUntilSafeForRendering if current render target is frame buffer
		if (m_currentBus == SCE_VIDEO_OUT_BUS_TYPE_MAIN) {
			if (m_option.enableHdrViewMain) {
				hdr = m_option.enableLinearHdrMain ? HdrFormat::kBT2020_Linear : HdrFormat::kBT2020_PQ;
			}
			if (!m_isFlipRequestPendingMain) {
				currentCtx->m_dcb.waitUntilSafeForRendering(m_videoHandleMain, m_flipCountMain % m_option.numFrameBufferMain);
				m_isFlipRequestPendingMain = true;
			}
		} else if (m_currentBus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY) {
			if (m_option.enableHdrViewOverlay) {
				hdr = m_option.enableLinearHdrOverlay ? HdrFormat::kBT2020_Linear : HdrFormat::kBT2020_PQ;
			}
			if (!m_isFlipRequestPendingOverlay) {
				currentCtx->m_dcb.waitUntilSafeForRendering(m_videoHandleOverlay, m_flipCountOverlay % m_option.numFrameBufferOverlay);
				m_isFlipRequestPendingOverlay = true;
			}
		} else if (m_currentBus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN) {
			hdr = HdrFormat::kP3D65;
			if (!m_isFlipRequestPendingHmd) {
				currentCtx->m_dcb.waitUntilSafeForRendering(m_videoHandleHmd, m_flipCountHmd % m_option.numFrameBufferHmdMain);
				m_isFlipRequestPendingHmd = true;
			}
		}

		pushHdr(hdr);

		// Clear flag for that ImGui is used for other than "drawDebugString"
		if (ImGui::GetCurrentContext() != nullptr)
		{
			ImGui::GetCurrentContext()->UsedOnlyDebugString = true;
		}

		return ret;
	}

	int	GraphicsContext::endScene(Agc::Toolkit::Result	renderResult, Agc::Core::BasicContext	*currentCtx)
	{
		int ret = SCE_OK;

		if (currentCtx == nullptr) currentCtx = m_currentCtx; // set default(m_currentCtx) if nullptr is set

		updateDirtyCaches(renderResult.m_dirtyCaches);
		popHdr();
		ret  = currentCtx->resetToolkitChangesAndSyncToGl2(renderResult).m_errorCode;
		if (m_currentFsrTarget != HmdFsrTarget::kNone)
		{
			currentCtx->m_sb
				.setState(FsrRenderState().init().enableFsr(false))
				.setState(sce::FrToolkit::UcFsrView().init());
			m_currentFsrTarget = HmdFsrTarget::kNone;
		}
		// reset overwrite combiner
		Agc::CxDccControl dccCtl;
		dccCtl.init();
		currentCtx->m_sb.setState(dccCtl);

		return ret;
	}

	int	GraphicsContext::getSubmittedFlipCount(uint64_t	&outFlipCount, int	bus)
	{
		SCE_SAMPLE_UTIL_ASSERT((m_option.videoOutBusUseBitmap & (1ul << bus)) != 0ul);
		if ((m_option.videoOutBusUseBitmap & (1ul << bus)) == 0ul)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			outFlipCount = m_flipCountMain;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			outFlipCount = m_flipCountOverlay;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			outFlipCount = m_flipCountHmd;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

	int	GraphicsContext::getVideoOutPortHandle(int	&outVideoOutPortHandle, int	bus)
	{
		SCE_SAMPLE_UTIL_ASSERT((m_option.videoOutBusUseBitmap & (1ul << bus)) != 0ul);
		if ((m_option.videoOutBusUseBitmap & (1ul << bus)) == 0ul)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			outVideoOutPortHandle = m_videoHandleMain;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			outVideoOutPortHandle = m_videoHandleOverlay;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			outVideoOutPortHandle = m_videoHandleHmd;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

	int	GraphicsContext::getFlipStatus(SceVideoOutFlipStatus	&outFlipStatus, int	bus)
	{
		int ret = SCE_OK;

		SCE_SAMPLE_UTIL_ASSERT((m_option.videoOutBusUseBitmap & (1ul << bus)) != 0ul);
		if ((m_option.videoOutBusUseBitmap & (1ul << bus)) == 0ul)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			ret = sceVideoOutGetFlipStatus(m_videoHandleMain, &outFlipStatus);
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			ret = sceVideoOutGetFlipStatus(m_videoHandleOverlay, &outFlipStatus);
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			ret = sceVideoOutGetFlipStatus(m_videoHandleHmd, &outFlipStatus);
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		return SCE_OK;
	}

	int	GraphicsContext::flip(uint32_t	numVSync, int	bus, uint64_t	flipArg, const ReprojectionParameter	*reprojectionParam, Agc::Core::BasicContext	*currentCtx)
	{
		int ret = SCE_OK; (void)ret;

		if (currentCtx == nullptr) currentCtx = m_currentCtx; // set default(m_currentCtx) if nullptr is set

		uint32_t *pFlipCount;
		uint32_t numFrameBuffer;
		SceVideoOutFlipMode flipMode;
		Agc::Label *pFlipLabels;
		int videoHandle;
		bool needRetile;
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			SCE_SAMPLE_UTIL_ASSERT_MSG(m_isFlipRequestPendingMain, "flip is called without relevant frame buffer beginScene/endScene.");
			if (!m_isFlipRequestPendingMain)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			}
			m_isFlipRequestPendingMain = false;
			pFlipCount = &m_flipCountMain;
			numFrameBuffer = m_option.numFrameBufferMain;
			flipMode = m_option.flipModeMain;
			pFlipLabels = m_flipLabelsMain.get();
			videoHandle = m_videoHandleMain;
			needRetile = m_option.enableDccMain;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			SCE_SAMPLE_UTIL_ASSERT_MSG(m_isFlipRequestPendingOverlay, "flip is called without relevant frame buffer beginScene/endScene.");
			if (!m_isFlipRequestPendingOverlay)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			}
			m_isFlipRequestPendingOverlay = false;
			pFlipCount = &m_flipCountOverlay;
			numFrameBuffer = m_option.numFrameBufferOverlay;
			flipMode = m_option.flipModeOverlay;
			pFlipLabels = m_flipLabelsOverlay.get();
			videoHandle = m_videoHandleOverlay;
			needRetile = m_option.enableDccOverlay;
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			SCE_SAMPLE_UTIL_ASSERT_MSG(m_isFlipRequestPendingHmd, "flip is called without relevant frame buffer beginScene/endScene.");
			if (!m_isFlipRequestPendingHmd)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			}
			m_isFlipRequestPendingHmd = false;
			pFlipCount = &m_flipCountHmd;
			numFrameBuffer = m_option.numFrameBufferHmdMain;
			flipMode = m_option.flipModeHmdMain;
			pFlipLabels = m_flipLabelsHmd.get();
			videoHandle = m_videoHandleHmd;
			needRetile = false;

			// reprojection settings
			SCE_SAMPLE_UTIL_ASSERT(reprojectionParam != nullptr);
			std::vector<SceHmd2ReprojectionSetting> settings;
			std::vector<SceHmd2ReprojectionSettingOption>	options;

			int settingCount = 0;

			if (m_option.enableHmdVr3d)
			{
				settings.emplace_back();
				settings.back() = {};

				settings.back().layerType						= SCE_HMD2_REPROJECTION_LAYER_TYPE_VR3D;
				settings.back().srcId							= (SceHmd2ReprojectionSrcId)(settingCount++);
				settings.back().data.vr3d.pose					= reprojectionParam->m_pose;
				settings.back().data.vr3d.srcSizeL.width		= m_option.frameBufferHmdMainVr3d.getWidth();
				settings.back().data.vr3d.srcSizeL.height		= m_option.frameBufferHmdMainVr3d.getHeight();
				settings.back().data.vr3d.srcSizeR.width		= m_option.frameBufferHmdMainVr3d.getWidth();
				settings.back().data.vr3d.srcSizeR.height		= m_option.frameBufferHmdMainVr3d.getHeight();
				settings.back().data.vr3d.fovL					= reprojectionParam->m_fov;
				settings.back().data.vr3d.fovR					= reprojectionParam->m_fov;
				settings.back().data.vr3d.fovR.tanIn			= reprojectionParam->m_fov.tanOut;
				settings.back().data.vr3d.fovR.tanOut			= reprojectionParam->m_fov.tanIn;
				settings.back().data.vr3d.viewportL.height		= m_option.frameBufferHmdMainVr3d.getHeight();
				settings.back().data.vr3d.viewportL.width		= m_option.frameBufferHmdMainVr3d.getWidth() / (m_option.enableHmdVr3dSideBySide ? 2 : 1);
				settings.back().data.vr3d.viewportL.x			= 0;
				settings.back().data.vr3d.viewportL.y			= 0;
				settings.back().data.vr3d.viewportR.height		= m_option.frameBufferHmdMainVr3d.getHeight();
				settings.back().data.vr3d.viewportR.width		= m_option.frameBufferHmdMainVr3d.getWidth() / (m_option.enableHmdVr3dSideBySide ? 2 : 1);
				settings.back().data.vr3d.viewportR.x			= m_option.enableHmdVr3dSideBySide ? m_option.frameBufferHmdMainVr3d.getWidth() / 2 : 0;
				settings.back().data.vr3d.viewportR.y			= 0;
				settings.back().data.vr3d.sampler.init()
					.setXyFilterMode(Agc::Core::Sampler::FilterMode::kBilinear, Agc::Core::Sampler::FilterMode::kBilinear)
					.setWrapMode(Agc::Core::Sampler::WrapMode::kClampBorder, Agc::Core::Sampler::WrapMode::kClampBorder, Agc::Core::Sampler::WrapMode::kClampBorder)
					.setBorderColor(Agc::Core::Sampler::BorderColor::kTransBlack);
				settings.back().data.vr3d.blendType				= SCE_HMD2_REPROJECTION_LAYER_BLEND_TYPE_LAYER0;
				settings.back().data.vr3d.disableReprojection	= false;
				if (m_option.enableHmdVr3dFsr)
				{
					settings.back().data.vr3d.fsrLutSizeL.horizontal	= m_hmdVr3dFsrLuts[0].horizontal.getWidth();
					settings.back().data.vr3d.fsrLutSizeL.vertical		= m_hmdVr3dFsrLuts[0].vertical.getWidth();
					settings.back().data.vr3d.fsrLutSizeR.horizontal	= m_hmdVr3dFsrLuts[1].horizontal.getWidth();
					settings.back().data.vr3d.fsrLutSizeR.vertical		= m_hmdVr3dFsrLuts[1].vertical.getWidth();

					for (int eye = 0; eye < 2; eye++)
					{
						/* Since our LUT textures we initialized with the help of FsrToolkit, converting
						*  them back to FsrToolkit::FsrLUTs is easy, because we can assume that the
						*  contained memory already has the expected size, and alignment.
						*/
						int bufferIndex = (*pFlipCount % numFrameBuffer) * 2 + eye;
						const uint32_t lutWidth		= m_hmdVr3dFsrLuts[0].horizontal.getWidth();
						const uint32_t lutHeight	= m_hmdVr3dFsrLuts[0].vertical.getWidth();
						sce::FrToolkit::FsrLUTs tmpLUTs;
						ret = sce::FrToolkit::initLUTs(&tmpLUTs, m_hmdVr3dFsrLuts[bufferIndex].horizontal.getDataAddress(), lutWidth, lutHeight, 1);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
						// Now we generate the LUT data
						sce::Agc::Toolkit::Result tkRes = sce::FrToolkit::generateLUT(&currentCtx->m_dcb, &tmpLUTs, sce::FrToolkit::LutType::k_resolve_curve, &m_hmdVr3dFsrViews[eye], lutWidth, lutHeight);
						SCE_SAMPLE_UTIL_ASSERT_EQUAL(tkRes.m_errorCode, SCE_OK);
						currentCtx->resetToolkitChangesAndSyncToGl2(tkRes);
					}
				}
				if (reprojectionParam->m_enablePositionalReprojection) {
					// setting for positional reprojection
					options.emplace_back();
					options.back() = {};
					options.back().tag 										= SCE_HMD2_REPROJECTION_OPTION_TAG_VR3D_POSITIONAL;
					options.back().option.positionalData.srcDepthL			= reprojectionParam->m_depthTextures[(int)HmdEye::kLeft];
					options.back().option.positionalData.srcDepthR			= reprojectionParam->m_depthTextures[(int)(m_option.enableHmdVr3dSideBySide ? HmdEye::kLeft : HmdEye::kRight)];
					options.back().option.positionalData.srcDepthRegionL	= settings.back().data.vr3d.viewportL;
					options.back().option.positionalData.srcDepthRegionR	= settings.back().data.vr3d.viewportR;

					options.back().option.positionalData.poseL				= reprojectionParam->m_leftEyePose;
					options.back().option.positionalData.poseR				= reprojectionParam->m_rightEyePose;

					options.back().option.positionalData.zNear 				= reprojectionParam->m_zNear;
					options.back().option.positionalData.zFar				= reprojectionParam->m_zFar;
				}
				if (reprojectionParam->m_enableAdditionalPose) {
					// setting for additional pose
					options.emplace_back();
					options.back() = {};
					options.back().tag										= SCE_HMD2_REPROJECTION_OPTION_TAG_VR3D_ADDITIONAL_POSE;
					options.back().option.additionalPoseData				= reprojectionParam->m_additionalPose;
				}
				settings.back().data.vr3d.numOptions	= options.size();
				settings.back().data.vr3d.option		= (options.size() > 0) ? options.data() : nullptr;
			}
			if (reprojectionParam->m_enableSeeThrough)
			{
				settings.emplace_back();
				settings.back() = {};
				settings.back().layerType = SCE_HMD2_REPROJECTION_LAYER_TYPE_SEE_THROUGH;
				settings.back().srcId = SCE_HMD2_REPROJECTION_SRC_ID_DUMMY;
				settings.back().data.seeThrough = { reprojectionParam->m_seeThroughStyle, {}, nullptr };
			}
			if (m_option.enableHmdUi2d && reprojectionParam->m_enableUi2d)
			{
				uint32_t viewportWidth = reprojectionParam->m_ui2dPlane.m_viewport.width;
				if (reprojectionParam->m_ui2dPlane.m_viewport.x + viewportWidth > m_option.frameBufferHmdMainUi2d.getWidth())
				{
					viewportWidth = m_option.frameBufferHmdMainUi2d.getWidth() - reprojectionParam->m_ui2dPlane.m_viewport.x;
				}
				uint32_t viewportHeight = reprojectionParam->m_ui2dPlane.m_viewport.height;
				if (reprojectionParam->m_ui2dPlane.m_viewport.y + viewportHeight > m_option.frameBufferHmdMainUi2d.getHeight())
				{
					viewportWidth = m_option.frameBufferHmdMainUi2d.getHeight() - reprojectionParam->m_ui2dPlane.m_viewport.y;
				}
				settings.emplace_back();
				settings.back() = {};
				settings.back().layerType						= SCE_HMD2_REPROJECTION_LAYER_TYPE_UI2D;
				settings.back().srcId							= (SceHmd2ReprojectionSrcId)(settingCount++);
				settings.back().data.ui2d.srcSize.width			= m_option.frameBufferHmdMainUi2d.getWidth();
				settings.back().data.ui2d.srcSize.height		= m_option.frameBufferHmdMainUi2d.getHeight();
				settings.back().data.ui2d.viewport.x			= reprojectionParam->m_ui2dPlane.m_viewport.x;
				settings.back().data.ui2d.viewport.y			= reprojectionParam->m_ui2dPlane.m_viewport.y;
				settings.back().data.ui2d.viewport.width		= viewportWidth;
				settings.back().data.ui2d.viewport.height		= viewportHeight;
				settings.back().data.ui2d.width					= reprojectionParam->m_ui2dPlane.m_width;
				settings.back().data.ui2d.height				= reprojectionParam->m_ui2dPlane.m_height;
				settings.back().data.ui2d.position.x			= reprojectionParam->m_ui2dPlane.m_position.getX();
				settings.back().data.ui2d.position.y			= reprojectionParam->m_ui2dPlane.m_position.getY();
				settings.back().data.ui2d.position.z			= reprojectionParam->m_ui2dPlane.m_position.getZ();
				settings.back().data.ui2d.rotation.x			= reprojectionParam->m_ui2dPlane.m_orientation.getX();
				settings.back().data.ui2d.rotation.y			= reprojectionParam->m_ui2dPlane.m_orientation.getY();
				settings.back().data.ui2d.rotation.z			= reprojectionParam->m_ui2dPlane.m_orientation.getZ();
				settings.back().data.ui2d.rotation.w			= reprojectionParam->m_ui2dPlane.m_orientation.getW();
				settings.back().data.ui2d.sampler.init()
					.setXyFilterMode(Agc::Core::Sampler::FilterMode::kBilinear, Agc::Core::Sampler::FilterMode::kBilinear)
					.setWrapMode(Agc::Core::Sampler::WrapMode::kClampBorder, Agc::Core::Sampler::WrapMode::kClampBorder, Agc::Core::Sampler::WrapMode::kClampBorder)
					.setBorderColor(Agc::Core::Sampler::BorderColor::kTransBlack);
				settings.back().data.ui2d.blendType				= SCE_HMD2_REPROJECTION_LAYER_BLEND_TYPE_INTERPOLATED_ALPHA;
			}
			if (m_option.enableHmdVr2d)
			{
				uint32_t viewportWidth = reprojectionParam->m_vr2dTexViewport.width;
				if (reprojectionParam->m_vr2dTexViewport.x + viewportWidth > m_option.frameBufferHmdMainVr2d.getWidth())
				{
					viewportWidth = m_option.frameBufferHmdMainVr2d.getWidth() - reprojectionParam->m_vr2dTexViewport.x;
				}
				uint32_t viewportHeight = reprojectionParam->m_vr2dTexViewport.height;
				if (reprojectionParam->m_vr2dTexViewport.y + viewportHeight > m_option.frameBufferHmdMainVr2d.getHeight())
				{
					viewportHeight = m_option.frameBufferHmdMainVr2d.getHeight() - reprojectionParam->m_vr2dTexViewport.y;
				}
				settings.emplace_back();
				settings.back() = {};
				settings.back().layerType						= SCE_HMD2_REPROJECTION_LAYER_TYPE_VR2D;
				settings.back().srcId							= (SceHmd2ReprojectionSrcId)(settingCount++);
				settings.back().data.vr2d.srcSize.width			= m_option.frameBufferHmdMainVr2d.getWidth();
				settings.back().data.vr2d.srcSize.height		= m_option.frameBufferHmdMainVr2d.getHeight();
				settings.back().data.vr2d.viewport.x			= reprojectionParam->m_vr2dTexViewport.x;
				settings.back().data.vr2d.viewport.y			= reprojectionParam->m_vr2dTexViewport.y;
				settings.back().data.vr2d.viewport.width		= viewportWidth;
				settings.back().data.vr2d.viewport.height		= viewportHeight;
				settings.back().data.vr2d.sampler.init()
					.setXyFilterMode(Agc::Core::Sampler::FilterMode::kBilinear, Agc::Core::Sampler::FilterMode::kBilinear)
					.setWrapMode(Agc::Core::Sampler::WrapMode::kClampBorder, Agc::Core::Sampler::WrapMode::kClampBorder, Agc::Core::Sampler::WrapMode::kClampBorder)
					.setBorderColor(Agc::Core::Sampler::BorderColor::kTransBlack);
			}
			SceHmd2ReprojectionParam param;
			param.settings		= settings.data();
			param.settingCount	= settings.size();
			param.option		= nullptr;

			sceHmd2ReprojectionSetParam(m_flipCountHmd % m_option.numFrameBufferHmdMain, flipArg, &param, nullptr);
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		// Retile temporary dcc buffer and set retiled-buffer to scanout
		if (needRetile)
		{
			Debug::ScopedPerfOf<Agc::DrawCommandBuffer> perf(&currentCtx->m_dcb, "sce::SampleUtil::Graphics::retileDccSurface");

			// write back CB metedata(DCC) for following compute shader(retilceDccMetadataForVideoOut)'s buffer read
			sce::Agc::Core::gpuSyncEvent(&currentCtx->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphicsAndCompute,
				sce::Agc::Core::SyncCacheOp::kCbMetadataWritebackInvalidate | sce::Agc::Core::SyncCacheOp::kInvalidateGl01);

			auto *pCurrentScanoutTarget = getNextRenderTarget(bus);
			sce::Agc::Core::RenderTargetSpec spec;
			int ret = sce::Agc::Core::translate(&spec, pCurrentScanoutTarget);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			auto dccSizeAlign = Agc::Core::getSize(&spec, Agc::Core::RenderTargetComponent::kDcc);
			auto *pRetiledDcc = pCurrentScanoutTarget->getDccAddress() - dccSizeAlign.m_size;
			// Re-tile DCC metadata for scan-out.
			sce::Agc::Toolkit::Result renderResult = Agc::Toolkit::retileDccMetadataForVideoOut(&currentCtx->m_dcb, pCurrentScanoutTarget, pRetiledDcc);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(renderResult.m_errorCode, SCE_OK);
		}

		// Submit a flip via the GPU.
		// Note: on Prospero, RenderTargets write into the GL2 cache, but the scan-out
		// does not snoop any GPU caches. As such, it is necessary to flush these writes to memory before they can
		// be displayed. This flush is performed internally by setFlip() so we don't need to do it 
		// on the application side.
		currentCtx->m_dcb.setFlip(videoHandle, *pFlipCount % numFrameBuffer, flipMode, flipArg);

		// The last thing we do in the command buffer is write 1 to the flip label to signal that command buffer 
		// processing has finished. 
		//
		// While Agc provides access to the lowest level of GPU synchronization faculties, it also provides
		// functionality that builds the correct synchronization steps in an easier fashion.
		// Since synchonization should be relatively rare, spending a few CPU cycles on letting the library
		// work out what needs to be done is generally a good idea.
		Agc::Core::gpuSyncEvent(
			&currentCtx->m_dcb,
			// The SyncWaitMode controls how the GPU's Command Processor (CP) handles the synchronisation.
			// By setting this to kAsynchronous, we tell the CP that it doesn't have to wait for this operation
			// to finish before it can start the next frame. Instead, we could ask it to drain all graphics work
			// first, but that would be more aggressive than we need to be here.
			Agc::Core::SyncWaitMode::kAsynchronous,
			// Since we are making the label write visible to the CPU, it is not necessary to flush any caches 
			// and we set the cache op to 'kNone'.
			Agc::Core::SyncCacheOp::kNone, 
			// Write the flip label and make it visible to the CPU.
			Agc::Core::SyncLabelVisibility::kCpu,
			&pFlipLabels[*pFlipCount % numFrameBuffer],
			// We write the value "1" to the flip label.
			1);
		pFlipLabels[*pFlipCount % numFrameBuffer].m_value = 0;

		if (bus == m_option.baseFrameBus && currentCtx == m_currentCtx)
		{
			m_videoRingMemory.endFrame(currentCtx->m_dcb);
		}

		if (bus == m_option.baseFrameBus)
		{
			if (*pFlipCount > 0)
			{
				currentCtx->m_dcb.setWorkloadComplete(m_option.workloadStreamId, (*pFlipCount) % 64);
			}
		}

		if (currentCtx == m_currentCtx)
		{
			// The RingBuffer will have submitted every segment dozens of times by the time we get here, but we still 
			// need to ensure that all GPU commands are being executed. By requesting the dcb to be submitted, we ensure
			// this happens. If we had set AutoSubmit::kDisable, this would be the only time a submit would be executed.
			// In that case, we would need many more segments, or much larger segments, to even come here, since no segments
			// could be reclaimed.
			m_rb.submit(&currentCtx->m_dcb, sce::Agc::Core::RingBuffer::SubmitMode::kSubmitAllSegments);
		}

		++*pFlipCount;

		if (bus == m_option.baseFrameBus)
		{
			if (currentCtx == m_currentCtx)
			{
				if (!m_option.disableSuspendPointInFlip)
				{
					Agc::suspendPoint();
				}

				m_videoRingMemory.beginFrame(currentCtx->m_dcb);
			}

			handleDeferredGpuMemoryRelease();

			currentCtx->m_dcb.setWorkloadActive(m_option.workloadStreamId, (*pFlipCount) % 64);
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatNewFrame();
			}
			if (m_enableRazorCpu)
			{
				sceRazorCpuSync();
			}
#endif
		}

		return SCE_OK;
	}

	int GraphicsContext::waitVBlank(int	bus)
	{
		int ret = SCE_OK;

		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			ret = sceVideoOutWaitVblank(m_videoHandleMain);
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			ret = sceVideoOutWaitVblank(m_videoHandleOverlay);
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			ret = sceVideoOutWaitVblank(m_videoHandleHmd);
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return ret;
	}

	int GraphicsContext::waitFlip(int	bus)
	{
		int ret = SCE_OK;

		SceKernelEvent ev;
		int num;
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			ret = sceKernelWaitEqueue(m_flipEventQueueMain, &ev, 1, &num, nullptr);
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			ret = sceKernelWaitEqueue(m_flipEventQueueOverlay, &ev, 1, &num, nullptr);
		} else if (bus == SCE_VIDEO_OUT_BUS_TYPE_HMD2_MAIN)
		{
			ret = sceKernelWaitEqueue(m_flipEventQueueHmd, &ev, 1, &num, nullptr);
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return ret;
	}

	int	GraphicsContext::setVideoOutBlendControl(SceVideoOutBusType	bus, int	globalAlphaMode, int	globalAlphaValue, uint32_t	space, const uint8_t	*a2lut)
	{
		int handle;
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_OVERLAY)
		{
			handle = m_videoHandleOverlay;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		int ret = sceVideoOutSetAlphaControl(handle, globalAlphaMode, globalAlphaValue, a2lut, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		ret = sceVideoOutSetGlobalBlendSpace(m_videoHandleMain, space, 0);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

    int GraphicsContext::createTextureFromFile(Agc::Core::Texture &outTexture, const char *filename)
    {
        return Texture::createFromFile(outTexture, filename, m_videoMemory, m_cpuMemory);
    }

    int GraphicsContext::createTextureFromMemory(Compat::Texture &outTexture, const void *pImageData)
    {
        return Texture::createFromMemory(outTexture, pImageData, m_videoMemory, m_cpuMemory);
    }

	int GraphicsContext::destroyTexture(Agc::Core::Texture &outTexture)
	{
		deferredReleaseGpuMemory(outTexture.getDataAddress());
		outTexture.setDataAddress(nullptr);
		return SCE_OK;
	}

	void	GraphicsContext::updateDirtyCaches(Agc::Toolkit::Result::Caches	&dirtyCaches)
	{
		if (m_numRenderTargets > 0)
		{
			if ((dirtyCaches & Agc::Toolkit::Result::Caches::kCbData) == Agc::Toolkit::Result::Caches::kCbData)
			{
				for (int mrt_i = 0; mrt_i < m_numRenderTargets; mrt_i++)
				{
					{
						if (m_pCurrentRenderTargets[mrt_i].getDccCompression() == Agc::CxRenderTarget::DccCompression::kEnable ||
							m_pCurrentRenderTargets[mrt_i].getFmaskCompression() == Agc::CxRenderTarget::FmaskCompression::kEnable ||
							m_pCurrentRenderTargets[mrt_i].getCmaskFastClear() == Agc::CxRenderTarget::CmaskFastClear::kEnable)
						{
							dirtyCaches |= Agc::Toolkit::Result::Caches::kCbMetadata;
						}
					}
				}
			}
		} else {
			dirtyCaches &= ~(Agc::Toolkit::Result::Caches::kCbData | Agc::Toolkit::Result::Caches::kCbMetadata);
		}
		if (m_currentDepthRenderTarget.getDepthWriteAddress() != nullptr || m_currentDepthRenderTarget.getStencilWriteAddress() != nullptr)
		{
			dirtyCaches |= Agc::Toolkit::Result::Caches::kDbData;
			if (m_currentDepthRenderTarget.getHtileAcceleration() == Agc::CxDepthRenderTarget::HtileAcceleration::kEnable)
			{
				dirtyCaches |= Agc::Toolkit::Result::Caches::kDbMetadata;
			}
		} else {
			dirtyCaches &= ~(Agc::Toolkit::Result::Caches::kDbData | Agc::Toolkit::Result::Caches::kDbMetadata);
		}
	}

	VideoAllocator	*GraphicsContext::DeferredGpuMemoryRelease::m_pAlloc = nullptr;

	void	GraphicsContext::handleDeferredGpuMemoryRelease(bool	isFinalizing)
	{
		if (m_deferredGpuMemoryRelease.size() > 0 && !m_deferredGpuMemoryRelease.back().m_isClosed)
		{
			Agc::Core::gpuSyncEvent(&m_currentCtx->m_dcb, Agc::Core::SyncWaitMode::kAsynchronous, Agc::Core::SyncCacheOp::kNone, Agc::Core::SyncLabelVisibility::kCpu, m_deferredGpuMemoryRelease.back().m_pReleaseLabel, 1ul);
			m_deferredGpuMemoryRelease.back().m_isClosed = true;
		}
		auto iter = m_deferredGpuMemoryRelease.begin();
		for (; iter != m_deferredGpuMemoryRelease.end() && (iter->m_pReleaseLabel->m_value == 1ul); iter++) {}
		for (auto delIter = m_deferredGpuMemoryRelease.begin(); delIter != iter; delIter++)
		{
			for (void *ptr : delIter->m_waitingPointers)
			{
				DeferredGpuMemoryRelease::m_pAlloc->free(ptr);
			}
			DeferredGpuMemoryRelease::m_pAlloc->free(delIter->m_pReleaseLabel);
		}
		m_deferredGpuMemoryRelease.erase(m_deferredGpuMemoryRelease.begin(), iter);
		if (!isFinalizing)
		{
			m_deferredGpuMemoryRelease.emplace_back();
			m_deferredGpuMemoryRelease.back().m_pReleaseLabel = reinterpret_cast<Agc::Label*>(DeferredGpuMemoryRelease::m_pAlloc->allocate(sizeof(Agc::Label), Agc::Alignment::kLabel));
			m_deferredGpuMemoryRelease.back().m_pReleaseLabel->m_value = 0ull;
			m_deferredGpuMemoryRelease.back().m_isClosed = false;
		} else {
			m_rb.submit(&m_currentCtx->m_dcb, sce::Agc::Core::RingBuffer::SubmitMode::kSubmitAllSegments);
		}
	}
	
#ifndef SCE_SAMPLE_UTIL_DISABLE_MODEL_RENDER
	void	GraphicsContext::deferredReleasePackModel(PackModel	*pPackModel)
	{
		if (pPackModel != nullptr) {
			for (auto &mesh : pPackModel->m_meshes) {
				deferredReleaseGpuMemory(mesh.m_gpuMemory.release());
			}
			for (auto &packMemory : pPackModel->m_packGpuMemory) {
				deferredReleaseGpuMemory(packMemory.release());
			}
			for (auto &meshInstanceMemory : pPackModel->m_meshInstancesGpuMemory) {
				deferredReleaseGpuMemory(meshInstanceMemory.release());
			}
			deferredReleaseGpuMemory(pPackModel->m_materialsMemory.release());
			delete	pPackModel;
		}
	}

	void	GraphicsContext::deferredReleasePackModelInstance(PackModelInstance	*pPackModelInstance)
	{
		if (pPackModelInstance != nullptr) {
			deferredReleaseGpuMemory(pPackModelInstance->m_occlusionQueryResults.release());
			delete	pPackModelInstance;
		}
	}
#endif
} } } // namespace sce::SampleUtil::Graphics

#endif //_SCE_TARGET_OS_PROSPERO
