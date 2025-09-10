/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc. 
 * 
 */

#include <cstring> // memset/memcpy
#include <string>
#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS
#include <kernel.h>
#include <sys/dmem.h>
#include <video_out.h>
#include <gnf.h>
#include <gnmx.h>
#include <mat.h>
#include <razorcpu.h>
#include <vectormath/cpp/vectormath_aos.h>
#include <libsysmodule.h>

#include "sampleutil/memory.h"
#include "sampleutil/graphics.h"
#include "sampleutil/graphics/render_target.h"
#include "sampleutil/graphics/texture_load.h"
#include "sampleutil/debug/perf.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/sampleutil_error.h"
#include "sampleutil/graphics/context.h"
#include "../source/imgui/imgui_internal.h"
#include "../source/imgui/imgui.h"

#pragma comment(lib,"libSceGnmDriver_stub_weak.a")
#ifdef _DEBUG
#pragma comment(lib,"libSceGnm_debug.a")
#pragma comment(lib,"libSceGnmx_debug.a")
#pragma comment(lib,"libSceGnf_debug.a")
#else
#pragma comment(lib,"libSceGnm.a")
#pragma comment(lib,"libSceGnmx.a")
#pragma comment(lib,"libSceGnf.a")
#endif

#pragma comment(lib, "SceVideoOut_stub_weak")
#pragma comment(lib, "libScePngDec_stub_weak.a")
#pragma comment(lib, "libSceJpegDec_stub_weak.a")

#ifdef _DEBUG
#define	PRINTF	printf
#else
#define	PRINTF(...)
#endif

namespace
{
	int createScanoutBuffers(const sce::SampleUtil::Graphics::RenderTargetWrapper	*rts, sce::SampleUtil::Graphics::GraphicsContext::DisplayMode	mode, bool	noGamma, SceVideoOutBusType port, uint32_t	count)
	{
		// First we need to select what we want to display on, which in this case is the TV, also known as SCE_VIDEO_OUT_BUS_TYPE_MAIN.
		int videoHandle = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, port, 0, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_MSG(videoHandle >= 0, "sceVideoOut() returns handle=%d\n", videoHandle);
		if (videoHandle < 0)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		uint32_t voutFormat = 0;
		sce::Gnm::DataFormat gnmFormat = rts->m_spec.m_colorFormat;
		if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatB8G8R8A8UnormSrgb, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal) && !noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A8R8G8B8_SRGB;
		} else if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatR8G8B8A8UnormSrgb, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal) && !noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A8B8G8R8_SRGB;
		} else if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatB8G8R8A8Unorm, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal) && noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A8R8G8B8_SRGB;
		} else if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatR8G8B8A8Unorm, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal) && noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A8B8G8R8_SRGB;
		} else if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatB10G10R10A2Unorm, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal) && !noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A2R10G10B10;
		} else if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatB10G10R10A2Unorm, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal) && !noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A2R10G10B10_SRGB;
		} else if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatB10G10R10A2Unorm, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kHdrView) && !noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A2R10G10B10_BT2020_PQ;
		} else if (memcmp(&gnmFormat, &sce::Gnm::kDataFormatB16G16R16A16Float, sizeof(sce::Gnm::DataFormat)) == 0 && (mode == sce::SampleUtil::Graphics::GraphicsContext::DisplayMode::kNormal) && !noGamma)
		{
			voutFormat = SCE_VIDEO_OUT_PIXEL_FORMAT_A16R16G16B16_FLOAT;
		} else {
			SCE_SAMPLE_UTIL_ASSERT_MSG(false, "Invalid data format is specified to scannout buffers");
		}

		// tilemode check
		SceVideoOutTilingMode voutTileMode = SCE_VIDEO_OUT_TILING_MODE_LINEAR;
		sce::Gnm::TileMode gnmTileMode = rts->m_spec.m_colorTileModeHint;
		switch (gnmTileMode)
		{
		case sce::Gnm::kTileModeDisplay_LinearAligned:
			voutTileMode = SCE_VIDEO_OUT_TILING_MODE_LINEAR;
			break;
		case sce::Gnm::kTileModeDisplay_2dThin:
		case sce::Gnm::kTileModeDisplay_ThinPrt:
		case sce::Gnm::kTileModeDisplay_2dThinPrt:
			voutTileMode = SCE_VIDEO_OUT_TILING_MODE_TILE;
			break;
		default:
			SCE_SAMPLE_UTIL_ASSERT_MSG(false, "Invalid tile mode is specified to scannout buffers");
			break;
		}

		// register scanout buffers
		SceVideoOutBufferAttribute voutAttr;
		sceVideoOutSetBufferAttribute(&voutAttr, voutFormat, voutTileMode, SCE_VIDEO_OUT_ASPECT_RATIO_16_9, rts->m_spec.m_width, rts->m_spec.m_height, rts->m_spec.m_pitch);

		static std::vector<void *> addresses;
		static int bufferSetIndex = -1;

		for (uint32_t i = 0; i < count; ++i)
		{
			addresses.push_back(rts[i].m_cxRenderTarget.getBaseAddress());
		}
		bufferSetIndex = sceVideoOutRegisterBuffers(videoHandle, 0, addresses.data(), count, &voutAttr);
		SCE_SAMPLE_UTIL_ASSERT(bufferSetIndex >= 0);

		return videoHandle;
	}

}

