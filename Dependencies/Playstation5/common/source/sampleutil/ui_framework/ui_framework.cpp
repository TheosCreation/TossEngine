/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
* 
*/

#include <kernel.h>
#include <sanitizer/asan_interface.h>

#include <sampleutil/memory/memory_analyzer.h>
#include <sampleutil/debug/perf.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/ui_framework/ui_framework.h>

namespace
{
	SceLibcMspace	s_heapMemoryAllocator			= nullptr;

	SceLibcMspace	s_defaultHeapMemoryAllocator	= nullptr;
	void			*s_defaultHeapStart				= nullptr;
	size_t			s_defaultHeapSize				= 0;
	bool			s_isNewFrameCalled				= false;

	void	*allocMem(size_t	size, void	*userData)
	{
		void *ptr = sceLibcMspaceMalloc((SceLibcMspace)userData, size);
#ifdef _DEBUG
		if (sce::SampleUtil::Memory::g_isMatInitialized)
		{
			sceMatAlloc(ptr, size, 0, SCEMAT_GROUP_AUTO);
			sceMatTagAllocation(ptr, "imgui");
		}
		sce::SampleUtil::Debug::Perf::tagBuffer("imgui", ptr, size, 1);
		ASAN_UNPOISON_MEMORY_REGION(ptr, size);
#endif
		return ptr;
	}

	void	freeMem(void	*ptr, void	*userData)
	{
#ifdef _DEBUG
		if (sce::SampleUtil::Memory::g_isMatInitialized)
		{
			sceMatFree(ptr);
		}
		sce::SampleUtil::Debug::Perf::unTagBuffer(ptr);
		size_t allocSize = sceLibcMspaceMallocUsableSize(ptr);
		ASAN_POISON_MEMORY_REGION(ptr, allocSize);
#endif
		sceLibcMspaceFree((SceLibcMspace)userData, ptr);
	}
} // anonymous namespace

namespace sce {	namespace SampleUtil { namespace UIFramework {
	int imGuiInitialize(const ImGuiOption	*option, SceLibcMspace	memoryAllocator, Graphics::VideoAllocator	*pVideoMemoryAllocator, Graphics::VideoRingAllocator	*pVideoMemoryRingAllocator,  System::UserIdManager	*idManagerForMouse)
	{
		int ret = SCE_OK;

		SCE_SAMPLE_UTIL_ASSERT(s_heapMemoryAllocator == nullptr && s_defaultHeapMemoryAllocator == nullptr && !s_isNewFrameCalled);
		if (!(s_heapMemoryAllocator == nullptr && s_defaultHeapMemoryAllocator == nullptr && !s_isNewFrameCalled))
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}

		s_heapMemoryAllocator = memoryAllocator;

		if (s_heapMemoryAllocator == nullptr)
		{
			off_t offsetOut;

			s_defaultHeapSize = (option && option->heapMemorySize > 0) ? ((option->heapMemorySize + 0xffffu) & ~0xffffu) : ImGuiOption::kDefaultHeapMemorySizeInBytes;

#if _SCE_TARGET_OS_PROSPERO
			ret = sceKernelAllocateMainDirectMemory(s_defaultHeapSize, 0x10000, SCE_KERNEL_MTYPE_C, &offsetOut);
#endif
#if _SCE_TARGET_OS_ORBIS
			ret = sceKernelAllocateMainDirectMemory(s_defaultHeapSize, 0x10000, SCE_KERNEL_WB_ONION, &offsetOut);
#endif
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			}

			ret = sceKernelMapNamedDirectMemory(&s_defaultHeapStart, s_defaultHeapSize, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW, 0, offsetOut, 0x10000, "imgui heap");
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
			}
			s_heapMemoryAllocator = s_defaultHeapMemoryAllocator = sceLibcMspaceCreate("imgui heap", s_defaultHeapStart, s_defaultHeapSize, 0);
		}

		ImGui::SetAllocatorFunctions(allocMem, freeMem, s_heapMemoryAllocator);
		ImGui::CreateContext();

#if _SCE_TARGET_OS_PROSPERO
		ImGui_ImplSampleUtil_Init(pVideoMemoryAllocator, pVideoMemoryRingAllocator, idManagerForMouse, option ? option->disableMouse : false);
#endif
#if _SCE_TARGET_OS_ORBIS
		ImGui_ImplSampleUtil_Init(*pVideoMemoryAllocator, idManagerForMouse, option ? option->disableMouse : false);
#endif

