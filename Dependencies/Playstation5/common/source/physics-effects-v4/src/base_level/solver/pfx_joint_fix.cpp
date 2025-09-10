/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/solver/pfx_joint_fix.h"
#include "pfx_constraint_row_solver.h"

namespace sce {
namespace pfxv4 {

static PfxInt32 pfxInitializeFixJointCommon(PfxJoint &joint,
	const PfxRigidState &stateA,const PfxRigidState &stateB)
{
	joint.m_active = 1;
	joint.m_numConstraints = 6;
	joint.m_userData = 0;
	joint.m_type = kPfxJointFix;
	joint.m_rigidBodyIdA = stateA.getRigidBodyId();
	joint.m_rigidBodyIdB = stateB.getRigidBodyId();
	
	for(int i=0;i<6;i++) {
		joint.m_constraints[i].reset();
		joint.m_constraints[i].m_lock = SCE_PFX_JOINT_LOCK_FIX;
		joint.m_constraints[i].m_positionBias = 0.0;
	}

	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeFixJoint( PfxJoint &joint,
	const PfxRigidState &stateA, const PfxRigidState &stateB,
	const PfxFixJointInitParam &param )
{
	PfxInt32 ret = pfxInitializeFixJointCommon( joint, stateA, stateB );
	if( ret != SCE_PFX_OK )
		return ret;

	// Calc joint frame
	PfxMatrix3 rotA = transpose(PfxMatrix3(stateA.getOrientation()));
	PfxMatrix3 rotB = transpose(PfxMatrix3(stateB.getOrientation()));
	
	joint.m_anchorA = rotA * (param.anchorPoint - stateA.getLargePosition()).convertToVector3();
	joint.m_anchorB = rotB * (param.anchorPoint - stateB.getLargePosition()).convertToVector3();
	
	joint.m_frameA = PfxMatrix3::identity();
	joint.m_frameB = rotB * PfxMatrix3(stateA.getOrientation()) * joint.m_frameA;
	
	return SCE_PFX_OK;
}

PfxInt32 pfxInitializeFixJoint( PfxJoint &joint,
	const PfxRigidState &stateA, const PfxRigidState &stateB,
	const PfxFixJointInitParamLocalSpace &param )
{
	PfxInt32 ret = pfxInitializeFixJointCommon(joint, stateA, stateB);
	if( ret != SCE_PFX_OK )
		return ret;

	joint.m_anchorA = param.anchorPointA;
	joint.m_anchorB = param.anchorPointB;
	joint.m_frameA = param.jointFrameA;
	joint.m_frameB = param.jointFrameB;

	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