namespace sce { namespace SampleUtil { namespace Graphics {
	GraphicsContext::GraphicsContext(uint32_t	width, uint32_t	height, GraphicsContextOption	*option)
		: m_errorCode					(SCE_OK)
		, m_videoMemory					(option ? option->videoHeapSize : 1024 * 1024 * 1024, SCE_KERNEL_WC_GARLIC, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, "sce::SampleUtil::GfxCtx::garlic")
		, m_onionVideoMemory			(option ? option->onionHeapSize : 64 * 1024 * 1024, SCE_KERNEL_WB_ONION, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, "sce::SampleUtil::GfxCtx::onion")
		, m_videoRingMemory				(option ? option->videoRingSize : 16 * 1024 * 1024, SCE_KERNEL_WC_GARLIC, "sce::SampleUtil::GfxCtx::ring")
		, m_pCurrentRenderTargets		(nullptr)
		, m_pCurrentDepthRenderTarget	(nullptr)
		, m_flipCountMain				(0)
		, m_width						(width)
		, m_height						(height)
		, m_texLib						(m_videoMemory)
	{
		int ret = SCE_OK; (void)ret;

		// create initial frame deferred free queue
		DeferredGpuMemoryRelease::m_pAlloc = &m_videoMemory;
		m_deferredGpuMemoryRelease.emplace_back();
		m_deferredGpuMemoryRelease.back().m_pReleaseLabel = reinterpret_cast<uint64_t *>(DeferredGpuMemoryRelease::m_pAlloc->allocate(sizeof(uint64_t), 8));
		*m_deferredGpuMemoryRelease.back().m_pReleaseLabel = 0ull;
		m_deferredGpuMemoryRelease.back().m_isClosed = false;

		// Load modules for "createTextureFromFile()"
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_PNG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceSysmoduleLoadModule(SCE_SYSMODULE_JPEG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		SampleUtil::Graphics::SurfaceUtil::initialize(m_videoMemory);

		if (option != nullptr)
		{
			m_option = *option;
		}

		if (m_option.enableHdrViewMain)
		{
			m_displayModeMain = DisplayMode::kHdrView; // todo: param.json check
		} else {
			m_displayModeMain = DisplayMode::kNormal;
		}

		{
			const size_t align = 128 * 1024 * 1024;
			ret = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE,  m_option.cpuHeapSize, align, SCE_KERNEL_WB_ONION, &m_cpuMemoryPhyAddr);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			m_cpuMemoryMappedAddr = nullptr;
			ret = sceKernelMapNamedDirectMemory(&m_cpuMemoryMappedAddr, m_option.cpuHeapSize, SCE_KERNEL_PROT_CPU_RW, 0, m_cpuMemoryPhyAddr, align, "SampleUtil::CPU_heap");
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			m_cpuMemory = sceLibcMspaceCreate("cpu memory", m_cpuMemoryMappedAddr,  m_option.cpuHeapSize, 0);
			SCE_SAMPLE_UTIL_ASSERT(m_cpuMemory != nullptr);
#ifdef _DEBUG
			if (Memory::g_isMatInitialized)
			{
				sceMatAllocPhysicalMemory(m_cpuMemoryPhyAddr, m_option.cpuHeapSize, align, SCE_KERNEL_WB_ONION);
				sceMatMapDirectMemory(m_cpuMemoryMappedAddr, m_option.cpuHeapSize, SCE_KERNEL_PROT_CPU_RW, 0, m_cpuMemoryPhyAddr, align, SCE_KERNEL_WB_ONION);
				sceMatMspaceCreate(m_cpuMemory, "cpu memory", m_cpuMemoryMappedAddr, m_option.cpuHeapSize, 0, SCEMAT_GROUP_AUTO);
				Memory::registerMatGroup(m_cpuMemory, "sce::SampleUtil::Graphics::GraphicsContext::m_cpuMemory");
			}
#endif
		}
		{
			// Set up the RenderTarget spec.
			m_pFrameBufferRenderTargetsMain = new RenderTargetWrapper[m_option.numFrameBufferMain];
			for (int i = 0; i < m_option.numFrameBufferMain; i++)
			{
				Gnm::DataFormat format = Gnm::kDataFormatR8G8B8A8Unorm;
				switch (m_displayModeMain)
				{
				case DisplayMode::kNormal:
					if (!m_option.noGammaMain)
					{
						format = Gnm::kDataFormatR8G8B8A8UnormSrgb;
					} else {
						format = Gnm::kDataFormatR8G8B8A8Unorm;
					}
					break;
				case DisplayMode::kHdrView:
					format = Gnm::kDataFormatR10G10B10A2Unorm;
					break;
				default:
					SCE_SAMPLE_UTIL_ASSERT(false);
					break;
				}
				RenderTargetParam rtParam("sce::SampleUtil::Graphics::GraphicsContext::ScanOut#" + std::to_string(i), { { m_width, m_height, 1, 1 } }, format);
				m_pFrameBufferRenderTargetsMain[i].init(rtParam, m_videoMemory);
				m_busTypeTable.insert({m_pFrameBufferRenderTargetsMain[i].m_cxRenderTarget.getBaseAddress(), SCE_VIDEO_OUT_BUS_TYPE_MAIN});
			}

			// We need the videoout handle to flip.
			m_videoHandleMain = createScanoutBuffers(m_pFrameBufferRenderTargetsMain, m_displayModeMain, m_option.noGammaMain, SCE_VIDEO_OUT_BUS_TYPE_MAIN, m_option.numFrameBufferMain);

			// Create equeue
			ret = sceKernelCreateEqueue(&m_flipEventQueueMain, "flip event queue main");
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			// Add flip event
			ret = sceVideoOutAddFlipEvent(m_flipEventQueueMain, m_videoHandleMain, nullptr);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

			// Setup DepthRenderTarget
			{
				Gnm::DepthRenderTargetSpec spec;
				spec.init();
				spec.m_width	= m_width;
				spec.m_height	= m_height;
				DepthRenderTargetParam param("sce::SampleUtil::Graphics::GraphicsContext::depthMain", spec, 0, 1.f, 0);
				m_frameBufferDepthRenderTargetMain.init(param, m_videoMemory);
			}
		}
		{
#if SCE_GNMX_ENABLE_GFX_LCUE
			m_lcueResBuf						= new Memory::Gpu::unique_ptr<uint8_t[]>[m_option.numContexts];
#else
			m_cueHeap							= new Memory::Gpu::unique_ptr<uint8_t[]>[m_option.numContexts];
			m_ccbData							= new Memory::Gpu::unique_ptr<uint8_t[]>[m_option.numContexts];
#endif
			m_dcbData							= new Memory::Gpu::unique_ptr<uint8_t[]>[m_option.numContexts];
			m_globalResourceTable				= new Memory::Gpu::unique_ptr<uint8_t[]>[m_option.numContexts];
			m_ctxs								= new Gnmx::GnmxGfxContext[m_option.numContexts];
			m_tessellationFactorRingBufferMem	= Memory::Gpu::make_unique<uint8_t>(Gnm::kTfRingSizeInBytes, 64 * 1024, { Gnm::kResourceTypeBufferBaseAddress }, m_videoMemory, "sce::SampleUtil::Graphics::GraphicsContext::tessellationFactorRingBuffer");
			m_tessellationFactorRingBuffer.initAsTessellationFactorBuffer(m_tessellationFactorRingBufferMem.get(), Gnm::kTfRingSizeInBytes);
			m_flipLabelsMain					= Memory::Gpu::make_unique<uint64_t>(m_option.numContexts, 8, { Gnm::kResourceTypeLabel }, m_videoMemory, "sce::SampleUtil::Graphics::GraphicsContext::flipLabelsMain");
			for (int i = 0; i < m_option.numContexts; i++)
			{
				const uint32_t dcbSizeInBytes = 16 * 1024 * 1024;
				m_dcbData[i] = Memory::Gpu::make_unique<uint8_t>(dcbSizeInBytes, Gnm::kAlignmentOfBufferInBytes, { Gnm::kResourceTypeDrawCommandBufferBaseAddress }, m_onionVideoMemory, "sce::SampleUtil::Graphics::DCB");
#if SCE_GNMX_ENABLE_GFX_LCUE
				const uint32_t numBatches = 2 * 1024;
				const uint32_t lcueResourceBufferSizeInBytes = Gnmx::LightweightConstantUpdateEngine::computeResourceBufferSize(Gnmx::LightweightConstantUpdateEngine::kResourceBufferGraphics, numBatches, true);
				m_lcueResBuf[i] = Memory::Gpu::make_unique<uint8_t>(lcueResourceBufferSizeInBytes, Gnm::kAlignmentOfBufferInBytes, { Gnm::kResourceTypeGenericBuffer }, m_onionVideoMemory, "sce::SampleUtil::Graphics::LCUE Resource Buffer");
				m_ctxs[i].init(m_dcbData[i].get(), dcbSizeInBytes, m_lcueResBuf[i].get(), lcueResourceBufferSizeInBytes, nullptr /* Global Resource Table Pointer*/);
#else
				const uint32_t ccbSizeInBytes = 4 * 1024 * 1024;
				const uint32_t numRingEntries = 8;
				m_ccbData[i] = Memory::Gpu::make_unique<uint8_t>(ccbSizeInBytes, Gnm::kAlignmentOfBufferInBytes, { Gnm::kResourceTypeConstantCommandBufferBaseAddress }, m_onionVideoMemory, "sce::SampleUtil::Graphics::CCB");
				m_cueHeap[i] = Memory::Gpu::make_unique<uint8_t>(Gnmx::ConstantUpdateEngine::computeHeapSize(numRingEntries), 16, { Gnm::kResourceTypeGenericBuffer }, m_videoMemory, "sce::SampleUtil::Graphics::CUE heap");
				m_ctxs[i].init(m_cueHeap[i].get(), numRingEntries, m_dcbData[i].get(), dcbSizeInBytes, m_ccbData[i].get(), ccbSizeInBytes);
#endif
				m_globalResourceTable[i] = Memory::Gpu::make_unique<uint8_t>(SCE_GNM_SHADER_GLOBAL_TABLE_SIZE, 16, m_videoMemory, "sce::SampleUtil::Graphics::GLOBAL_RESOURCE_TABLE");
				m_ctxs[i].setGlobalResourceTableAddr(m_globalResourceTable[i].get());
				m_ctxs[i].setGlobalDescriptor(Gnm::kShaderGlobalResourceTessFactorBuffer, &m_tessellationFactorRingBuffer);
				m_ctxs[i].setTessellationFactorBuffer(m_tessellationFactorRingBufferMem.get());
				m_flipLabelsMain[i] = 1;
			}
		}
		ret = sceKernelCreateEqueue(&m_eopEqueue, "EopEventQueue");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = Gnm::addEqEvent(m_eopEqueue, Gnm::kEqEventGfxEop, nullptr);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = Gnm::createWorkloadStream(&m_workload, "sce::SampleUtil::Graphics::GraphicsContext");
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		
		ret = Gnm::beginWorkload(&m_workloadId, m_workload);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		m_currentCtx = &m_ctxs[0];
		m_currentCtx->reset();
		m_currentCtx->m_dcb.initializeDefaultHardwareState();

		m_videoRingMemory.beginFrame(m_currentCtx->m_dcb);
	}

