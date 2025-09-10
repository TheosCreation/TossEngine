/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_moving_sphere_cylinder.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectRayCylinderTorus(
	const PfxVector3 &intersection,
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	PfxFloat rayRadius,
	const PfxVector3 &cylinderPosition,
	const PfxVector3 &cylinderNormal,
	PfxFloat cylinderRadius,
	PfxFloat &variable)
{
	bool exit = false;
	
	PfxVector3 pointOnSphere;
	
	// Find the initial sphere on the edge of the cylinder circle
	{
		PfxVector3 v1 = intersection - cylinderPosition;
		PfxVector3 v2 = dot(v1,cylinderNormal) * cylinderNormal;
		PfxVector3 centerToEdgeVec = cylinderRadius * normalize(v1 - v2);

		PfxVector3 candidates[3];
		candidates[0] = cylinderPosition + centerToEdgeVec;
		candidates[1] = cylinderPosition + rotate(PfxQuat::rotation( SCE_PFX_PI*0.15f,cylinderNormal),centerToEdgeVec);
		candidates[2] = cylinderPosition + rotate(PfxQuat::rotation(-SCE_PFX_PI*0.15f,cylinderNormal),centerToEdgeVec);

		int i;
		for(i=0;i<3;i++) {
			PfxFloat t;
			if(pfxIntersectRaySphere(rayStartPosition,rayDirection,candidates[i],rayRadius,t)) {
				pointOnSphere = rayStartPosition + t * rayDirection;
				break;
			}
		}
		
		// If there is no intersection, this function returns false.
		if(i == 3) {
			return false;
		}
	}

	// Search for intersection in an iterative manner.
	for(int i=0;i<10&&!exit;i++) {
		PfxVector3 v1 = pointOnSphere - cylinderPosition;
		PfxVector3 v2 = dot(v1,cylinderNormal) * cylinderNormal;

		PfxVector3 pointOnEdge = cylinderPosition + cylinderRadius * normalize(v1 - v2);
		
		PfxFloat t;
		if(!pfxIntersectRaySphere(rayStartPosition,rayDirection,pointOnEdge,rayRadius,t)) {
			return false;
		}
		
		PfxVector3 newPointOnSphere = rayStartPosition + t * rayDirection;
		
		if(lengthSqr(pointOnSphere - newPointOnSphere) < SCE_PFX_INTERSECT_COMMON_EPSILON) {
			variable = SCE_PFX_CLAMP(t,0.0f,1.0f);
			return true;
		}
		
		pointOnSphere = newPointOnSphere;
	}
	
	return false;
}

