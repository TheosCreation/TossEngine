/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_JOINT_CONSTRAINT_FUNC_H_
#define _SCE_PFX_JOINT_CONSTRAINT_FUNC_H_

#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/solver/pfx_solver_body.h"
#include "../../base_level/solver/pfx_joint.h"
#include "../../base_level/solver/pfx_joint_constraint.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Setup Joint Constraint Func

/// @brief Definition of the joint setup function
/// @details Define the function called in the joint setup stage.
/// @param[in,out] joint Joint
/// @param[in] stateA Rigid body A state
/// @param[in] stateB Rigid body B state
/// @param[out] solverBodyA Solver body A
/// @param[out] solverBodyB Solver body B
/// @param[in] timeStep Time step
SCE_PFX_API typedef void(*PfxSetupJointConstraintFunc)(
	PfxJoint &joint,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat timeStep);

/// @brief Get the joint setup function
/// @details Get the joint setup function defined by jointType
/// @param jointType Joint type
/// @return Return the joint setup function
SCE_PFX_API PfxSetupJointConstraintFunc pfxGetSetupJointConstraintFunc(PfxUInt8 jointType);

/// @brief Set the joint setup function
/// @details Set the joint setup function defined by jointType
/// @param jointType Joint type
/// @param func Joint setup function
/// @return Return the joint setup function
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxSetSetupJointConstraintFunc(PfxUInt8 jointType,PfxSetupJointConstraintFunc func);

///////////////////////////////////////////////////////////////////////////////
// Warm Start Joint Constraint Func

/// @brief Definition of the joint warm starting function
/// @details Define the function called in the joint warm starting stage.
/// @param[in,out] joint Joint
/// @param[in,out] solverBodyA Solver body A
/// @param[in,out] solverBodyB Solver body B
typedef void (*PfxWarmStartJointConstraintFunc)(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

/// @brief Get the joint warm starting function
/// @details Get the joint warm starting function defined by jointType
/// @param jointType Joint type
/// @return Return the joint warm starting function
SCE_PFX_API PfxWarmStartJointConstraintFunc pfxGetWarmStartJointConstraintFunc(PfxUInt8 jointType);

/// @brief Set the joint warm starting function
/// @details Set the joint warm starting function defined by jointType
/// @param jointType Joint type
/// @param func Joint warm starting function
/// @return Return the joint warm starting function
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxSetWarmStartJointConstraintFunc(PfxUInt8 jointType,PfxWarmStartJointConstraintFunc func);

///////////////////////////////////////////////////////////////////////////////
// Solve Joint Constraint Func

/// @brief Definition of the joint joint solving function
/// @details Define the function called in the joint solving stage.
/// @param[in,out] joint Joint
/// @param[in,out] solverBodyA Solver body A
/// @param[in,out] solverBodyB Solver body B
typedef void (*PfxSolveJointConstraintFunc)(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

/// @brief Get the joint solving function
/// @details Get the joint solving function defined by jointType
/// @param jointType Joint type
/// @return Return the joint solving function
SCE_PFX_API PfxSolveJointConstraintFunc pfxGetSolveJointConstraintFunc(PfxUInt8 jointType);

/// @brief Set the joint solving function
/// @details Set the joint solving function defined by jointType
/// @param jointType Joint type
/// @param func Joint solving function
/// @return Return the joint solving function
/// @retval SCE_PFX_OK Success 
/// @retval (<0) Error code 
SCE_PFX_API PfxInt32 pfxSetSolveJointConstraintFunc(PfxUInt8 jointType,PfxSolveJointConstraintFunc func);

typedef void (*PfxCorrectJointPositionConstraintFunc)(
	PfxJoint &joint,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB);

SCE_PFX_API PfxCorrectJointPositionConstraintFunc pfxGetCorrectJointPositionConstraintFunc(PfxUInt8 jointType);

SCE_PFX_API PfxInt32 pfxSetCorrectJointPositionConstraintFunc(PfxUInt8 jointType, PfxCorrectJointPositionConstraintFunc func);

} //namespace pfxv4
} //namespace sce

#endif /* _PFX_JOINT_CONSTRAINT_FUNC_H_ */
