/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_BALL_H
#define _SCE_PFX_JOINT_BALL_H

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {

/// @brief Initialization parameter of a ball joint
/// @details This structure is used to initialize the ball joint.
/// If enableJointFrame is true, jointFrame is used as the local coordinates of the joint.
struct SCE_PFX_API PfxBallJointInitParam {
	PfxLargePosition anchorPoint; ///< @brief Connection position
	PfxMatrix3 jointFrame;        ///< @brief Joint local coordinates
	PfxBool enableJointFrame;     ///< @brief Enable joint frame
	SCE_PFX_PADDING3
	PfxFloat damping;             ///< @brief Damping factor (default:0.0)
	SCE_PFX_PADDING8

	SCE_PFX_EXCLUDE_DOC(PfxBallJointInitParam() : anchorPoint() {jointFrame = PfxMatrix3::identity();enableJointFrame = false;damping = 0.0f;})
};

struct SCE_PFX_API PfxBallJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxFloat damping;             ///< @brief Damping factor (default:0.0)
	SCE_PFX_PADDING12

	SCE_PFX_EXCLUDE_DOC(PfxBallJointInitParamLocalSpace() { damping = 0.0f; })
};

/// @brief Initialize the ball joint
/// @details Initialize the joint as a ball joint.
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for the ball joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeBallJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxBallJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeBallJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxBallJointInitParamLocalSpace &param);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_BALL_H