	GraphicsContext::~GraphicsContext()
	{
		int ret = SCE_OK; (void)ret;

		for (uint32_t i = 0; i < m_option.numContexts; ++i)
		{
			while (m_flipLabelsMain[i] != 1)
			{
				SceKernelEvent ev;
				int num;
				ret = sceKernelWaitEqueue(m_eopEqueue, &ev, 1, &num, nullptr);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			}
		}
		ret = Gnm::deleteEqEvent(m_eopEqueue, Gnm::kEqEventGfxEop);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = Gnm::endWorkload(m_workloadId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = Gnm::destroyWorkloadStream(m_workload);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

#if SCE_GNMX_ENABLE_GFX_LCUE
		delete[] m_lcueResBuf;
#else
		delete[] m_cueHeap;
		delete[] m_ccbData;
#endif
		delete[] m_dcbData;
		delete[] m_globalResourceTable;
		delete[] m_ctxs;

		// Delete flip event
		ret = sceVideoOutDeleteFlipEvent(m_flipEventQueueMain, m_videoHandleMain);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		// Delete equeue
		ret = sceKernelDeleteEqueue(m_flipEventQueueMain);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		ret = sceVideoOutClose(m_videoHandleMain);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		for (int i = 0; i < m_option.numFrameBufferMain; i++)
		{
			m_pFrameBufferRenderTargetsMain[i].fini();
		}
		delete[] m_pFrameBufferRenderTargetsMain;
		m_frameBufferDepthRenderTargetMain.fini();

		m_tessellationFactorRingBuffer.setBaseAddress(nullptr);
		m_tessellationFactorRingBufferMem.reset(nullptr);
		m_flipLabelsMain.reset(nullptr);

#ifdef _DEBUG
		if (Memory::g_isMatInitialized)
		{
			sceMatMspaceDestroy(m_cpuMemory);
			sceMatUnmapMemory(m_cpuMemoryMappedAddr, m_option.cpuHeapSize);
			sceMatReleasePhysicalMemory(m_cpuMemoryPhyAddr, m_option.cpuHeapSize);
		}
#endif
		ret = sceLibcMspaceDestroy(m_cpuMemory);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelMunmap(m_cpuMemoryMappedAddr, m_option.cpuHeapSize);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceKernelReleaseDirectMemory(m_cpuMemoryPhyAddr, m_option.cpuHeapSize);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		SampleUtil::Graphics::SurfaceUtil::finalize();

		// Unload modules
		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_JPEG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_PNG_DEC);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}

