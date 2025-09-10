/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_CONSTRAINT_H
#define _SCE_PFX_CONTACT_CONSTRAINT_H

#include "../rigidbody/pfx_rigid_state.h"
#include "pfx_constraint_row.h"
#include "pfx_solver_body.h"

namespace sce {
namespace pfxv4 {

SCE_PFX_API void pfxSetupContactConstraint(
	PfxConstraintRow &constraintResponse,
	PfxConstraintRow &constraintFriction1,
	PfxConstraintRow &constraintFriction2,
	PfxFloat penetrationDepth,
	PfxFloat restitution,
	PfxFloat friction,
	const PfxVector3 &contactNormal,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSolverBody &solverBodyA,
	const PfxSolverBody &solverBodyB,
	PfxFloat separateBias,
	PfxFloat timeStep
	);

SCE_PFX_API void pfxSolveContactConstraint(
	PfxConstraintRow &constraintResponse,
	PfxConstraintRow &constraintFriction1,
	PfxConstraintRow &constraintFriction2,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat friction
	);

SCE_PFX_API void pfxCorrectContactPositionConstraint(
	PfxConstraintRow &constraintResponse,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat contactBias
	);

SCE_PFX_API void pfxSetupClosestConstraint(
	PfxConstraintRow &constraintResponse,
	PfxFloat distance,
	const PfxVector3 &contactNormal,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSolverBody &solverBodyA,
	const PfxSolverBody &solverBodyB,
	PfxFloat timeStep
	);

SCE_PFX_API void pfxSolveClosestConstraint(
	PfxConstraintRow &constraintResponse,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat friction
	);

SCE_PFX_API void pfxSetupRollingFriction(
	PfxConstraintRow &rollingFrictionConstraint,
	PfxFloat rollingFriction,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSolverBody &solverBodyA,
	const PfxSolverBody &solverBodyB
	);

SCE_PFX_API void pfxSolveRollingFriction(
	PfxConstraintRow &rollingFrictionConstraint,
	PfxFloat rollingFriction,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB
	);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_CONTACT_CONSTRAINT_H
