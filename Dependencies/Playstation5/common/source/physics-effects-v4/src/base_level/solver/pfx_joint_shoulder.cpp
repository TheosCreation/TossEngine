/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_shoulder.h"
#include "pfx_constraint_row_solver.h"


namespace sce {
namespace pfxv4 {

PfxInt32 pfxInitializeShoulderJointCommon(PfxJoint &joint,
	const PfxRigidState &stateA,const PfxRigidState &stateB,
	float bias, float damping, 
	float twistLowerAngle, float twistUpperAngle,
	float swing1UpperAngle, float swing2UpperAngle )
{
	joint.m_active = 1;
	joint.m_numConstraints = 6;
	joint.m_userData = 0;
	joint.m_type = kPfxJointShoulder;
	joint.m_rigidBodyIdA = stateA.getRigidBodyId();
	joint.m_rigidBodyIdB = stateB.getRigidBodyId();
	
	for(int i=0;i<6;i++) {
		joint.m_constraints[i].reset();
		joint.m_constraints[i].m_positionBias = 0.0f;
	}
	
	joint.m_constraints[0].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	joint.m_constraints[1].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	joint.m_constraints[2].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	joint.m_constraints[3].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[4].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[5].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;

	joint.m_constraints[3].m_damping = damping;
	joint.m_constraints[3].m_velocityBias = bias;
	joint.m_constraints[4].m_damping = damping;
	joint.m_constraints[4].m_velocityBias = bias;
	joint.m_constraints[5].m_damping = damping;
	joint.m_constraints[5].m_velocityBias = bias;
	
	// Set twist angle limit
	if(twistLowerAngle > twistUpperAngle || 
		!SCE_PFX_RANGE_CHECK(twistLowerAngle,-SCE_PFX_PI-0.01f,SCE_PFX_PI+0.01f) || 
		!SCE_PFX_RANGE_CHECK(twistUpperAngle,-SCE_PFX_PI-0.01f,SCE_PFX_PI+0.01f)) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	joint.m_constraints[3].m_movableLowerLimit = twistLowerAngle;
	joint.m_constraints[3].m_movableUpperLimit = twistUpperAngle;
	
	// Set swing angle limit
	if( !SCE_PFX_RANGE_CHECK(swing1UpperAngle,0.0f,SCE_PFX_PI+0.01f)) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	if( !SCE_PFX_RANGE_CHECK(swing2UpperAngle,0.0f,SCE_PFX_PI+0.01f)) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	joint.m_constraints[4].m_movableLowerLimit = 0.0f;
	joint.m_constraints[4].m_movableUpperLimit = SCE_PFX_MAX(swing1UpperAngle, swing2UpperAngle);
	joint.m_constraints[5].m_movableLowerLimit = 0.0f;
	joint.m_constraints[5].m_movableUpperLimit = SCE_PFX_MIN(swing1UpperAngle, swing2UpperAngle);
	
	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeShoulderJoint( PfxJoint &joint,
									 const PfxRigidState &stateA, const PfxRigidState &stateB,
									 const PfxShoulderJointInitParam &param )
{
	PfxInt32 ret = pfxInitializeShoulderJointCommon( joint, stateA, stateB, param.bias, param.damping,
													 param.twistLowerAngle, param.twistUpperAngle,
													 param.swing1UpperAngle, param.swing2UpperAngle );
	if( ret != SCE_PFX_OK )
		return ret;

	// Calc joint frame
	PfxMatrix3 rotA = transpose(PfxMatrix3(stateA.getOrientation()));
	PfxMatrix3 rotB = transpose(PfxMatrix3(stateB.getOrientation()));
	
	joint.m_anchorA = rotA * (param.anchorPoint - stateA.getLargePosition()).convertToVector3();
	joint.m_anchorB = rotB * (param.anchorPoint - stateB.getLargePosition()).convertToVector3();
	
	PfxMatrix3 frame = PfxMatrix3::identity();
	
	if(param.enableJointFrame && pfxIsOrthogonal(param.jointFrame)) {
		frame = param.jointFrame;
	}
	else {
		PfxVector3 axis0, axis1, axis2;
		axis0 = normalize(param.twistAxis);
		pfxGetPlaneSpace(axis0, axis1, axis2 );
		frame = PfxMatrix3(axis0, axis1, axis2);
	}
	
	joint.m_frameA = rotA * frame;
	joint.m_frameB = rotB * frame;
	
	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeShoulderJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxShoulderJointInitParamLocalSpace &param )
{
	PfxInt32 ret = pfxInitializeShoulderJointCommon( joint, stateA, stateB, param.bias, param.damping,
													 param.twistLowerAngle, param.twistUpperAngle,
													 param.swing1UpperAngle, param.swing2UpperAngle );
	if( ret != SCE_PFX_OK )
		return ret;

	joint.m_anchorA = param.anchorPointA;
	joint.m_anchorB = param.anchorPointB;
	joint.m_frameA = param.jointFrameA;
	joint.m_frameB = param.jointFrameB;

	return SCE_PFX_OK;
}

void pfxSetupShoulderJoint(
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

	PfxVector3 angAB = stateA.getAngularVelocity() - stateB.getAngularVelocity();

	PfxVector3 distance = (stateA.getLargePosition() - stateB.getLargePosition()).convertToVector3() + rA - rB;

	PfxMatrix3 worldFrameA,worldFrameB;
	worldFrameA = PfxMatrix3(solverBodyA.m_orientation) * joint.m_frameA;
	worldFrameB = PfxMatrix3(solverBodyB.m_orientation) * joint.m_frameB;
	
	PfxBool warmStartingOk = isWarmStaringOk(vAB, angAB);

	// Linear Constraint
	PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(solverBodyA.m_massInv + solverBodyB.m_massInv)) - 
			crossMatrix(rA) * solverBodyA.m_inertiaInv * crossMatrix(rA) - 
			crossMatrix(rB) * solverBodyB.m_inertiaInv * crossMatrix(rB);
	
	for(int c=0;c<3;c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];
		PfxConstraintRow &constraint = jointConstraint.m_constraintRow;

		PfxVector3 normal = worldFrameA[c];
		
		PfxFloat posErr = dot(distance,-normal);
		PfxFloat lowerLimit = -jointConstraint.m_maxImpulse;
		PfxFloat upperLimit =  jointConstraint.m_maxImpulse;
		PfxFloat velocityAmp = 1.0f;
		
		pfxCalcLinearLimit(jointConstraint,posErr,velocityAmp,lowerLimit,upperLimit);
		
		jointConstraint.m_lowerLimit = lowerLimit;
		jointConstraint.m_upperLimit = upperLimit;

		PfxFloat bias = pfxModifyLinearBias(fabsf(posErr),jointConstraint.m_velocityBias);
		
		PfxFloat denom = dot(K*normal, normal) + SCE_PFX_CONSTRAINT_STABILIZE_FACTOR;;
		PfxFloat jacDiagInv = velocityAmp/denom;
		if(fabsf(denom - jointConstraint.m_jacobian) > SCE_PFX_JOINT_WARM_STARTING_LINEAR_THRESHOLD) { //} || jointConstraint.m_warmStarting != 1) {
			constraint.m_accumImpulse = 0.0f;
		}

		constraint.m_rhs = -dot(vAB,normal);
		constraint.m_rhs -= (bias * (-posErr)) / timeStep;
		constraint.m_rhs *= jacDiagInv;
		constraint.m_jacDiagInv = jacDiagInv;
		constraint.setNormal(normal);
		jointConstraint.m_jacobian = denom;
	}
	