		ImGui::SetEnableDepth(option ? option->enableDepth : false);

		ImGui::StyleColorsDark();

		return SCE_OK;
	}

	int	imGuiFinalize()
	{
		int ret = SCE_OK;

		if (s_isNewFrameCalled)
		{
			ret = imGuiEndFrame();
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return ret;
			}
		}

		ImGui_ImplSampleUtil_Shutdown();
		ImGui::DestroyContext();
		if (s_defaultHeapMemoryAllocator != nullptr)
		{
			ret = sceLibcMspaceDestroy(s_defaultHeapMemoryAllocator);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			}

			SceKernelVirtualQueryInfo info;
			ret = sceKernelVirtualQuery(s_defaultHeapStart, 0, &info, sizeof(info));
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			}

			ret = sceKernelMunmap(s_defaultHeapStart, s_defaultHeapSize);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			}

			ret = sceKernelReleaseDirectMemory(info.offset, s_defaultHeapSize);
			SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
			if (ret != SCE_OK)
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
			}

			s_defaultHeapMemoryAllocator	= nullptr;
			s_defaultHeapStart				= nullptr;
			s_defaultHeapSize				= 0;
		}
		s_heapMemoryAllocator = nullptr;

		return SCE_OK;
	}

	int	imGuiStartNewFrame(const ImGuiUpdateParam	&param)
	{
		SCE_SAMPLE_UTIL_ASSERT(!s_isNewFrameCalled);
		if (s_isNewFrameCalled)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}

		SCE_SAMPLE_UTIL_ASSERT(s_heapMemoryAllocator != nullptr);
		if (s_heapMemoryAllocator == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED;
		}

		ImGuiIO	&io = ImGui::GetIO();
		if (ImGuiLibFont::BuildFontAtlas(io.Fonts, s_heapMemoryAllocator))
		{
			ImGui_ImplSampleUtil_InvalidateDeviceObjects();
			if (!ImGui_ImplSampleUtil_CreateDeviceObjects())
			{
				return SCE_SAMPLE_UTIL_ERROR_FATAL;
			}
		}
#if _SCE_TARGET_OS_PROSPERO
		ImGui_ImplSampleUtil_NewFrame(param.renderTargetWidth, param.renderTargetHeight, param.keyboard, param.pad, param.osk, param.isVrController, param.vrControllerLeft);
#endif
#if _SCE_TARGET_OS_ORBIS
		ImGui_ImplSampleUtil_NewFrame(param.renderTargetWidth, param.renderTargetHeight, param.keyboard, param.pad, param.osk);
#endif

		ImGui::NewFrame();
		ImGui::PushFont(ImGui_ImplSampleUtil_defaultFont);

		s_isNewFrameCalled = true;

		return SCE_OK;
	}

	int	imGuiEndFrame()
	{
		SCE_SAMPLE_UTIL_ASSERT(s_isNewFrameCalled);
		if (!s_isNewFrameCalled)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		ImGui::PopFont();
		ImGui::Render();

		s_isNewFrameCalled = false;

		return SCE_OK;
	}

#if _SCE_TARGET_OS_PROSPERO
	int	imGuiRender(sce::Agc::DrawCommandBuffer	&dcb)
#endif
#if _SCE_TARGET_OS_ORBIS
	int	imGuiRender(sce::Gnmx::GfxContext	&gfxc)
#endif
	{
		SCE_SAMPLE_UTIL_ASSERT(!s_isNewFrameCalled);
		if (s_isNewFrameCalled)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}
		SCE_SAMPLE_UTIL_ASSERT(s_heapMemoryAllocator != nullptr);
		if (s_heapMemoryAllocator == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED;
		}
#if _SCE_TARGET_OS_PROSPERO
		ImGui_ImplSampleUtil_RenderDrawData(&dcb, ImGui::GetDrawData());
#endif
#if _SCE_TARGET_OS_ORBIS
		ImGui_ImplSampleUtil_RenderDrawData(&gfxc, ImGui::GetDrawData());
#endif

		return SCE_OK;
	}

} } } // namespace sce::SampleUtil::UIFramework