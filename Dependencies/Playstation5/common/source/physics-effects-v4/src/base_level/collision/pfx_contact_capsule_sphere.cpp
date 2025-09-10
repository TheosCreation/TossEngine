/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/collision/pfx_capsule.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "pfx_contact_capsule_sphere.h"

namespace sce {
namespace pfxv4 {

inline
void
segmentPointClosestPoints(
	PfxVector3& ptsVector,
	PfxVector3& offsetA,
	PfxFloat& tA,
	const PfxVector3 &translation,
	const PfxVector3 &dirA, PfxFloat hLenA )
{
	// compute the parameters of the closest points on each line segment

	tA = dot(dirA,translation);

	if ( tA < -hLenA )
		tA = -hLenA;
	else if ( tA > hLenA )
		tA = hLenA;

	// compute the closest point on segment relative to its center.

	offsetA = dirA * tA;
	ptsVector = translation - offsetA;
}

inline
void
segmentPointNormal(     PfxVector3& normal, const  PfxVector3 &ptsVector )
{
	// compute the unit direction vector between the closest points.
	// with convex objects, you want the unit direction providing the largest gap between the
	// objects when they're projected onto it.  So, if you have a few candidates covering different
	// configurations of the objects, you can compute them all, test the gaps and pick best axis
	// based on this.  Some directions might be degenerate, and the normalized() function tests for
	// degeneracy and returns an arbitrary unit vector in that case.

	// closest points vector

	normal = pfxSafeNormalize(ptsVector);
}

PfxFloat pfxContactCapsuleSphere(
	PfxVector3 &normal,PfxPoint3 &pointA,PfxPoint3 &pointB,
	void *shapeA,const PfxTransform3 &transformA,
	void *shapeB,const PfxTransform3 &transformB,
	PfxFloat distanceThreshold)
{
	PfxCapsule &capsuleA = *((PfxCapsule*)shapeA);
	PfxSphere &sphereB = *((PfxSphere*)shapeB);

	PfxVector3 directionA = transformA.getUpper3x3().getCol0();
	PfxVector3 translationA = transformA.getTranslation();
	PfxVector3 translationB = transformB.getTranslation();

	// translation between centers of capsule and sphere

	PfxVector3 translation = translationB - translationA;

	// compute the closest point on the capsule line segment to the sphere center

	PfxVector3 ptsVector;
	PfxVector3 offsetA;
	PfxFloat tA;

	segmentPointClosestPoints( ptsVector, offsetA, tA, translation, directionA, capsuleA.m_halfLen );

	PfxFloat distance = length(ptsVector) - capsuleA.m_radius - sphereB.m_radius;

	if ( distance > distanceThreshold )
		return distance;

	// compute the contact normal

	segmentPointNormal( normal, ptsVector );

	// compute points on capsule and sphere

	pointA = PfxPoint3( transpose(transformA.getUpper3x3()) * ( offsetA + normal * capsuleA.m_radius ) );
	pointB = PfxPoint3( transpose(transformB.getUpper3x3()) * ( -normal * sphereB.m_radius ) );

	return distance;
}

} //namespace pfxv4
} //namespace sce
