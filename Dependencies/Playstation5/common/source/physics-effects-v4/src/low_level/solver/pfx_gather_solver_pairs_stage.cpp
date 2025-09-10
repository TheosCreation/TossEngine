/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_gather_solver_pairs_stage.h"
#include "../task/pfx_job_system.h"
#include "../rigidbody/pfx_context.h"
#include "../../../include/physics_effects/base_level/solver/pfx_constraint_pair.h"
//#include "../../../src/base_level/broadphase/pfx_check_collidable.h"
#include "../../../src/base_level/solver/pfx_check_solver.h"

namespace sce {
namespace pfxv4 {

void PfxGatherSolverPairsStage::job_gatherSolverPairs(void *data, int uid)
{
	GatherJobArg *arg = (GatherJobArg*)data;

	auto copyPair = [&](const PfxPairContainer::PairNode &pn) {
		PfxBroadphasePair pout;
		pfxSetObjectIdA(pout, pn.rigidbodyIdA);
		pfxSetObjectIdB(pout, pn.rigidbodyIdB);
		pfxSetMotionMaskA(pout, pn.motionMaskA);
		pfxSetMotionMaskB(pout, pn.motionMaskB);
		pfxSetActive(pout, true);
		pfxSetContactId(pout, pn.data);
		pfxSetSolverQuality(pout, pn.solverQuality);
		pfxSetNumConstraints(pout, pn.numConstraints);
		return pout;
	};

	const int maxWork = 10;
	PfxBroadphasePair workPairs[maxWork];
	int workNum = 0;

	auto filterPair = [](const PfxPairContainer::PairNode &pair)
	{
		PfxUInt32 motionTypeA = (pair.motionMaskA)&SCE_PFX_MOTION_MASK_TYPE;
		PfxUInt32 motionTypeB = (pair.motionMaskB)&SCE_PFX_MOTION_MASK_TYPE;
		//PfxUInt32 sleepA = (pair.motionMaskA)&SCE_PFX_MOTION_MASK_SLEEPING;
		//PfxUInt32 sleepB = (pair.motionMaskB)&SCE_PFX_MOTION_MASK_SLEEPING;

		return
			pair.numConstraints > 0 &&
			pfxCheckSolverTable((ePfxMotionType)motionTypeA, (ePfxMotionType)motionTypeB);// && // モーションタイプ別衝突判定テーブル
			//!((sleepA != 0 && sleepB != 0) || (sleepA != 0 && motionTypeB == kPfxMotionTypeFixed) || (sleepB != 0 && motionTypeA == kPfxMotionTypeFixed)); // スリープ時のチェック
	};

	PfxUInt32 total = arg->pairContainer->getPairLength();
	PfxUInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxUInt32 start = uid * batch;
	PfxUInt32 end = SCE_PFX_MIN(start + batch, total);

	for (PfxUInt32 i = start; i < end; i++) {
		PfxPairContainer::PairNode &pairNode = arg->pairContainer->getPairNode(i);
		if (pairNode.state != NODE_IS_NOT_ASSIGNED && filterPair(pairNode)) {

			workPairs[workNum++] = copyPair(pairNode);
			if (workNum == maxWork) {
				PfxInt32 dstId = arg->stage->numWorkPairs.fetch_add(workNum);
				for (int k = 0; k < workNum; k++) {
					arg->stage->workPairs[dstId + k] = workPairs[k];
				}
				workNum = 0;
			}
		}
	}

	if (workNum > 0) {
		PfxInt32 dstId = arg->stage->numWorkPairs.fetch_add(workNum);
		for (int k = 0; k < workNum; k++) {
			arg->stage->workPairs[dstId + k] = workPairs[k];
		}
	}
}

void PfxGatherSolverPairsStage::job_initSort(void *data, int uid)
{
	InitSortJobArg *arg = (InitSortJobArg*)data;
	arg->stage->sorter.initialize(arg->stage->numWorkPairs, arg->radixSortBuff, arg->radixSortBytes);
	*arg->numOutPairs = arg->stage->numWorkPairs;
}

PfxInt32 PfxGatherSolverPairsStage::dispatchGatherSolverPairs(PfxContactComplex &contactComplex, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs)
{
	// Sort pairs for deterministic results (args : 1)

	numWorkPairs.store(0);

	PfxInt32 numJobs_ = getJobSystem()->getNumWorkerThreads();
	gatherJobArg.stage = this;
	gatherJobArg.numJobs = numJobs_;
	gatherJobArg.pairContainer = &contactComplex.pairContainer;

	PfxInt32 ret = SCE_OK;

	ret = getJobSystem()->dispatch(job_gatherSolverPairs, &gatherJobArg, numJobs_, "gather pairs");
	if(ret != SCE_OK) return ret;

	ret = getJobSystem()->barrier();
	if(ret != SCE_OK) return ret;

	sorter.setNumWorkers(getJobSystem()->getNumWorkerThreads());

	// sorterを初期化するジョブを発行 (args : 1)
	{
		initSortJobArg.stage = this;
		initSortJobArg.radixSortBytes = radixSortBytes;
		initSortJobArg.radixSortBuff = sortWorkBuff;
		initSortJobArg.numOutPairs = numOutPairs;
		
		ret = getJobSystem()->dispatch(job_initSort, &initSortJobArg, "sort pairs");
		if(ret != SCE_OK) return ret;
	}

	ret = getJobSystem()->barrier();
	if(ret != SCE_OK) return ret;

	// dispatch sort jobs
	ret = sorter.sort(getJobSystem(), (RadixData*)workPairs, (RadixData*)outPairs);
	if(ret != SCE_OK) return ret;

	ret = getJobSystem()->barrier();
	if(ret != SCE_OK) return ret;

	// print pairs
	//for (int i = 0; i < getPhysicsBuffer()->getNumCurrentPairs(); i++) {
	//	PfxBroadphasePair *pair = getPhysicsBuffer()->getCurrentPairs() + i;
	//	PfxUInt32 iA = pfxGetObjectIdA(*pair);
	//	PfxUInt32 iB = pfxGetObjectIdB(*pair);
	//	PfxUInt32 motionMaskA = pfxGetMotionMaskA(*pair);
	//	PfxUInt32 motionMaskB = pfxGetMotionMaskA(*pair);
	//	PfxUInt32 solverQuality = pfxGetSolverQuality(*pair);
	//	SCE_PFX_PRINTF("pair %d RB %d %d MM %d %d SQ %d\n", i, iA, iB, motionMaskA, motionMaskB, solverQuality);
	//}

	return SCE_PFX_OK;
}

PfxInt32 PfxGatherSolverPairsStage::preparePipeline(PfxUInt32 maxContacts)
{
	workPairs = getRigidBodyContext()->allocate<PfxBroadphasePair>(maxContacts);
	if(!workPairs) return SCE_PFX_ERR_OUT_OF_BUFFER;
	
	// maxペア数で多めにバッファを確保しておく
	radixSortBytes = sorter.queryBytes(maxContacts, getJobSystem()->getNumWorkerThreads());
	sortWorkBuff = getRigidBodyContext()->allocate<PfxUInt8>(radixSortBytes);
	if(!sortWorkBuff) return SCE_PFX_ERR_OUT_OF_BUFFER;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
