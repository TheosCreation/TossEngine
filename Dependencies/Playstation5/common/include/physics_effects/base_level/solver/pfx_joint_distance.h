/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_DISTANCE_H
#define _SCE_PFX_JOINT_DISTANCE_H

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {


/// @brief Initialization parameter of a distance joint
/// @details This structure is used to initialize the distance joint.
/// If enableJointFrame is true, jointFrame is used as the local coordinates of the joint.
struct SCE_PFX_API PfxDistanceJointInitParam {
	PfxLargePosition anchorPoint; ///< @brief Connection position
	PfxMatrix3 jointFrame;        ///< @brief Joint local coordinates
	PfxBool enableJointFrame;     ///< @brief Enable joint frame
	SCE_PFX_PADDING3
	PfxFloat damping;             ///< @brief Damping factor (default:0.0)
	PfxFloat distance;
	SCE_PFX_PADDING4

	SCE_PFX_EXCLUDE_DOC(PfxDistanceJointInitParam() : anchorPoint() {jointFrame = PfxMatrix3::identity();enableJointFrame = false;damping = 0.0f;distance = 0.0f;})
};

struct SCE_PFX_API PfxDistanceJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxMatrix3 jointFrameA;			///< @brief Rotation from A to the joint frame
	PfxMatrix3 jointFrameB;			///< @brief Rotation from B to the joint frame
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat distance;

	SCE_PFX_EXCLUDE_DOC(
		PfxDistanceJointInitParamLocalSpace() {
		const PfxMatrix3 identityMatrix(PfxMatrix3::identity());
		jointFrameA = identityMatrix;
		jointFrameB = identityMatrix;
		damping = 0.0f;
		distance = 0.0f;
	}
	)
};

/// @brief Initialize the distance joint
/// @details Initialize the joint as a distance joint.
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for the distance joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeDistanceJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxDistanceJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeDistanceJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxDistanceJointInitParamLocalSpace &param);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_DISTANCE_H
