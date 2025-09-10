/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/solver/pfx_contact_constraint.h"
#include "../../../include/physics_effects/base_level/collision/pfx_contact_manifold.h"
#include "pfx_constraint_row_solver.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_CONTACT_SLOP 0.001f

void pfxSetupContactConstraint(
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
	)
{
	(void)friction;

	PfxVector3 rA = rotate(solverBodyA.m_orientation,contactPointA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation,contactPointB);

	PfxFloat massInvA = solverBodyA.m_massInv;
	PfxFloat massInvB = solverBodyB.m_massInv;
	PfxMatrix3 inertiaInvA = solverBodyA.m_inertiaInv;
	PfxMatrix3 inertiaInvB = solverBodyB.m_inertiaInv;

	if(SCE_PFX_UNLIKELY(solverBodyA.m_motionType == kPfxMotionTypeOneWay)) {
		massInvB = 0.0f;
		inertiaInvB = PfxMatrix3(0.0f);
	}
	if(SCE_PFX_UNLIKELY(solverBodyB.m_motionType == kPfxMotionTypeOneWay)) {
		massInvA = 0.0f;
		inertiaInvA = PfxMatrix3(0.0f);
	}

	PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(massInvA + massInvB)) - 
			crossMatrix(rA) * inertiaInvA * crossMatrix(rA) - 
			crossMatrix(rB) * inertiaInvB * crossMatrix(rB);

	PfxVector3 vA = stateA.getLinearVelocity() + cross(stateA.getAngularVelocity(),rA);
	PfxVector3 vB = stateB.getLinearVelocity() + cross(stateB.getAngularVelocity(),rB);
	PfxVector3 vAB = vA-vB;

	PfxVector3 tangent1,tangent2;
	pfxGetPlaneSpace(contactNormal,tangent1,tangent2);

	separateBias = pfxModifyLinearBias(fabsf(penetrationDepth),separateBias);
	
	if (dot(vAB, contactNormal) > -0.5f) restitution = 0.0f;

	// Contact Constraint
	PfxFloat denom0 = dot(K*contactNormal, contactNormal) + SCE_PFX_CONSTRAINT_STABILIZE_FACTOR;
	PfxFloat denom1 = dot(K*tangent1, tangent1) + SCE_PFX_CONSTRAINT_STABILIZE_FACTOR;
	PfxFloat denom2 = dot(K*tangent2, tangent2) + SCE_PFX_CONSTRAINT_STABILIZE_FACTOR;

	PfxVector4 denInv(denom0, denom1, denom2, timeStep);
#ifdef PFX_ENABLE_AVX
	denInv.set128(sce_vectormath_recip_f4(denInv.get128()));
#else
	denInv = divPerElem(PfxVector4(1.0f), denInv);
#endif

	constraintResponse.m_jacDiagInv = denInv[0];
	constraintFriction1.m_jacDiagInv = denInv[1];
	constraintFriction2.m_jacDiagInv = denInv[2];

	constraintResponse.m_rhs = -(1.0f+restitution)*dot(vAB,contactNormal) - (separateBias * SCE_PFX_MIN(0.0f,penetrationDepth+SCE_PFX_CONTACT_SLOP)) * denInv[3]; // velocity error + position error
	constraintResponse.m_rhs *= constraintResponse.m_jacDiagInv;
	constraintFriction1.m_rhs = -dot(vAB,tangent1) * constraintFriction1.m_jacDiagInv;
	constraintFriction2.m_rhs = -dot(vAB,tangent2) * constraintFriction2.m_jacDiagInv;

	constraintResponse.setNormal(contactNormal);
	constraintFriction1.setNormal(tangent1);
	constraintFriction2.setNormal(tangent2);
}

