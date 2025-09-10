/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_motor.h"
#include "pfx_constraint_row_solver.h"

#define ENABLE_FORCE_MOTOR_V3

namespace sce {
namespace pfxv4 {

static PfxInt32 pfxInitializeMotorJointCommon(PfxJoint &joint,
	const PfxRigidState &stateA,const PfxRigidState &stateB)
{
	joint.m_active = 1;
	joint.m_numConstraints = 6;
	joint.m_userData = 0;
	joint.m_type = kPfxJointMotor;
	joint.m_rigidBodyIdA = stateA.getRigidBodyId();
	joint.m_rigidBodyIdB = stateB.getRigidBodyId();
	
	for(int i=0;i<6;i++) {
		joint.m_constraints[i].reset();
		joint.m_constraints[i].m_positionBias = 0.0f;
	}
	
	joint.m_constraints[0].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[1].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[2].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[3].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[4].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	joint.m_constraints[5].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	
	for(int i=0;i<6;i++) {
		joint.m_constraints[i].m_velocityBias = 0.2f;
		joint.m_constraints[i].m_damping = 1.0f;
		joint.m_constraints[i].m_movableLowerLimit = 0.0f;
		joint.m_constraints[i].m_movableUpperLimit = 0.0f;
	}

	joint.m_targetFrame = PfxQuat::identity();

	joint.m_parameters[0] = 1200.0f; // linear stiffness
	joint.m_parameters[1] = 80.0f; // linear damping
	joint.m_parameters[2] = 1200.0f; // angular stiffness
	joint.m_parameters[3] = 80.0f; // angular damping

	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeMotorJoint( PfxJoint &joint,
	const PfxRigidState &stateA, const PfxRigidState &stateB,
	const PfxMotorJointInitParam &param )
{
	PfxInt32 ret = pfxInitializeMotorJointCommon( joint, stateA, stateB );
	if( ret != SCE_PFX_OK )
		return ret;
	
	// Calc joint frame
	PfxMatrix3 rotA = transpose(PfxMatrix3(stateA.getOrientation()));
	PfxMatrix3 rotB = transpose(PfxMatrix3(stateB.getOrientation()));
	
	joint.m_anchorA = rotA * (param.anchorPoint - stateA.getLargePosition()).convertToVector3();
	joint.m_anchorB = rotB * (param.anchorPoint - stateB.getLargePosition()).convertToVector3();
	
	PfxMatrix3 frame = PfxMatrix3::identity();
	
	if(pfxIsOrthogonal(param.initialFrame)) {
		frame = param.initialFrame;
	}
	
	joint.m_frameA = rotA * frame;
	joint.m_frameB = rotB * frame;
	joint.m_targetFrame = PfxQuat::identity();

	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeMotorJoint( PfxJoint &joint,
	const PfxRigidState &stateA, const PfxRigidState &stateB,
	const PfxMotorJointInitParamLocalSpace &param )
{
	PfxInt32 ret = pfxInitializeMotorJointCommon(joint, stateA, stateB);
	if( ret != SCE_PFX_OK )
		return ret;

	joint.m_anchorA = param.anchorPointA;
	joint.m_anchorB = param.anchorPointB;
	joint.m_frameA = param.jointFrameA;
	joint.m_frameB = param.jointFrameB;

	return SCE_PFX_OK;
}

void pfxSetupMotorJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep
)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation, joint.m_anchorA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation, joint.m_anchorB);

	PfxVector3 vA = stateA.getLinearVelocity() + cross(stateA.getAngularVelocity(), rA);
	PfxVector3 vB = stateB.getLinearVelocity() + cross(stateB.getAngularVelocity(), rB);
	PfxVector3 vAB = vA - vB;

	PfxVector3 angAB = stateA.getAngularVelocity() - stateB.getAngularVelocity();

	PfxVector3 distance = (stateA.getLargePosition() - stateB.getLargePosition()).convertToVector3() + rA - rB;

	PfxMatrix3 worldFrameA, worldFrameB;
	worldFrameA = PfxMatrix3(solverBodyA.m_orientation) * joint.m_frameA;
	worldFrameB = PfxMatrix3(solverBodyB.m_orientation) * joint.m_frameB;

	PfxBool warmStartingOk = isWarmStaringOk(vAB, angAB);