	int	GraphicsContext::setRenderTarget()
	{
		for (int rt_i = 0; rt_i < m_numRenderTargets; rt_i++)
		{
			m_currentCtx->setRenderTarget(rt_i, &m_pCurrentRenderTargets[rt_i]);
		}
		m_currentCtx->setRenderTargetMask((1 << (4 * m_numRenderTargets)) - 1);
		m_currentCtx->setDepthRenderTarget(m_pCurrentDepthRenderTarget);

		uint32_t width = 0, height = 0;
		for (int rt_i = 0; rt_i < m_numRenderTargets; rt_i++)
		{
			if (m_pCurrentRenderTargets[rt_i].getBaseAddress() != nullptr) {
				width = m_pCurrentRenderTargets[rt_i].getWidth();
				height = m_pCurrentRenderTargets[rt_i].getHeight();
				break;
			}
		}
		if ((width == 0 || height == 0) && m_pCurrentDepthRenderTarget != nullptr) {
			width = m_pCurrentDepthRenderTarget->getWidth();
			height = m_pCurrentDepthRenderTarget->getHeight();
		}
		m_currentCtx->setupScreenViewport(0, 0, width, height, 0.5f, 0.5f);

		return	SCE_OK;
	}

	void	GraphicsContext::clearRenderTarget(uint32_t	color)
	{
		Debug::ScopedPerfOf<Gnmx::GnmxGfxContext> perf(m_currentCtx, __FUNCTION__);

		Vectormath::Simd::Aos::Vector4 vColor((float)(color & 0xff) / 255.f, (float)((color >> 8) & 0xff) / 255.f, (float)((color >> 16) & 0xff) / 255.f, (float)((color >> 24) & 0xff) / 255.f);

		// Clear RenderTarget
		for (int rt_i = 0; rt_i < m_numRenderTargets; rt_i++)
		{
			bool isCleared = false;

			if (m_pCurrentRenderTargets[rt_i].getCmaskFastClearEnable())
			{
				// CMASK
				SurfaceUtil::clearCmaskSurface(*m_currentCtx, m_pCurrentRenderTargets[rt_i]);
				isCleared = true;
			} else if (m_pCurrentRenderTargets[rt_i].getDccCompressionEnable())
			{
				// DCC
				bool isUncompressed = SurfaceUtil::clearDccSurface(*m_currentCtx, m_pCurrentRenderTargets[rt_i], vColor);
				isCleared = !isUncompressed;
			}
			if (!isCleared)
			{
				SurfaceUtil::clearRenderTarget(*m_currentCtx, m_pCurrentRenderTargets[rt_i], vColor);
			}
		}

		// Clear Depth
		if (m_pCurrentDepthRenderTarget != nullptr)
		{
			if (m_pCurrentDepthRenderTarget->getHtileAccelerationEnable())
			{
				Gnm::Htile htile;
				htile.m_asInt = 0;
				SurfaceUtil::clearHtileSurface(*m_currentCtx, *m_pCurrentDepthRenderTarget, htile);
				m_currentCtx->setDepthClearValue(1.f);
			} else {
				SurfaceUtil::clearDepthTarget(*m_currentCtx, *m_pCurrentDepthRenderTarget, 1.f);
			}
		}

		// revert render target/depth render target settings to ones set by beginScene()
		setRenderTarget();
	}

