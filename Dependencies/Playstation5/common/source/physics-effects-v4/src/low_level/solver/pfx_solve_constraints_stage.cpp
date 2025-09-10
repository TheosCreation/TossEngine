/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_solve_constraints_stage.h"
#include "../task/pfx_job_system.h"
#include "../rigidbody/pfx_context.h"
#include "../../../include/physics_effects/base_level/solver/pfx_constraint_pair.h"
#include "../../../include/physics_effects/base_level/solver/pfx_contact_constraint.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_constraint_func.h"
#include "../../../include/physics_effects/base_level/solver/pfx_integrate.h"
#include "../../../src/base_level/broadphase/pfx_check_collidable.h"
#include "../../../src/base_level/solver/pfx_check_solver.h"

namespace sce {
namespace pfxv4 {

void PfxSplitPairsStage::splitPairs(PfxContactComplex &contactComplex, PfxRigidState *states, PfxUInt32 numRigidBodies)
{
	// check algorithm
	PfxPairContainer *pairContainer = &contactComplex.pairContainer;

	for (int i = 0; i < numRigidBodies; i++) {
		isFixed[i] = (SCE_PFX_MOTION_MASK_DYNAMIC(states[i].getMotionType())&SCE_PFX_MOTION_MASK_TYPE) == 0;
		batches[i] = i;
		numPairsInBatch[i] = 0;
	}

	PfxUInt32 numValidPairs = 0;
	PfxUInt32 numPairs[2] = { 0, 0 };

	auto filterPair = [](const PfxPairContainer::PairNode &pair)
	{
		PfxUInt32 motionTypeA = (pair.motionMaskA)&SCE_PFX_MOTION_MASK_TYPE;
		PfxUInt32 motionTypeB = (pair.motionMaskB)&SCE_PFX_MOTION_MASK_TYPE;
		PfxUInt32 sleepA = (pair.motionMaskA)&SCE_PFX_MOTION_MASK_SLEEPING;
		PfxUInt32 sleepB = (pair.motionMaskB)&SCE_PFX_MOTION_MASK_SLEEPING;

		return
			pair.numConstraints > 0 &&
			pfxCheckCollidableTable((ePfxMotionType)motionTypeA, (ePfxMotionType)motionTypeB) && // モーションタイプ別衝突判定テーブル
			!((sleepA != 0 && sleepB != 0) || (sleepA != 0 && motionTypeB == kPfxMotionTypeFixed) || (sleepB != 0 && motionTypeA == kPfxMotionTypeFixed)); // スリープ時のチェック
	};

	for (int i = 0; i < pairContainer->getPairLength(); i++) {
		const PfxPairContainer::PairNode &pairNode = pairContainer->getPairNode(i);
		if(pairNode.state != NODE_IS_NOT_ASSIGNED && (!isFixed[pairNode.rigidbodyIdA] || !isFixed[pairNode.rigidbodyIdB])) {
			if (!filterPair(pairNode)) {
				continue;
			}
			pairInBatch[0][numValidPairs].phaseId = (PfxUInt16)-1;
			pairInBatch[0][numValidPairs].batchId = (PfxUInt16)-1;
			pairInBatch[0][numValidPairs].pairId = i;
			pairInBatch[0][numValidPairs].rigidbodyIdA = pairNode.rigidbodyIdA;
			pairInBatch[0][numValidPairs].rigidbodyIdB = pairNode.rigidbodyIdB;
			pairs[0][numPairs[0]++] = numValidPairs;
			numValidPairs++;
			//SCE_PFX_PRINTF("%3d A %3d B %3d\n", i, pc->m_pairPool[i].rigidbodyIdA, pc->m_pairPool[i].rigidbodyIdB);
		}
	}

	int sw = 0;
	int phaseId = 0;

	auto findRoot = [&](PfxUInt32 rigidbodyId) {
		PfxUInt32 next = batches[rigidbodyId];
		while (next != rigidbodyId) {
			PfxUInt32 prev = rigidbodyId;
			rigidbodyId = next;
			next = batches[rigidbodyId];
			batches[prev] = next;
		}
		return rigidbodyId;
	};

	while(numPairs[sw] > 0) {
		const int maxRBsInBatch = 8;//SCE_PFX_CLAMP(numPairs[sw] / (4u /* threads */ * 4u), 8u, 128u);

		for (int i = 0; i < numPairs[sw]; i++) {
			int pairId = pairs[sw][i];
			PairInBatch &pair = pairInBatch[0][pairId];

			PfxUInt32 rootA = findRoot(pair.rigidbodyIdA);
			PfxUInt32 rootB = findRoot(pair.rigidbodyIdB);

			if(isFixed[pair.rigidbodyIdA]) {
				if (numPairsInBatch[rootB] + 1 <= maxRBsInBatch) {
					pair.phaseId = phaseId;
					pair.batchId = rootB;
					numPairsInBatch[rootB]++;
				}
				else {
					// next phase
					pairs[1 - sw][numPairs[1 - sw]++] = pairId;
				}
			}
			else if(isFixed[pair.rigidbodyIdB]) {
				if (numPairsInBatch[rootA] + 1 <= maxRBsInBatch) {
					pair.phaseId = phaseId;
					pair.batchId = rootA;
					numPairsInBatch[rootA]++;
				}
				else {
					// next phase
					pairs[1 - sw][numPairs[1 - sw]++] = pairId;
				}
			}
			else if (rootA == rootB) {
				if (numPairsInBatch[rootA] + 1 <= maxRBsInBatch) {
					pair.phaseId = phaseId;
					pair.batchId = rootA;
					numPairsInBatch[rootA]++;
				}
				else {
					// next phase
					pairs[1 - sw][numPairs[1 - sw]++] = pairId;
				}
			}
			else if (rootA < rootB) {
				if (numPairsInBatch[rootA] + numPairsInBatch[rootB] + 1 <= maxRBsInBatch) {
					pair.phaseId = phaseId;
					pair.batchId = rootA;
					batches[rootB] = rootA;
					numPairsInBatch[rootA] += numPairsInBatch[rootB] + 1;
				}
				else {
					// next phase
					pairs[1 - sw][numPairs[1 - sw]++] = pairId;
				}
			}
			else if (rootA > rootB) {
				if (numPairsInBatch[rootA] + numPairsInBatch[rootB] + 1 <= maxRBsInBatch) {
					pair.phaseId = phaseId;
					pair.batchId = rootB;
					batches[rootA] = rootB;
					numPairsInBatch[rootB] += numPairsInBatch[rootA] + 1;
				}
				else {
					// next phase
					pairs[1 - sw][numPairs[1 - sw]++] = pairId;
				}
			}
		}

		// for (int i = 0; i < numRigidBodies; i++) {
		// 	// batches[i] = findRoot(i);
		// 	SCE_PFX_PRINTF("batch(%3d) %3d num　%3d\n", i, batches[i], numPairsInBatch[i]);
		// }

		// aggregate small batches -------------------------------------

		// sort batches by numbers
		PfxUInt32 batchCount[128] = {0};
		PfxUInt32 maxCount = 0;
		for (int i = 0; i < numRigidBodies; i++) {
			if (batches[i] == i && numPairsInBatch[i] > 0) {
				batchCount[numPairsInBatch[i]]++;
				maxCount = SCE_PFX_MAX(maxCount, numPairsInBatch[i]);
			}
		}
		maxCount++;

		PfxUInt32 sum = batchCount[0];
		batchCount[0] = 0;
		for (int i = 1; i < maxCount; i++) {
			PfxUInt32 t = batchCount[i];
			batchCount[i] = sum;
			sum += t;
		}

		for (int i = 0; i < numRigidBodies; i++) {
			if (batches[i] == i && numPairsInBatch[i] > 0) {
				PfxUInt32 outId = batchCount[numPairsInBatch[i]]++;
				sortedBatchId[outId] = i;
			}
		}

		for (int i = 0; i < sum; i++) {
			PfxUInt32 bi = sortedBatchId[i];
			if (numPairsInBatch[bi] == 0) continue; 
			for (int j = i + 1; j < sum; j++) {
				PfxUInt32 bj = sortedBatchId[j];
				if (numPairsInBatch[bj] == 0) continue; 
				if (numPairsInBatch[bi] + numPairsInBatch[bj] <= maxRBsInBatch) {
					batches[bj] = bi;
					numPairsInBatch[bi] += numPairsInBatch[bj];
					numPairsInBatch[bj] = 0;
				}
				else {
					break;
				}
			}
		}

		// ------------------------------------- aggregate small batches 

		for (int i = 0; i < numRigidBodies; i++) {
			batches[i] = findRoot(i);
			//SCE_PFX_PRINTF("agg batch(%3d) %3d num　%3d\n", i, batches[i], numPairsInBatch[i]);
		}

		for (int i = 0; i < numPairs[sw]; i++) {
			int pairId = pairs[sw][i];
			PairInBatch &pair = pairInBatch[0][pairId];
			if (pair.phaseId == phaseId) {
				pair.batchId = batches[pair.batchId];
			}
			//SCE_PFX_PRINTF("%5d phase %3d root %3d A %3d B %3d \n", pair.pairId, pair.phaseId, pair.batchId, pair.rigidbodyIdA, pair.rigidbodyIdB);
		}
		//SCE_PFX_PRINTF("--------------- phase %d ---------------\n", phaseId);
		// reset batches info for a next phase
		for (int i = 0; i < numRigidBodies; i++) {
			batches[i] = i;
			numPairsInBatch[i] = 0;
		}

		numPairs[sw] = 0;

		sw = 1 - sw;
		phaseId++;
	}

	// sort pairs by roots
	{
		PfxUInt32 rootCount[MAX_RIGIDBODIES] = { 0 };
		PfxUInt32 maxRB = 0;
		for (int i = 0; i < numValidPairs; i++) {
			rootCount[pairInBatch[0][i].batchId]++;
			maxRB = SCE_PFX_MAX(maxRB, pairInBatch[0][i].batchId);
		}
		maxRB++;

		PfxUInt32 sum = rootCount[0];
		rootCount[0] = 0;
		for (int i = 1; i < maxRB; i++) {
			PfxUInt32 t = rootCount[i];
			rootCount[i] = sum;
			sum += t;
		}

		for (int i = 0; i < numValidPairs; i++) {
			PfxUInt32 outId = rootCount[pairInBatch[0][i].batchId]++;
			pairInBatch[1][outId] = pairInBatch[0][i];
		}
	}

	// sort pairs by phases
	{
		SCE_PFX_ASSERT(phaseId <= 32);
		PfxUInt32 phaseCount[32] = { 0 };

		for (int i = 0; i < numValidPairs; i++) {
			SCE_PFX_ASSERT(pairInBatch[1][i].pairId != (PfxUInt32)-1);
			phaseCount[pairInBatch[1][i].phaseId]++;
		}

		PfxUInt32 sum = phaseCount[0];
		phaseCount[0] = 0;
		for (int i = 1; i < phaseId; i++) {
			PfxUInt32 t = phaseCount[i];
			phaseCount[i] = sum;
			sum += t;
		}

		for (int i = 0; i < numValidPairs; i++) {
			SCE_PFX_ASSERT(pairInBatch[1][i].pairId != (PfxUInt32)-1);
			PfxUInt32 outId = phaseCount[pairInBatch[1][i].phaseId]++;
			pairInBatch[0][outId] = pairInBatch[1][i];
		}
	}

	// print results and check validation
	#if 1
	PfxUInt32 phase = 0;
	PfxUInt32 batch = 0;
	PfxUInt32 dependency[MAX_RIGIDBODIES] = { 0 };
	for (int i = 0; i < numValidPairs; i++) {
		PairInBatch &pib = pairInBatch[0][i];

		if (phase != pib.phaseId) {
			memset(dependency, 0, sizeof(PfxUInt32) * numRigidBodies);
			phase = pib.phaseId;
		}

		PfxUInt32 rbA = pib.rigidbodyIdA;
		PfxUInt32 rbB = pib.rigidbodyIdB;

		SCE_PFX_PRINTF("%5d phase %3d batch %3d A %3d B %3d \n", pib.pairId, pib.phaseId, pib.batchId, rbA, rbB);

		batch = pib.batchId;

		auto checkDependency = [&](PfxUInt32 rigidbodyId) {
			return dependency[rigidbodyId] == 0 || dependency[rigidbodyId] == batch + 1;
		};

		if (!checkDependency(rbA)) { SCE_PFX_PRINTF("dependency error!! %d batch %d in dep %d\n", rbA, batch, dependency[rbA]-1); }
		if (!checkDependency(rbB)) { SCE_PFX_PRINTF("dependency error!! %d batch %d in dep %d\n", rbB, batch, dependency[rbB]-1); }

		if (!isFixed[rbA]) dependency[rbA] = batch + 1;
		if (!isFixed[rbB]) dependency[rbB] = batch + 1;
	}
	#endif
}

///////////////////////////////////////////////////////////////////////////////
// Solve Constraints

void PfxSolveConstraintsStage::job_setupSolverBodies(void *data, int uid)
{
	SetupCommonJobArg *arg = (SetupCommonJobArg*)data;
	
	SolverSharedArg *shared = &arg->stage->shared;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	for(PfxInt32 i=start;i<start+num;i++) {
		PfxRigidState &state = shared->states[i];
		PfxRigidBody &body = shared->bodies[i];
		PfxSolverBody &solverBody = shared->solverBodies[i];
		
		solverBody.m_orientation = state.getOrientation();
		solverBody.m_deltaLinearVelocity = PfxVector3::zero();
		solverBody.m_deltaAngularVelocity = PfxVector3::zero();
		solverBody.m_motionType = state.getMotionMask();

		if(SCE_PFX_MOTION_MASK_DYNAMIC(state.getMotionType())) {
			PfxMatrix3 ori(solverBody.m_orientation);
			solverBody.m_massInv = body.getMassInv();
			solverBody.m_inertiaInv = ori * body.getInertiaInv() * transpose(ori);
		}
		else {
			solverBody.m_massInv = 0.0f;
			solverBody.m_inertiaInv = PfxMatrix3(0.0f);
		}
	}
}

void PfxSolveConstraintsStage::job_applyImpulse(void *data, int uid)
{
	SetupCommonJobArg *arg = (SetupCommonJobArg*)data;
	
	SolverSharedArg *shared = &arg->stage->shared;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	for(PfxInt32 i=start;i<start+num;i++) {
		PfxRigidState &state = shared->states[i];
		
		PfxSolverBody &solverBody = shared->solverBodies[i];
		
		SCE_PFX_VALIDATE_VECTOR3(solverBody.m_deltaLinearVelocity);
		SCE_PFX_VALIDATE_VECTOR3(solverBody.m_deltaAngularVelocity);

		state.setLinearVelocity(
			state.getLinearVelocity()+solverBody.m_deltaLinearVelocity);
		state.setAngularVelocity(
			state.getAngularVelocity()+solverBody.m_deltaAngularVelocity);
		
		// Integration is needed before position correction
		PfxRigidBody &body = shared->bodies[i];
		
		pfxIntegrate(state, body, shared->timeStep);
		
		state.setClosestHit(false);
		
		if(SCE_PFX_MOTION_MASK_DYNAMIC(state.getMotionType())) {
			PfxMatrix3 ori(state.getOrientation());
			solverBody.m_massInv = body.getMassInv();
			solverBody.m_inertiaInv = ori * body.getInertiaInv() * transpose(ori);
		}
		
		if (shared->enableLargePosition) {
			state.updateSegment();
		}
	}
}

void PfxSolveConstraintsStage::job_setupContactConstraints(void *data, int uid)
{
	SetupCommonJobArg *arg = (SetupCommonJobArg*)data;
	
	SolverSharedArg *shared = &arg->stage->shared;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	PfxFloat separateBias = 0.0f;
	
	for(int i=0;i<num;i++) {
		PfxConstraintPair &pair = shared->contactPairs[start+i];
		if(!pfxCheckSolver(pair)) {
			continue;
		}

		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);

		PfxContactHolder &contact = shared->contactManager->getContactHolder(pfxGetConstraintId(pair));

		const PfxRigidState &stateA = shared->states[iA];
		const PfxRigidBody &bodyA = shared->bodies[iA];
		PfxSolverBody solverBodyA = shared->solverBodies[iA];

		const PfxRigidState &stateB = shared->states[iB];
		const PfxRigidBody &bodyB = shared->bodies[iB];
		PfxSolverBody solverBodyB = shared->solverBodies[iB];
		
		PfxFloat friction = contact.getCompositeFriction();
		PfxFloat restitution = contact.getCompositeRestitution();
		PfxFloat rollingFriction = contact.getCompositeRollingFriction();

		restitution = 0.5f * (bodyA.getRestitution() + bodyB.getRestitution());
		contact.setCompositeRestitution(restitution);

		friction = sqrtf(bodyA.getFriction() * bodyB.getFriction());
		contact.setCompositeFriction(friction);

		rollingFriction = 0.5f * (bodyA.getRollingFriction() + bodyB.getRollingFriction());
		contact.setCompositeRollingFriction(rollingFriction);

		if(contact.getDuration() > 1) restitution = 0.0f;

		if(shared->contactDepth[iA] < shared->contactDepth[iB]) {
			solverBodyA.m_massInv *= shared->massAmpInvForContactPairs;
			solverBodyA.m_inertiaInv *= shared->massAmpInvForContactPairs;
		}
		else if(shared->contactDepth[iA] > shared->contactDepth[iB]) {
			solverBodyB.m_massInv *= shared->massAmpInvForContactPairs;
			solverBodyB.m_inertiaInv *= shared->massAmpInvForContactPairs;
		}
		
		PfxContactManifold *contactManifold = contact.findFirstContactManifold();
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
		while (contactManifold) {
#endif
			for (PfxUInt32 j = 0; j < contactManifold->getNumContactPoints(); j++) {
				PfxContactPoint &cp = contactManifold->getContactPoint(j);

				cp.m_constraintRow[0].m_accumImpulse *= shared->timeStepRatio;
				cp.m_constraintRow[1].m_accumImpulse *= shared->timeStepRatio;
				cp.m_constraintRow[2].m_accumImpulse *= shared->timeStepRatio;

				pfxSetupContactConstraint(
					cp.m_constraintRow[0],
					cp.m_constraintRow[1],
					cp.m_constraintRow[2],
					cp.m_distance,
					restitution,
					friction,
					cp.m_constraintRow[0].getNormal(),
					pfxReadVector3(cp.m_localPointA),
					pfxReadVector3(cp.m_localPointB),
					stateA,
					stateB,
					solverBodyA,
					solverBodyB,
					separateBias,
					shared->timeStep
				);
			}

			if (stateA.getClosestHit() || stateB.getClosestHit()) {
				for (PfxUInt32 j = 0; j < contactManifold->getNumClosestPoints(); j++) {
					PfxClosestPoint &cp = contactManifold->getClosestPoint(j);

					pfxSetupClosestConstraint(
						cp.m_constraintRow,
						cp.m_distance,
						cp.m_constraintRow.getNormal(),
						pfxReadVector3(cp.m_localPointA),
						pfxReadVector3(cp.m_localPointB),
						stateA,
						stateB,
						solverBodyA,
						solverBodyB,
						shared->timeStep
					);
				}
			}
			else {
				contactManifold->clearClosestPoints();
			}
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
			contactManifold = contact.findNextContactManifold(contactManifold);
		}
#endif

		contact.getRollingFrictionConstraint().m_accumImpulse *= shared->timeStepRatio;
		
		pfxSetupRollingFriction(
			contact.getRollingFrictionConstraint(),
			contact.getCompositeRollingFriction(),
			stateA,
			stateB,
			solverBodyA,
			solverBodyB);
	}
}

void PfxSolveConstraintsStage::job_setupJointConstraints(void *data, int uid)
{
	SetupCommonJobArg *arg = (SetupCommonJobArg*)data;

	SolverSharedArg *shared = &arg->stage->shared;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	PfxUInt8 *constraintDepth = (shared->enableSortOfPairs ? shared->jointDepth : shared->contactDepth);

	for(int i=0;i<num;i++) {
		PfxConstraintPair &pair = shared->jointPairs[start+i];
		PfxJoint &joint = shared->joints[pfxGetConstraintId(pair)];
		
		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		if(iA != iB) {
			pfxSetActive(pair, joint.m_active>0);
		}
		else {
			pfxSetActive(pair, false);
		}
		
		if (!pfxGetActive(pair)) {
			continue;
		}
		
		const PfxRigidState &stateA = shared->states[iA];
		PfxSolverBody solverBodyA = shared->solverBodies[iA];

		const PfxRigidState &stateB = shared->states[iB];
		PfxSolverBody solverBodyB = shared->solverBodies[iB];
		
		if (constraintDepth[iA] < constraintDepth[iB]) {
			solverBodyA.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyA.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}
		else if (constraintDepth[iA] > constraintDepth[iB]) {
			solverBodyB.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyB.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}
		
		// Update rigid bodies' state
		pfxSetMotionMaskA(pair,stateA.getMotionMask());
		pfxSetMotionMaskB(pair,stateB.getMotionMask());
		pfxSetSolverQuality(pair, SCE_PFX_MAX(stateA.getSolverQuality(), stateB.getSolverQuality()));
		
		joint.m_constraints[0].m_constraintRow.m_accumImpulse *= shared->timeStepRatio;
		joint.m_constraints[1].m_constraintRow.m_accumImpulse *= shared->timeStepRatio;
		joint.m_constraints[2].m_constraintRow.m_accumImpulse *= shared->timeStepRatio;
		joint.m_constraints[3].m_constraintRow.m_accumImpulse *= shared->timeStepRatio;
		joint.m_constraints[4].m_constraintRow.m_accumImpulse *= shared->timeStepRatio;
		joint.m_constraints[5].m_constraintRow.m_accumImpulse *= shared->timeStepRatio;
		
		pfxGetSetupJointConstraintFunc(joint.m_type)(
			joint,
			stateA,
			stateB,
			solverBodyA,
			solverBodyB,
			shared->timeStep);
	}
}

void PfxSolveConstraintsStage::job_splitPairs(void *data, int uid)
{
	PfxSplitPairsArg *arg = ((PfxSplitPairsArg*)data) + uid;
	
	PfxInt32 n = postProcessOfSplitPairs(*arg);

	if(n < 0) {
		arg->result = -1;
	}
	else {
		arg->result = 0;
	}
}

void PfxSolveConstraintsStage::job_contactWarmStarting(void *data, int uid)
{
	SolverCommonJobArg *arg = ((SolverCommonJobArg*)data);
	
	SolverSharedArg *shared = &arg->stage->shared;
	PfxPairBatch *batch = arg->batch + uid;

	for(PfxUInt32 i=0;i<batch->numPairs;i++) {
		PfxConstraintPair &pair = shared->contactPairs[arg->pairIdBuffer[batch->startPairId + i]];
		
		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		PfxContactHolder &contact = shared->contactManager->getContactHolder(pfxGetConstraintId(pair));
		
		PfxSolverBody &solverBodyA = shared->solverBodies[iA];
		PfxSolverBody &solverBodyB = shared->solverBodies[iB];
		
		PfxFloat massInvA = solverBodyA.m_massInv;
		PfxFloat massInvB = solverBodyB.m_massInv;
		PfxMatrix3 inertiaInvA = solverBodyA.m_inertiaInv;
		PfxMatrix3 inertiaInvB = solverBodyB.m_inertiaInv;

		if(SCE_PFX_UNLIKELY(solverBodyA.m_motionType == kPfxMotionTypeOneWay)) {
			massInvB = 0.0f;
			inertiaInvB = PfxMatrix3(0.0f);
		}
		if(SCE_PFX_UNLIKELY(solverBodyB.m_motionType == kPfxMotionTypeOneWay)) {
			massInvA = 0.0f;
			inertiaInvA = PfxMatrix3(0.0f);
		}

		if(shared->contactDepth[iA] < shared->contactDepth[iB]) {
			massInvA *= shared->massAmpInvForContactPairs;
			inertiaInvA *= shared->massAmpInvForContactPairs;
		}
		else if(shared->contactDepth[iA] > shared->contactDepth[iB]) {
			massInvB *= shared->massAmpInvForContactPairs;
			inertiaInvB *= shared->massAmpInvForContactPairs;
		}

		PfxContactManifold *contactManifold = contact.findFirstContactManifold();
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
		while (contactManifold) {
#endif
			for (PfxUInt32 j = 0; j < contactManifold->getNumContactPoints(); j++) {
				PfxContactPoint &cp = contactManifold->getContactPoint(j);

				PfxVector3 rA = rotate(solverBodyA.m_orientation, pfxReadVector3(cp.m_localPointA));
				PfxVector3 rB = rotate(solverBodyB.m_orientation, pfxReadVector3(cp.m_localPointB));

				for (int k = 0; k < 3; k++) {
					PfxVector3 normal = cp.m_constraintRow[k].getNormal();
					PfxFloat deltaImpulse = cp.m_constraintRow[k].m_accumImpulse;
					SCE_PFX_VALIDATE_VECTOR3(normal);
					SCE_PFX_VALIDATE_FLOAT(deltaImpulse);
					solverBodyA.m_deltaLinearVelocity += deltaImpulse * massInvA * normal;
					solverBodyA.m_deltaAngularVelocity += deltaImpulse * inertiaInvA * cross(rA, normal);
					solverBodyB.m_deltaLinearVelocity -= deltaImpulse * massInvB * normal;
					solverBodyB.m_deltaAngularVelocity -= deltaImpulse * inertiaInvB * cross(rB, normal);
				}
			}
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
			contactManifold = contact.findNextContactManifold(contactManifold);
		}
#endif

		const PfxConstraintRow &rollingFriction = contact.getRollingFrictionConstraint();
		if(lengthSqr(rollingFriction.getNormal()) > 1e-4f) {
			PfxVector3 normal = rollingFriction.getNormal();
			PfxFloat deltaImpulse = rollingFriction.m_accumImpulse;
			SCE_PFX_VALIDATE_VECTOR3(normal);
			SCE_PFX_VALIDATE_FLOAT(deltaImpulse);
			solverBodyA.m_deltaAngularVelocity += deltaImpulse * inertiaInvA * normal;
			solverBodyB.m_deltaAngularVelocity -= deltaImpulse * inertiaInvB * normal;
		}
	}
}

void PfxSolveConstraintsStage::job_jointWarmStarting(void *data, int uid)
{
	SolverCommonJobArg *arg = ((SolverCommonJobArg*)data);

	SolverSharedArg *shared = &arg->stage->shared;
	PfxPairBatch *batch = arg->batch + uid;

	PfxUInt8 *constraintDepth = (shared->enableSortOfPairs ? shared->jointDepth : shared->contactDepth);
	
	for(PfxUInt32 i=0;i<batch->numPairs;i++) {
		PfxConstraintPair &pair = shared->jointPairs[arg->pairIdBuffer[batch->startPairId + i]];

		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		PfxJoint &joint = shared->joints[pfxGetConstraintId(pair)];
		
		PfxSolverBody solverBodyA = shared->solverBodies[iA];
		PfxSolverBody solverBodyB = shared->solverBodies[iB];
		
		if (constraintDepth[iA] < constraintDepth[iB]) {
			solverBodyA.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyA.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}
		else if (constraintDepth[iA] > constraintDepth[iB]) {
			solverBodyB.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyB.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}
		
		pfxGetWarmStartJointConstraintFunc(joint.m_type)(
			joint,
			solverBodyA,
			solverBodyB);

		shared->solverBodies[iA].m_deltaLinearVelocity = solverBodyA.m_deltaLinearVelocity;
		shared->solverBodies[iA].m_deltaAngularVelocity = solverBodyA.m_deltaAngularVelocity;
		shared->solverBodies[iB].m_deltaLinearVelocity = solverBodyB.m_deltaLinearVelocity;
		shared->solverBodies[iB].m_deltaAngularVelocity = solverBodyB.m_deltaAngularVelocity;
	}
}

void PfxSolveConstraintsStage::job_contactConstraintSolver(void *data, int uid)
{
	SolverCommonJobArg *arg = ((SolverCommonJobArg*)data);

	SolverSharedArg *shared = &arg->stage->shared;
	PfxPairBatch *batch = arg->batch + uid;

	for(PfxUInt32 i=0;i<batch->numPairs;i++) {
		PfxConstraintPair &pair = shared->contactPairs[arg->pairIdBuffer[batch->startPairId + i]];

		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		PfxContactHolder &contact = shared->contactManager->getContactHolder(pfxGetConstraintId(pair));

		PfxSolverBody solverBodyA = shared->solverBodies[iA];
		PfxSolverBody solverBodyB = shared->solverBodies[iB];

		if(shared->contactDepth[iA] < shared->contactDepth[iB]) {
			solverBodyA.m_massInv *= shared->massAmpInvForContactPairs;
			solverBodyA.m_inertiaInv *= shared->massAmpInvForContactPairs;
		}
		else if(shared->contactDepth[iA] > shared->contactDepth[iB]) {
			solverBodyB.m_massInv *= shared->massAmpInvForContactPairs;
			solverBodyB.m_inertiaInv *= shared->massAmpInvForContactPairs;
		}

		PfxContactManifold *contactManifold = contact.findFirstContactManifold();
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
		while (contactManifold) {
#endif
			for(PfxUInt32 j=0;j<contactManifold->getNumContactPoints();j++) {
				PfxContactPoint &cp = contactManifold->getContactPoint(j);
			
				pfxSolveContactConstraint(
					cp.m_constraintRow[0],
					cp.m_constraintRow[1],
					cp.m_constraintRow[2],
					pfxReadVector3(cp.m_localPointA),
					pfxReadVector3(cp.m_localPointB),
					solverBodyA,
					solverBodyB,
					contact.getCompositeFriction()
					);
			}
		
			for(PfxUInt32 j=0;j<contactManifold->getNumClosestPoints();j++) {
				PfxClosestPoint &cp = contactManifold->getClosestPoint(j);

				pfxSolveClosestConstraint(
					cp.m_constraintRow,
					pfxReadVector3(cp.m_localPointA),
					pfxReadVector3(cp.m_localPointB),
					solverBodyA,
					solverBodyB,
					contact.getCompositeFriction()
					);
			}
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
			contactManifold = contact.findNextContactManifold(contactManifold);
		}
#endif

		pfxSolveRollingFriction(
			contact.getRollingFrictionConstraint(),
			contact.getCompositeRollingFriction(),
			solverBodyA,
			solverBodyB
			);
		
		shared->solverBodies[iA].m_deltaLinearVelocity = solverBodyA.m_deltaLinearVelocity;
		shared->solverBodies[iA].m_deltaAngularVelocity = solverBodyA.m_deltaAngularVelocity;
		shared->solverBodies[iB].m_deltaLinearVelocity = solverBodyB.m_deltaLinearVelocity;
		shared->solverBodies[iB].m_deltaAngularVelocity = solverBodyB.m_deltaAngularVelocity;
	}
}

void PfxSolveConstraintsStage::job_jointConstraintSolver(void *data, int uid)
{
	SolverCommonJobArg *arg = ((SolverCommonJobArg*)data);

	SolverSharedArg *shared = &arg->stage->shared;
	PfxPairBatch *batch = arg->batch + uid;

	PfxUInt8 *constraintDepth = (shared->enableSortOfPairs ? shared->jointDepth : shared->contactDepth);

	for(PfxUInt32 i=0;i<batch->numPairs;i++) {
		PfxConstraintPair &pair = shared->jointPairs[arg->pairIdBuffer[batch->startPairId + i]];
	
		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		PfxJoint &joint = shared->joints[pfxGetConstraintId(pair)];
		
		PfxSolverBody solverBodyA = shared->solverBodies[iA];
		PfxSolverBody solverBodyB = shared->solverBodies[iB];

		if (constraintDepth[iA] < constraintDepth[iB]) {
			solverBodyA.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyA.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}
		else if (constraintDepth[iA] > constraintDepth[iB]) {
			solverBodyB.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyB.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}
		
		pfxGetSolveJointConstraintFunc(joint.m_type)(
			joint,
			solverBodyA,
			solverBodyB);

		shared->solverBodies[iA].m_deltaLinearVelocity = solverBodyA.m_deltaLinearVelocity;
		shared->solverBodies[iA].m_deltaAngularVelocity = solverBodyA.m_deltaAngularVelocity;
		shared->solverBodies[iB].m_deltaLinearVelocity = solverBodyB.m_deltaLinearVelocity;
		shared->solverBodies[iB].m_deltaAngularVelocity = solverBodyB.m_deltaAngularVelocity;
	}
}

void PfxSolveConstraintsStage::job_contactPositionCorrection(void *data, int uid)
{
	SolverCommonJobArg *arg = ((SolverCommonJobArg*)data);

	SolverSharedArg *shared = &arg->stage->shared;
	PfxPairBatch *batch = arg->batch + uid;

	for(PfxUInt32 i=0;i<batch->numPairs;i++) {
		PfxConstraintPair &pair = shared->contactPairs[arg->pairIdBuffer[batch->startPairId + i]];

		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		PfxContactHolder &contact = shared->contactManager->getContactHolder(pfxGetConstraintId(pair));

		PfxSolverBody solverBodyA = shared->solverBodies[iA];
		PfxSolverBody solverBodyB = shared->solverBodies[iB];

		if(shared->contactDepth[iA] < shared->contactDepth[iB]) {
			solverBodyA.m_massInv *= shared->massAmpInvForContactPairs;
			solverBodyA.m_inertiaInv *= shared->massAmpInvForContactPairs;
		}
		else if(shared->contactDepth[iA] > shared->contactDepth[iB]) {
			solverBodyB.m_massInv *= shared->massAmpInvForContactPairs;
			solverBodyB.m_inertiaInv *= shared->massAmpInvForContactPairs;
		}

		PfxRigidState &stateA = shared->states[iA];
		PfxRigidState &stateB = shared->states[iB];

		PfxLargePosition posA = stateA.getLargePosition();
		PfxLargePosition posB = stateB.getLargePosition();
		posB.changeSegment(posA.segment);

		solverBodyA.m_deltaLinearVelocity = posA.offset;// Note : use m_deltaLinearVelocity as position
		solverBodyB.m_deltaLinearVelocity = posB.offset;// Note : use m_deltaLinearVelocity as position
		solverBodyA.m_orientation = stateA.getOrientation();
		solverBodyB.m_orientation = stateB.getOrientation();

		PfxContactManifold *contactManifold = contact.findFirstContactManifold();
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
		while (contactManifold) {
#endif
			for (PfxUInt32 j = 0; j < contactManifold->getNumContactPoints(); j++) {
				PfxContactPoint &cp = contactManifold->getContactPoint(j);

				pfxCorrectContactPositionConstraint(
					cp.m_constraintRow[0],
					pfxReadVector3(cp.m_localPointA),
					pfxReadVector3(cp.m_localPointB),
					solverBodyA,
					solverBodyB,
					shared->contactBias
				);
			}
#ifdef SCE_PFX_ENABLE_MULTIPLE_CONTACT_MANIFOLDS
			contactManifold = contact.findNextContactManifold(contactManifold);
		}
#endif
		
		posA.offset = solverBodyA.m_deltaLinearVelocity;// Note : use m_deltaLinearVelocity as position
		posB.offset = solverBodyB.m_deltaLinearVelocity;// Note : use m_deltaLinearVelocity as position
				
		if (SCE_PFX_MOTION_MASK_DYNAMIC(stateA.getMotionType())) {
			stateA.setLargePosition(posA);
			stateA.setOrientation(solverBodyA.m_orientation);
		}

		if (SCE_PFX_MOTION_MASK_DYNAMIC(stateB.getMotionType())) {
			stateB.setLargePosition(posB);
			stateB.setOrientation(solverBodyB.m_orientation);
		}
	}
}

void PfxSolveConstraintsStage::job_jointPositionCorrection(void *data, int uid)
{
	SolverCommonJobArg *arg = ((SolverCommonJobArg*)data);

	SolverSharedArg *shared = &arg->stage->shared;
	PfxPairBatch *batch = arg->batch + uid;

	PfxUInt8 *constraintDepth = (shared->enableSortOfPairs ? shared->jointDepth : shared->contactDepth);

	for(PfxUInt32 i=0;i<batch->numPairs;i++) {
		PfxConstraintPair &pair = shared->jointPairs[arg->pairIdBuffer[batch->startPairId + i]];
		
		PfxUInt32 iA = pfxGetObjectIdA(pair);
		PfxUInt32 iB = pfxGetObjectIdB(pair);
		PfxJoint &joint = shared->joints[pfxGetConstraintId(pair)];
		
		PfxSolverBody solverBodyA = shared->solverBodies[iA];
		PfxSolverBody solverBodyB = shared->solverBodies[iB];

		if (constraintDepth[iA] < constraintDepth[iB]) {
			solverBodyA.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyA.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}
		else if (constraintDepth[iA] > constraintDepth[iB]) {
			solverBodyB.m_massInv *= shared->massAmpInvForJointPairs;
			solverBodyB.m_inertiaInv *= shared->massAmpInvForJointPairs;
		}

		PfxRigidState &stateA = shared->states[iA];
		PfxRigidState &stateB = shared->states[iB];

		PfxLargePosition posA = stateA.getLargePosition();
		PfxLargePosition posB = stateB.getLargePosition();
		posB.changeSegment(posA.segment);

		solverBodyA.m_deltaLinearVelocity = posA.offset;// Note : use m_deltaLinearVelocity as position
		solverBodyB.m_deltaLinearVelocity = posB.offset;// Note : use m_deltaLinearVelocity as position
		solverBodyA.m_orientation = stateA.getOrientation();
		solverBodyB.m_orientation = stateB.getOrientation();
		
		pfxGetCorrectJointPositionConstraintFunc(joint.m_type)(
			joint,
			solverBodyA,
			solverBodyB);
		
		posA.offset = solverBodyA.m_deltaLinearVelocity;// Note : use m_deltaLinearVelocity as position
		posB.offset = solverBodyB.m_deltaLinearVelocity;// Note : use m_deltaLinearVelocity as position

		if (SCE_PFX_MOTION_MASK_DYNAMIC(stateA.getMotionType())) {
			stateA.setLargePosition(posA);
			stateA.setOrientation(solverBodyA.m_orientation);
		}
		if (SCE_PFX_MOTION_MASK_DYNAMIC(stateB.getMotionType())) {
			stateB.setLargePosition(posB);
			stateB.setOrientation(solverBodyB.m_orientation);
		}
	}
}

void PfxSolveConstraintsStage::job_clearDeactivationCount(void *data, int uid)
{
	DeactivationJobArg *arg = (DeactivationJobArg*)data;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	for (PfxUInt32 i = start; i < start + num; i++) {
		arg->stage->deactivationCounts[i].numActive = 0;
		arg->stage->deactivationCounts[i].numSleep = 0;
		arg->stage->deactivationCounts[i].numCanSleep = 0;
	}
}

void PfxSolveConstraintsStage::job_countSleepOrWakeup(void *data, int uid)
{
	DeactivationJobArg *arg = (DeactivationJobArg*)data;
	SolverSharedArg *shared = &arg->stage->shared;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	for (PfxUInt32 i = start; i < start + num; i++) {
		PfxRigidState &state = shared->states[i];
		if(SCE_PFX_MOTION_MASK_CAN_SLEEP(state.getMotionType())) {
			PfxFloat linVelSqr = lengthSqr(state.getLinearVelocity());
			PfxFloat angVelSqr = lengthSqr(state.getAngularVelocity());

			DeactivationCount &dcount = arg->stage->deactivationCounts[arg->stage->shared.roots[i]];

			if(state.isAwake()) {
				if( linVelSqr < arg->sleepVelSqr && angVelSqr < arg->sleepVelSqr ) {
					state.incrementSleepCount();
				}
				else {
					state.resetSleepCount();
				}
				dcount.numActive++;
				if(state.getSleepCount() > arg->sleepCount) {
					dcount.numCanSleep++;
				}
			}
			else {
				dcount.numSleep++;
			}
		}
	}
}

void PfxSolveConstraintsStage::job_deactivateStates(void *data, int uid)
{
	DeactivationJobArg *arg = (DeactivationJobArg*)data;
	SolverSharedArg *shared = &arg->stage->shared;

	PfxInt32 total = arg->total;
	PfxInt32 batch = (total + arg->numJobs - 1) / arg->numJobs;
	PfxInt32 start = batch * uid;
	PfxInt32 num = SCE_PFX_MAX(0, SCE_PFX_MIN(batch, (total - start)));

	for (PfxUInt32 i = start; i < start + num; i++) {
		PfxRigidState &state = shared->states[i];

		if(SCE_PFX_MOTION_MASK_CAN_SLEEP(state.getMotionType())) {
			DeactivationCount &dcount = arg->stage->deactivationCounts[arg->stage->shared.roots[i]];

			// Deactivate Island
			if(dcount.numCanSleep > 0 && dcount.numCanSleep == dcount.numActive + dcount.numSleep) {
				state.sleep();
			}

			// Activate Island
			else if(dcount.numSleep > 0 && dcount.numActive > 0) {
				state.wakeup();
			}
		}
	}
}

void PfxSolveConstraintsStage::job_solveConstraintsSub(void *data, int uid)
{
	MainJobArg *arg = (MainJobArg*)data;

	PfxJobSystem *jobSystem = arg->stage->getJobSystem();

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();
	SolverSharedArg *shared = &arg->stage->shared;
	PfxUInt32 numRigidBodies = shared->numRigidBodies;

	PfxInt32 ret = SCE_OK;

	int nSolverJobs = 0;
	int startArgId[3] = {0, 0, 0};
	int numArgs[3] = {0, 0, 0};

	// count and create all dispatch args
	for (int l = 0; l < 3; l++) {
		startArgId[l] = nSolverJobs;
		{
			int numBatchesInPhase = 0;
			int phase = 0;
			PfxPairBatch *batch = arg->stage->jointBatchInfo[l].pairBatches;
			for (PfxUInt32 b = 0; b < arg->stage->jointBatchInfo[l].maxBatches; b++) {
				if (batch->phase > phase) { // phaseが変わっていたら進める
					nSolverJobs++;
					numBatchesInPhase = 0;
					phase = batch->phase;
				}
				batch++;
				numBatchesInPhase++;
			}

			if (numBatchesInPhase > 0) {
				nSolverJobs++;
			}
		}

		{
			int numBatchesInPhase = 0;
			int phase = 0;
			PfxPairBatch *batch = arg->stage->contactBatchInfo[l].pairBatches;
			for (PfxUInt32 b = 0; b < arg->stage->contactBatchInfo[l].maxBatches; b++) {
				if (batch->phase > phase) { // phaseが変わっていたら進める
					nSolverJobs++;
					numBatchesInPhase = 0;
					phase = batch->phase;
				}
				batch++;
				numBatchesInPhase++;
			}

			if (numBatchesInPhase > 0) {
				nSolverJobs++;
			}
		}
		numArgs[l] = nSolverJobs - startArgId[l];
	}

	SolverCommonJobArg *solverCommonArgs = context->allocate<SolverCommonJobArg>(nSolverJobs);
	if(!solverCommonArgs) {context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER);return;}

