/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _PFX_SOLVE_CONSTRAINTS_STAGE_H
#define _PFX_SOLVE_CONSTRAINTS_STAGE_H

#include "../rigidbody/pfx_simulation_stage.h"
#include "../collision/pfx_contact_complex.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_state.h"
#include "../../../include/physics_effects/base_level/rigidbody/pfx_rigid_body.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint.h"
#include "../../../include/physics_effects/base_level/solver/pfx_solver_body.h"
#include "../../../src/low_level/solver/pfx_pair_graph.h"

namespace sce {
namespace pfxv4 {

// Just for checking the possibility of algorithm's concurrency
class PfxSplitPairsStage : public PfxSimulationStage {
private:
	const static int MAX_CONTACTS = 30000;
	const static int MAX_RIGIDBODIES = 5000;

public:
	struct PairInBatch {
		PfxUInt32 pairId;
		PfxUInt16 rigidbodyIdA, rigidbodyIdB;
		PfxUInt16 phaseId;
		PfxUInt16 batchId;
	};

	PfxUInt16 batches[MAX_RIGIDBODIES];
	PfxUInt32 pairs[2][MAX_CONTACTS];
	PairInBatch pairInBatch[2][MAX_CONTACTS];
	PfxBool isFixed[MAX_RIGIDBODIES] = { 0 };
	PfxUInt8 numPairsInBatch[MAX_RIGIDBODIES] = {0};
	PfxUInt16 sortedBatchId[MAX_RIGIDBODIES] = {0};

	void splitPairs(PfxContactComplex &contactComplex, PfxRigidState *states, PfxUInt32 numRigidBodies);
};

class PfxSolveConstraintsStage : public PfxSimulationStage {
private:
	struct SolverSharedArg {
		PfxRigidState *states;
		PfxRigidBody *bodies;
		PfxJoint *joints;
		PfxInt32 *numContactPairsPtr;
		PfxInt32 *numJointPairsPtr;
		PfxInt32 *roots;

		PfxContactManager *contactManager;
		PfxSolverBody *solverBodies;
		PfxConstraintPair *contactPairs;
		PfxConstraintPair *jointPairs;
		PfxUInt8 *contactDepth;
		PfxUInt8 *jointDepth;
		PfxUInt32 numRigidBodies;
		PfxBool enableSortOfPairs;
		PfxBool enableLargePosition;
		PfxFloat timeStep;
		PfxFloat timeStepRatio;
		PfxFloat contactBias;
		PfxFloat massAmpInvForContactPairs;
		PfxFloat massAmpInvForJointPairs;
		PfxUInt32 contextFlags;
		PfxUInt32 velocitySolverIteration[3];
		PfxUInt32 positionSolverIteration;
		PfxUInt32 sleepCount;
		PfxFloat sleepThreshold;
	} shared;

	struct SetupCommonJobArg {
		PfxSolveConstraintsStage *stage;
		PfxInt32 total, numJobs;
	};

	SetupCommonJobArg applyImpulseArg;

	struct SolverCommonJobArg {
		PfxSolveConstraintsStage *stage;
		PfxPairBatch *batch;
		PfxUInt32 numBatches;
		PfxUInt32 *pairIdBuffer;
		PfxBool isContact;
	};

	struct MainJobArg {
		PfxSolveConstraintsStage *stage;
		PfxInt32 dispatchId;
		PfxBool isBlocking;
	} mainJobArg;

	struct DeactivationJobArg {
		PfxSolveConstraintsStage *stage;
		PfxUInt32 sleepCount;
		PfxFloat sleepVelSqr;
		PfxInt32 total, numJobs;
	} deactivationJobArgs[32 * 3];

	struct DeactivationCount {
		std::atomic<PfxUInt32> numActive;
		std::atomic<PfxUInt32> numSleep;
		std::atomic<PfxUInt32> numCanSleep;
	} *deactivationCounts;

	PfxBatchInfo contactBatchInfo[3], jointBatchInfo[3];

	static void job_setupSolverBodies(void *data, int uid);
	static void job_applyImpulse(void *data, int uid);
	static void job_setupContactConstraints(void *data, int uid);
	static void job_setupJointConstraints(void *data, int uid);
	static void job_splitPairs(void *data, int uid);
	static void job_contactWarmStarting(void *data, int uid);
	static void job_jointWarmStarting(void *data, int uid);
	static void job_contactConstraintSolver(void *data, int uid);
	static void job_jointConstraintSolver(void *data, int uid);
	static void job_contactPositionCorrection(void *data, int uid);
	static void job_jointPositionCorrection(void *data, int uid);
	static void job_clearDeactivationCount(void *data, int uid);
	static void job_countSleepOrWakeup(void *data, int uid);
	static void job_deactivateStates(void *data, int uid);

	static void job_solveConstraintsSub(void *data, int uid);
	static void job_solveMain(void *data, int uid);

public:
	struct SolverParam {
		PfxJoint *joints = nullptr;
		PfxInt32 *numContactPairsPtr = nullptr;
		PfxInt32 *numJointPairsPtr = nullptr;
		PfxConstraintPair *contactPairs = nullptr;
		PfxConstraintPair *jointPairs = nullptr;
		PfxBool enableSortOfPairs = true;
		PfxBool enableLargePosition = false;
		PfxFloat timeStepRatio = 1.0f;
		PfxFloat contactBias = 1.0f;
		PfxFloat massAmpForContacts = 1.6f;
		PfxFloat massAmpForJoints = 1.4f;
		PfxUInt32 velocitySolverIteration[3];
		PfxUInt32 positionSolverIteration = 1;
		PfxUInt32 sleepCount = 180;
		PfxFloat sleepThreshold = 0.1f;
	};

	PfxInt32 preparePipeline(PfxFloat timeStep, PfxInt32 *roots, 
		PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies);

	PfxInt32 solve(PfxContactComplex &contactComplex, SolverParam &param, PfxBool isBlocking);
};

} //namespace pfxv4
} //namespace sce

#endif // _PFX_SOLVE_CONSTRAINTS_STAGE_H