	bool	GraphicsContext::isDisplaymodeSupported(DisplayMode	mode) const
	{
		return true; // fixme
	}

	int	GraphicsContext::setDisplaymode(DisplayMode	mode, int	bus)
	{
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			m_displayModeMain = mode; // fixme
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

	GraphicsContext::DisplayMode	GraphicsContext::getDisplaymode(int	bus) const
	{
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			return m_displayModeMain;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return DisplayMode::kNormal;
		}
	}

	RenderTargetWrapper	*GraphicsContext::getNextRenderTargetWrapper(int	bus, HmdLayer layer, HmdEye eye)
	{
		(void)layer; // fixme: implement it later
		(void)eye;  // fixme: implement it later
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			return &m_pFrameBufferRenderTargetsMain[m_flipCountMain % m_option.numFrameBufferMain];
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return nullptr;
		}
	}

	Gnm::RenderTarget	*GraphicsContext::getNextRenderTarget(int	bus, HmdLayer layer, HmdEye eye)
	{
		RenderTargetWrapper *pRenderTargetWrapper = getNextRenderTargetWrapper(bus, layer, eye);
		return (pRenderTargetWrapper != nullptr) ? &pRenderTargetWrapper->m_cxRenderTarget : nullptr;
	}

	Gnm::DepthRenderTarget	*GraphicsContext::getDepthStencilSurface(int	bus, HmdLayer layer, HmdEye eye)
	{
		(void)layer; // fixme: implement it later
		(void)eye;  // fixme: implement it later
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			return &m_frameBufferDepthRenderTargetMain.m_cxDepthRenderTarget;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return nullptr;
		}
	}

