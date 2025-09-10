/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2022 Sony Interactive Entertainment Inc. 
* 
*/

#pragma once

#include <scebase_common.h>
#include <mspace.h>
#include <sampleutil/sampleutil_common.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Input
		{
			struct VrTrackerOption
			{
				int highThreadPriority;
				int highThreadCpuAffinityMask;
				int lowThreadPriority;
				int lowThreadCpuAffinityMask;
				int maxDurationForControllerInertialTracking;
				int coordinateResetType;
				int disableControllerOcclusionHandling;
				int controllerOutOfSightHandlingMode;
				VrTrackerOption():
					highThreadPriority							(SCE_KERNEL_PRIO_FIFO_HIGHEST),
#if _SCE_TARGET_OS_PROSPERO
					highThreadCpuAffinityMask					(SCE_KERNEL_CPUMASK_13CPU),
#endif
#if _SCE_TARGET_OS_ORBIS
					highThreadCpuAffinityMask					(SCE_KERNEL_CPUMASK_6CPU_ALL),
#endif
					lowThreadPriority							(SCE_KERNEL_PRIO_FIFO_DEFAULT),
#if _SCE_TARGET_OS_PROSPERO
					lowThreadCpuAffinityMask					(SCE_KERNEL_CPUMASK_13CPU),
#endif
#if _SCE_TARGET_OS_ORBIS
					lowThreadCpuAffinityMask					(SCE_KERNEL_CPUMASK_6CPU_ALL),
#endif
					maxDurationForControllerInertialTracking	(0),
					coordinateResetType							(0),
					disableControllerOcclusionHandling			(0),
					controllerOutOfSightHandlingMode			(0)
				{}
			};

			class VrTracker
			{
			public:
				VrTracker() {}
				VrTracker(VrTrackerOption *option = nullptr, SceLibcMspace	allocator = nullptr);
				virtual ~VrTracker();

				SceLibcMspace	m_allocator;
				void			*m_pMemory;
			};
		}
	}
}
