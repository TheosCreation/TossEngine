/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_JOINT_FIX_H
#define _SCE_PFX_JOINT_FIX_H

#include "pfx_joint.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {


/// @brief Initialization parameter for the fix joint
/// @details This structure is used to initialize the fix joint.
struct SCE_PFX_API  PfxFixJointInitParam {
	PfxLargePosition anchorPoint; ///< @brief Connection position
	
	SCE_PFX_EXCLUDE_DOC(PfxFixJointInitParam() : anchorPoint() {})
};

struct SCE_PFX_API PfxFixJointInitParamLocalSpace {
	PfxVector3 anchorPointA;	    ///< @brief Connection position in A's local space
	PfxVector3 anchorPointB;	    ///< @brief Connection position in B's local space
	PfxMatrix3 jointFrameA;			///< @brief Rotation from A to the joint frame
	PfxMatrix3 jointFrameB;			///< @brief Rotation from B to the joint frame

	SCE_PFX_EXCLUDE_DOC(
		PfxFixJointInitParamLocalSpace() {
		const PfxMatrix3 identityMatrix(PfxMatrix3::identity());
		jointFrameA = identityMatrix;
		jointFrameB = identityMatrix;
	}
	)
};

/// @brief Initialize the fix joint
/// @details Initialize the joint as the fix joint
/// @param[in,out] joint Joint
/// @param stateA Rigid body A state
/// @param stateB Rigid body B state
/// @param param initialization parameter for fix joint
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxInitializeFixJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxFixJointInitParam &param);

SCE_PFX_API PfxInt32 pfxInitializeFixJoint(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxFixJointInitParamLocalSpace &param);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_JOINT_FIX_H
