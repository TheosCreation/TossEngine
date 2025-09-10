/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectRaySphere(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &spherePosition,
	PfxFloat sphereRadius,
	PfxFloat &variable)
{
	PfxVector3 v = rayStartPosition - spherePosition;

	PfxFloat a = dot(rayDirection,rayDirection);
	PfxFloat b = dot(v,rayDirection);
	PfxFloat c = dot(v,v) - sphereRadius * sphereRadius;

	if(c < 0.0f) {
		variable = 0.0f;
		return true;
	}

	PfxFloat d = b * b - a * c;
	
	if(d < 0.0f || a < 0.00001f) return false;
	
	PfxFloat tt = ( -b - sqrtf(d) ) / a;
	
	if(tt < 0.0f || tt > 1.0f) return false;
	
	variable = tt;
	
	return true;
}

PfxBool pfxIntersectRayCapsule(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &capsuleP1,
	const PfxVector3 &capsuleP2,
	PfxFloat capsuleRadius,
	PfxFloat &variable)
{
	const PfxFloat radSqr = capsuleRadius * capsuleRadius;
	const PfxVector3 capDir = capsuleP2 - capsuleP1;
	const PfxFloat capLenSqr = dot(capDir,capDir);
	const PfxFloat rayLenSqr = dot(rayDirection,rayDirection);
	
	if(rayLenSqr < SCE_PFX_INTERSECT_COMMON_EPSILON) return false;
	
	if (capLenSqr < SCE_PFX_INTERSECT_COMMON_EPSILON) {
		return pfxIntersectRaySphere(rayStartPosition, rayDirection, capsuleP1, capsuleRadius, variable);
	}

	PfxVector3 unitCapDir = capDir / sqrtf(capLenSqr);
	
	// Check if the start position is in the capsule.
	{
		PfxVector3 s;
		PfxFloat tt = dot(rayStartPosition - capsuleP1,capDir) / capLenSqr;
		tt = SCE_PFX_CLAMP(tt,0.0f,1.0f);
		s = capsuleP1 + tt * capDir;
		if(lengthSqr(s-rayStartPosition) < radSqr) {
			variable = 0.0f;
			return true;
		}
	}
	
	// Calc intersection between the segment and the body of the capsule.
	do {
		PfxVector3 m = rayStartPosition - capsuleP1;
		PfxFloat md = dot(m,unitCapDir);
		PfxFloat nd = dot(rayDirection,unitCapDir);
		PfxFloat nn = rayLenSqr;
		PfxFloat mm = dot(m,m);
		PfxFloat nm = dot(rayDirection,m);

		PfxFloat a = nn - nd * nd;
		PfxFloat b = nm - md * nd;
		PfxFloat c = mm - md * md - radSqr;

		PfxFloat d = b * b - a * c;
		
		if(d < 0.0f) return false; // レイは逸れている
		if(fabsf(a) < SCE_PFX_INTERSECT_COMMON_EPSILON) break; // レイが軸に平行
		
		PfxFloat tt = ( -b - sqrtf(d) ) / a;
		
		if(tt < 0.0f || tt > 1.0f) break;

		PfxFloat chk = md + tt * nd;
		if(chk < 0.0f || chk * chk > capLenSqr) break;

		variable = tt;
		
		return true;
	} while(0);
	
	// Calc intersection between the segment and the x2 spheres.
	const PfxVector3 capP[2] = {capsuleP1,capsuleP2};
	PfxFloat tmin = 999.0f;
	
	for(int i=0;i<2;i++) {
		PfxVector3 v = rayStartPosition - capP[i];
		
		PfxFloat a = rayLenSqr;
		PfxFloat b = dot(v,rayDirection);
		PfxFloat c = dot(v,v) - radSqr;
		
		PfxFloat d = b * b - a * c;
		
		if(d < 0.0f) continue;
		
		PfxFloat tt = (-b - sqrtf(d)) / a;
		
		if(tt < 0.0f || tt > 1.0f) continue;
		
		if(tt < tmin) {
			tmin = tt;
		}
	}

	if(tmin < 1.0f) {
		variable = tmin;
		return true;
	}

	return false;
}

