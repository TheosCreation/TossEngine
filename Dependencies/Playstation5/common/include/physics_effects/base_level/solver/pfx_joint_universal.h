/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_UNIVERSAL_H
#define _SCE_PFX_JOINT_UNIVERSAL_H

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {


/// @brief Initialization parameter for universal joint
/// @details Specify that the swing angle is between 0 degrees and 180 degrees.
/// When the movable scopes of both axes exceed 90 degrees and the two axes
/// become closer to being perpendicular, behavior will become unstable.
/// When the movable scope of one axis exceeds 90 degrees, make sure to
/// specify a value that does not exceed 90 for the other axis. 
/// If enableJointFrame is true, axis is ignored and jointFrame is used as
/// the local coordinates of the joint.
struct SCE_PFX_API PfxUniversalJointInitParam {
	PfxLargePosition anchorPoint;	///< @brief Connection position
	PfxVector3 axis;				///< @brief Front axis
	PfxMatrix3 jointFrame;			///< @brief Joint local coordinates
	PfxBool enableJointFrame;		///< @brief Enable joint frame
	SCE_PFX_PADDING3
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat swing1LowerAngle;		///< @brief Lower swing1 angle limit
	PfxFloat swing1UpperAngle;		///< @brief Upper swing1 angle limit
	PfxFloat swing2LowerAngle;		///< @brief Lower swing2 angle limit
	PfxFloat swing2UpperAngle;		///< @brief Upper swing2 angle limit
	SCE_PFX_PADDING4

	SCE_PFX_EXCLUDE_DOC(
		PfxUniversalJointInitParam() : anchorPoint() {
			axis = PfxVector3(1.0f,0.0f,0.0f);
			swing1LowerAngle = -0.7f;
			swing1UpperAngle =  0.7f;
			swing2LowerAngle = -0.7f;
			swing2UpperAngle =  0.7f;
			damping = 0.0f;
			bias = 0.2f;
			enableJointFrame = false;
		}
	)
};

struct SCE_PFX_API PfxUniversalJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxMatrix3 jointFrameA;			///< @brief Rotation from A to the joint frame
	PfxMatrix3 jointFrameB;			///< @brief Rotation from B to the joint frame
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat swing1LowerAngle;		///< @brief Lower swing1 angle limit
	PfxFloat swing1UpperAngle;		///< @brief Upper swing1 angle limit
	PfxFloat swing2LowerAngle;		///< @brief Lower swing2 angle limit
	PfxFloat swing2UpperAngle;		///< @brief Upper swing2 angle limit
	SCE_PFX_PADDING8

		SCE_PFX_EXCLUDE_DOC(
		PfxUniversalJointInitParamLocalSpace() {
		const PfxMatrix3 identityMatrix(PfxMatrix3::identity());
		jointFrameA = identityMatrix;
		jointFrameB = identityMatrix;
		swing1LowerAngle = -0.7f;
		swing1UpperAngle = 0.7f;
		swing2LowerAngle = -0.7f;
		swing2UpperAngle = 0.7f;
		damping = 0.0f;
		bias = 0.2f;
	}
	)
};

/// @brief Initialize the universal joint
/// @details Initialize the joint as universal joint.
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for universal joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeUniversalJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxUniversalJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeUniversalJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxUniversalJointInitParamLocalSpace &param);

/// @brief Get angle limit of the universal joint
/// @details Get angle limit of the universal joint.
/// @param joint Joint
/// @param[out] swing1LowerAngle Lower swing1 angle limit
/// @param[out] swing1UpperAngle Upper swing1 angle limit
/// @param[out] swing2LowerAngle Lower swing2 angle limit
/// @param[out] swing2UpperAngle Upper swing2 angle limit
SCE_PFX_API void pfxUniversalJointGetAngleLimit(
	const PfxJoint &joint,
	PfxFloat &swing1LowerAngle,
	PfxFloat &swing1UpperAngle,
	PfxFloat &swing2LowerAngle,
	PfxFloat &swing2UpperAngle);

/// @brief Set angle limit of the universal joint
/// @details Set angle limit of the universal joint.
/// @param[in,out] joint Joint
/// @param swing1LowerAngle Lower swing1 angle limit
/// @param swing1UpperAngle Upper swing1 angle limit
/// @param swing2LowerAngle Lower swing2 angle limit
/// @param swing2UpperAngle Upper swing2 angle limit
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxUniversalJointSetAngleLimit(
	PfxJoint &joint,
	PfxFloat swing1LowerAngle,
	PfxFloat swing1UpperAngle,
	PfxFloat swing2LowerAngle,
	PfxFloat swing2UpperAngle);

SCE_PFX_API void pfxComputeUniversalJointError(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	float(&out_error)[6]);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_UNIVERSAL_H
