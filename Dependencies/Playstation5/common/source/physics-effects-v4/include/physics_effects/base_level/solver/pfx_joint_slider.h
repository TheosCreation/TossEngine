/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_SLIDER_H
#define _SCE_PFX_JOINT_SLIDER_H

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {


/// @brief Initialization parameter for slider joint
/// @details If enableJointFrame is true, direction is ignored and jointFrame is used as
/// the local coordinates of the joint.
struct SCE_PFX_API PfxSliderJointInitParam {
	PfxLargePosition anchorPoint;	///< @brief Connection position
	PfxVector3 direction;			///< @brief Slide axis
	PfxMatrix3 jointFrame;			///< @brief Joint local coordinates
	PfxBool enableJointFrame;		///< @brief Enable joint frame
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat lowerDistance;			///< @brief Lower distance limit
	PfxFloat upperDistance;			///< @brief Upper distance limit
	SCE_PFX_PADDING(1,8)
	
	SCE_PFX_EXCLUDE_DOC(
		PfxSliderJointInitParam() : anchorPoint() {
			direction = PfxVector3(1.0f,0.0f,0.0f);
			lowerDistance = 0.0f;
			upperDistance = 0.0f;
			jointFrame = PfxMatrix3::identity();
			enableJointFrame = false;
			damping = 0.0f;
			bias = 0.2f;
		}
	)
};

struct SCE_PFX_API PfxSliderJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxMatrix3 jointFrameA;			///< @brief Rotation from A to the joint frame
	PfxMatrix3 jointFrameB;			///< @brief Rotation from B to the joint frame
	PfxFloat damping;				///< @brief Damping factor (default:0.0)
	PfxFloat bias;					///< @brief Bias factor (default:0.2)
	PfxFloat lowerDistance;			///< @brief Lower distance limit
	PfxFloat upperDistance;			///< @brief Upper distance limit

	SCE_PFX_EXCLUDE_DOC(
		PfxSliderJointInitParamLocalSpace() {
		const PfxMatrix3 identityMatrix(PfxMatrix3::identity());
		jointFrameA = identityMatrix;
		jointFrameB = identityMatrix;
		lowerDistance = 0.0f;
		upperDistance = 0.0f;
		damping = 0.0f;
		bias = 0.2f;
	}
	)
};

/// @brief Initialize the slider joint
/// @details Initialize the joint as slider joint.
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for slider joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeSliderJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSliderJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeSliderJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSliderJointInitParamLocalSpace &param);

/// @brief Get distance limit of the slider joint
/// @details Get distance limit of the slider joint.
/// @param  joint Joint
/// @param[out] lowerDistance Lower distance limit
/// @param[out] upperDistance Upper distance limit
SCE_PFX_API void pfxSliderJointGetDistanceLimit(
	const PfxJoint &joint,
	PfxFloat &lowerDistance,
	PfxFloat &upperDistance);

/// @brief Set distance limit of the slider joint
/// @details Set distance limit of the slider joint.
/// @param[in,out] joint Joint
/// @param lowerDistance Lower distance limit
/// @param upperDistance Upper distance limit
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxSliderJointSetDistanceLimit(
	PfxJoint &joint,
	PfxFloat lowerDistance,
	PfxFloat upperDistance);

SCE_PFX_API PfxFloat pfxComputeSliderJointDistance(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_SLIDER_H