void pfxClosestPointCylinder(
	const PfxVector3 &position,
	const PfxVector3 &cylinderP1,
	const PfxVector3 &cylinderP2,
	PfxFloat cylinderRadius,
	PfxVector3 &s)
{
	PfxVector3 n;
	pfxClosestPointAndNormalOnCylinder(position, cylinderP1, cylinderP2, cylinderRadius, s, n);
}

void pfxClosestPointAndNormalOnCylinder(
	const PfxVector3 &position,
	const PfxVector3 &cylinderP1,
	const PfxVector3 &cylinderP2,
	PfxFloat cylinderRadius,
	PfxVector3 &s,
	PfxVector3 &n )
{
	PfxVector3 cylinderAxis = cylinderP2 - cylinderP1;
	PfxVector3 cylinderCenter = cylinderP1 + cylinderAxis * 0.5f;
	PfxFloat cylinderFullLen = length( cylinderAxis );
	if( cylinderFullLen < SCE_PFX_INTERSECT_COMMON_EPSILON ){
		// It's an invalid input. So it returns the center of the cylinder.
		s = cylinderCenter;
		return;
	}

	// Get the closest points
	cylinderAxis /= cylinderFullLen;
	PfxVector3 closestPointInsideCylinderAxis, closestPointOnCylinderAxis;
	PfxFloat t = dot( position - cylinderP1, cylinderAxis );
	closestPointOnCylinderAxis = cylinderP1 + cylinderAxis * t;

	PfxFloat tFrom0To1 = SCE_PFX_CLAMP( t, 0.0f, cylinderFullLen );
	closestPointInsideCylinderAxis = cylinderP1 + cylinderAxis * tFrom0To1;

	// The vectors from closest points to the given position
	PfxVector3 vectorFromAxis = position - closestPointOnCylinderAxis;
	PfxFloat distFromAxis = length( vectorFromAxis );

	// The distance to plane and circle edge
	PfxFloat d = distFromAxis - cylinderRadius;
	PfxFloat t2 = t - cylinderFullLen;
	PfxFloat distToCircle1 = sqrtf( t * t + d * d );
	PfxFloat distToCircle2 = sqrtf( t2 * t2 + d * d );

	PfxFloat distToSideCurve;
	if( t > 0.0f && t < cylinderFullLen ) {
		distToSideCurve = fabsf( d );
	}
	else {
		distToSideCurve = SCE_PFX_MIN( distToCircle1, distToCircle2 );
	}

	PfxFloat distToPlane1, distToPlane2, distToPlane;
	if (distFromAxis > cylinderRadius) {
		distToPlane1 = distToCircle1;
		distToPlane2 = distToCircle2;
	}
	else {
		distToPlane1 = fabsf( t );
		distToPlane2 = fabsf( t2 );
	}
	distToPlane = SCE_PFX_MIN( distToPlane1, distToPlane2 );

	// Set the closest point according to the shortest distance
	if ( distToSideCurve < distToPlane + SCE_PFX_INTERSECT_COMMON_EPSILON ) {
		if (distFromAxis < SCE_PFX_INTERSECT_COMMON_EPSILON) {
			if (t < 0.f){
				s = closestPointInsideCylinderAxis;
				n = -cylinderAxis;
			}
			else if (t > cylinderFullLen){
				s = closestPointInsideCylinderAxis;
				n = cylinderAxis;
			}
			else {
				n = PfxVector3::zAxis();
				s = closestPointInsideCylinderAxis + n * cylinderRadius;
			}
		}
		else {
			n = vectorFromAxis / distFromAxis;
			s = closestPointInsideCylinderAxis + n * cylinderRadius;
		}
	}
	else if (distToPlane1 < distToPlane2) {
		s = (distFromAxis < SCE_PFX_INTERSECT_COMMON_EPSILON ? cylinderP1 : cylinderP1 + vectorFromAxis * (SCE_PFX_MIN(cylinderRadius, distFromAxis) / distFromAxis));
		n = -cylinderAxis;
	}
	else {
		s = (distFromAxis < SCE_PFX_INTERSECT_COMMON_EPSILON ? cylinderP2 : cylinderP2 + vectorFromAxis * (SCE_PFX_MIN(cylinderRadius, distFromAxis) / distFromAxis));
		n = cylinderAxis;
	}
}