PfxBool pfxIntersectMovingSphereCylinder(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	PfxFloat rayRadius,
	const PfxVector3 &cylinderP1,
	const PfxVector3 &cylinderP2,
	PfxFloat cylinderRadius,
	PfxFloat &variable)
{
	// [SMS_CHANGE] START: moved the check at the beginning so it will work even with a zero length ray.
	// Check if the start position is in the cylinder. 
	{
		PfxVector3 closestPoint;
		pfxClosestPointCylinder( rayStartPosition, cylinderP1, cylinderP2, cylinderRadius, closestPoint );
		const PfxFloat squaredDistance = lengthSqr( rayStartPosition - closestPoint );
		const PfxFloat squaredRadius   = rayRadius * rayRadius;
		if( squaredDistance < squaredRadius )
		{
			variable = 0.0f;
			return true;
		}
	}
	// [SMS_CHANGE] END

	const PfxVector3 cylDir = cylinderP2 - cylinderP1;
	const PfxFloat cylLenSqr = dot(cylDir,cylDir);
	const PfxFloat rayLenSqr = dot(rayDirection,rayDirection);
	
	if(rayLenSqr < SCE_PFX_INTERSECT_COMMON_EPSILON) return false;

	if(cylLenSqr < SCE_PFX_INTERSECT_COMMON_EPSILON) return false;
	
	PfxFloat cylLen = sqrtf(cylLenSqr);
	PfxVector3 unitCylDir = cylDir / cylLen;
	
	const PfxVector3 cylinderExP1 = cylinderP1 - rayRadius * unitCylDir;
	const PfxVector3 cylinderExP2 = cylinderP2 + rayRadius * unitCylDir;
	const PfxVector3 cylExDir = cylinderExP2 - cylinderExP1;
	PfxFloat cylinderExRadius = cylinderRadius + rayRadius;
	PfxFloat radExSqr = cylinderExRadius * cylinderExRadius;
	PfxFloat cylExLen = cylLen + rayRadius * 2.0f;
	
	// Check if the start position is in the cylinder. 
	{
		PfxVector3 closestPoint;
		pfxClosestPointCylinder(rayStartPosition,cylinderP1,cylinderP2,cylinderRadius,closestPoint);
		if(lengthSqr(rayStartPosition - closestPoint) < rayRadius * rayRadius) {
			variable = 0.0f;
			return true;
		}

		PfxFloat positionOnCylinderCentralAxis = dot(rayStartPosition - cylinderP1, unitCylDir);
		PfxVector3 projectedPointOnCylinderCentralAxis = positionOnCylinderCentralAxis * unitCylDir + cylinderP1;
		PfxVector3 deltaVector = rayStartPosition - projectedPointOnCylinderCentralAxis;

		if (positionOnCylinderCentralAxis >= 0.f && positionOnCylinderCentralAxis <= cylLen && lengthSqr(deltaVector) < cylinderRadius * cylinderRadius) {
			variable = 0.f;
			return true;
		}
	}
	
	// Calc intersection between the segment and the body of the capsule to define its voronoi region.
	{
		// Calc intersection between the segment and the body of the cylinder.
		do {
			PfxVector3 m = rayStartPosition - cylinderExP1;
			PfxFloat md = dot(m,unitCylDir);
			PfxFloat nd = dot(rayDirection,unitCylDir);
			PfxFloat nn = rayLenSqr;
			PfxFloat mm = dot(m,m);
			PfxFloat nm = dot(rayDirection,m);

			PfxFloat a = nn - nd * nd;
			PfxFloat b = nm - md * nd;
			PfxFloat c = mm - md * md - radExSqr;

			PfxFloat d = b * b - a * c;
			
			if(d < 0.0f) return false; // レイは逸れている
			if(fabsf(a) < SCE_PFX_INTERSECT_COMMON_EPSILON) break;
			
			PfxFloat tt = ( -b - sqrtf(d) ) / a;
			
			PfxFloat chk = md + tt * nd;
			if(chk < 0.0f || chk > cylExLen) break;
			
			if(chk < rayRadius) {
				PfxVector3 intersection = rayStartPosition + tt * rayDirection;
				if(pfxIntersectRayCylinderTorus(intersection,rayStartPosition,rayDirection,rayRadius,
					cylinderP1,-unitCylDir,cylinderRadius,tt)) {
					variable = tt;
					return true;
				}
				else {
					return false;
				}
			}
			
			if(chk > cylExLen - rayRadius) {
				PfxVector3 intersection = rayStartPosition + tt * rayDirection;
				if(pfxIntersectRayCylinderTorus(intersection,rayStartPosition,rayDirection,rayRadius,
					cylinderP2,unitCylDir,cylinderRadius,tt)) {
					variable = tt;
					return true;
				}
				else {
					return false;
				}
			}
			
			if(tt >= 0.0f && tt <= 1.0f) {
				variable = tt;
				return true;
			}
		} while(0);
		
		// Calc intersection between the segment and the x2 circles.
		do {
			PfxVector3 m = rayStartPosition - cylinderExP1;
			
			PfxFloat md = dot(m,cylExDir);
			PfxFloat nd = dot(rayDirection,cylExDir);
			
			if(fabsf(nd) < SCE_PFX_INTERSECT_COMMON_EPSILON) break;
			
			PfxFloat t1 = -md / nd;
			PfxFloat t2 = (cylExLen * cylExLen - md ) / nd;
			
			PfxVector3 l1 = m + t1 * rayDirection;
			PfxVector3 l2 = (rayStartPosition - cylinderExP2) + t2 * rayDirection;
			
			PfxFloat chk1 = lengthSqr(l1);
			PfxFloat chk2 = lengthSqr(l2);
			
			if(t1 < t2) {
				if(chk1 <= radExSqr) {
					if(chk1 > cylinderRadius * cylinderRadius) {
						PfxVector3 intersection = rayStartPosition + t1 * rayDirection;
						if(pfxIntersectRayCylinderTorus(intersection,rayStartPosition,rayDirection,rayRadius,
							cylinderP1,-unitCylDir,cylinderRadius,t1)) {
							variable = t1;
							return true;
						}
					}
					else if(t1 >= 0.0f && t1 <= 1.0f) {
						variable = t1;
						return true;
					}
				}
			}
			else {
				if(chk2 <= radExSqr) {
					if(chk2 > cylinderRadius * cylinderRadius) {
						PfxVector3 intersection = rayStartPosition + t2 * rayDirection;
						if(pfxIntersectRayCylinderTorus(intersection,rayStartPosition,rayDirection,rayRadius,
							cylinderP2,unitCylDir,cylinderRadius,t2)) {
							variable = t2;
							return true;
						}
					}
					else if(t2 >= 0.0f && t2 <= 1.0f) {
						variable = t2;
						return true;
					}
				}
			}
		} while(0);
	}

	return false;
}

