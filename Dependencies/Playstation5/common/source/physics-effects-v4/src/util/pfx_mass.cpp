/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../include/physics_effects/util/pfx_mass.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Box

PfxFloat pfxCalcMassBox(PfxFloat density,const PfxVector3 &halfExtent)
{
	return density * halfExtent[0] * halfExtent[1] * halfExtent[2] * 8;
}

PfxMatrix3 pfxCalcInertiaBox(const PfxVector3 &halfExtent,PfxFloat mass)
{
	PfxVector3 sqrSz = halfExtent * 2.0f;
	sqrSz = mulPerElem(sqrSz,sqrSz);
	PfxMatrix3 inertia = PfxMatrix3::identity();
	inertia[0][0] = (mass*(sqrSz[1]+sqrSz[2]))/12.0f;
	inertia[1][1] = (mass*(sqrSz[0]+sqrSz[2]))/12.0f;
	inertia[2][2] = (mass*(sqrSz[0]+sqrSz[1]))/12.0f;
	return inertia;
}

///////////////////////////////////////////////////////////////////////////////
// Sphere

PfxFloat pfxCalcMassSphere(PfxFloat density,PfxFloat radius)
{
	return (4.0f/3.0f) * SCE_PFX_PI * radius * radius * radius * density;
}

PfxMatrix3 pfxCalcInertiaSphere(PfxFloat radius,PfxFloat mass)
{
	PfxMatrix3 inertia = PfxMatrix3::identity();
	inertia[0][0] = inertia[1][1] = inertia[2][2] = 0.4f * mass * radius * radius;
	return inertia;
}

///////////////////////////////////////////////////////////////////////////////
// Cylinder

PfxFloat pfxCalcMassCylinder(PfxFloat density,PfxFloat halfLength,PfxFloat radius)
{
	return SCE_PFX_PI * radius * radius * 2.0f * halfLength * density;
}

static inline
PfxMatrix3 pfxCalcInertiaCylinder(PfxFloat halfLength,PfxFloat radius,PfxFloat mass,int axis)
{
	PfxMatrix3 inertia = PfxMatrix3::identity();
	inertia[0][0] = inertia[1][1] = inertia[2][2] = mass / 12.0f * (3.0f * radius * radius + 4.0f * halfLength * halfLength);
	inertia[axis][axis] = 0.5f * mass * radius * radius;
	return inertia;
}

PfxMatrix3 pfxCalcInertiaCylinderX(PfxFloat halfLength,PfxFloat radius,PfxFloat mass)
{
	return pfxCalcInertiaCylinder(halfLength,radius,mass,0);
}

PfxMatrix3 pfxCalcInertiaCylinderY(PfxFloat halfLength,PfxFloat radius,PfxFloat mass)
{
	return pfxCalcInertiaCylinder(halfLength,radius,mass,1);
}

PfxMatrix3 pfxCalcInertiaCylinderZ(PfxFloat halfLength,PfxFloat radius,PfxFloat mass)
{
	return pfxCalcInertiaCylinder(halfLength,radius,mass,2);
}

///////////////////////////////////////////////////////////////////////////////
// Capsule

PfxFloat pfxCalcMassCapsule(PfxFloat density,PfxFloat halfLength,PfxFloat radius)
{
	return SCE_PFX_PI * radius * radius * (4.0f + 2.0f * halfLength) * density;
}

static inline
PfxMatrix3 pfxCalcInertiaCapsule(PfxFloat halfLength,PfxFloat radius,PfxFloat mass,int axis)
{
	PfxMatrix3 inertia = PfxMatrix3::identity();
	PfxFloat volumeRatio = 2.0f / (2.0f + halfLength);
	PfxFloat massSphere = volumeRatio * mass;
	PfxFloat massCylinder = (1.0f - volumeRatio) * mass;
	
	// Cylinder + Sphere
	PfxFloat Iaxis = 0.5f * massCylinder * radius * radius + 0.4f * massSphere * radius * radius;
	
	// Cylinder + x2 Hemi-Sphere
	PfxFloat Iside = massCylinder / 12.0f * (3.0f * radius * radius + 4.0f * halfLength * halfLength);
	Iside += 0.6475f * 0.4f * massSphere * radius * radius + 
		massSphere * 0.125f * (halfLength * halfLength + 6.0f * radius * halfLength);
	
	inertia[0][0] = inertia[1][1] = inertia[2][2] = Iside;
	inertia[axis][axis] = Iaxis;
	return inertia;
}

PfxMatrix3 pfxCalcInertiaCapsuleX(PfxFloat halfLength,PfxFloat radius,PfxFloat mass)
{
	return pfxCalcInertiaCapsule(halfLength,radius,mass,0);
}

PfxMatrix3 pfxCalcInertiaCapsuleY(PfxFloat halfLength,PfxFloat radius,PfxFloat mass)
{
	return pfxCalcInertiaCapsule(halfLength,radius,mass,1);
}

PfxMatrix3 pfxCalcInertiaCapsuleZ(PfxFloat halfLength,PfxFloat radius,PfxFloat mass)
{
	return pfxCalcInertiaCapsule(halfLength,radius,mass,2);
}

///////////////////////////////////////////////////////////////////////////////

PfxMatrix3 pfxMassTranslate(PfxFloat mass,const PfxMatrix3 &inertia,const PfxVector3 &translation)
{
	PfxMatrix3 m = crossMatrix(translation);
	return inertia + mass * (-m*m);
}

PfxMatrix3 pfxMassRotate(const PfxMatrix3 &inertia,const PfxMatrix3 &rotate)
{
	return rotate * inertia * transpose(rotate);
}

} //namespace pfxv4
} //namespace sce