	Gnm::RenderTarget	*GraphicsContext::getCurrentRenderTarget(int	mrt_i)
	{
		return (mrt_i < m_numRenderTargets) ? &m_pCurrentRenderTargets[mrt_i] : nullptr;
	}

	Gnm::DepthRenderTarget	*GraphicsContext::getCurrentDepthStencilSurface()
	{
		return m_pCurrentDepthRenderTarget;
	}

	int	GraphicsContext::beginScene(Gnm::RenderTarget	*renderTarget, Gnm::DepthRenderTarget	*depthStencilSurface, int	numRenderTargets)
	{
		m_numRenderTargets = renderTarget ? numRenderTargets : 0;
		m_pCurrentRenderTargets = renderTarget;
		m_pCurrentDepthRenderTarget = depthStencilSurface;

		m_currentBus = -1;
		if (m_numRenderTargets > 0)
		{
			auto busSearch = m_busTypeTable.find(m_pCurrentRenderTargets[0].getBaseAddress());
			if (busSearch != m_busTypeTable.end())
			{
				// do waitUntilSafeForRendering if current render target is frame buffer
				m_currentBus = busSearch->second;
			}
		}

		setRenderTarget();

		HdrFormat hdr = HdrFormat::kBT709;

		if (m_currentBus == SCE_VIDEO_OUT_BUS_TYPE_MAIN) {
			m_currentCtx->m_dcb.waitUntilSafeForRendering(m_videoHandleMain, m_flipCountMain % m_option.numFrameBufferMain);
			if (m_option.enableHdrViewMain) {
				hdr = HdrFormat::kBT2020_PQ;
			}
		}

		pushHdr(hdr);

		// Clear flag for that ImGui is used for other than "drawDebugString"
		if (nullptr != ImGui::GetCurrentContext())
		{
			ImGui::GetCurrentContext()->UsedOnlyDebugString = true;
		}

		return SCE_OK;
	}

