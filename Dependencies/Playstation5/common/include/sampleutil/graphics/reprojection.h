/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once
#include <scebase_common.h>
#include <sampleutil/graphics/graphics_memory.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Graphics
		{
			struct ReprojectionOption
			{
				int threadPriority;
				int threadCpuAffinityMask;
				int computeQueuePipeId;
				int computeQueueQueueId;
#if _SCE_TARGET_OS_PROSPERO
				bool enablePositionalReprojection;
				bool enableCameraSeeThrough;
#endif
				ReprojectionOption()
					: threadPriority				(SCE_KERNEL_PRIO_FIFO_HIGHEST)
#if _SCE_TARGET_OS_PROSPERO
					, threadCpuAffinityMask			(SCE_KERNEL_CPUMASK_13CPU)
#endif
#if _SCE_TARGET_OS_ORBIS
					, threadCpuAffinityMask			(SCE_KERNEL_CPUMASK_6CPU_ALL)
#endif
					, computeQueuePipeId			(5)
					, computeQueueQueueId			(0)
#if _SCE_TARGET_OS_PROSPERO
					, enablePositionalReprojection	(true)
					, enableCameraSeeThrough		(false)
#endif
				{}
			};

			class Reprojection
			{
			public:
				Reprojection() {}
				Reprojection(ReprojectionOption	*option, VideoAllocator	*allocator = nullptr);
				virtual ~Reprojection();

				VideoAllocator	*m_pAllocator;
#if _SCE_TARGET_OS_PROSPERO
				Agc::SizeAlign	m_bufferSizeAlign;
				void			*m_bufferMemory;
				Agc::SizeAlign	m_displayBufferSizeAlign;
				void			*m_displayBufferMemory;
				Agc::SizeAlign	m_seeThroughBufferSizeAlign;
				void			*m_seeThroughBufferMemory;
#endif
			};
		}
	}
}