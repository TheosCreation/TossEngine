/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2022 Sony Interactive Entertainment Inc. 
* 
*/

#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <libsysmodule.h>
#include <vision/vr_tracker2.h>
#include <sampleutil/input/vr_tracker.h>

#pragma comment(lib, "libSceVrTracker2_stub_weak.a")

sce::SampleUtil::Input::VrTracker::VrTracker(VrTrackerOption *option, SceLibcMspace allocator)
{
	int ret = SCE_OK; (void)ret;

	m_allocator = allocator;

	ret = sceSysmoduleLoadModule(SCE_SYSMODULE_VR_TRACKER2);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	SceVrTracker2QueryMemoryParam queryMemParam = {};
	queryMemParam.sizeOfThis	= sizeof(queryMemParam);
	queryMemParam.profile		= SCE_VR_TRACKER2_PROFILE_000;

	SceVrTracker2QueryMemoryResult queryMemResult = {};
	queryMemResult.sizeOfThis = sizeof(queryMemResult);
	ret = sceVrTracker2QueryMemory(&queryMemParam, &queryMemResult);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	if (m_allocator != nullptr)
	{
		m_pMemory = sceLibcMspaceAlignedAlloc(m_allocator, queryMemResult.memoryAlignment, queryMemResult.memorySize);
	} else {
		m_pMemory = memalign(queryMemResult.memoryAlignment, queryMemResult.memorySize);
	}

	SceVrTracker2InitializeParam initParam = {};
	VrTrackerOption defaultOption;

	initParam.sizeOfThis								= sizeof(initParam);
	initParam.profile									= SCE_VR_TRACKER2_PROFILE_000;
	initParam.highThreadPriority						= option ? option->highThreadPriority : defaultOption.highThreadPriority;
	initParam.lowThreadPriority							= option ? option->lowThreadPriority : defaultOption.lowThreadPriority;
	initParam.highThreadAffinity						= option ? option->highThreadCpuAffinityMask : defaultOption.highThreadCpuAffinityMask;
	initParam.lowThreadAffinity							= option ? option->lowThreadCpuAffinityMask : defaultOption.lowThreadCpuAffinityMask;
	initParam.memoryPointer								= m_pMemory;
	initParam.memorySize								= queryMemResult.memorySize;
	initParam.memoryAlignment							= queryMemResult.memoryAlignment;
	initParam.maxDurationForControllerInertialTracking	= option ? option->maxDurationForControllerInertialTracking : 0;
	initParam.coordinateResetType						= option ? option->coordinateResetType : 0;
	initParam.disableControllerOcclusionHandling		= option ? option->disableControllerOcclusionHandling : 0;
	initParam.controllerOutOfSightHandlingMode			= option ? option->controllerOutOfSightHandlingMode : 0;

	ret = sceVrTracker2Initialize(&initParam);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

sce::SampleUtil::Input::VrTracker::~VrTracker()
{
	int ret = SCE_OK; (void)ret;

	ret = sceVrTracker2Finalize();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_VR_TRACKER2);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

	if (m_allocator != nullptr)
	{
		sceLibcMspaceFree(m_allocator, m_pMemory);
	} else {
		free(m_pMemory);
	}
}

#endif // _SCE_TARGET_OS_PROSPERO