	int	GraphicsContext::endScene()
	{
		popHdr();

		Gnmx::ResourceBarrier barrier;
		for (int rt_i = 0; rt_i < m_numRenderTargets; rt_i++)
		{
			if (m_pCurrentRenderTargets[rt_i].getCmaskFastClearEnable())
			{
				Gnmx::eliminateFastClear(m_currentCtx, &m_pCurrentRenderTargets[rt_i]);
			}
			barrier.init(&m_pCurrentRenderTargets[rt_i], Gnmx::ResourceBarrier::kUsageRenderTarget, Gnmx::ResourceBarrier::kUsageRoTexture);
			m_currentCtx->writeResourceBarrier(&barrier);
		}
		if (m_pCurrentDepthRenderTarget != nullptr)
		{
			if (m_pCurrentDepthRenderTarget->getHtileAccelerationEnable())
			{
				Gnmx::decompressDepthSurface(m_currentCtx, m_pCurrentDepthRenderTarget);
			}
			barrier.init(m_pCurrentDepthRenderTarget, Gnmx::ResourceBarrier::kUsageDepthSurface, Gnmx::ResourceBarrier::kUsageRoTexture);
			m_currentCtx->writeResourceBarrier(&barrier);
		}

		return SCE_OK;
	}

	int	GraphicsContext::getSubmittedFlipCount(uint64_t	&outFlipCount, int	bus)
	{
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			outFlipCount = m_flipCountMain;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

	int	GraphicsContext::getVideoOutPortHandle(int	&outVideoOutPortHandle, int	bus)
	{
		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			outVideoOutPortHandle = m_videoHandleMain;
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		return SCE_OK;
	}

	int	GraphicsContext::getFlipStatus(SceVideoOutFlipStatus	&outFlipStatus, int	bus)
	{
		int ret = SCE_OK;

		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			ret = sceVideoOutGetFlipStatus(m_videoHandleMain, &outFlipStatus);
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

	int	GraphicsContext::flip(uint32_t	numVSync, int	bus, uint64_t	flipArg)
	{
		int ret = SCE_OK;

		m_videoRingMemory.endFrame(m_currentCtx->m_dcb);
		m_flipLabelsMain[m_flipCountMain % m_option.numContexts] = 0;
		ret = m_currentCtx->submitAndFlipWithEopInterrupt(m_workloadId, m_videoHandleMain, m_flipCountMain % m_option.numFrameBufferMain, m_option.flipModeMain, flipArg,
												Gnm::kEopFlushCbDbCaches, (void *)&m_flipLabelsMain.get()[m_flipCountMain % m_option.numContexts], 1u, Gnm::kCacheActionNone);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		ret = Gnm::endWorkload(m_workloadId);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}

		Gnm::submitDone();

		++m_flipCountMain;
		// Check if the command buffer has been fully processed, if so it's safe for us to overwrite it on the CPU.
		while (m_flipLabelsMain[m_flipCountMain % m_option.numContexts] != 1)
		{
			SceKernelEvent ev;
			int num;
			ret = sceKernelWaitEqueue(m_eopEqueue, &ev, 1, &num, nullptr);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
		}

		m_videoRingMemory.beginFrame(m_currentCtx->m_dcb);
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
		ret = Gnm::beginWorkload(&m_workloadId, m_workload);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_FATAL;
		}
		m_currentCtx = &m_ctxs[m_flipCountMain % m_option.numContexts];
		m_currentCtx->reset();

		return SCE_OK;
	}