	nSolverJobs = 0;

	for (int l = 0; l < 3; l++) {
		{
			int numBatchesInPhase = 0;
			int phase = 0;
			PfxPairBatch *batch = arg->stage->jointBatchInfo[l].pairBatches;
			for (PfxUInt32 b = 0; b < arg->stage->jointBatchInfo[l].maxBatches; b++) {
				if (batch->phase > phase) { // phaseが変わっていたら進める
					SolverCommonJobArg &solverArg = solverCommonArgs[nSolverJobs++];
					solverArg.stage = arg->stage;
					solverArg.batch = batch - numBatchesInPhase;
					solverArg.numBatches = numBatchesInPhase;
					solverArg.pairIdBuffer = arg->stage->jointBatchInfo[l].pairIdBuffer;
					solverArg.isContact = false;
					numBatchesInPhase = 0;
					// dispatch / barrier
					phase = batch->phase;
				}
				batch++;
				numBatchesInPhase++;
			}

			if (numBatchesInPhase > 0) {
				SolverCommonJobArg &solverArg = solverCommonArgs[nSolverJobs++];
				solverArg.stage = arg->stage;
				solverArg.batch = batch - numBatchesInPhase;
				solverArg.numBatches = numBatchesInPhase;
				solverArg.pairIdBuffer = arg->stage->jointBatchInfo[l].pairIdBuffer;
				solverArg.isContact = false;
				// dispatch / barrier
			}
		}

		{
			int numBatchesInPhase = 0;
			int phase = 0;
			PfxPairBatch *batch = arg->stage->contactBatchInfo[l].pairBatches;
			for (PfxUInt32 b = 0; b < arg->stage->contactBatchInfo[l].maxBatches; b++) {
				if (batch->phase > phase) { // phaseが変わっていたら進める
					SolverCommonJobArg &solverArg = solverCommonArgs[nSolverJobs++];
					solverArg.stage = arg->stage;
					solverArg.batch = batch - numBatchesInPhase;
					solverArg.numBatches = numBatchesInPhase;
					solverArg.pairIdBuffer = arg->stage->contactBatchInfo[l].pairIdBuffer;
					solverArg.isContact = true;
					// dispatch / barrier
					numBatchesInPhase = 0;
					phase = batch->phase;
				}
				batch++;
				numBatchesInPhase++;
			}

			if (numBatchesInPhase > 0) {
				SolverCommonJobArg &solverArg = solverCommonArgs[nSolverJobs++];
				solverArg.stage = arg->stage;
				solverArg.batch = batch - numBatchesInPhase;
				solverArg.numBatches = numBatchesInPhase;
				solverArg.pairIdBuffer = arg->stage->contactBatchInfo[l].pairIdBuffer;
				solverArg.isContact = true;
				// dispatch / barrier
			}
		}
	}

