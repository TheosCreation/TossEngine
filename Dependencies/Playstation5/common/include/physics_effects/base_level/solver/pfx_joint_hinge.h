/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_HINGE_H_
#define _SCE_PFX_JOINT_HINGE_H_

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {


/// @brief Initialization parameter for hinge joint
/// @details Specify that the hinge angle is between -180 degrees and 180 degrees.
/// If enableJointFrame is true, axis is ignored and jointFrame is used as
/// the local coordinates of the joint.
struct SCE_PFX_API PfxHingeJointInitParam {
	PfxLargePosition anchorPoint;	///< @brief Connection position
	PfxVector3 axis;				///< @brief Rotation axis
	PfxMatrix3 jointFrame;			///< @brief Joint local coordinates
	PfxBool enableJointFrame;		///< @brief Enable joint frame
	SCE_PFX_PADDING3
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat lowerAngle;			///< @brief Lower angle limit
	PfxFloat upperAngle;			///< @brief Upper angle limit
	SCE_PFX_PADDING(1,8)
	SCE_PFX_PADDING4

	SCE_PFX_EXCLUDE_DOC(
		PfxHingeJointInitParam() : anchorPoint() {
			axis = PfxVector3(1.0f,0.0f,0.0f);
			lowerAngle = 0.0f;
			upperAngle = 0.0f;
			jointFrame = PfxMatrix3::identity();
			enableJointFrame = false;
			damping = 0.0f;
			bias = 0.2f;
		}
	)
};

struct SCE_PFX_API PfxHingeJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxMatrix3 jointFrameA;			///< @brief Rotation from A to the joint frame
	PfxMatrix3 jointFrameB;			///< @brief Rotation from B to the joint frame
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat lowerAngle;			///< @brief Lower angle limit
	PfxFloat upperAngle;			///< @brief Upper angle limit

	SCE_PFX_EXCLUDE_DOC(
		PfxHingeJointInitParamLocalSpace() {
		const PfxMatrix3 identityMatrix(PfxMatrix3::identity());
		jointFrameA = identityMatrix;
		jointFrameB = identityMatrix;
		lowerAngle = 0.0f;
		upperAngle = 0.0f;
		damping = 0.0f;
		bias = 0.2f;
	}
	)
};

/// @brief Initialize the hinge joint
/// @details Initialize the joint as a hinge joint.
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for the hinge joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeHingeJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxHingeJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeHingeJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxHingeJointInitParamLocalSpace &param);

/// @brief Get angle limit of the hinge joint
/// @details Get angle limit of the hinge joint.
/// @param  joint Joint
/// @param[out] lowerAngle Lower angle limit
/// @param[out] upperAngle Upper angle limit
SCE_PFX_API void pfxHingeJointGetAngleLimit(
	const PfxJoint &joint,
	PfxFloat &lowerAngle,
	PfxFloat &upperAngle);

/// @brief Set angle limit of the hinge joint
/// @details Set angle limit of the hinge joint
/// @param[in,out] joint Joint
/// @param lowerAngle Lower angle limit
/// @param upperAngle Upper angle limit
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxHingeJointSetAngleLimit(
	PfxJoint &joint,
	PfxFloat lowerAngle,
	PfxFloat upperAngle);

SCE_PFX_API void pfxComputeHingeJointError(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	float(&out_error)[6]);

SCE_PFX_API PfxFloat pfxComputeHingeJointAngle(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_HINGE_H
