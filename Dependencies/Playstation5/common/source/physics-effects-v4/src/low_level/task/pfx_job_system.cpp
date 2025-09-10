/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_job_system.h"

#if defined(__ORBIS__) || defined(__PROSPERO__)
#include <libsysmodule.h>
#endif

namespace sce {
namespace pfxv4 {

PfxInt32 PfxMultiJobSystem::initialize(int numWorkerThreads, sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface)
{
	SCE_PFX_ASSERT(userJobManagerSequenceFactoryInterface);

	m_numWorkerThreads = numWorkerThreads;

	m_userJobManagerSequenceFactoryInterface = userJobManagerSequenceFactoryInterface;

	int err = userJobManagerSequenceFactoryInterface->createSequenceInterface(
		&m_sequence, jobCount, dispatchCount, barrierCount, waitCount, "PFXv4");

	SCE_PFX_ASSERT_MSG(err == SCE_OK, "createSequenceInterface() failed.");

	return err;
}

PfxInt32 PfxMultiJobSystem::initialize(PfxUInt8 **workBuff, PfxUInt32 workBytes, int numWorkerThreads, int affinityMask, int threadPriority)
{
	SCE_PFX_ASSERT(numWorkerThreads <= maxThreads);
	SCE_PFX_ASSERT(workBuff);

	m_numWorkerThreads = numWorkerThreads;
	m_userJobManagerSequenceFactoryInterface = NULL;

#if defined(__ORBIS__) || defined(__PROSPERO__)
	//E Load the SCE Job Manager
	int ret = sceSysmoduleLoadModule(SCE_SYSMODULE_JOB_MANAGER);
	SCE_PFX_ALWAYS_ASSERT_MSG(ret == SCE_OK, "Failed to load libSceJobManager.prx. Physics Effects needs it for parallel computation.");
#endif

	PfxUInt8 *p = *workBuff;
	uintptr_t e = (uintptr_t)p + workBytes;
	p = (PfxUInt8*)SCE_PFX_PTR_ALIGN16(p);

	size_t jobManagerBytes = SCE_PFX_BYTES_ALIGN16(sizeof(sce::Job::JobManager));

	if (e < (uintptr_t)p + jobManagerBytes) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	void *jobManagerBuff = p;
	p += jobManagerBytes;

	m_jobManager = new(jobManagerBuff) sce::Job::JobManager;

	// Get sequence counts used by Physics Effects
	sce::Job::JobManager::MemorySizeQueryParams queryParams;
	memset(&queryParams, 0, sizeof(queryParams));
	queryParams.flags = 0;
	queryParams.workerThreadCount = m_numWorkerThreads;
	queryParams.sequenceCount = 1;
	queryParams.jobCount = jobCount;	// The maximum number of in-flight jobs
	queryParams.dispatchCount = dispatchCount;	// The maximum number of in-flight dispatches.
	queryParams.barrierCount = barrierCount;	// The maximum number of in-flight barriers.
	queryParams.waitCount = waitCount;	// The maximum number of in-flight waits.

	// Calculate memory size for SCE Job Manager
	size_t jobRuntimeMemorySize = sce::Job::JobManager::calculateRequiredMemorySize(&queryParams);
	jobRuntimeMemorySize = SCE_PFX_BYTES_ALIGN_X(jobRuntimeMemorySize, sce::Job::JobManager::kRuntimeMemoryAlignment);

	// Allocate memory for SCE Job Manager
	p = (PfxUInt8*)SCE_PFX_PTR_ALIGN_X(p, sce::Job::JobManager::kRuntimeMemoryAlignment);

	if (e < (uintptr_t)p + jobRuntimeMemorySize) {
		return SCE_PFX_ERR_OUT_OF_BUFFER;
	}

	void *jobWorkAreaBuff = p;
	p += jobRuntimeMemorySize;
	*workBuff = p;

	// Create and initialize SCE Job Manager
	sce::Job::JobManager::ConfigParams configParams{};
	configParams.init();
	configParams.threadName = "PfxWorker";
	configParams.threadStackSize	= 256 * 1024;
	configParams.threadPriority		= threadPriority;//SCE_JOB_MANAGER_PRIORITY_LOWEST;
	configParams.threadSuspendTimeout = 50U;

// #if defined(__ORBIS__) || defined(__PROSPERO__)
	configParams.threadAffinityMask = affinityMask | sce::Job::JobManager::CpuAffinityMaskType::kShared;
// #elif defined(_WIN32)
// 	configParams.threadAffinityMask = ((1U << m_numWorkerThreads) - 1) | sce::Job::JobManager::CpuAffinityMaskType::kIsolated;
// #endif

	int err = m_jobManager->initialize(jobWorkAreaBuff, jobRuntimeMemorySize, queryParams.workerThreadCount, queryParams.sequenceCount,
#if defined(__ORBIS__) || defined(__PROSPERO__)
		0, 0,
#endif
		&configParams);
	
	if (err != SCE_OK) {
		SCE_PFX_PRINTF("Pfx Job manager initialization failed %d.\n", err);
		return err;
	}

	// Create the sequence for external jobs
	err = m_jobManager->getSequenceFactoryInterface(sce::Job::JobManager::Priority::kLow)->createSequenceInterface(&m_sequence, 
		jobCount, dispatchCount, barrierCount, waitCount, "PFXv4");
	SCE_PFX_ASSERT_MSG(err == SCE_OK, "Create the sequence failed.");

	return err;
}

void PfxMultiJobSystem::finalize()
{
	// Procedure of finalizing SCE Job Manager
	 if(m_userJobManagerSequenceFactoryInterface) {
		m_userJobManagerSequenceFactoryInterface->deleteSequenceInterface(m_sequence);
	 }
	 else {
		m_jobManager->getSequenceFactoryInterface(sce::Job::JobManager::Priority::kHigh)->deleteSequenceInterface(m_sequence);
		m_jobManager->shutdown();
		m_jobManager->~JobManager();
#if defined(__ORBIS__) || defined(__PROSPERO__)
		sceSysmoduleUnloadModule(SCE_SYSMODULE_JOB_MANAGER);
#endif
	 }
}

} //namespace pfxv4
} //namespace sce