	// warm starting
	for (int i = startArgId[2]; i < startArgId[2] + numArgs[2]; i++) {
		SolverCommonJobArg &solverArg = solverCommonArgs[i];
		if (solverArg.isContact) {
			ret = jobSystem->dispatch(job_contactWarmStarting, &solverArg, solverArg.numBatches, "contact warm starting");
		}
		else {
			ret = jobSystem->dispatch(job_jointWarmStarting, &solverArg, solverArg.numBatches, "joint warm starting");
		}
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
		
		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}

	// Solver iteration
	int iterationCount[3] = { (int)shared->velocitySolverIteration[0], (int)shared->velocitySolverIteration[1], (int)shared->velocitySolverIteration[2] };
	if (iterationCount[0] > iterationCount[1]) { SCE_PFX_SWAP(PfxUInt32, iterationCount[0], iterationCount[1]); }
	if (iterationCount[1] > iterationCount[2]) { SCE_PFX_SWAP(PfxUInt32, iterationCount[1], iterationCount[2]); }
	if (iterationCount[0] > iterationCount[1]) { SCE_PFX_SWAP(PfxUInt32, iterationCount[0], iterationCount[1]); }

	int lv = 0;
	for (int itr = iterationCount[2]; itr > 0; itr--) {
		if (itr == iterationCount[1]) lv++;
		if (itr == iterationCount[0]) lv++;

		for (int i = startArgId[lv]; i < startArgId[lv] + numArgs[lv]; i++) {
			SolverCommonJobArg &solverArg = solverCommonArgs[i];
			if (solverArg.isContact) {
				ret = jobSystem->dispatch(job_contactConstraintSolver, &solverArg, solverArg.numBatches, "contact solver");
			}
			else {
				ret = jobSystem->dispatch(job_jointConstraintSolver, &solverArg, solverArg.numBatches, "joint solver");
			}
			if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
			
			ret = jobSystem->barrier();
			if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
		}
	}