	PfxVector3 axis[3];
	PfxFloat angle[3];
	pfxCalcJointAngleShoulder(worldFrameA, worldFrameB, angle, axis);

	// Angular Constraint
	for(int c=3;c<6;c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];
		PfxConstraintRow &constraint = jointConstraint.m_constraintRow;

		PfxVector3 normal = axis[c-3];

		PfxFloat posErr = angle[c-3];
		PfxFloat lowerLimit = -jointConstraint.m_maxImpulse;
		PfxFloat upperLimit =  jointConstraint.m_maxImpulse;
		PfxFloat velocityAmp = 1.0f;
		
		pfxCalcAngularLimit(jointConstraint, posErr, velocityAmp, lowerLimit, upperLimit);
	
		jointConstraint.m_lowerLimit = lowerLimit;
		jointConstraint.m_upperLimit = upperLimit;

		PfxFloat bias = pfxModifyAngularBias(fabsf(posErr),jointConstraint.m_velocityBias);
		
		PfxFloat denom = dot((solverBodyA.m_inertiaInv + solverBodyB.m_inertiaInv)*normal, normal) + SCE_PFX_CONSTRAINT_STABILIZE_FACTOR;
		if(denom > 0.0f) {
			PfxFloat jacDiagInv = velocityAmp/denom;
			if (!warmStartingOk) { constraint.m_accumImpulse = 0.0f; }

			constraint.m_rhs = -dot(angAB,normal); // velocity error
			constraint.m_rhs -= (bias * (-posErr)) / timeStep; // position error
			constraint.m_rhs *= jacDiagInv;
			constraint.m_jacDiagInv = jacDiagInv;
		}
		else {
			constraint.m_rhs = 0.0f;
			constraint.m_jacDiagInv = 0.0f;
			constraint.m_accumImpulse = 0.0f;
		}
		constraint.setNormal(normal);
		jointConstraint.m_jacobian = denom;
	}
}

