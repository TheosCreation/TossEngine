/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_moving_sphere_capsule.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectMovingSphereCapsule(const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,const PfxCapsule &capsule,const PfxTransform3 &transform)
{
	// レイをカプセルのローカル座標へ変換
	PfxTransform3 transformCapsule = orthoInverse(transform);
	PfxVector3 startPosL = transformCapsule.getUpper3x3() * sphereIn.m_startPosition + transformCapsule.getTranslation();
	PfxVector3 rayDirL = transformCapsule.getUpper3x3() * sphereIn.m_direction;
	PfxFloat rayRadius = sphereIn.m_radius;
	
	PfxVector3 capsuleP1(-capsule.m_halfLen, 0.f, 0.f);
	PfxVector3 capsuleP2( capsule.m_halfLen, 0.f, 0.f);

	PfxFloat tempVariable = 0.f;
	if (pfxIntersectRayCapsule(startPosL, rayDirL, capsuleP1, capsuleP2, rayRadius + capsule.m_radius, tempVariable) && tempVariable < sphereOut.m_variable) {
		PfxVector3 normalOnCapsule;
		PfxVector3 normal;

		// Detect the closest point on capsule A
		PfxVector3 stopPoint = sphereIn.m_startPosition + tempVariable * sphereIn.m_direction;
		PfxVector3 closestPoint = stopPoint;
		PfxVector3 capA1 = transform.getTranslation() + transform.getUpper3x3() * capsuleP1;
		PfxVector3 capA2 = transform.getTranslation() + transform.getUpper3x3() * capsuleP2;
		pfxClosestPointLine(stopPoint, capA1, capA2 - capA1, closestPoint);
		if (lengthSqr(stopPoint - closestPoint) > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			normalOnCapsule = normalize(stopPoint - closestPoint);
			closestPoint = closestPoint + capsule.m_radius * normalOnCapsule;
		}
		else {
			normalOnCapsule = transform.getUpper3x3() * PfxVector3::zAxis();
			closestPoint = closestPoint + capsule.m_radius * normalOnCapsule;
		}

		// Adjust the position of contact point
		PfxVector3 vectorFromStopPointToCapsule = closestPoint - stopPoint;
		PfxFloat distanceFromStopPointToCapsule = length(vectorFromStopPointToCapsule);
		if (distanceFromStopPointToCapsule > rayRadius + SCE_PFX_INTERSECT_COMMON_EPSILON){
			vectorFromStopPointToCapsule /= distanceFromStopPointToCapsule;
			closestPoint = stopPoint + vectorFromStopPointToCapsule * rayRadius;
			normal = -vectorFromStopPointToCapsule;
		}
		else if (distanceFromStopPointToCapsule >= SCE_PFX_INTERSECT_COMMON_EPSILON)
			normal = -vectorFromStopPointToCapsule / distanceFromStopPointToCapsule;

		else
			normal = normalOnCapsule;

		// Set the output parameters
		sphereOut.m_contactPoint = closestPoint;
		sphereOut.m_contactNormal = normal;
		sphereOut.m_contactFlag = true;
		sphereOut.m_variable = tempVariable;
		sphereOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	return false;
}

} //namespace pfxv4
} //namespace sce
