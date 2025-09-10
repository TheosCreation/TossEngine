/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/solver/pfx_integrate.h"

namespace sce {
namespace pfxv4 {

#define SCE_PFX_MOTION_MASK_INTEGRATE ((1<<kPfxMotionTypeFixed)|(1<<kPfxMotionTypeTrigger))
#define SCE_PFX_MOTION_MASK_APPLYFORCE ((1<<kPfxMotionTypeFixed)|(1<<kPfxMotionTypeTrigger)|(1<<kPfxMotionTypeKeyframe)|(1<<kPfxMotionTypeCollisionKeyframe))

void pfxIntegrate(
	PfxRigidState &state,
	const PfxRigidBody &body,
	PfxFloat timeStep
	)
{
	(void) body;
	if(((1<<state.getMotionType())&SCE_PFX_MOTION_MASK_INTEGRATE) || state.isAsleep()) return;
	
	PfxVector3 position = state.getPosition();
	PfxQuat    orientation = state.getOrientation();
	PfxVector3 linearVelocity = state.getLinearVelocity();
	PfxVector3 angularVelocity = state.getAngularVelocity();
	
	SCE_PFX_VALIDATE_VECTOR3(position);
	SCE_PFX_VALIDATE_QUAT(orientation);

	position += linearVelocity * timeStep;
	state.setPosition(position);
#if 0
	orientation += PfxQuat(angularVelocity,0) * orientation * 0.5f * timeStep;
	orientation = normalize(orientation);
#else
	const PfxFloat epsilon = 0.000001f;
	PfxFloat w = length(angularVelocity);
	if( w > epsilon ) {
		PfxQuat dAng = PfxQuat::rotation(timeStep * w,angularVelocity / w);
		if(dot(dAng.getXYZ(),angularVelocity) < 0.0f) dAng = -dAng;
		orientation = normalize(dAng * orientation);
	}
	else {
		orientation += PfxQuat(angularVelocity, 0) * orientation * 0.5f * timeStep;
		orientation = normalize(orientation);
	}
#endif
	state.setOrientation(orientation);

	state.setLinearVelocity(linearVelocity * state.getLinearDamping());
	state.setAngularVelocity(angularVelocity * state.getAngularDamping());
}

void pfxApplyExternalForce(
	PfxRigidState &state,
	const PfxRigidBody &body,
	const PfxVector3 &extForce,
	const PfxVector3 &extTorque,
	PfxFloat timeStep
	)
{
	if(((1<<state.getMotionType())&SCE_PFX_MOTION_MASK_APPLYFORCE) || state.isAsleep()) return;
	
	SCE_PFX_ASSERT(body.getMass() > 0.0f);

	PfxVector3 angularVelocity = state.getAngularVelocity();
	PfxMatrix3 orientation(state.getOrientation());
	PfxMatrix3 worldInertia = orientation * body.getInertia() * transpose(orientation);
	PfxMatrix3 worldInertiaInv = orientation * body.getInertiaInv() * transpose(orientation);
	PfxVector3 angularMomentum = worldInertia * angularVelocity;
	PfxVector3 linearVelocity = state.getLinearVelocity();
	
	linearVelocity += extForce * body.getMassInv() * timeStep;
	angularMomentum += extTorque * timeStep;
	angularVelocity = worldInertiaInv * angularMomentum;
	
	PfxFloat linVelSqr = lengthSqr(linearVelocity);
	if(linVelSqr > state.getMaxLinearVelocity() * state.getMaxLinearVelocity()) {
		linearVelocity = (linearVelocity*rsqrtf(PfxFloatInVec(linVelSqr))) * state.getMaxLinearVelocity();
	}

	PfxFloat angVelSqr = lengthSqr(angularVelocity);
	if(angVelSqr > state.getMaxAngularVelocity() * state.getMaxAngularVelocity()) {
		angularVelocity = (angularVelocity*rsqrtf(PfxFloatInVec(angVelSqr))) * state.getMaxAngularVelocity();
	}

	SCE_PFX_VALIDATE_VECTOR3(linearVelocity);
	SCE_PFX_VALIDATE_VECTOR3(angularVelocity);

	state.setLinearVelocity(linearVelocity);
	state.setAngularVelocity(angularVelocity);
}

} //namespace pfxv4
} //namespace sce
