/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "pfx_contact_sphere_sphere.h"

namespace sce {
namespace pfxv4 {

const PfxFloat lenSqrTol = 1.0e-30f;

PfxFloat pfxContactSphereSphere(
	PfxVector3 &normal,PfxPoint3 &pointA,PfxPoint3 &pointB,
	void *shapeA,const PfxTransform3 &transformA,
	void *shapeB,const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	PfxSphere &sphereA = *((PfxSphere*)shapeA);
	PfxSphere &sphereB = *((PfxSphere*)shapeB);

	PfxVector3 direction(0.0f);

	PfxVector3 translationA = transformA.getTranslation();
	PfxVector3 translationB = transformB.getTranslation();

	// get the offset vector between sphere centers

	PfxVector3 offsetAB;

	offsetAB = translationB - translationA;

	// normalize the offset to compute the direction vector

	PfxFloat distSqr = dot(offsetAB,offsetAB);
	PfxFloat dist = sqrtf(distSqr);
	PfxFloat sphereDist = dist - sphereA.m_radius - sphereB.m_radius;

	if ( sphereDist > distanceThreshold ) {
		return sphereDist;
	}

	if ( distSqr > lenSqrTol ) {
		PfxFloat distInv = 1.0f / dist;

		direction = offsetAB * distInv;
	} else {
		direction = PfxVector3::zAxis();
	}

	normal = direction;

	// compute the points on the spheres, in world space

	pointA = PfxPoint3( transpose(transformA.getUpper3x3()) * ( direction * sphereA.m_radius ) );
	pointB = PfxPoint3( transpose(transformB.getUpper3x3()) * ( -direction * sphereB.m_radius ) );

	// return the distance between the spheres

	return sphereDist;
}

} //namespace pfxv4
} //namespace sce
