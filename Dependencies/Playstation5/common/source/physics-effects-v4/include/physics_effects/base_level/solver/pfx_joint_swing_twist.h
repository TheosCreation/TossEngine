/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_SWING_TWIST_H
#define _SCE_PFX_JOINT_SWING_TWIST_H

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {


/// @brief Initialization parameter for swing twist joint
/// @details Specify that the twist angle is between -180 degrees and 180 degrees,
/// and the swing angle is between 0 degrees and 180 degrees.
/// If enableJointFrame is true, twistAxis is ignored and jointFrame is used as
/// the local coordinates of the joint.
struct SCE_PFX_API  PfxSwingTwistJointInitParam {
	PfxLargePosition anchorPoint;	///< @brief Connection position
	PfxVector3 twistAxis;			///< @brief Twist axis
	PfxMatrix3 jointFrame;			///< @brief Joint local coordinates
	PfxBool enableJointFrame;		///< @brief Enable joint frame
	SCE_PFX_PADDING3
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat twistLowerAngle;		///< @brief Lower twist angle limit
	PfxFloat twistUpperAngle;		///< @brief Upper twist angle limit
	PfxFloat swingLowerAngle;		///< @brief Lower swing angle limit
	PfxFloat swingUpperAngle;		///< @brief Upper swing angle limit
	SCE_PFX_PADDING4

	SCE_PFX_EXCLUDE_DOC(
		PfxSwingTwistJointInitParam() : anchorPoint() {
			twistAxis = PfxVector3(1.0f,0.0f,0.0f);
			twistLowerAngle = -0.26f;
			twistUpperAngle =  0.26f;
			swingLowerAngle = 0.0f;
			swingUpperAngle = 0.7f;
			jointFrame = PfxMatrix3::identity();
			enableJointFrame = false;
			damping = 0.0f;
			bias = 0.2f;
		}
	)
};

struct SCE_PFX_API PfxSwingTwistJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxMatrix3 jointFrameA;			///< @brief Rotation from A to the joint frame
	PfxMatrix3 jointFrameB;			///< @brief Rotation from B to the joint frame
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat twistLowerAngle;		///< @brief Lower twist angle limit
	PfxFloat twistUpperAngle;		///< @brief Upper twist angle limit
	PfxFloat swingLowerAngle;		///< @brief Lower swing angle limit
	PfxFloat swingUpperAngle;		///< @brief Upper swing angle limit
	SCE_PFX_PADDING8

		SCE_PFX_EXCLUDE_DOC(
		PfxSwingTwistJointInitParamLocalSpace() {
		const PfxMatrix3 identityMatrix(PfxMatrix3::identity());
		jointFrameA = identityMatrix;
		jointFrameB = identityMatrix;
		twistLowerAngle = -0.26f;
		twistUpperAngle = 0.26f;
		swingLowerAngle = 0.0f;
		swingUpperAngle = 0.7f;
		damping = 0.0f;
		bias = 0.2f;
	}
	)
};

/// @brief Initialize the swing twist joint
/// @details Initialize the joint as swing twist joint.
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for swing twist joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeSwingTwistJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSwingTwistJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeSwingTwistJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSwingTwistJointInitParamLocalSpace &param);

/// @brief Get angle limit of the swing twist joint
/// @details Get angle limit of the swing twist joint.
/// @param joint Joint
/// @param[out] twistLowerAngle Lower twist angle limit
/// @param[out] twistUpperAngle Upper twist angle limit
/// @param[out] swingLowerAngle Lower swing angle limit
/// @param[out] swingUpperAngle Upper swing angle limit
SCE_PFX_API void pfxSwingTwistJointGetAngleLimit(
	const PfxJoint &joint,
	PfxFloat &twistLowerAngle,
	PfxFloat &twistUpperAngle,
	PfxFloat &swingLowerAngle,
	PfxFloat &swingUpperAngle);

/// @brief Set angle limit of the swing twist joint
/// @details Set angle limit of the swing twist joint.
/// @param[in,out] joint Joint
/// @param twistLowerAngle Lower twist angle limit
/// @param twistUpperAngle Upper twist angle limit
/// @param swingLowerAngle Lower swing angle limit
/// @param swingUpperAngle Upper swing angle limit
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxSwingTwistJointSetAngleLimit(
	PfxJoint &joint,
	PfxFloat twistLowerAngle,
	PfxFloat twistUpperAngle,
	PfxFloat swingLowerAngle,
	PfxFloat swingUpperAngle);

SCE_PFX_API void pfxComputeSwingTwistJointError(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	float(&out_error)[6]);

SCE_PFX_API void pfxComputeSwingTwistJointAngle(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	float(&angles)[2]);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_SWING_TWIST_H