void pfxCorrectPositionShoulderJoint(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation, joint.m_anchorA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation, joint.m_anchorB);

	PfxVector3 distance = solverBodyA.m_deltaLinearVelocity + rA - solverBodyB.m_deltaLinearVelocity - rB;// Note : use m_deltaLinearVelocity as position

	PfxMatrix3 worldFrameA, worldFrameB;
	worldFrameA = PfxMatrix3(solverBodyA.m_orientation) * joint.m_frameA;
	worldFrameB = PfxMatrix3(solverBodyB.m_orientation) * joint.m_frameB;

	// Linear Constraint
	PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(solverBodyA.m_massInv + solverBodyB.m_massInv)) -
		crossMatrix(rA) * solverBodyA.m_inertiaInv * crossMatrix(rA) -
		crossMatrix(rB) * solverBodyB.m_inertiaInv * crossMatrix(rB);

	for (int c = 0; c<3; c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];

		PfxVector3 normal = worldFrameA[c];

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

	PfxVector3 axis[3];
	PfxFloat angle[3];
	pfxCalcJointAngleShoulder(worldFrameA, worldFrameB, angle, axis);

	// Angular Constraint
	for (int c = 3; c<6; c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];

		PfxVector3 normal = axis[c - 3];

		PfxFloat posErr = angle[c - 3];
		
		pfxCalcAngularLimit(jointConstraint, posErr);

		posErr = SCE_PFX_CLAMP(posErr , -0.15f, 0.15f);

		PfxFloat denom = dot((solverBodyA.m_inertiaInv + solverBodyB.m_inertiaInv)*normal, normal);
		PfxFloat impulse = 0.0f;
		if (fabsf(denom) > 1e-8f) impulse = jointConstraint.m_positionBias * posErr / denom;

		PfxVector3 dOriA = impulse * solverBodyA.m_inertiaInv * normal;
		solverBodyA.m_orientation = pfxAddRotation(solverBodyA.m_orientation, dOriA);
		
		PfxVector3 dOriB = -impulse * solverBodyB.m_inertiaInv * normal;
		solverBodyB.m_orientation = pfxAddRotation(solverBodyB.m_orientation, dOriB);
	}
}

void pfxShoulderJointGetAngleLimit(
	const PfxJoint &joint,
	PfxFloat &twistLowerAngle,
	PfxFloat &twistUpperAngle,
	PfxFloat &swing1UpperAngle,
	PfxFloat &swing2UpperAngle)
{
	SCE_PFX_ASSERT(joint.m_type == kPfxJointShoulder);
	twistLowerAngle = joint.m_constraints[3].m_movableLowerLimit;
	twistUpperAngle = joint.m_constraints[3].m_movableUpperLimit;
	swing1UpperAngle = joint.m_constraints[4].m_movableUpperLimit;
	swing2UpperAngle = joint.m_constraints[5].m_movableUpperLimit;
}

PfxInt32 pfxShoulderJointSetAngleLimit(
	PfxJoint &joint,
	PfxFloat twistLowerAngle,
	PfxFloat twistUpperAngle,
	PfxFloat swing1UpperAngle,
	PfxFloat swing2UpperAngle)
{
	SCE_PFX_ASSERT(joint.m_type == kPfxJointShoulder);

	if(twistLowerAngle > twistUpperAngle || 
		!SCE_PFX_RANGE_CHECK(twistLowerAngle,-SCE_PFX_PI-0.01f,SCE_PFX_PI+0.01f) || 
		!SCE_PFX_RANGE_CHECK(twistUpperAngle,-SCE_PFX_PI-0.01f,SCE_PFX_PI+0.01f)) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}

	if(	!SCE_PFX_RANGE_CHECK(swing1UpperAngle,0.0f,SCE_PFX_PI+0.01f)) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}

	if( !SCE_PFX_RANGE_CHECK(swing2UpperAngle,0.0f,SCE_PFX_PI+0.01f)) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	joint.m_constraints[3].m_movableLowerLimit = twistLowerAngle;
	joint.m_constraints[3].m_movableUpperLimit = twistUpperAngle;
	joint.m_constraints[4].m_movableUpperLimit = swing1UpperAngle;
	joint.m_constraints[5].m_movableUpperLimit = swing2UpperAngle;
	
	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