	// apply impulse and integrate (args : 1)
	{
		PfxInt32 numJobs_ = jobSystem->getNumWorkerThreads();

		SetupCommonJobArg &aiarg = arg->stage->applyImpulseArg;
		aiarg.stage = arg->stage;
		aiarg.total = numRigidBodies;
		aiarg.numJobs = numJobs_;

		ret = jobSystem->dispatch(job_applyImpulse, &aiarg, numJobs_, "apply impulse - integrate");
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }

		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}

	// position correction
	for (int i = startArgId[2]; i < startArgId[2] + numArgs[2]; i++) {
		SolverCommonJobArg &solverArg = solverCommonArgs[i];
		if (solverArg.isContact) {
			ret = jobSystem->dispatch(job_contactPositionCorrection, &solverArg, solverArg.numBatches, "contact position correction");
		}
		else {
			ret = jobSystem->dispatch(job_jointPositionCorrection, &solverArg, solverArg.numBatches, "joint position correction");
		}
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }

		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}

	// sleep or wakeup
	{
		int nDeactivationJob = 0;

		// Reset counters (args : 1)
		{
			PfxInt32 numJobs_ = jobSystem->getNumWorkerThreads();

			DeactivationJobArg &darg = arg->stage->deactivationJobArgs[nDeactivationJob++];
			darg.stage = arg->stage;
			darg.total = numRigidBodies;
			darg.numJobs = numJobs_;

			ret = jobSystem->dispatch(job_clearDeactivationCount, &darg, numJobs_, "reset counters");
			if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
		}

		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }

		// count sleep or active (args : 1)
		{
			PfxInt32 numJobs_ = jobSystem->getNumWorkerThreads();

			DeactivationJobArg &darg = arg->stage->deactivationJobArgs[nDeactivationJob++];
			darg.stage = arg->stage;
			darg.sleepCount = shared->sleepCount;
			darg.sleepVelSqr = shared->sleepThreshold * shared->sleepThreshold;
			darg.total = numRigidBodies;
			darg.numJobs = numJobs_;

			ret = jobSystem->dispatch(job_countSleepOrWakeup, &darg, numJobs_, "count sleep or active");
			if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
		}

		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }

		// deactivate rigid bodies (args : 1)
		{
			PfxInt32 numJobs_ = jobSystem->getNumWorkerThreads();

			DeactivationJobArg &darg = arg->stage->deactivationJobArgs[nDeactivationJob++];
			darg.stage = arg->stage;
			darg.total = numRigidBodies;
			darg.numJobs = numJobs_;

			ret = jobSystem->dispatch(job_deactivateStates, &darg, numJobs_, "deactivate rigid bodies");
			if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
		}

		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}
}