	// calculate spring and damper values
#ifndef ENABLE_FORCE_MOTOR_V3
	PfxFloat linearStiffness = joint.m_parameters[0];
	PfxFloat linearDamping = joint.m_parameters[1];
	PfxFloat tmp1 = linearDamping + timeStep * linearStiffness;
	PfxFloat linearErp = linearStiffness / tmp1;
	PfxFloat linearCfm = 1.0f / tmp1;
	if (tmp1 < 1e-5f) {linearErp = linearCfm = 0.0f;}
#endif

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
		
		if(posErr < jointConstraint.m_movableLowerLimit) {
			posErr = posErr - jointConstraint.m_movableLowerLimit;
			posErr = SCE_PFX_MIN(0.0f,posErr+SCE_PFX_JOINT_LINEAR_SLOP);
			upperLimit = SCE_PFX_MIN(0.0f,upperLimit);
			velocityAmp *= jointConstraint.m_damping;
		}
		else { // posErr > movableUpperLimit
			posErr = posErr - jointConstraint.m_movableUpperLimit;
			posErr = SCE_PFX_MAX(0.0f,posErr-SCE_PFX_JOINT_LINEAR_SLOP);
			lowerLimit = SCE_PFX_MAX(0.0f,lowerLimit);
			velocityAmp *= jointConstraint.m_damping;
		}
		
		jointConstraint.m_lowerLimit = lowerLimit;
		jointConstraint.m_upperLimit = upperLimit;
		
#ifdef ENABLE_FORCE_MOTOR_V3
		PfxFloat bias = pfxModifyLinearBias(fabsf(posErr),jointConstraint.m_velocityBias);
		PfxFloat denom = dot(K*normal, normal);
		PfxFloat jacDiagInv = velocityAmp/denom;
		if(fabsf(denom - jointConstraint.m_jacobian) > SCE_PFX_JOINT_WARM_STARTING_LINEAR_THRESHOLD) { //} || jointConstraint.m_warmStarting != 1) {
			constraint.m_accumImpulse = 0.0f;
		}
		constraint.m_rhs = -velocityAmp*dot(vAB,normal);
		constraint.m_rhs -= (bias * (-posErr)) / timeStep;
		constraint.m_rhs /= denom;
#else
		PfxFloat denom = dot(K*normal, normal) + linearCfm;
		PfxFloat jacDiagInv = velocityAmp/denom;
		if(fabsf(denom - jointConstraint.m_jacobian) > SCE_PFX_JOINT_WARM_STARTING_LINEAR_THRESHOLD) { //} || jointConstraint.m_warmStarting != 1) {
			constraint.m_accumImpulse = 0.0f;
		}
		constraint.m_rhs = -dot(vAB,normal);
		constraint.m_rhs -= (linearErp * (-posErr));
		constraint.m_rhs *= jacDiagInv;
#endif

		constraint.m_jacDiagInv = jacDiagInv;
		constraint.setNormal(normal);
		jointConstraint.m_jacobian = denom;
	}

	worldFrameA = worldFrameA * PfxMatrix3(joint.m_targetFrame);

	PfxVector3 axis[3];
	PfxFloat angle[3];
	pfxCalcJointAngleSwingTwist(worldFrameA,worldFrameB,angle,axis);

	// calculate spring and damper values
#ifndef ENABLE_FORCE_MOTOR_V3
	PfxFloat angularStiffness = joint.m_parameters[2];
	PfxFloat angularDamping = joint.m_parameters[3];
	PfxFloat tmp2 = angularDamping + timeStep * angularStiffness;
	PfxFloat angularErp = angularStiffness / tmp2;
	PfxFloat angularCfm = 1.0f / tmp2;
	if (tmp2 < 1e-5f) { angularErp = angularCfm = 0.0f; }
#endif

	// Angular Constraint
	for(int c=3;c<6;c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];
		PfxConstraintRow &constraint = jointConstraint.m_constraintRow;

		PfxVector3 normal = axis[c-3];

		PfxFloat posErr = angle[c-3];
		PfxFloat lowerLimit = -jointConstraint.m_maxImpulse;
		PfxFloat upperLimit =  jointConstraint.m_maxImpulse;
		PfxFloat velocityAmp = 1.0f;
		
		if(jointConstraint.m_lock != SCE_PFX_JOINT_LOCK_FIX) {
			PfxFloat diffLower = pfxModifyAngle(0.0f - posErr);
			PfxFloat diffUpper = pfxModifyAngle(0.0f - posErr);
			if(0.0f < diffLower) {
				posErr = -diffLower;
				posErr = SCE_PFX_MIN(0.0f,posErr+0.0001f);
				upperLimit = SCE_PFX_MIN(0.0f,upperLimit);
				velocityAmp *= jointConstraint.m_damping;
			}
			else {
				posErr = -diffUpper;
				posErr = SCE_PFX_MAX(0.0f,posErr-0.0001f);
				lowerLimit = SCE_PFX_MAX(0.0f,lowerLimit);
				velocityAmp *= jointConstraint.m_damping;
			}
		}

