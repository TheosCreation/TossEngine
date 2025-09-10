/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_H
#define _SCE_PFX_JOINT_H

#include "../rigidbody/pfx_rigid_state.h"
#include "pfx_joint_constraint.h"
#include "pfx_constraint_pair.h"

#define SCE_PFX_JOINT_WARM_STARTING_LINEAR_THRESHOLD	0.002f

namespace sce {
namespace pfxv4 {

/// @brief Joint type
enum ePfxJointType {
	kPfxJointBall = 0,   ///< @brief Ball joint
	kPfxJointSwingTwist, ///< @brief Swing twist joint
	kPfxJointHinge,      ///< @brief Hinge joint
	kPfxJointSlider,     ///< @brief Slider joint
	kPfxJointFix,        ///< @brief Fix joint
	kPfxJointUniversal,  ///< @brief Universal joint
	kPfxJointMotor,      ///< @brief Motor joint
	kPfxJointShoulder,   ///< @brief Shoulder joint
	kPfxJointDistance,   ///< @brief Distance joint
	kPfxJointReserved0,  ///< @brief Reserved
	kPfxJointUser0,      ///< @brief Reserved
	kPfxJointUser1,      ///< @brief Reserved
	kPfxJointUser2,      ///< @brief Reserved
	kPfxJointUser3,      ///< @brief Reserved
	kPfxJointUser4,      ///< @brief Reserved
	kPfxJointCount // = 15
};

/// @brief The structure of the joint
/// @details This is the structure for representing the joint
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxJoint{
	PfxVector3 m_anchorA;      ///< @brief Anchor position of A
	PfxVector3 m_anchorB;      ///< @brief Anchor position of B
	PfxMatrix3 m_frameA;       ///< @brief Joint frame of A
	PfxMatrix3 m_frameB;       ///< @brief Joint frame of B
	PfxQuat m_targetFrame;     ///< @brief Target frame used by a motor
	PfxUInt8 m_active;         ///< @brief Active or not
	PfxUInt8 m_type;           ///< @brief Type
	PfxUInt8 m_numConstraints; ///< @brief Number of constraints
	PfxUInt32 m_rigidBodyIdA;  ///< @brief Rigid body A's ID
	PfxUInt32 m_rigidBodyIdB;  ///< @brief Rigid body B's ID
	PfxUInt32 m_userData;      ///< @brief User data
	PfxFloat m_parameters[4];	///< @brief Parameters for a specific joint
	PfxJointConstraint m_constraints[6];///< @brief Constraints
	void *m_userPointer = nullptr;

	static const PfxUInt32 bytesOfJoint = 528; 

	void save(PfxUInt8 *pout, PfxUInt32 bytes) const;

	void load(const PfxUInt8 *pout, PfxUInt32 bytes);
};

/// @brief Update joint pair
/// @details Update joint pair as an input of pfxSolveConstraints().
/// @param[in,out] pair Pair
/// @param jointId Joint ID
/// @param joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
static SCE_PFX_FORCE_INLINE 
void pfxUpdateJointPairs(
	PfxConstraintPair &pair,
	PfxUInt32 jointId,
	const PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB
	)
{
	SCE_PFX_ASSERT(stateA.getRigidBodyId()==joint.m_rigidBodyIdA);
	SCE_PFX_ASSERT(stateB.getRigidBodyId()==joint.m_rigidBodyIdB);
	pfxSetObjectIdA(pair,stateA.getRigidBodyId());
	pfxSetObjectIdB(pair,stateB.getRigidBodyId());
	pfxSetMotionMaskA(pair,stateA.getMotionMask());
	pfxSetMotionMaskB(pair,stateB.getMotionMask());
	pfxSetConstraintId(pair,jointId);
	pfxSetNumConstraints(pair,joint.m_numConstraints);
	pfxSetSolverQuality(pair, SCE_PFX_MAX(stateA.getSolverQuality(), stateB.getSolverQuality()));
	pfxSetActive(pair,joint.m_active>0);
}

/// @brief Set the values for velocity bias
/// @details Set the values for velocity bias of joint constraints.
/// The velocity bias is used to correct a positonal error to satisfy a constraint condition.
/// by changing its velocity. If a zero is set, the position correction is cancelled for this constraint.
/// @param[out] joint Joint
/// @param linearBias bias for linear constraints
/// @param angularBias bias for angular constraints
static SCE_PFX_FORCE_INLINE 
void pfxSetJointVelocityBias(PfxJoint &joint,PfxFloat linearBias,PfxFloat angularBias)
{
	joint.m_constraints[0].m_velocityBias = linearBias;
	joint.m_constraints[1].m_velocityBias = linearBias;
	joint.m_constraints[2].m_velocityBias = linearBias;
	joint.m_constraints[3].m_velocityBias = angularBias;
	joint.m_constraints[4].m_velocityBias = angularBias;
	joint.m_constraints[5].m_velocityBias = angularBias;
}

/// @brief Set the values for position bias
/// @details Set the values for position bias of joint constraints.
/// The position bias is used to correct a positonal error to satisfy a constraint condition
/// by changing its position. If a zero is set, the position correction is cancelled for this constraint.
/// @param[out] joint Joint
/// @param linearBias bias for linear constraints
/// @param angularBias bias for angular constraints
static SCE_PFX_FORCE_INLINE
void pfxSetJointPositionBias(PfxJoint &joint, PfxFloat linearBias, PfxFloat angularBias)
{
	joint.m_constraints[0].m_positionBias = linearBias;
	joint.m_constraints[1].m_positionBias = linearBias;
	joint.m_constraints[2].m_positionBias = linearBias;
	joint.m_constraints[3].m_positionBias = angularBias;
	joint.m_constraints[4].m_positionBias = angularBias;
	joint.m_constraints[5].m_positionBias = angularBias;
}

/// @brief Get the damping value
/// @details Get the damping value of the joint constraint.
/// @param joint Joint
/// @param jointConstraintId Joint constraint ID
/// @return Damping value
static SCE_PFX_FORCE_INLINE 
PfxFloat pfxGetJointDamping(const PfxJoint &joint,PfxUInt32 jointConstraintId)
{
	SCE_PFX_ASSERT(jointConstraintId < joint.m_numConstraints);
	return joint.m_constraints[jointConstraintId].m_damping;
}

/// @brief Set the damping value
/// @details Set the damping value of the joint constraint.
/// @param[in,out] joint Joint
/// @param jointConstraintId Joint constraint ID
/// @param damping Damping value
static SCE_PFX_FORCE_INLINE 
void pfxSetJointDamping(PfxJoint &joint,PfxUInt32 jointConstraintId,PfxFloat damping)
{
	SCE_PFX_ASSERT(jointConstraintId < joint.m_numConstraints);
	joint.m_constraints[jointConstraintId].m_damping = damping;
}

} //namespace pfxv4
} //namespace sce


#endif // _PFX_JOINT_H
