/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_distance.h"
#include "pfx_constraint_row_solver.h"

namespace sce {
namespace pfxv4 {

static PfxInt32 pfxInitializeDistanceJointCommon(PfxJoint &joint,
	const PfxRigidState &stateA, const PfxRigidState &stateB,
	float damping, float distance)
{
	joint.m_active = 1;
	joint.m_numConstraints = 6;

	joint.m_userData = 0;
	joint.m_type = kPfxJointDistance;
	joint.m_rigidBodyIdA = stateA.getRigidBodyId();
	joint.m_rigidBodyIdB = stateB.getRigidBodyId();

	for (int i = 0; i<joint.m_numConstraints; i++) {
		joint.m_constraints[i].reset();
		joint.m_constraints[i].m_positionBias = 0.0;
	}

	joint.m_constraints[0].m_damping = damping;
	joint.m_constraints[1].m_damping = damping;
	joint.m_constraints[2].m_damping = damping;

	joint.m_constraints[0].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[1].m_lock = SCE_PFX_JOINT_LOCK_FREE;
	joint.m_constraints[2].m_lock = SCE_PFX_JOINT_LOCK_FREE;
	joint.m_constraints[3].m_lock = SCE_PFX_JOINT_LOCK_FREE;
	joint.m_constraints[4].m_lock = SCE_PFX_JOINT_LOCK_FREE;
	joint.m_constraints[5].m_lock = SCE_PFX_JOINT_LOCK_FREE;

	if (distance < 0.0f) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}

	// Note : Store the maximum distance value here
	joint.m_constraints[0].m_movableLowerLimit = -distance;
	joint.m_constraints[0].m_movableUpperLimit = distance;
	joint.m_constraints[1].m_damping = damping;
	joint.m_constraints[2].m_damping = damping;

	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeDistanceJoint(PfxJoint &joint,
	const PfxRigidState &stateA,const PfxRigidState &stateB,
	const PfxDistanceJointInitParam &param)
{
	PfxInt32 ret = pfxInitializeDistanceJointCommon(joint, stateA, stateB, param.damping, param.distance);
	if (ret != SCE_PFX_OK)
		return ret;

	PfxMatrix3 rotA = transpose(PfxMatrix3(stateA.getOrientation()));
	PfxMatrix3 rotB = transpose(PfxMatrix3(stateB.getOrientation()));
	
	joint.m_anchorA = rotA * (param.anchorPoint - stateA.getLargePosition()).convertToVector3();
	joint.m_anchorB = rotB * (param.anchorPoint - stateB.getLargePosition()).convertToVector3();
	
	PfxMatrix3 frame = PfxMatrix3::identity();
	if(param.enableJointFrame && pfxIsOrthogonal(param.jointFrame)) {
		frame = param.jointFrame;
	}
	
	joint.m_frameA = rotA * frame;
	joint.m_frameB = rotB * frame;
	
	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeDistanceJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxDistanceJointInitParamLocalSpace &param)
{
	PfxInt32 ret = pfxInitializeDistanceJointCommon(joint, stateA, stateB, param.damping, param.distance);
	if (ret != SCE_PFX_OK)
		return ret;

	joint.m_anchorA = param.anchorPointA;
	joint.m_anchorB = param.anchorPointB;
	joint.m_frameA = param.jointFrameA;
	joint.m_frameB = param.jointFrameB;

	return SCE_PFX_OK;
}

void pfxSetupDistanceJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation,joint.m_anchorA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation,joint.m_anchorB);
	
	PfxVector3 vA = stateA.getLinearVelocity() + cross(stateA.getAngularVelocity(),rA);
	PfxVector3 vB = stateB.getLinearVelocity() + cross(stateB.getAngularVelocity(),rB);
	PfxVector3 vAB = vA-vB;

	PfxMatrix3 worldFrameA;
	worldFrameA = PfxMatrix3(solverBodyA.m_orientation) * joint.m_frameA;

	PfxVector3 distance = (stateA.getLargePosition() - stateB.getLargePosition()).convertToVector3() + rA - rB;

	PfxFloat len = lengthSqr(distance);
	if (len > 1e-8f) {
		PfxMatrix3 rot = PfxMatrix3::rotation(PfxQuat::rotation(worldFrameA[0], normalize(distance)));
		worldFrameA = rot * worldFrameA;
	}

	// Linear Constraint
	PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(solverBodyA.m_massInv + solverBodyB.m_massInv)) - 
			crossMatrix(rA) * solverBodyA.m_inertiaInv * crossMatrix(rA) - 
			crossMatrix(rB) * solverBodyB.m_inertiaInv * crossMatrix(rB);
	
	for (int c = 0; c<3; c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];
		PfxConstraintRow &constraint = joint.m_constraints[c].m_constraintRow;
		
		PfxVector3 normal = distance;

		normal = worldFrameA[c];
		
		PfxFloat posErr = dot(distance, -normal);
		PfxFloat lowerLimit = -jointConstraint.m_maxImpulse;
		PfxFloat upperLimit = jointConstraint.m_maxImpulse;
		PfxFloat velocityAmp = 1.0f;
		
		pfxCalcLinearLimit(jointConstraint, posErr, velocityAmp, lowerLimit, upperLimit);

		jointConstraint.m_lowerLimit = lowerLimit;
		jointConstraint.m_upperLimit = upperLimit;

		PfxFloat bias = pfxModifyLinearBias(fabsf(posErr),jointConstraint.m_velocityBias);
		
		PfxFloat denom = dot(K*normal, normal) + SCE_PFX_CONSTRAINT_STABILIZE_FACTOR;
		PfxFloat jacDiagInv = velocityAmp/denom;
		if(fabsf(denom - jointConstraint.m_jacobian) > SCE_PFX_JOINT_WARM_STARTING_LINEAR_THRESHOLD) {//} || jointConstraint.m_warmStarting != 1) {
			constraint.m_accumImpulse = 0.0f;
		}
		
		constraint.m_rhs = -dot(vAB,normal);
		constraint.m_rhs -= (bias * (-posErr)) / timeStep;
		constraint.m_rhs *= jacDiagInv;
		constraint.m_jacDiagInv = jacDiagInv;
		constraint.setNormal(normal);
		jointConstraint.m_jacobian = denom;
	}
}

void pfxWarmStartDistanceJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation,joint.m_anchorA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation, joint.m_anchorB);

	// Linear Constraint
	for (int c = 0; c<3; c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];
		PfxConstraintRow &constraint = jointConstraint.m_constraintRow;

		PfxVector3 normal = constraint.getNormal();
		PfxFloat deltaImpulse = constraint.m_accumImpulse;
		solverBodyA.m_deltaLinearVelocity += deltaImpulse * solverBodyA.m_massInv * normal;
		solverBodyA.m_deltaAngularVelocity += deltaImpulse * solverBodyA.m_inertiaInv * cross(rA, normal);
		solverBodyB.m_deltaLinearVelocity -= deltaImpulse * solverBodyB.m_massInv * normal;
		solverBodyB.m_deltaAngularVelocity -= deltaImpulse * solverBodyB.m_inertiaInv * cross(rB, normal);
	}
}

void pfxSolveDistanceJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation,joint.m_anchorA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation,joint.m_anchorB);

	// Linear Constraint
	pfxSolveLinearConstraintRow(joint.m_constraints[0].m_constraintRow,
		joint.m_constraints[0].m_lowerLimit, joint.m_constraints[0].m_upperLimit,
		solverBodyA.m_deltaLinearVelocity, solverBodyA.m_deltaAngularVelocity, solverBodyA.m_massInv, solverBodyA.m_inertiaInv, rA,
		solverBodyB.m_deltaLinearVelocity, solverBodyB.m_deltaAngularVelocity, solverBodyB.m_massInv, solverBodyB.m_inertiaInv, rB);

	pfxSolveLinearConstraintRow(joint.m_constraints[1].m_constraintRow,
		joint.m_constraints[1].m_lowerLimit, joint.m_constraints[1].m_upperLimit,
		solverBodyA.m_deltaLinearVelocity, solverBodyA.m_deltaAngularVelocity, solverBodyA.m_massInv, solverBodyA.m_inertiaInv, rA,
		solverBodyB.m_deltaLinearVelocity, solverBodyB.m_deltaAngularVelocity, solverBodyB.m_massInv, solverBodyB.m_inertiaInv, rB);

	pfxSolveLinearConstraintRow(joint.m_constraints[2].m_constraintRow,
		joint.m_constraints[2].m_lowerLimit, joint.m_constraints[2].m_upperLimit,
		solverBodyA.m_deltaLinearVelocity, solverBodyA.m_deltaAngularVelocity, solverBodyA.m_massInv, solverBodyA.m_inertiaInv, rA,
		solverBodyB.m_deltaLinearVelocity, solverBodyB.m_deltaAngularVelocity, solverBodyB.m_massInv, solverBodyB.m_inertiaInv, rB);
}

void pfxCorrectPositionDistanceJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation, joint.m_anchorA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation, joint.m_anchorB);

	PfxVector3 distance = solverBodyA.m_deltaLinearVelocity + rA - solverBodyB.m_deltaLinearVelocity - rB;// Note : use m_deltaLinearVelocity as position

	PfxMatrix3 worldFrameA;
	worldFrameA = PfxMatrix3(solverBodyA.m_orientation) * joint.m_frameA;

	// Linear Constraint
	PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(solverBodyA.m_massInv + solverBodyB.m_massInv)) -
		crossMatrix(rA) * solverBodyA.m_inertiaInv * crossMatrix(rA) -
		crossMatrix(rB) * solverBodyB.m_inertiaInv * crossMatrix(rB);

	{
		PfxJointConstraint &jointConstraint = joint.m_constraints[0];

		PfxVector3 normal = distance;

		PfxFloat len = length(distance);
		if(len < 1e-5f) {
			normal = worldFrameA[0];
		}
		else {
			normal /= len;
		}

		PfxFloat posErr = dot(distance, -normal);

		pfxCalcLinearLimit(jointConstraint, posErr);

		PfxFloat denom = dot(K*normal, normal);
		PfxFloat impulse = jointConstraint.m_positionBias * posErr / denom;
		
		solverBodyA.m_deltaLinearVelocity += impulse * solverBodyA.m_massInv * normal;// Note : use m_deltaLinearVelocity as position
		solverBodyB.m_deltaLinearVelocity -= impulse * solverBodyB.m_massInv * normal;// Note : use m_deltaLinearVelocity as position

		PfxVector3 dOriA = impulse * solverBodyA.m_inertiaInv * cross(rA, normal);
		solverBodyA.m_orientation = pfxAddRotation(solverBodyA.m_orientation, dOriA);
		
		PfxVector3 dOriB = -impulse * solverBodyB.m_inertiaInv * cross(rB, normal);
		solverBodyB.m_orientation = pfxAddRotation(solverBodyB.m_orientation, dOriB);
	}
}

} //namespace pfxv4
} //namespace sce
