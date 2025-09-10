/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/low_level/solver/pfx_constraint_solver.h"
#include "../rigidbody/pfx_context.h"
#include "../collision/pfx_contact_complex.h"

namespace sce {
namespace pfxv4 {

PfxInt32 pfxCheckParamOfSolveConstraints(const PfxSolveConstraintsParam &param)
{
	if (!param.numContactPairsPtr || !param.numJointPairsPtr || !param.contactContainer) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}
	bool contactExists = *param.numContactPairsPtr > 0;
	bool jointExists = *param.numJointPairsPtr > 0;
	if ((contactExists && (!param.contactPairs)) || (jointExists && (!param.jointPairs || !param.joints))) {
		return SCE_PFX_ERR_INVALID_VALUE;
	}

	if ((contactExists && !SCE_PFX_PTR_IS_ALIGNED16(param.contactPairs)) ||
		(jointExists && (!SCE_PFX_PTR_IS_ALIGNED16(param.jointPairs) || !SCE_PFX_PTR_IS_ALIGNED16(param.joints)))) {
		return SCE_PFX_ERR_INVALID_ALIGN;
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxSolveConstraints(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam, PfxSolveConstraintsParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	PfxInt32 ret = pfxCheckSharedParam(sharedParam);
	if(ret != SCE_PFX_OK) return ret;

	ret = pfxCheckParamOfSolveConstraints(param);
	if(ret != SCE_PFX_OK) return ret;
	
	PfxSolveConstraintsStage::SolverParam solverParam;

	// Solver parameters
	solverParam.joints = param.joints;
	solverParam.contactPairs = param.contactPairs;
	solverParam.numContactPairsPtr = param.numContactPairsPtr;
	solverParam.jointPairs = param.jointPairs;
	solverParam.numJointPairsPtr = param.numJointPairsPtr;
	solverParam.velocitySolverIteration[0] = param.velocitySolverIteration[0];
	solverParam.velocitySolverIteration[1] = param.velocitySolverIteration[1];
	solverParam.velocitySolverIteration[2] = param.velocitySolverIteration[2];
	solverParam.positionSolverIteration = param.positionSolverIteration;
	solverParam.enableSortOfPairs = param.enableSortOfPairs;
	solverParam.timeStepRatio = param.timeStepRatio;
	solverParam.contactBias = param.contactBias;
	solverParam.massAmpForContacts = param.massAmpForContacts;
	solverParam.massAmpForJoints = param.massAmpForJoints;
	solverParam.sleepCount = param.sleepCount;
	solverParam.sleepThreshold = param.sleepThreshold;

	ret = context_->solveConstraints(*(PfxContactComplex*)param.contactContainer, solverParam,
		sharedParam.timeStep, sharedParam.states, sharedParam.bodies, sharedParam.numRigidBodies, sharedParam.maxContacts);
	return ret;
}

PfxInt32 pfxDispatchSolveConstraints(PfxRigidBodyContext &context, PfxSolveConstraintsParam &param)
{
	PfxContext *context_ = (PfxContext*)&context;
	
	PfxInt32 ret = pfxCheckParamOfSolveConstraints(param);
	if(ret != SCE_PFX_OK) return ret;
	
	PfxSolveConstraintsStage::SolverParam solverParam;

	// Solver parameters
	solverParam.joints = param.joints;
	solverParam.contactPairs = param.contactPairs;
	solverParam.numContactPairsPtr = param.numContactPairsPtr;
	solverParam.jointPairs = param.jointPairs;
	solverParam.numJointPairsPtr = param.numJointPairsPtr;
	solverParam.velocitySolverIteration[0] = param.velocitySolverIteration[0];
	solverParam.velocitySolverIteration[1] = param.velocitySolverIteration[1];
	solverParam.velocitySolverIteration[2] = param.velocitySolverIteration[2];
	solverParam.positionSolverIteration = param.positionSolverIteration;
	solverParam.enableSortOfPairs = param.enableSortOfPairs;
	solverParam.timeStepRatio = param.timeStepRatio;
	solverParam.contactBias = param.contactBias;
	solverParam.massAmpForContacts = param.massAmpForContacts;
	solverParam.massAmpForJoints = param.massAmpForJoints;
	solverParam.sleepCount = param.sleepCount;
	solverParam.sleepThreshold = param.sleepThreshold;

	ret = context_->dispatchSolveConstraints(*(PfxContactComplex*)param.contactContainer, solverParam);
	return ret;
}


} //namespace pfxv4
} //namespace sce