PfxBool pfxIntersectRayCylinder(
	const PfxVector3 &rayStartPosition,
	const PfxVector3 &rayDirection,
	const PfxVector3 &cylinderP1,
	const PfxVector3 &cylinderP2,
	PfxFloat cylinderRadius,
	PfxFloat &variable)
{
	const PfxFloat radSqr = cylinderRadius * cylinderRadius;
	const PfxVector3 cylDir = cylinderP2 - cylinderP1;
	const PfxFloat cylLenSqr = dot(cylDir,cylDir);
	const PfxFloat rayLenSqr = dot(rayDirection,rayDirection);
	
	if(rayLenSqr < SCE_PFX_INTERSECT_COMMON_EPSILON) return false;
	
	if(cylLenSqr < SCE_PFX_INTERSECT_COMMON_EPSILON) return false;

	PfxVector3 unitCylDir = cylDir / sqrtf(cylLenSqr);
	
	// Check if the start position is in the cylinder. 
	{
		PfxVector3 s;
		PfxFloat tt = dot(rayStartPosition - cylinderP1,cylDir) / cylLenSqr;
		s = cylinderP1 + tt * cylDir;
		if(lengthSqr(s-rayStartPosition) < radSqr && tt >= 0.0f && tt <= 1.0f) {
			variable = 0.0f;
			return true;
		}
	}
	
	// Calc intersection between the segment and the body of the cylinder.
	do {
		PfxVector3 m = rayStartPosition - cylinderP1;
		PfxFloat md = dot(m,unitCylDir);
		PfxFloat nd = dot(rayDirection,unitCylDir);
		PfxFloat nn = rayLenSqr;
		PfxFloat mm = dot(m,m);
		PfxFloat nm = dot(rayDirection,m);

		PfxFloat a = nn - nd * nd;
		PfxFloat b = nm - md * nd;
		PfxFloat c = mm - md * md - radSqr;

		PfxFloat d = b * b - a * c;
		
		if(d < 0.0f) return false; // レイは逸れている
		if(fabsf(a) < SCE_PFX_INTERSECT_COMMON_EPSILON) break;
		
		PfxFloat tt = ( -b - sqrtf(d) ) / a;
		
		if(tt < 0.0f || tt > 1.0f) break;

		PfxFloat chk = md + tt * nd;
		if(chk < 0.0f || chk * chk > cylLenSqr) break;

		variable = tt;
		
		return true;
	} while(0);
	
	// Calc intersection between the segment and the x2 circles.
	do {
		PfxVector3 m = rayStartPosition - cylinderP1;
		
		PfxFloat md = dot(m,cylDir);
		PfxFloat nd = dot(rayDirection,cylDir);
		
		if(fabsf(nd) < SCE_PFX_INTERSECT_COMMON_EPSILON) break;
		
		PfxFloat t1 = -md / nd;
		PfxFloat t2 = (cylLenSqr - md ) / nd;
		
		PfxVector3 l1 = m + t1 * rayDirection;
		PfxVector3 l2 = (rayStartPosition - cylinderP2) + t2 * rayDirection;
		
		PfxFloat tmin = 1.0f;
		
		if(dot(l1,l1) <= radSqr && t1 >= 0.0f && t1 <= 1.0f) {
			tmin = t1;
		}
		if(dot(l2,l2) <= radSqr && t2 >= 0.0f && t2 <= 1.0f && t2 < tmin) {
			tmin = t2;
		}
		
		if(tmin < 1.0f) {
			variable = tmin;
			return true;
		}
		
	} while(0);

	return false;
}


} //namespace pfxv4
} //namespace sce
