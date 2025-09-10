/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_ball.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_swing_twist.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_hinge.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_slider.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_fix.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_universal.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_constraint_func.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Setup Joint Constraint Function Table

void setupJointConstraintDummy(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep)
{
	(void)joint,(void)stateA,(void)stateB,(void)solverBodyA,(void)solverBodyB,(void)timeStep;
}

void pfxSetupBallJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep);

void pfxWarmStartBallJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxSolveBallJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxSetupHingeJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep
	);

void pfxSetupSwingTwistJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep);

void pfxWarmStartSwingTwistJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxSolveSwingTwistJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxCorrectPositionBallJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxCorrectPositionHingeJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxCorrectPositionSwingTwistJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxCorrectPositionUniversalJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxCorrectPositionMotorJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxCorrectPositionShoulderJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxCorrectPositionDistanceJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxSetupUniversalJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep);

void pfxSetupMotorJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep);

void pfxSetupShoulderJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep);

void pfxSetupDistanceJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep);

void pfxWarmStartDistanceJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

void pfxSolveDistanceJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

PfxSetupJointConstraintFunc funcTbl_setupJointConstraint[kPfxJointCount] = {
	pfxSetupBallJoint,
	pfxSetupSwingTwistJoint,
	pfxSetupHingeJoint,
	pfxSetupSwingTwistJoint,
	pfxSetupSwingTwistJoint,
	pfxSetupUniversalJoint,
	pfxSetupMotorJoint,
	pfxSetupShoulderJoint,
	pfxSetupDistanceJoint,
	setupJointConstraintDummy,
	setupJointConstraintDummy,
	setupJointConstraintDummy,
	setupJointConstraintDummy,
	setupJointConstraintDummy,
	setupJointConstraintDummy,
};

///////////////////////////////////////////////////////////////////////////////
// Setup Joint Constraint Function Table Interface

PfxSetupJointConstraintFunc pfxGetSetupJointConstraintFunc(PfxUInt8 jointType)
{
	SCE_PFX_ASSERT(jointType<kPfxJointCount);
	return funcTbl_setupJointConstraint[jointType];
}

PfxInt32 pfxSetSetupJointConstraintFunc(PfxUInt8 jointType,PfxSetupJointConstraintFunc func)
{
	if(jointType >= kPfxJointCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	funcTbl_setupJointConstraint[jointType] = func;
	
	return SCE_PFX_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Warm Start Joint Constraint Function Table

void warmStartJointConstraintDummy(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB)
{
	(void)joint,(void)solverBodyA,(void)solverBodyB;
}

PfxWarmStartJointConstraintFunc funcTbl_warmStartJointConstraint[kPfxJointCount] = {
	pfxWarmStartBallJoint,
	pfxWarmStartSwingTwistJoint,
	pfxWarmStartSwingTwistJoint,
	pfxWarmStartSwingTwistJoint,
	pfxWarmStartSwingTwistJoint,
	pfxWarmStartSwingTwistJoint,
	pfxWarmStartSwingTwistJoint,
	pfxWarmStartSwingTwistJoint,
	pfxWarmStartDistanceJoint,
	warmStartJointConstraintDummy,
	warmStartJointConstraintDummy,
	warmStartJointConstraintDummy,
	warmStartJointConstraintDummy,
	warmStartJointConstraintDummy,
	warmStartJointConstraintDummy,
};

///////////////////////////////////////////////////////////////////////////////
// Warm Start Joint Constraint Function Table Interface

PfxWarmStartJointConstraintFunc pfxGetWarmStartJointConstraintFunc(PfxUInt8 jointType)
{
	SCE_PFX_ASSERT(jointType<kPfxJointCount);
	return funcTbl_warmStartJointConstraint[jointType];
}

PfxInt32 pfxSetWarmStartJointConstraintFunc(PfxUInt8 jointType,PfxWarmStartJointConstraintFunc func)
{
	if(jointType >= kPfxJointCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	funcTbl_warmStartJointConstraint[jointType] = func;
	
	return SCE_PFX_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Solve Joint Constraint Function Table

void solveJointConstraintDummy(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB)
{
	(void)joint,(void)solverBodyA,(void)solverBodyB;
}

PfxSolveJointConstraintFunc funcTbl_solveJointConstraint[kPfxJointCount] = {
	pfxSolveBallJoint,
	pfxSolveSwingTwistJoint,
	pfxSolveSwingTwistJoint,
	pfxSolveSwingTwistJoint,
	pfxSolveSwingTwistJoint,
	pfxSolveSwingTwistJoint,
	pfxSolveSwingTwistJoint,
	pfxSolveSwingTwistJoint,
	pfxSolveDistanceJoint,
	solveJointConstraintDummy,
	solveJointConstraintDummy,
	solveJointConstraintDummy,
	solveJointConstraintDummy,
	solveJointConstraintDummy,
	solveJointConstraintDummy,
};

///////////////////////////////////////////////////////////////////////////////
// Solve Joint Constraint Function Table Interface

PfxSolveJointConstraintFunc pfxGetSolveJointConstraintFunc(PfxUInt8 jointType)
{
	SCE_PFX_ASSERT(jointType<kPfxJointCount);
	return funcTbl_solveJointConstraint[jointType];
}

PfxInt32 pfxSetSolveJointConstraintFunc(PfxUInt8 jointType,PfxSolveJointConstraintFunc func)
{
	if(jointType >= kPfxJointCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	funcTbl_solveJointConstraint[jointType] = func;
	
	return SCE_PFX_OK;
}


///////////////////////////////////////////////////////////////////////////////
// Correct Joint Position Constraint Function Table

void correctJointPositionConstraintDummy(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB)
{
	(void)joint, (void)solverBodyA, (void)solverBodyB;
}

PfxCorrectJointPositionConstraintFunc funcTbl_correctJointPositionConstraint[kPfxJointCount] = {
	pfxCorrectPositionBallJoint,
	pfxCorrectPositionSwingTwistJoint,
	pfxCorrectPositionHingeJoint,
	pfxCorrectPositionSwingTwistJoint,
	pfxCorrectPositionSwingTwistJoint,
	pfxCorrectPositionUniversalJoint,
	pfxCorrectPositionMotorJoint,
	pfxCorrectPositionShoulderJoint,
	pfxCorrectPositionDistanceJoint,
	correctJointPositionConstraintDummy,
	correctJointPositionConstraintDummy,
	correctJointPositionConstraintDummy,
	correctJointPositionConstraintDummy,
	correctJointPositionConstraintDummy,
	correctJointPositionConstraintDummy,
};

///////////////////////////////////////////////////////////////////////////////
// Correct Joint Position Constraint Function Table Interface

PfxCorrectJointPositionConstraintFunc pfxGetCorrectJointPositionConstraintFunc(PfxUInt8 jointType)
{
	SCE_PFX_ASSERT(jointType<kPfxJointCount);
	return funcTbl_correctJointPositionConstraint[jointType];
}

PfxInt32 pfxSetCorrectJointPositionConstraintFunc(PfxUInt8 jointType, PfxCorrectJointPositionConstraintFunc func)
{
	if (jointType >= kPfxJointCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}

	funcTbl_correctJointPositionConstraint[jointType] = func;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