		jointConstraint.m_lowerLimit = lowerLimit;
		jointConstraint.m_upperLimit = upperLimit;
		
#ifdef ENABLE_FORCE_MOTOR_V3
		PfxFloat bias = pfxModifyAngularBias(fabsf(posErr),jointConstraint.m_velocityBias);

		PfxFloat denom = dot((solverBodyA.m_inertiaInv + solverBodyB.m_inertiaInv)*normal, normal);
		PfxFloat jacDiagInv = 0.0f;
		if (fabsf(denom) > 1e-8f) jacDiagInv = velocityAmp / denom;

		if (!warmStartingOk) { constraint.m_accumImpulse = 0.0f; }

		constraint.m_rhs = -velocityAmp*dot(angAB,normal); // velocity error
		constraint.m_rhs -= (bias * (-posErr)) / timeStep; // position error
		constraint.m_rhs *= jacDiagInv;
#else
		PfxFloat denom = dot((solverBodyA.m_inertiaInv + solverBodyB.m_inertiaInv)*normal, normal) + angularCfm;
		PfxFloat jacDiagInv = 0.0f;
		if (fabsf(denom) > 1e-8f) jacDiagInv = velocityAmp / denom;

		if (!warmStartingOk) { constraint.m_accumImpulse = 0.0f; }

		constraint.m_rhs = -dot(angAB,normal); // velocity error
		constraint.m_rhs -= (angularErp * (-posErr)); // position error
		constraint.m_rhs *= jacDiagInv;
#endif

		constraint.m_jacDiagInv = jacDiagInv;
		constraint.setNormal(normal);
		jointConstraint.m_jacobian = denom;
	}
}

void pfxCorrectPositionMotorJoint(
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

		if(posErr < jointConstraint.m_movableLowerLimit) {
			posErr = posErr - jointConstraint.m_movableLowerLimit;
			posErr = SCE_PFX_MIN(0.0f,posErr+SCE_PFX_JOINT_LINEAR_SLOP);
		}
		else { // posErr > movableUpperLimit
			posErr = posErr - jointConstraint.m_movableUpperLimit;
			posErr = SCE_PFX_MAX(0.0f,posErr-SCE_PFX_JOINT_LINEAR_SLOP);
		}

		PfxFloat denom = dot(K*normal, normal);
		PfxFloat impulse = jointConstraint.m_positionBias * posErr / denom;
		
		solverBodyA.m_deltaLinearVelocity += impulse * solverBodyA.m_massInv * normal;// Note : use m_deltaLinearVelocity as position
		solverBodyB.m_deltaLinearVelocity -= impulse * solverBodyB.m_massInv * normal;// Note : use m_deltaLinearVelocity as position

		PfxVector3 dOriA = impulse * solverBodyA.m_inertiaInv * cross(rA, normal);
		solverBodyA.m_orientation = pfxAddRotation(solverBodyA.m_orientation, dOriA);
		
		PfxVector3 dOriB = -impulse * solverBodyB.m_inertiaInv * cross(rB, normal);
		solverBodyB.m_orientation = pfxAddRotation(solverBodyB.m_orientation, dOriB);
	}

	worldFrameA = worldFrameA * PfxMatrix3(joint.m_targetFrame);

	PfxVector3 axis[3];
	PfxFloat angle[3];
	pfxCalcJointAngleSwingTwist(worldFrameA, worldFrameB, angle, axis);

	// Angular Constraint
	for (int c = 3; c<6; c++) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[c];

		PfxVector3 normal = axis[c - 3];

		PfxFloat posErr = angle[c - 3];

		if(jointConstraint.m_lock != SCE_PFX_JOINT_LOCK_FIX) {
			PfxFloat diffLower = pfxModifyAngle(0.0f - posErr);
			PfxFloat diffUpper = pfxModifyAngle(0.0f - posErr);
			if(0.0f < diffLower) {
				posErr = -diffLower;
				posErr = SCE_PFX_MIN(0.0f, posErr + 0.0001f);
			}
			else {
				posErr = -diffUpper;
				posErr = SCE_PFX_MAX(0.0f, posErr - 0.0001f);
			}
		}

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

