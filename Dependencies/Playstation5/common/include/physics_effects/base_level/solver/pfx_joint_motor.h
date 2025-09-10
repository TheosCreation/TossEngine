/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_MOTOR_H
#define _SCE_PFX_JOINT_MOTOR_H

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {

/// @brief Initialization parameter for the motor joint
/// @details This structure is used to initialize the motor joint.
struct SCE_PFX_API PfxMotorJointInitParam {
	PfxLargePosition anchorPoint; ///< @brief Connection position
	PfxMatrix3 initialFrame;
	SCE_PFX_EXCLUDE_DOC(PfxMotorJointInitParam() : anchorPoint() {})
};

struct SCE_PFX_API PfxMotorJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxMatrix3 jointFrameA;			///< @brief Rotation from A to the joint frame
	PfxMatrix3 jointFrameB;			///< @brief Rotation from B to the joint frame

	SCE_PFX_EXCLUDE_DOC(
		PfxMotorJointInitParamLocalSpace() {
		const PfxMatrix3 identityMatrix(PfxMatrix3::identity());
		jointFrameA = identityMatrix;
		jointFrameB = identityMatrix;
	}
	)
};

/// @brief Initialize the motor joint
/// @details Initialize the joint as the motor joint
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for motor joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeMotorJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxMotorJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeMotorJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxMotorJointInitParamLocalSpace &param);

/// @brief Drive the motor joint to the specified orientation
/// @details The target orientation is relative to the joint frameA. The power of the motor is controlled with stiffness and damping parameter.
/// @param[in,out] joint Joint
/// @param targetOrientation Target orientation
/// @param stiffness Stiffness
/// @param damping Damping
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxMotorJointMoveOrientation(
	PfxJoint &joint,
	const PfxQuat &targetOrientation,
	PfxFloat stiffness,
	PfxFloat damping);

/// @brief Drive the motor joint to the specified position
/// @details The target position is relative to the joint frameA. The power of the motor is controlled with stiffness and damping parameter.
/// @param[in,out] joint Joint
/// @param targetPosition Target position
/// @param stiffness Stiffness
/// @param damping Damping
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxMotorJointMovePosition(
	PfxJoint &joint,
	const PfxVector3 &targetPosition,
	PfxFloat stiffness,
	PfxFloat damping);

/// @brief Drive the motor joint to the specified position
/// @details The target position is relative to the joint frameA. The power of the motor is controlled with stiffness and damping parameter.
/// @param[in,out] joint Joint
/// @param axis Joint is driven along this axis (0=X or 1=Y or 2=Z)
/// @param targetPosition Target position
/// @param stiffness Stiffness
/// @param damping Damping
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxMotorJointMovePosition(
	PfxJoint &joint,
	PfxUInt32 axis,
	PfxFloat targetPosition,
	PfxFloat stiffness,
	PfxFloat damping);

SCE_PFX_API void pfxComputeMotorJointError(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	float(&out_error)[6]);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_MOTOR_H