void PfxSolveConstraintsStage::job_solveMain(void *data, int uid)
{
	MainJobArg *arg = (MainJobArg*)data;

	PfxJobSystem *jobSystem = arg->stage->getJobSystem();

	SolverSharedArg *shared = &arg->stage->shared;

	PfxContext *context = (PfxContext*)arg->stage->getRigidBodyContext();

	PfxUInt32 numRigidBodies = shared->numRigidBodies;
	PfxUInt32 numContactPairs = *shared->numContactPairsPtr;
	PfxUInt32 numJointPairs = *shared->numJointPairsPtr;
	PfxUInt32 numWorkerThreads = jobSystem->getNumWorkerThreads();

	PfxSolverBody *solverBodies = context->allocate<PfxSolverBody>(numRigidBodies);
	if (!solverBodies) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

	PfxUInt8 *contactDepth = context->allocate<PfxUInt8>(numRigidBodies);
	if (!contactDepth) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

	PfxUInt8 *jointDepth = context->allocate<PfxUInt8>(numRigidBodies);
	if (!jointDepth) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

	shared->solverBodies = solverBodies;
	shared->contactDepth = contactDepth;
	shared->jointDepth = jointDepth;

	// Preparing for splitting
	PfxUInt32 numJointPairsLevel[3] = { 0 };
	PfxUInt32 numContactPairsLevel[3] = { 0 };

	PfxPairSimpleData *contactPairDatemBuff = context->allocate<PfxPairSimpleData>(numContactPairs * 3);
	if (!contactPairDatemBuff) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

	PfxPairSimpleData *jointPairDatemBuff = context->allocate<PfxPairSimpleData>(numJointPairs * 3);
	if (!jointPairDatemBuff) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

	PfxPairSimpleData *jointPairDatem[3] = { jointPairDatemBuff, jointPairDatemBuff + numJointPairs, jointPairDatemBuff + numJointPairs * 2 };
	PfxPairSimpleData *contactPairDatem[3] = { contactPairDatemBuff, contactPairDatemBuff + numContactPairs, contactPairDatemBuff + numContactPairs * 2 };

	// Find the fixed rigid bodies
	PfxBool *isRigidBodyMovable = context->allocate<PfxBool>( numRigidBodies );
	if( !isRigidBodyMovable ) { context->appendError( PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER ); return; }

	for( PfxUInt32 i = 0; i < numRigidBodies; ++i ){
		isRigidBodyMovable[ i ] = ( SCE_PFX_MOTION_MASK_DYNAMIC( shared->states[ i ].getMotionType() )&SCE_PFX_MOTION_MASK_TYPE );
	}

	// gather pairs to each level
	gatherContactPairsToEachLevel(numContactPairsLevel, contactPairDatem, shared->contactPairs, numContactPairs, isRigidBodyMovable );
	gatherJointPairsToEachLevel(numJointPairsLevel, jointPairDatem, shared->jointPairs, shared->joints, numJointPairs, isRigidBodyMovable );

	// Pair Batches
	PfxUInt32 maxContactBatches = SCE_PFX_MAX(64, numContactPairs / 8);
	PfxUInt32 maxJointBatches = SCE_PFX_MAX(64, numJointPairs / 8);

	auto initBatchInfo = [&](PfxBatchInfo &batchInfo, int maxBatches, int numPairs)
	{
		batchInfo.pairBatches = context->allocate<PfxPairBatch>(maxBatches);
		batchInfo.pairIdBuffer = context->allocate<PfxUInt32>(numPairs);
		if (!batchInfo.pairBatches || !batchInfo.pairIdBuffer) return false;

		batchInfo.numValidPairs = 0;
		batchInfo.maxBatches = 0;
		batchInfo.maxPhases = 0;

		return true;
	};

	if (!initBatchInfo(arg->stage->contactBatchInfo[0], maxContactBatches, numContactPairsLevel[0])) {
		context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return;
	}
	if (!initBatchInfo(arg->stage->contactBatchInfo[1], maxContactBatches, numContactPairsLevel[1])) {
		context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return;
	}
	if (!initBatchInfo(arg->stage->contactBatchInfo[2], maxContactBatches, numContactPairsLevel[2])) {
		context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return;
	}
	if (!initBatchInfo(arg->stage->jointBatchInfo[0], maxJointBatches, numJointPairsLevel[0])) {
		context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return;
	}
	if (!initBatchInfo(arg->stage->jointBatchInfo[1], maxJointBatches, numJointPairsLevel[1])) {
		context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return;
	}
	if (!initBatchInfo(arg->stage->jointBatchInfo[2], maxJointBatches, numJointPairsLevel[2])) {
		context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return;
	}

	PfxInt32 ret = SCE_OK;
	
	// Split pairs (args : 6)
	{
		PfxSplitPairsArg *splitPairsArg = context->allocate<PfxSplitPairsArg>(6);
		if (!splitPairsArg) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

		PfxBool *isRigidBodyJointRoot = context->allocate<PfxBool>(numRigidBodies);
		if (!isRigidBodyJointRoot) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

		PfxUInt32 maxContactPairsPerBatch[3];
		PfxUInt32 maxJointPairsPerBatch[3];

		PfxUInt32 contactPairTaskQueueBytes[3];
		PfxUInt32 jointPairTaskQueueBytes[3];

		void *contactPairTaskQueueBuff[3] = { nullptr,nullptr,nullptr };
		void *jointPairTaskQueueBuff[3] = { nullptr,nullptr,nullptr };

		auto queryWorkBytesOfSplitPairsPostProcess = [](PfxUInt32 numValidPairs, PfxUInt32 maxNumPairs, PfxUInt32 numRigidBodies, PfxUInt32 numWorkThreads)
		{
			PfxUInt32 maxPairsPerBatch = SCE_PFX_CLAMP((numValidPairs / (SCE_PFX_MAX(numWorkThreads, 1) * 4)), 8, 128);
			return	SCE_PFX_BYTES_ALIGN16(sizeof(PfxUInt32) * (2574u + 6u * numValidPairs + 2u * maxNumPairs + 2u * numRigidBodies + 2u * SCE_PFX_MAX(numValidPairs, maxPairsPerBatch)) +
				sizeof(PfxInt32)  * (6u * numRigidBodies));
		};

		for (PfxUInt32 i = 0; i < 3; ++i) {
			maxContactPairsPerBatch[i] = SCE_PFX_CLAMP((numContactPairsLevel[i] / (numWorkerThreads * 4)), 8, 128);
			maxJointPairsPerBatch[i] = SCE_PFX_CLAMP((numJointPairsLevel[i] / (numWorkerThreads * 4)), 8, 128);

			contactPairTaskQueueBytes[i] = queryWorkBytesOfSplitPairsPostProcess(numContactPairsLevel[i], numContactPairs, numRigidBodies, numWorkerThreads);
			jointPairTaskQueueBytes[i] = queryWorkBytesOfSplitPairsPostProcess(numJointPairsLevel[i], numJointPairs, numRigidBodies, numWorkerThreads);

			contactPairTaskQueueBuff[i] = context->allocateBytes(contactPairTaskQueueBytes[i]);
			if (!contactPairTaskQueueBuff[i]) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

			jointPairTaskQueueBuff[i] = context->allocateBytes(jointPairTaskQueueBytes[i]);
			if (!jointPairTaskQueueBuff[i]) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }
		}

		for (int i = 0; i < 3; i++) {
			splitPairsArg[i].maxPairsPerBatch = maxContactPairsPerBatch[i];
			splitPairsArg[i].batchInfo = &arg->stage->contactBatchInfo[i];
			splitPairsArg[i].workBuff = contactPairTaskQueueBuff[i];
			splitPairsArg[i].workBytes = contactPairTaskQueueBytes[i];
			splitPairsArg[i].pairs = shared->contactPairs;
			splitPairsArg[i].numPairs = numContactPairs;
			splitPairsArg[i].pairDatem = contactPairDatem[i];
			splitPairsArg[i].numPairDatem = numContactPairsLevel[i];
			splitPairsArg[i].states = shared->states;
			splitPairsArg[i].numRigidBodies = numRigidBodies;
			splitPairsArg[i].makeConstraintDepth = false;
			splitPairsArg[i].makeContactDepth = false;
			splitPairsArg[i].makeJointDepth = false;
			splitPairsArg[i].isRigidBodyMovable = isRigidBodyMovable;
			splitPairsArg[i].isRigidBodyJointRoot = isRigidBodyJointRoot;
			splitPairsArg[i].constraintDepth = contactDepth;
			splitPairsArg[i].numWorkThreads = SCE_PFX_MAX(numWorkerThreads, 1u);
			splitPairsArg[i].enableSortOfPairs = shared->enableSortOfPairs;
			splitPairsArg[i + 3].maxPairsPerBatch = maxJointPairsPerBatch[i];
			splitPairsArg[i + 3].batchInfo = &arg->stage->jointBatchInfo[i];
			splitPairsArg[i + 3].workBuff = jointPairTaskQueueBuff[i];
			splitPairsArg[i + 3].workBytes = jointPairTaskQueueBytes[i];
			splitPairsArg[i + 3].pairs = shared->jointPairs;
			splitPairsArg[i + 3].numPairs = numJointPairs;
			splitPairsArg[i + 3].pairDatem = jointPairDatem[i];
			splitPairsArg[i + 3].numPairDatem = numJointPairsLevel[i];
			splitPairsArg[i + 3].states = shared->states;
			splitPairsArg[i + 3].numRigidBodies = numRigidBodies;
			splitPairsArg[i + 3].makeConstraintDepth = false;
			splitPairsArg[i + 3].makeContactDepth = false;
			splitPairsArg[i + 3].makeJointDepth = false;
			splitPairsArg[i + 3].isRigidBodyMovable = isRigidBodyMovable;
			splitPairsArg[i + 3].isRigidBodyJointRoot = isRigidBodyJointRoot;
			splitPairsArg[i + 3].numWorkThreads = SCE_PFX_MAX(numWorkerThreads, 1u);
			splitPairsArg[i + 3].constraintDepth = (shared->enableSortOfPairs ? jointDepth : contactDepth);
			splitPairsArg[i + 3].enableSortOfPairs = shared->enableSortOfPairs;
		}

		// Pre-process for splitting the pairs
		preProcessOfSplitContactPairs(splitPairsArg[2]);
		if (shared->enableSortOfPairs)	preProcessOfSplitJointPairs(splitPairsArg[5]);

		ret = jobSystem->dispatch(job_splitPairs, splitPairsArg, 6, "split pairs");
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}

	SetupCommonJobArg *setupCommonArgs = context->allocate<SetupCommonJobArg>(4);
	if (!setupCommonArgs) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, SCE_PFX_ERR_OUT_OF_BUFFER); return; }

	int njob = 0;

	// setup solver bodies (args : 1)
	{
		PfxInt32 numJobs_ = jobSystem->getNumWorkerThreads();

		SetupCommonJobArg &scarg = setupCommonArgs[njob++];
		scarg.stage = arg->stage;
		scarg.total = numRigidBodies;
		scarg.numJobs = numJobs_;

		ret = jobSystem->dispatch(job_setupSolverBodies, &scarg, numJobs_, "setup solver bodies");
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}

	ret = jobSystem->barrier();
	if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }

	// setup contact constraints (args : 1)
	{
		PfxInt32 numJobs_ = jobSystem->getNumWorkerThreads();

		SetupCommonJobArg &scarg = setupCommonArgs[njob++];
		scarg.stage = arg->stage;
		scarg.total = numContactPairs;
		scarg.numJobs = numJobs_;

		ret = jobSystem->dispatch(job_setupContactConstraints, &scarg, numJobs_, "setup contact constraints");
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}

	// setup joint constraints (args : 1)
	{
		PfxInt32 numJobs_ = jobSystem->getNumWorkerThreads();

		SetupCommonJobArg &scarg = setupCommonArgs[njob++];
		scarg.stage = arg->stage;
		scarg.total = numJointPairs;
		scarg.numJobs = numJobs_;

		ret = jobSystem->dispatch(job_setupJointConstraints, &scarg, numJobs_, "setup joint constraints");
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}

	if (arg->isBlocking) {
		jobSystem->wait();
		job_solveConstraintsSub(arg, 0);
	}
	else {
		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }

		// dispatch solver sub (args : 1)
		ret = jobSystem->dispatch(job_solveConstraintsSub, arg, "solverSub");
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }

		ret = jobSystem->barrier();
		if (ret != SCE_OK) { context->appendError(PfxContext::kStageSolveConstraints, arg->dispatchId, ret); return; }
	}
}