void pfxSolveContactConstraint(
	PfxConstraintRow &constraintResponse,
	PfxConstraintRow &constraintFriction1,
	PfxConstraintRow &constraintFriction2,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat friction
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation,contactPointA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation,contactPointB);

	PfxFloat massInvA = solverBodyA.m_massInv;
	PfxFloat massInvB = solverBodyB.m_massInv;
	PfxMatrix3 inertiaInvA = solverBodyA.m_inertiaInv;
	PfxMatrix3 inertiaInvB = solverBodyB.m_inertiaInv;

	if(SCE_PFX_UNLIKELY(solverBodyA.m_motionType == kPfxMotionTypeOneWay)) {
		massInvB = 0.0f;
		inertiaInvB = PfxMatrix3(0.0f);
	}
	if(SCE_PFX_UNLIKELY(solverBodyB.m_motionType == kPfxMotionTypeOneWay)) {
		massInvA = 0.0f;
		inertiaInvA = PfxMatrix3(0.0f);
	}

	pfxSolveLinearConstraintRow(constraintResponse, 0.0f, SCE_PFX_FLT_MAX,
		solverBodyA.m_deltaLinearVelocity,solverBodyA.m_deltaAngularVelocity,massInvA,inertiaInvA,rA,
		solverBodyB.m_deltaLinearVelocity,solverBodyB.m_deltaAngularVelocity,massInvB,inertiaInvB,rB);

	PfxFloat mf = friction*fabsf(constraintResponse.m_accumImpulse);
	PfxFloat lowerLimit = -mf;
	PfxFloat upperLimit =  mf;

	pfxSolveLinearConstraintRow(constraintFriction1, lowerLimit, upperLimit,
		solverBodyA.m_deltaLinearVelocity,solverBodyA.m_deltaAngularVelocity,massInvA,inertiaInvA,rA,
		solverBodyB.m_deltaLinearVelocity,solverBodyB.m_deltaAngularVelocity,massInvB,inertiaInvB,rB);

	pfxSolveLinearConstraintRow(constraintFriction2, lowerLimit, upperLimit,
		solverBodyA.m_deltaLinearVelocity,solverBodyA.m_deltaAngularVelocity,massInvA,inertiaInvA,rA,
		solverBodyB.m_deltaLinearVelocity,solverBodyB.m_deltaAngularVelocity,massInvB,inertiaInvB,rB);
}

void pfxCorrectContactPositionConstraint(
	PfxConstraintRow &constraintResponse,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat contactBias
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation, contactPointA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation, contactPointB);

	PfxVector3 pA = solverBodyA.m_deltaLinearVelocity + rA; // Note : use m_deltaLinearVelocity as position
	PfxVector3 pB = solverBodyB.m_deltaLinearVelocity + rB; // Note : use m_deltaLinearVelocity as position

	PfxFloat massInvA = solverBodyA.m_massInv;
	PfxFloat massInvB = solverBodyB.m_massInv;
	PfxMatrix3 inertiaInvA = solverBodyA.m_inertiaInv;
	PfxMatrix3 inertiaInvB = solverBodyB.m_inertiaInv;

	if (SCE_PFX_UNLIKELY(solverBodyA.m_motionType == kPfxMotionTypeOneWay)) {
		massInvB = 0.0f;
		inertiaInvB = PfxMatrix3(0.0f);
	}
	if (SCE_PFX_UNLIKELY(solverBodyB.m_motionType == kPfxMotionTypeOneWay)) {
		massInvA = 0.0f;
		inertiaInvA = PfxMatrix3(0.0f);
	}

	PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(massInvA + massInvB)) - 
			crossMatrix(rA) * inertiaInvA * crossMatrix(rA) - 
			crossMatrix(rB) * inertiaInvB * crossMatrix(rB);

	PfxVector3 normal = constraintResponse.getNormal();
	PfxFloat penetrationDepth = dot(normal, pA - pB);
	PfxFloat posErr = SCE_PFX_MIN(0.0f,penetrationDepth + SCE_PFX_CONTACT_SLOP);
	PfxFloat denom = dot(K * normal, normal);
	
	contactBias = pfxModifyLinearBias(fabsf(penetrationDepth),contactBias);
	
	PfxFloat impulse = -contactBias * posErr / denom;
	
	solverBodyA.m_deltaLinearVelocity += impulse * massInvA * normal;// Note : use m_deltaLinearVelocity as position
	solverBodyB.m_deltaLinearVelocity -= impulse * massInvB * normal;// Note : use m_deltaLinearVelocity as position
	
	PfxVector3 dOriA = impulse * inertiaInvA * cross(rA, normal);
	solverBodyA.m_orientation = pfxAddRotation(solverBodyA.m_orientation, dOriA);
	
	PfxVector3 dOriB = -impulse * inertiaInvB * cross(rB, normal);
	solverBodyB.m_orientation = pfxAddRotation(solverBodyB.m_orientation, dOriB);
}

