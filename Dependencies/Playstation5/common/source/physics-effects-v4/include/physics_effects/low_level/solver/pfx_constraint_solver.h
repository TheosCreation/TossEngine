/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONSTRAINT_SOLVER_H
#define _SCE_PFX_CONSTRAINT_SOLVER_H

#include "../../base_level/rigidbody/pfx_rigid_body.h"
#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/solver/pfx_constraint_pair.h"
#include "../../base_level/solver/pfx_joint.h"
#include "../../base_level/collision/pfx_contact_manifold.h"
#include "../../low_level/rigidbody/pfx_compound_container.h"
#include "../rigidbody/pfx_rigid_body_context.h"
#include "../collision/pfx_contact_container.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Solve Velocity and Position Constraints

/// @brief Solver callback
/// @details This callback is called before or after the solver calculation.
/// @param userData User data
typedef void(*pfxSolveConstraintsCallback)(void *userData);

/// @brief Input parameter for pfxSolveConstraints()
/// @details This structure is used to specify the parameters to be passed to the pfxSolveConstraints().
/// In order to simultaneously solve the collision and joint constraints,
/// operations are repeated for all the constraints for the specified number of iterations.
/// The larger the number of iterations, the higher the accuracy is, but at the price of lower performance.
struct SCE_PFX_API PfxSolveConstraintsParam {
	PfxConstraintPair *contactPairs = nullptr;					///< @brief Array of contact pairs
	PfxInt32 *numContactPairsPtr = nullptr;						///< @brief Pointer to a valuable storing the number of contact pairs
	PfxContactContainer *contactContainer = nullptr;			///< @brief Contact container
	PfxConstraintPair *jointPairs = nullptr;					///< @brief Array of joint pairs
	PfxInt32 *numJointPairsPtr = nullptr;						///< @brief Pointer to a valuable storing the number of joint pairs
	PfxJoint *joints = nullptr;									///< @brief Array of joints
	PfxUInt32 velocitySolverIteration[3];						///< @brief Loop count for the velocity solver
	PfxUInt32 positionSolverIteration = 1;						///< @brief Loop count for the position solver
	PfxFloat timeStepRatio = 1.0f;								///< @brief Time step ratio (current time step / previous time step)
	PfxFloat contactBias = 1.0f;								///< @brief Weight factor for contact constraints
	PfxFloat massAmpForContacts = 1.6f;							///< @brief Mass amp factor used in shock propagation for contact pairs
	PfxFloat massAmpForJoints = 1.2f;							///< @brief Mass amp factor used in shock propagation for joint pairs
	pfxSolveConstraintsCallback preSolverCallback = nullptr;	///< @brief Callback called before solver calculation
	pfxSolveConstraintsCallback postSolverCallback = nullptr;	///< @brief Callback called after solver calculation before position correction
	PfxUInt32 numCompoundContainer = 0;							///< @brief 
	PfxCompoundContainer *compoundContainers = nullptr;			///< @brief 
	PfxBool enableSortOfPairs = true;							///< @brief Enable stabilization by sorting the pairs
	PfxUInt32 sleepCount = 180;									///< @brief Sleep count
	PfxFloat sleepThreshold = 0.1f;								///< @brief Sleep velocity
	void *preSolverUserData = nullptr;							///< @brief User data transferred to preSolverCallback
	void *postSolverUserData = nullptr;							///< @brief User data transferred to postSolverCallback

	SCE_PFX_EXCLUDE_DOC(
		PfxSolveConstraintsParam() {
			velocitySolverIteration[0] = velocitySolverIteration[1] = velocitySolverIteration[2] = 4;
		}
	)
};

/// @brief Solve constraints
/// @details This is a solver function that simultaneously solves multiple constraints caused by collisions
/// and joints between rigid bodies, using a reiterative operation solving process.
/// The constraint force output from the solver is converted to velocity and is finally reflected to
/// the rigid body state.
/// @param context Rigid body context
/// @param sharedParam Shared parameter
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxSolveConstraints(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam, PfxSolveConstraintsParam &param);

/// @brief Non blocking version of pfxSolveConstraints()
SCE_PFX_API PfxInt32 pfxDispatchSolveConstraints(PfxRigidBodyContext &context, PfxSolveConstraintsParam &param);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONSTRAINT_SOLVER_H