PfxInt32 PfxSolveConstraintsStage::solve(PfxContactComplex &contactComplex, SolverParam &param, PfxBool isBlocking)
{
	mainJobArg.stage = this;
	mainJobArg.dispatchId = getRigidBodyContext()->getDispatchId();
	mainJobArg.isBlocking = isBlocking;

	shared.joints = param.joints;
	shared.numContactPairsPtr = param.numContactPairsPtr;
	shared.numJointPairsPtr = param.numJointPairsPtr;
	shared.contactManager = (PfxContactManager*)&contactComplex.contactManager;

	shared.contactPairs = param.contactPairs;
	shared.jointPairs = param.jointPairs;
	shared.timeStepRatio = param.timeStepRatio;
	shared.contactBias = param.contactBias;
	shared.enableSortOfPairs = param.enableSortOfPairs;
	shared.enableLargePosition = param.enableLargePosition;
	shared.massAmpInvForContactPairs = 1.0f / SCE_PFX_MAX(param.massAmpForContacts, 1.0f);
	shared.massAmpInvForJointPairs = 1.f / SCE_PFX_MAX(param.massAmpForJoints, 1.f);
	shared.enableLargePosition = getRigidBodyContext()->getEnableLargePosition();
	shared.velocitySolverIteration[0] = param.velocitySolverIteration[0];
	shared.velocitySolverIteration[1] = param.velocitySolverIteration[1];
	shared.velocitySolverIteration[2] = param.velocitySolverIteration[2];
	shared.positionSolverIteration = param.positionSolverIteration;
	shared.sleepCount = param.sleepCount;
	shared.sleepThreshold = param.sleepThreshold;

	if (isBlocking) {
		job_solveMain(&mainJobArg, 0);
	}
	else {
		PfxInt32 ret = getJobSystem()->dispatch(job_solveMain, &mainJobArg, "solverMain");
		if (ret != SCE_OK) return ret;
	}

	return SCE_PFX_OK;
}

PfxInt32 PfxSolveConstraintsStage::preparePipeline(PfxFloat timeStep, PfxInt32 *roots, 
	PfxRigidState *states, PfxRigidBody *bodies, PfxUInt32 numRigidBodies)
{
	if(timeStep <= 0.0f || !roots || !states || !bodies) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	shared.timeStep = timeStep;
	shared.roots = roots;
	shared.states = states;
	shared.bodies = bodies;
	shared.numRigidBodies = numRigidBodies;

	deactivationCounts = getRigidBodyContext()->allocate<DeactivationCount>(numRigidBodies);
	if(!deactivationCounts) return SCE_PFX_ERR_OUT_OF_BUFFER;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