void pfxSetupClosestConstraint(
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
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation,contactPointA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation,contactPointB);

	PfxFloat massInvA = solverBodyA.m_massInv;
	PfxFloat massInvB = solverBodyB.m_massInv;
	PfxMatrix3 inertiaInvA = solverBodyA.m_inertiaInv;
	PfxMatrix3 inertiaInvB = solverBodyB.m_inertiaInv;

	if(SCE_PFX_UNLIKELY(solverBodyA.m_motionType == kPfxMotionTypeOneWay)) {
		massInvB = 0.0f;
		inertiaInvB = PfxMatrix3(0.0f);
	}
	if(SCE_PFX_UNLIKELY(solverBodyB.m_motionType == kPfxMotionTypeOneWay)) {
		massInvA = 0.0f;
		inertiaInvA = PfxMatrix3(0.0f);
	}

	PfxMatrix3 K = PfxMatrix3::scale(PfxVector3(massInvA + massInvB)) - 
			crossMatrix(rA) * inertiaInvA * crossMatrix(rA) - 
			crossMatrix(rB) * inertiaInvB * crossMatrix(rB);

	PfxVector3 vA = stateA.getLinearVelocity() + cross(stateA.getAngularVelocity(),rA);
	PfxVector3 vB = stateB.getLinearVelocity() + cross(stateB.getAngularVelocity(),rB);
	PfxVector3 vAB = vA-vB;

	PfxVector3 normal = contactNormal;
	
	{
		PfxFloat denom = dot(K*normal,normal);
		constraintResponse.m_jacDiagInv = 1.0f/denom;
		constraintResponse.m_rhs = -(dot(vAB,normal) + distance / timeStep) * constraintResponse.m_jacDiagInv;
		constraintResponse.setNormal(normal);
	}
}

void pfxSolveClosestConstraint(
	PfxConstraintRow &constraintResponse,
	const PfxVector3 &contactPointA,
	const PfxVector3 &contactPointB,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB,
	PfxFloat friction
	)
{
	PfxVector3 rA = rotate(solverBodyA.m_orientation,contactPointA);
	PfxVector3 rB = rotate(solverBodyB.m_orientation,contactPointB);

	PfxFloat massInvA = solverBodyA.m_massInv;
	PfxFloat massInvB = solverBodyB.m_massInv;
	PfxMatrix3 inertiaInvA = solverBodyA.m_inertiaInv;
	PfxMatrix3 inertiaInvB = solverBodyB.m_inertiaInv;

	if(SCE_PFX_UNLIKELY(solverBodyA.m_motionType == kPfxMotionTypeOneWay)) {
		massInvB = 0.0f;
		inertiaInvB = PfxMatrix3(0.0f);
	}
	if(SCE_PFX_UNLIKELY(solverBodyB.m_motionType == kPfxMotionTypeOneWay)) {
		massInvA = 0.0f;
		inertiaInvA = PfxMatrix3(0.0f);
	}

	pfxSolveLinearConstraintRow(constraintResponse, 0.0f, SCE_PFX_FLT_MAX,
		solverBodyA.m_deltaLinearVelocity,solverBodyA.m_deltaAngularVelocity,massInvA,inertiaInvA,rA,
		solverBodyB.m_deltaLinearVelocity,solverBodyB.m_deltaAngularVelocity,massInvB,inertiaInvB,rB);
}

void pfxSetupRollingFriction(
	PfxConstraintRow &rollingFrictionConstraint,
	PfxFloat rollingFriction,
	const PfxRigidState &stateA,
	const PfxRigidState &stateB,
	const PfxSolverBody &solverBodyA,
	const PfxSolverBody &solverBodyB
	)
{
	PfxVector3 angAB = stateA.getAngularVelocity() - stateB.getAngularVelocity();
	if(lengthSqr(angAB) > 1e-8f) {
		PfxVector3 normal = normalize(angAB);
		PfxFloat denom = dot((solverBodyA.m_inertiaInv + solverBodyB.m_inertiaInv)*normal, normal);
		PfxFloat jacDiagInv = 0.0f;
		if (fabsf(denom) > 1e-8f) jacDiagInv = 1.0f / denom;
		rollingFrictionConstraint.m_rhs = -dot(angAB,normal); // velocity error
		rollingFrictionConstraint.m_rhs *= jacDiagInv;
		rollingFrictionConstraint.m_jacDiagInv = jacDiagInv;
		rollingFrictionConstraint.setNormal(normal);
	}
	else {
		rollingFrictionConstraint.setNormal(PfxVector3::zero());
		rollingFrictionConstraint.m_accumImpulse = 0.0f;
	}
}

void pfxSolveRollingFriction(
	PfxConstraintRow &rollingFrictionConstraint,
	PfxFloat rollingFriction,
	PfxSolverBody &solverBodyA,
	PfxSolverBody &solverBodyB
	)
{
	if(lengthSqr(rollingFrictionConstraint.getNormal()) > 1e-4f) {
		PfxVector3 rA(0.0f),rB(0.0f);
		pfxSolveAngularConstraintRow(
			rollingFrictionConstraint, -rollingFriction, rollingFriction,
			solverBodyA.m_deltaAngularVelocity,solverBodyA.m_inertiaInv,rA,
			solverBodyB.m_deltaAngularVelocity,solverBodyB.m_inertiaInv,rB);
	}
}

} //namespace pfxv4
} //namespace sce
