/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_slider.h"
#include "pfx_constraint_row_solver.h"

namespace sce {
namespace pfxv4 {

static PfxInt32 pfxInitializeSliderJointCommon(PfxJoint &joint,
	const PfxRigidState &stateA,const PfxRigidState &stateB,
	float damping, float bias, float lowerDistance, float upperDistance)
{
	joint.m_active = 1;
	joint.m_numConstraints = 6;
	joint.m_userData = 0;
	joint.m_type = kPfxJointSlider;
	joint.m_rigidBodyIdA = stateA.getRigidBodyId();
	joint.m_rigidBodyIdB = stateB.getRigidBodyId();
	
	for(int i=0;i<6;i++) {
		joint.m_constraints[i].reset();
		joint.m_constraints[i].m_positionBias = 0.0f;
	}

	if(lowerDistance == 0.0f && upperDistance == 0.0f) {
		joint.m_constraints[0].m_lock = SCE_PFX_JOINT_LOCK_FREE;
	}
	else {
		joint.m_constraints[0].m_lock = SCE_PFX_JOINT_LOCK_LIMIT;
	}
	
	joint.m_constraints[0].m_damping = damping;
	joint.m_constraints[0].m_velocityBias = bias;
	
	joint.m_constraints[1].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	joint.m_constraints[2].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	joint.m_constraints[3].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	joint.m_constraints[4].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	joint.m_constraints[5].m_lock = SCE_PFX_JOINT_LOCK_FIX;
	
	if(lowerDistance > upperDistance ) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	joint.m_constraints[0].m_movableLowerLimit = lowerDistance;
	joint.m_constraints[0].m_movableUpperLimit = upperDistance;
	
    return SCE_PFX_OK;
}

PfxInt32 pfxInitializeSliderJoint( PfxJoint &joint,
    const PfxRigidState &stateA, const PfxRigidState &stateB,
    const PfxSliderJointInitParam &param )
{
	PfxInt32 ret = pfxInitializeSliderJointCommon( joint, stateA, stateB, param.damping, param.bias, param.lowerDistance, param.upperDistance );
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
		axis0 = normalize(param.direction);
		pfxGetPlaneSpace(axis0, axis1, axis2 );
		frame = PfxMatrix3(axis0, axis1, axis2);
	}
	
	joint.m_frameA = rotA * frame;
	joint.m_frameB = rotB * frame;
	
	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeSliderJoint( PfxJoint &joint,
    const PfxRigidState &stateA, const PfxRigidState &stateB,
    const PfxSliderJointInitParamLocalSpace &param )
{
    PfxInt32 ret = pfxInitializeSliderJointCommon( joint, stateA, stateB, param.damping, param.bias, param.lowerDistance, param.upperDistance );
    if( ret != SCE_PFX_OK )
        return ret;

    joint.m_anchorA = param.anchorPointA;
    joint.m_anchorB = param.anchorPointB;
    joint.m_frameA = param.jointFrameA;
    joint.m_frameB = param.jointFrameB;

    return SCE_PFX_OK;
}

void pfxSliderJointGetDistanceLimit(
	const PfxJoint &joint,
	PfxFloat &lowerDistance,
	PfxFloat &upperDistance)
{
	SCE_PFX_ASSERT(joint.m_type == kPfxJointSlider);
	lowerDistance = joint.m_constraints[0].m_movableLowerLimit;
	upperDistance = joint.m_constraints[0].m_movableUpperLimit;
}

PfxInt32 pfxSliderJointSetDistanceLimit(
	PfxJoint &joint,
	PfxFloat lowerDistance,
	PfxFloat upperDistance)
{
	SCE_PFX_ASSERT(joint.m_type == kPfxJointSlider);

	if(lowerDistance > upperDistance ) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	joint.m_constraints[0].m_movableLowerLimit = lowerDistance;
	joint.m_constraints[0].m_movableUpperLimit = upperDistance;
	
	return SCE_PFX_OK;
}

PfxFloat pfxComputeSliderJointDistance(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB)
{
	PfxVector3 rA = rotate(stateA.getOrientation(), joint.m_anchorA);
	PfxVector3 rB = rotate(stateB.getOrientation(), joint.m_anchorB);
	PfxVector3 distance = (stateA.getLargePosition() - stateB.getLargePosition()).convertToVector3() + rA - rB;

	PfxMatrix3 worldFrameA, worldFrameB;
	worldFrameA = PfxMatrix3(stateA.getOrientation()) * joint.m_frameA;
	worldFrameB = PfxMatrix3(stateB.getOrientation()) * joint.m_frameB;

	PfxVector3 normal = worldFrameA[0];

	return dot(distance, -normal);
}

} //namespace pfxv4
} //namespace sce
