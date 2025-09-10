/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2023 Sony Interactive Entertainment Inc. 
* 
*/

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <libsysmodule.h>
#include <hmd2/reprojection.h>
#include <sampleutil/memory.h>
#include <sampleutil/graphics/reprojection.h>
#include <sampleutil/sampleutil_common.h>

#pragma comment(lib, "libSceHmd2_stub_weak.a") // todo: add nosubmission.a when its' get available

sce::SampleUtil::Graphics::Reprojection::Reprojection(ReprojectionOption *option, Graphics::VideoAllocator	*allocator)
{
	int ret = SCE_OK; (void)ret;

	m_pAllocator = allocator;

	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_HMD2);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	SceHmd2InitializeParam hmdInitParam;
	memset(&hmdInitParam, 0, sizeof(hmdInitParam));
	ret = sceHmd2Initialize(&hmdInitParam);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	ret = sceHmd2ReprojectionSetAllowPositionalReprojection((option && !option->enablePositionalReprojection) ? 0 : 1);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	m_bufferSizeAlign = sceHmd2ReprojectionQueryBufferSizeAlign();
	m_displayBufferSizeAlign = sceHmd2ReprojectionQueryDisplayBufferSizeAlign();
	if (option && option->enableCameraSeeThrough) {
		m_seeThroughBufferSizeAlign = sceHmd2ReprojectionQuerySeeThroughBufferSizeAlign();
		SCE_SAMPLE_UTIL_ASSERT(m_seeThroughBufferSizeAlign.m_size > 0);
	}
	if (m_pAllocator != nullptr)
	{
		m_bufferMemory = m_pAllocator->allocate(m_bufferSizeAlign, "sce::SampleUtil::Graphics::Reprojection");
		m_displayBufferMemory = m_pAllocator->allocate(m_displayBufferSizeAlign, "sce::SampleUtil::Graphics::ReprojectionDisplay");
		if (option && option->enableCameraSeeThrough) {
			m_seeThroughBufferMemory = m_pAllocator->allocate(m_seeThroughBufferSizeAlign, "sce::SampleUtil::Graphics::SeeThroughBuffer");
		} else {
			m_seeThroughBufferMemory = nullptr;
		}
	} else {
		m_bufferMemory = sce::SampleUtil::Memory::allocDmem(m_bufferSizeAlign.m_size, m_bufferSizeAlign.m_align, SCE_KERNEL_MTYPE_C, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, "SampleUtil::Reprojection");
		m_displayBufferMemory = sce::SampleUtil::Memory::allocDmem(m_displayBufferSizeAlign.m_size, m_displayBufferSizeAlign.m_align, SCE_KERNEL_MTYPE_C, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, "SampleUtil::ReprojectionDisplay");
		if (option && option->enableCameraSeeThrough) {
			m_seeThroughBufferMemory = sce::SampleUtil::Memory::allocDmem(m_seeThroughBufferSizeAlign.m_size, m_seeThroughBufferSizeAlign.m_align, SCE_KERNEL_MTYPE_C, SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, "SampleUtil::SeeThroughBuffer");
		} else {
			m_seeThroughBufferMemory = nullptr;
		}
	}

	SceHmd2ReprojectionInitializeParam reprojectionInitParam = {};
	ReprojectionOption defaultOption;

	reprojectionInitParam.pReprojectionBuff	= m_bufferMemory;
	reprojectionInitParam.pDisplayBuff		= m_displayBufferMemory;
	reprojectionInitParam.pSeeThroughBuff	= m_seeThroughBufferMemory;
	reprojectionInitParam.threadPriority	= option ? option->threadPriority : defaultOption.threadPriority;
	reprojectionInitParam.cpuAffinityMask	= option ? option->threadCpuAffinityMask : defaultOption.threadCpuAffinityMask;
	reprojectionInitParam.pipeId			= option ? option->computeQueuePipeId : defaultOption.computeQueuePipeId;
	reprojectionInitParam.queueId			= option ? option->computeQueueQueueId : defaultOption.computeQueueQueueId;
	reprojectionInitParam.type				= SCE_HMD2_REPROJECTION_TYPE_BORDER_FOR_SINGLE;
	ret = sceHmd2ReprojectionInitialize(&reprojectionInitParam, nullptr);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

sce::SampleUtil::Graphics::Reprojection::~Reprojection()
{
	int ret = SCE_OK; (void)ret;

	ret = sceHmd2ReprojectionTerminate();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_HMD2);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	if (m_pAllocator != nullptr)
	{
		m_pAllocator->free(m_bufferMemory);
		m_pAllocator->free(m_displayBufferMemory);
		if (m_seeThroughBufferMemory != nullptr)
		{
			m_pAllocator->free(m_seeThroughBufferMemory);
		}
	} else {
		sce::SampleUtil::Memory::freeDmem(m_bufferMemory);
		sce::SampleUtil::Memory::freeDmem(m_displayBufferMemory);
		if (m_seeThroughBufferMemory != nullptr)
		{
			sce::SampleUtil::Memory::freeDmem(m_seeThroughBufferMemory);
		}
	}
}

#endif // _SCE_TARGET_OS_PROSPERO