	int GraphicsContext::changeOption(const GraphicsContextOption	&option, uint32_t	width, uint32_t	height)
	{
		// todo: implement this
		return SCE_OK;
	}

	int GraphicsContext::waitVBlank(int	bus)
	{
		int ret = SCE_OK;

		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			ret = sceVideoOutWaitVblank(m_videoHandleMain);
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

		if (bus == SCE_VIDEO_OUT_BUS_TYPE_MAIN)
		{
			SceKernelEvent ev;
			int num;
			ret = sceKernelWaitEqueue(m_flipEventQueueMain, &ev, 1, &num, nullptr);
		} else {
			SCE_SAMPLE_UTIL_ASSERT(false);
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		return ret;
	}

	int GraphicsContext::createTextureFromFile(sce::Gnm::Texture &outTexture, const char *filename)
	{
		return Texture::createFromFile(outTexture, filename, m_videoMemory, m_cpuMemory);
	}

	int GraphicsContext::createTextureFromMemory(Compat::Texture &outTexture, const void *pImageData)
	{
        return Texture::createFromMemory(outTexture, pImageData, m_videoMemory, m_cpuMemory);
	}

	int GraphicsContext::destroyTexture(sce::Gnm::Texture &outTexture)
	{
		deferredReleaseGpuMemory(outTexture.getBaseAddress());
		outTexture.setBaseAddress(nullptr);
		return SCE_OK;
	}

	VideoAllocator	*GraphicsContext::DeferredGpuMemoryRelease::m_pAlloc = nullptr;

	void	GraphicsContext::handleDeferredGpuMemoryRelease(bool	isFinalizing)
	{
		if (m_deferredGpuMemoryRelease.size() > 0 && !m_deferredGpuMemoryRelease.back().m_isClosed)
		{
			m_currentCtx->writeAtEndOfPipe(Gnm::kEopCbDbReadsDone, Gnm::kEventWriteDestMemory, m_deferredGpuMemoryRelease.back().m_pReleaseLabel, Gnm::kEventWriteSource64BitsImmediate, 1ul, Gnm::kCacheActionNone, Gnm::kCachePolicyBypass);
			m_deferredGpuMemoryRelease.back().m_isClosed = true;
		}
		auto iter = m_deferredGpuMemoryRelease.begin();
		for (; iter != m_deferredGpuMemoryRelease.end() && (*iter->m_pReleaseLabel == 1ul); iter++) {}
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
			m_deferredGpuMemoryRelease.back().m_pReleaseLabel = reinterpret_cast<uint64_t *>(DeferredGpuMemoryRelease::m_pAlloc->allocate(sizeof(uint64_t), 8));
			*m_deferredGpuMemoryRelease.back().m_pReleaseLabel = 0ull;
			m_deferredGpuMemoryRelease.back().m_isClosed = false;
		} else {
			uint32_t dcbSizeInBytes = (m_currentCtx->m_dcb.m_cmdptr - m_currentCtx->m_dcb.m_beginptr) * 4;
			uint32_t ccbSizeInBytes = (m_currentCtx->m_ccb.m_cmdptr - m_currentCtx->m_ccb.m_beginptr) * 4;
			Gnm::submitCommandBuffers(1, (void **)&m_currentCtx->m_dcb.m_beginptr, &dcbSizeInBytes, (void **)&m_currentCtx->m_ccb.m_beginptr, &ccbSizeInBytes);
		}
	}
} } } // namespace sce::SampleUtil::Graphics

#endif //_SCE_TARGET_OS_ORBIS
