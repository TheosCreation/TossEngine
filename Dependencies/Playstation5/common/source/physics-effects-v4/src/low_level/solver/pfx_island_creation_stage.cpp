/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_island_creation_stage.h"
#include "../task/pfx_job_system.h"
#include "../rigidbody/pfx_context.h"
#include "../../../src/low_level/task/pfx_atomic.h"

namespace sce {
namespace pfxv4 {

void PfxIslandCreationStage::job_resetIslands(void *data, int uid)
{
	JobArg *arg = (JobArg*)data;

	PfxUInt32 total = arg->stage->shared.numRigidBodies;
	PfxUInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	for (PfxUInt32 i = start; i < end; i++) {
		arg->stage->shared.roots[i] = i;
		arg->stage->isFixed[i] = (SCE_PFX_MOTION_MASK_DYNAMIC(arg->stage->shared.states[i].getMotionType())&SCE_PFX_MOTION_MASK_TYPE) == 0;
	}
}

void PfxIslandCreationStage::job_updateIslands2(void *data, int uid)
{
	JobArg *arg = (JobArg*)data;

	PfxUInt32 total = *arg->numPairsPtr;
	PfxUInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	auto findRoot = [&](PfxUInt32 rigidbodyId) {
		PfxInt32 next = arg->stage->shared.roots[rigidbodyId];
		while (next != rigidbodyId) {
			PfxInt32 prev = rigidbodyId;
			rigidbodyId = next;
			next = arg->stage->shared.roots[rigidbodyId];

			PfxInt32 exp = rigidbodyId;
			//if (exp != next) arg->stage->roots[prev].compare_exchange_strong(exp, next);
			if (exp != next) exp = pfxAtomicCompareAndSwap(&arg->stage->shared.roots[prev], exp, next);
		}
		return rigidbodyId;
	};

	for (PfxUInt32 i = start; i < end; i++) {
		PfxConstraintPair &pair = *(arg->pairs + i);

		PfxUInt32 rigidbodyIdA = pfxGetObjectIdA(pair);
		PfxUInt32 rigidbodyIdB = pfxGetObjectIdB(pair);

		if(!pfxGetActive(pair) || pfxGetNumConstraints(pair) == 0 ||
			arg->stage->isFixed[rigidbodyIdA] || arg->stage->isFixed[rigidbodyIdB]) continue;

		PfxInt32 rootA = findRoot(rigidbodyIdA);
		PfxInt32 rootB = findRoot(rigidbodyIdB);

		// try to link children to this node
		// bool ret = false;
		// do {
		// 	if (rootA < rootB) {
		// 		ret = arg->stage->roots[rootB].compare_exchange_strong(rootB, rootA);
		// 	}
		// 	else if (rootA > rootB) {
		// 		ret = arg->stage->roots[rootA].compare_exchange_strong(rootA, rootB);
		// 	}
		// } while (!ret && rootA != rootB);
		do {
			if (rootA < rootB) {
				rootB = pfxAtomicCompareAndSwap(&arg->stage->shared.roots[rootB], rootB, rootA);
			}
			else if (rootA > rootB) {
				rootA = pfxAtomicCompareAndSwap(&arg->stage->shared.roots[rootA], rootA, rootB);
			}
		} while (rootA != rootB);
	}
}

void PfxIslandCreationStage::job_compressRoute(void *data, int uid)
{
	JobArg *arg = (JobArg*)data;

	PfxUInt32 total = arg->stage->shared.numRigidBodies;
	PfxUInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	auto findRoot = [&](PfxUInt32 rigidbodyId) {
		PfxInt32 itr = arg->stage->shared.roots[rigidbodyId];
		while (itr != rigidbodyId) {
			rigidbodyId = itr;
			itr = arg->stage->shared.roots[rigidbodyId];
		}
		return rigidbodyId;
	};

	for (PfxUInt32 i = start; i < end; i++) {
		PfxInt32 rootId = findRoot(i);
		arg->stage->shared.roots[i] = rootId;
		arg->stage->shared.states[i].setIslandRootId((PfxUInt16)rootId);
	}
}

PfxInt32 PfxIslandCreationStage::dispatchCreateIslands(PfxConstraintPair *contactPairs, PfxInt32 *numContactPairsPtr, PfxConstraintPair *jointPairs, PfxInt32 *numJointPairsPtr)
{
	int njob = 0;
	PfxInt32 ret = SCE_OK;

	// Reset all nodes (args : 1)
	{
		JobArg &arg = jobArgs[njob++];
		arg.stage = this;
		arg.stage = this;
		arg.numJobs = getJobSystem()->getNumWorkerThreads();
		ret = getJobSystem()->dispatch(job_resetIslands, &arg, arg.numJobs, "reset islands");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	// update islands (args : 1)
	{
		JobArg &arg = jobArgs[njob++];
		arg.pairs = contactPairs;
		arg.numPairsPtr = numContactPairsPtr;
		arg.stage = this;
		arg.numJobs = numJobs;
		ret = getJobSystem()->dispatch(job_updateIslands2, &arg, numJobs, "union contact pairs");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	// update islands (args : 1)
	{
		JobArg &arg = jobArgs[njob++];
		arg.pairs = jointPairs;
		arg.numPairsPtr = numJointPairsPtr;
		arg.stage = this;
		arg.numJobs = numJobs;

		ret = getJobSystem()->dispatch(job_updateIslands2, &arg, numJobs, "union joint pairs");
		if (ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	// Compact paths all nodes to a root (args : 1)
	{
		JobArg &arg = jobArgs[njob++];
		arg.stage = this;
		arg.numJobs = getJobSystem()->getNumWorkerThreads();

		ret = getJobSystem()->dispatch(job_compressRoute, &arg, arg.numJobs, "path compaction");
		if(ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if (ret != SCE_OK) return ret;

	return ret;
}

PfxInt32 PfxIslandCreationStage::preparePipeline(PfxInt32 *roots, PfxRigidState *states, PfxUInt32 numRigidBodies)
{
	if(!roots || !states) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	shared.roots = roots;
	shared.states = states;
	shared.numRigidBodies = numRigidBodies;

	isFixed = getRigidBodyContext()->allocate<PfxUInt8>(numRigidBodies);
	if(!isFixed) return SCE_PFX_ERR_OUT_OF_BUFFER;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
