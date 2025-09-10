/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_JOB_SYSTEM_H
#define _PFX_JOB_SYSTEM_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"

#include <job_common.h>
#include <job_manager.h>
#include <thread>
#include <atomic>

namespace sce {
namespace pfxv4 {

typedef void (*JobEntry)(void *data, int uid);

class PfxJobSystem {
protected:
	int m_numWorkerThreads = 1;

public:
	virtual PfxInt32 initialize( int numWorkerThreads, sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface ) = 0;
	virtual PfxInt32 initialize( PfxUInt8 **workBuff, PfxUInt32 workBytes, int numWorkerThreads, int affinityMask, int threadPriority ) = 0;
	virtual void finalize() = 0;

	virtual void clearCounter() = 0;

	int getNumWorkerThreads() { return m_numWorkerThreads; }

	virtual void enablePersistentThreads() = 0;
	virtual void disablePersistentThreads() = 0;

	virtual PfxInt32 dispatch( JobEntry entry, void *arg, const char *name ) = 0;
	virtual PfxInt32 dispatch( JobEntry entry, void *arg, int numJobs, const char *name ) = 0;
	virtual PfxInt32 barrier() = 0;
	virtual PfxInt32 wait() = 0;
};

class PfxSingleJobSystem : public PfxJobSystem {
public:
	PfxInt32 initialize(int numWorkerThreads, sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface) { return 0; }
	PfxInt32 initialize(PfxUInt8 **workBuff, PfxUInt32 workBytes, int numWorkerThreads, int affinityMask, int threadPriority) { return 0; }
	void finalize() {}

	std::atomic<PfxUInt32> nbarrier;
	std::atomic<PfxUInt32> ndispatch;

	void clearCounter() { nbarrier = ndispatch = 0; }

	void enablePersistentThreads() { }
	void disablePersistentThreads() { }

	inline PfxInt32 dispatch(JobEntry entry, void *arg, const char *name);
	inline PfxInt32 dispatch(JobEntry entry, void *arg, int numJobs, const char *name);
	inline PfxInt32 barrier();
	inline PfxInt32 wait();
};

PfxInt32 PfxSingleJobSystem::dispatch(JobEntry entry, void *arg, const char *name)
{
	return dispatch(entry, arg, 1, name);
}

PfxInt32 PfxSingleJobSystem::dispatch(JobEntry entry, void *arg, int numJobs, const char *name)
{
	if (numJobs == 0) return 0;
	ndispatch++;
	for (int i = 0; i < numJobs; i++) {
		entry(arg, i);
	}
	return 0;
}

PfxInt32 PfxSingleJobSystem::barrier()
{
	nbarrier++;
	return 0;
}

PfxInt32 PfxSingleJobSystem::wait()
{
	return 0;
}

class PfxMultiJobSystem : public PfxJobSystem {
private:
	const static int maxThreads = 12;
	const static int jobCount = sce::Job::SequenceFactoryInterface::kMaxInFlightJobCountPerSequence;
	const static int dispatchCount = sce::Job::SequenceFactoryInterface::kMaxInFlightDispatchCountPerSequence;
	const static int barrierCount = sce::Job::SequenceFactoryInterface::kMaxInFlightBarrierCountPerSequence;
	const static int waitCount = sce::Job::SequenceFactoryInterface::kMaxInFlightWaitCountPerSequence;

	sce::Job::SequenceInterface *m_sequence = nullptr;
	sce::Job::JobManager *m_jobManager = nullptr;

	sce::Job::SequenceFactoryInterface *m_userJobManagerSequenceFactoryInterface;

public:
	PfxInt32 initialize(int numWorkerThreads, sce::Job::SequenceFactoryInterface *userJobManagerSequenceFactoryInterface);
	PfxInt32 initialize(PfxUInt8 **workBuff, PfxUInt32 workBytes, int numWorkerThreads, int affinityMask, int threadPriority);
	void finalize();

	std::atomic<PfxUInt32> nbarrier;
	std::atomic<PfxUInt32> ndispatch;

	void clearCounter() {nbarrier = ndispatch = 0;}
	
	void enablePersistentThreads() { if(m_jobManager) m_jobManager->setPersistentThreadCount(m_numWorkerThreads); }
	void disablePersistentThreads() { if(m_jobManager) m_jobManager->setPersistentThreadCount(0); }

	sce::Job::SequenceInterface *getSequence() { return m_sequence; }
	sce::Job::JobManager *getJobManager() {return m_jobManager;}

	inline PfxInt32 dispatch(JobEntry entry, void *arg, const char *name);
	inline PfxInt32 dispatch(JobEntry entry, void *arg, int numJobs, const char *name);
	inline PfxInt32 barrier();
	inline PfxInt32 wait();
};

PfxInt32 PfxMultiJobSystem::dispatch(JobEntry entry, void *arg, const char *name)
{
	ndispatch++;
	PfxInt32 ret = m_sequence->dispatch(entry, arg, 1, name);
	return ret;
}

PfxInt32 PfxMultiJobSystem::dispatch(JobEntry entry, void *arg, int numJobs, const char *name)
{
	if (numJobs == 0) return 0;
	ndispatch++;
	PfxInt32 ret = m_sequence->dispatch(entry, arg, numJobs, name);
	return ret;
}

PfxInt32 PfxMultiJobSystem::barrier()
{
	nbarrier++;
	PfxInt32 ret = m_sequence->barrier();
	return ret;
}

PfxInt32 PfxMultiJobSystem::wait()
{
	PfxInt32 ret = m_sequence->wait();
	return ret;
}

} //namespace pfxv4
} //namespace sce

#endif // _PFX_JOB_SYSTEM_H