PfxBool pfxIntersectMovingSphereCylinder(const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,const PfxCylinder &cylinder,const PfxTransform3 &transform)
{
	PfxVector3 startPos = sphereIn.m_startPosition;
	PfxVector3 rayDir = sphereIn.m_direction;
	PfxFloat rayRadius = sphereIn.m_radius;
	
	PfxVector3 cylinderP1(-cylinder.m_halfLen,0.0f,0.0f);
	PfxVector3 cylinderP2( cylinder.m_halfLen,0.0f,0.0f);
	cylinderP1 = transform.getTranslation() + transform.getUpper3x3() * cylinderP1;
	cylinderP2 = transform.getTranslation() + transform.getUpper3x3() * cylinderP2;
	
	PfxFloat tempVariable = 0.f;
	if (pfxIntersectMovingSphereCylinder(startPos, rayDir, rayRadius, cylinderP1, cylinderP2, cylinder.m_radius, tempVariable) && tempVariable < sphereOut.m_variable){
		PfxVector3 stopPoint = startPos + tempVariable * rayDir;
		PfxVector3 pointOnCylinder;
		PfxVector3 normalOnCylinder;
		PfxVector3 normal;
		pfxClosestPointAndNormalOnCylinder(stopPoint, cylinderP1, cylinderP2, cylinder.m_radius, pointOnCylinder, normalOnCylinder);
		
		// Adjust the position of contact point
		PfxVector3 vectorFromStopPointToCylinder = pointOnCylinder - stopPoint;
		PfxFloat distanceFromStopPointToCylinder = length(vectorFromStopPointToCylinder);
		if (distanceFromStopPointToCylinder > rayRadius + SCE_PFX_INTERSECT_COMMON_EPSILON){
			vectorFromStopPointToCylinder /= distanceFromStopPointToCylinder;
			pointOnCylinder = stopPoint + vectorFromStopPointToCylinder * rayRadius;
			normal = -vectorFromStopPointToCylinder;
		}
		else if (distanceFromStopPointToCylinder >= SCE_PFX_INTERSECT_COMMON_EPSILON)
			normal = -vectorFromStopPointToCylinder / distanceFromStopPointToCylinder;

		else
			normal = normalOnCylinder;

		// Set the output parameters
		sphereOut.m_contactPoint = pointOnCylinder;
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