PfxInt32 pfxMotorJointMoveOrientation(
	PfxJoint &joint,
	const PfxQuat &targetOrientation,
	PfxFloat stiffness,
	PfxFloat damping)
{
	if(joint.m_type != kPfxJointMotor) return SCE_PFX_ERR_INVALID_TYPE;
	
	// targetOrientation (Pose B) is the relative orientation to the joint frame A
	joint.m_targetFrame = targetOrientation;

	joint.m_parameters[2] = stiffness;
	joint.m_parameters[3] = damping;
	
	return SCE_PFX_OK;
}

PfxInt32 pfxMotorJointMovePosition(
	PfxJoint &joint,
	const PfxVector3 &targetPosition,
	PfxFloat stiffness,
	PfxFloat damping)
{
	if(joint.m_type != kPfxJointMotor) return SCE_PFX_ERR_INVALID_TYPE;
	
	// targetPosition is the relative position to the joint frame A
	joint.m_constraints[0].m_movableLowerLimit = joint.m_constraints[0].m_movableUpperLimit = targetPosition.getX();
	joint.m_constraints[1].m_movableLowerLimit = joint.m_constraints[1].m_movableUpperLimit = targetPosition.getY();
	joint.m_constraints[2].m_movableLowerLimit = joint.m_constraints[2].m_movableUpperLimit = targetPosition.getZ();

	joint.m_parameters[0] = stiffness;
	joint.m_parameters[1] = damping;

	return SCE_PFX_OK;
}

PfxInt32 pfxMotorJointMovePosition(
	PfxJoint &joint,
	PfxUInt32 axis,
	PfxFloat targetPosition,
	PfxFloat stiffness,
	PfxFloat damping)
{
	if(joint.m_type != kPfxJointMotor) return SCE_PFX_ERR_INVALID_TYPE;
	if(axis > 2) return SCE_PFX_ERR_INVALID_VALUE;
	
	joint.m_parameters[0] = stiffness;
	joint.m_parameters[1] = damping;

	joint.m_constraints[axis].m_movableLowerLimit = joint.m_constraints[axis].m_movableUpperLimit = targetPosition;

	return SCE_PFX_OK;
}

void pfxComputeMotorJointError(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	float (&out_error)[ 6 ]
	)
{
	PfxVector3 rA = rotate( stateA.getOrientation(), joint.m_anchorA );
	PfxVector3 rB = rotate( stateB.getOrientation(), joint.m_anchorB );

	PfxVector3 distance = rA + stateA.getPosition() - rB - stateB.getPosition();

	PfxMatrix3 worldFrameA = PfxMatrix3( stateA.getOrientation() ) * joint.m_frameA;
	PfxMatrix3 worldFrameB = PfxMatrix3( stateB.getOrientation() ) * joint.m_frameB;

	for( int c = 0; c < 3; c++ ) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[ c ];

		PfxVector3 normal = worldFrameA[ c ];

		PfxFloat posErr = dot( distance, -normal );

		if( posErr < jointConstraint.m_movableLowerLimit ) {
			posErr = posErr - jointConstraint.m_movableLowerLimit;
			posErr = SCE_PFX_MIN( 0.0f, posErr + SCE_PFX_JOINT_LINEAR_SLOP );
		}
		else {
			posErr = posErr - jointConstraint.m_movableUpperLimit;
			posErr = SCE_PFX_MAX( 0.0f, posErr - SCE_PFX_JOINT_LINEAR_SLOP );
		}

		out_error[ c ] = posErr;
	}

	worldFrameA = worldFrameA * PfxMatrix3( joint.m_targetFrame );

	PfxVector3 axis[ 3 ];
	PfxFloat angle[ 3 ];
	pfxCalcJointAngleSwingTwist( worldFrameA, worldFrameB, angle, axis );

	// Angular Constraint
	for( int c = 3; c < 6; c++ ) {
		PfxJointConstraint &jointConstraint = joint.m_constraints[ c ];

		PfxFloat posErr = angle[ c - 3 ];

		if( jointConstraint.m_lock != SCE_PFX_JOINT_LOCK_FIX ) {
			PfxFloat diffLower = pfxModifyAngle( 0.0f - posErr );
			PfxFloat diffUpper = pfxModifyAngle( 0.0f - posErr );
			if( 0.0f < diffLower ) {
				posErr = -diffLower;
				posErr = SCE_PFX_MIN( 0.0f, posErr + 0.0001f );
			}
			else {
				posErr = -diffUpper;
				posErr = SCE_PFX_MAX( 0.0f, posErr - 0.0001f );
			}
		}

		out_error[ c ] = posErr;
	}
}

} //namespace pfxv4
} //namespace sce
