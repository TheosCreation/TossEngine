/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_BROADPHASE_STAGE_H
#define _PFX_BROADPHASE_STAGE_H

#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../include/physics_effects/base_level/collision/pfx_collidable.h"
#include "../collision/pfx_contact_complex.h"
#include "../rigidbody/pfx_simulation_stage.h"
#include "pfx_progressive_bvh.h"

namespace sce {
namespace pfxv4 {

void calculateAabb( const PfxRigidState &state, const PfxCollidable &collidable, const PfxFloat timeStep, PfxVector3& center, PfxVector3& extent );

class PfxBroadphaseStage : public PfxSimulationStage {
private:
	struct UpdateBvhJobArg {
		PfxBroadphaseStage *stage;
		PfxInt32 dispatchId;
		PfxUInt32 start = 0, num = 0, total = 0, layer = 0;
		PfxUInt32 numJobs = 0;
		PfxProgressiveBvh *bvh;
		PfxUInt32 outOfWorldBehavior = 0;
		PfxBool fixOutOfWorldBody = false;
		PfxBool resetShiftFlag = true;
		void(*outOfWorldCallback)(PfxUInt32, void *) = nullptr;
		void *userDataForOutOfWorldCallback = nullptr;
	};

	struct FindPairsJobArg {
		PfxBroadphaseStage *stage;
		PfxInt32 dispatchId;
		PfxUInt32 start = 0, num = 0, total = 0, layer = 0;
		PfxUInt32 flip = 0, numJobs = 0;
		PfxProgressiveBvh *bvh;
		const PfxProgressiveBvh *bvhA, *bvhB;
		PfxBroadphasePair *outPairs;
		PfxInt32 *numOutPairs;
	};

	static const PfxUInt32 maxSwapBodies = 100;

	PfxUInt32 *swapBodies;

	std::atomic<PfxUInt32> numTmpPairs[3];
	PfxUInt16 *tmpPairs[3];

	static void job_updateBvh(void *data, int uid);
	static void job_generateRandomSwap(void *data, int uid);
	static void job_swapBvh(void *data, int uid);

	static void job_refineBvh(void *data, int uid);
	static void job_refineBvhTop(void *data, int uid);
	static void job_setSkipPointersTop(void *data, int uid);
	static void job_setSkipPointers(void *data, int uid);

	static void job_findOverlapPairsTopS(void *data, int uid);
	static void job_findOverlapPairsSelfS(void *data, int uid);
	static void job_findOverlapPairsInterS(void *data, int uid);
	static void job_findOverlapPairsBottomS(void *data, int uid);

	static void job_findOverlapPairsTopX(void *data, int uid);
	static void job_findOverlapPairsInterX(void *data, int uid);
	static void job_findOverlapPairsBottomX(void *data, int uid);

	PfxInt32 findOverlapPairsS(const PfxProgressiveBvh &bvh, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs);
	PfxInt32 findOverlapPairsX(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs);

public:
	struct BroadphaseSharedArg {
		PfxFloat timeStep;
		PfxLargePosition worldMin;
		PfxLargePosition worldMax;
		PfxRigidState *states;
		PfxCollidable *collidables;
		PfxUInt32 numRigidBodies;
		PfxUInt32 maxPairs;
	} shared;

	PfxInt32 preparePipelineForUpdateBvh(const PfxLargePosition &worldMin, const PfxLargePosition &worldMax, PfxFloat timeStep,
		PfxRigidState *states, PfxCollidable *collidables, PfxUInt32 numRigidBodies);

	PfxInt32 preparePipelineForFindPairs(PfxUInt32 maxContacts);

	PfxInt32 preparePipelineForRefinePairs(PfxRigidState *states, PfxUInt32 maxContacts);

	void printOverlapPairs(const PfxBroadphasePair *outPairs, const PfxInt32 numOutPairs);

	PfxInt32 updateBvh(PfxProgressiveBvh &bvh, PfxBool fixOutOfWorldBody, PfxBool resetShiftFlag, void(*outOfWorldCallback)(PfxUInt32, void *), void *userDataForOutOfWorldCallback);

	PfxInt32 findOverlapPairs(const PfxProgressiveBvh &bvhA, const PfxProgressiveBvh &bvhB, PfxBroadphasePair *outPairs, PfxInt32 *numOutPairs);

	///////////////////////////////////////////////////////////////////////////////
	// Refine Pairs

private:
	PfxUInt32 *workBuff1;
	PfxUInt32 *workBuff2;

	std::atomic<PfxUInt32> pairLen;

	struct RefinePairsJobArg {
		PfxBroadphaseStage *stage;
		PfxInt32 dispatchId;
		PfxPairContainer *pairContainer;
		PfxContactManager *contactManager;
		PfxUInt32 start, num, flip;
		const PfxBroadphasePair *outPairs;
		const PfxInt32 *numOutPairs;
		PfxBool dontWakeUp = false;
	};

	PfxBool updateContactManifolds(
		PfxContactManager *contactManager, PfxUInt32 contactId,
		const PfxRigidState &stateA, const PfxCollidable &collA,
		const PfxRigidState &stateB, const PfxCollidable &collB);

	static void job_resetPairState(void *data, int uid);
	static void job_AddPairs(void *data, int uid);
	static void job_removeUnusedPairs(void *data, int uid);

public:

	PfxInt32 refinePairs(PfxContactComplex &contactComplex, const PfxBroadphasePair *outPairs, const PfxInt32 *numOutPairs, PfxBool dontWakeUp);
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_BROADPHASE_STAGE_H

