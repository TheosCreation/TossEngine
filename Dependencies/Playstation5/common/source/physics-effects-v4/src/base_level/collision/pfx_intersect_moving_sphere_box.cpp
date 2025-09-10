/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "pfx_intersect_common.h"
#include "pfx_intersect_moving_sphere_box.h"
#include "pfx_intersect_moving_sphere_convex.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectMovingSphereAABB(const PfxVector3 &rayStartPosition,const PfxVector3 &rayDirection,PfxFloat rayRadius,const PfxBox &box,PfxFloat &variable)
{
	PfxVector3 aabbMin(-box.m_half);
	PfxVector3 aabbMax(box.m_half);
	aabbMin -= PfxVector3(rayRadius);
	aabbMax += PfxVector3(rayRadius);

	PfxFloat t(0.0f);

	// Check if the sphere intersects the AABB at the start position.
	{
		PfxVector3 P_ = rayStartPosition;
		P_ = maxPerElem(P_,PfxVector3(-box.m_half));
		P_ = minPerElem(P_,PfxVector3(box.m_half));
		if(lengthSqr(rayStartPosition - P_) < rayRadius * rayRadius) {
			variable = 0.0f;
			return true;
		}
	}

	// Check the segment intersects the AABB expanded by the input radius.
	if(allElemLessThan(aabbMin,rayStartPosition) && allElemLessThan(rayStartPosition,aabbMax)) { // 
		// The ray starts inside the AABB expanded by the sphere radius even though the sphere does not intersect the original box at the		// 
		// start position. This can happen when the sphere starts in the Voronoi region of an edge or a vertex of the box.						// 
		// We need to treat this separately because pfxIntersectRayAABBFast would accept the intersection but produce a negative value for t.	// 
		t = 0.f;																																// 
	}																							 // 
	else if(!pfxIntersectRayAABBFast(rayStartPosition,rayDirection,0.5f * (aabbMax + aabbMin),0.5f * (aabbMax - aabbMin),t)) {
		return false;
	}
	else if(t < 0.0f || t > 1.0f) {
		return false;
	}

	PfxVector3 P = rayStartPosition + t * rayDirection;

	// Check which voronoi region the intersection point exists.
	int counter = 0;
	PfxVector3 chkMin(1.0f),chkMax(-1.0f);
	if(P[0] < -box.m_half[0]) {chkMin[0] = -1.0f;counter++;}
	if(P[1] < -box.m_half[1]) {chkMin[1] = -1.0f;counter++;}
	if(P[2] < -box.m_half[2]) {chkMin[2] = -1.0f;counter++;}
	if(P[0] > box.m_half[0])  {chkMax[0] = 1.0f;counter++;}
	if(P[1] > box.m_half[1])  {chkMax[1] = 1.0f;counter++;}
	if(P[2] > box.m_half[2])  {chkMax[2] = 1.0f;counter++;}

	if(counter == 1) {
		variable = t;
		return true;
	}

	// If the intersection point is in a corner voronoi region, check which edge is the best and proceed to following capsule test.
	if(counter == 3) {
		counter = 2;
		PfxVector3 sizeSqr = mulPerElem(rayDirection, rayDirection);
		PfxFloat maxVal = maxElem(sizeSqr);
		if (sizeSqr[0] == maxVal) {
			chkMin[0] = 1.0f;
			chkMax[0] = -1.0f;
		}
		else if(sizeSqr[1] == maxVal) {
			chkMin[1] = 1.0f;
			chkMax[1] = -1.0f;
		}
		else {
			chkMin[2] = 1.0f;
			chkMax[2] = -1.0f;
		}
	}

	// Find intersection betweem the segment and the capsule of a edge voronoi region.
	if(counter == 2) {
		PfxVector3 edgeP1 = mulPerElem(chkMin,box.m_half);
		PfxVector3 edgeP2 = mulPerElem(chkMax,box.m_half);
		if(pfxIntersectRayCapsule(rayStartPosition,rayDirection,edgeP1,edgeP2,rayRadius,t) && t >= 0.0f && t <= 1.0f ) {
			variable = t;
			return true;
		}
	}
	else {
		SCE_PFX_ASSERT(false);
	}

	return false;
}

PfxBool pfxIntersectMovingSphereBox(const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,const PfxBox &box,const PfxTransform3 &transform)
{
	// レイをBoxのローカル座標へ変換
	PfxTransform3 transformBox = orthoInverse(transform);
	PfxVector3 startPosL = transformBox.getUpper3x3() * sphereIn.m_startPosition + transformBox.getTranslation();
	PfxVector3 rayDirL = transformBox.getUpper3x3() * sphereIn.m_direction;
	PfxFloat rayRadius = sphereIn.m_radius;
	
	// 交差判定
	PfxFloat tmpVariable=0.0f;
	if(pfxIntersectMovingSphereAABB(startPosL,rayDirL,rayRadius,box.m_half,tmpVariable) && tmpVariable < sphereOut.m_variable) {
		PfxVector3 stopPoint = startPosL + tmpVariable * rayDirL;
		PfxVector3 pointOnBox;
		
		pfxClosestPointOnAABBSurface(stopPoint, box.m_half, pointOnBox); // get a valid normal when the sphere starts inside the box
        PfxVector3 normal = stopPoint - pointOnBox;
        PfxFloat distanceSqr = lengthSqr(normal);
        if(distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON)
            normal = PfxVector3::xAxis();
        else
            normal = normal / sqrtf(distanceSqr);

		// If the center of the sphere inside the box, then check the pointOnBox
		PfxVector3 pointSignsPerElem = copySignPerElem(PfxVector3(1.f), startPosL);
		PfxVector3 absPoint = mulPerElem(startPosL, pointSignsPerElem);
		if (allElemLessThanOrEqual(absPoint, box.m_half))
			if (distanceSqr > rayRadius * rayRadius)
				pointOnBox = stopPoint - normal * rayRadius;
    	
		sphereOut.m_contactPoint = transform.getTranslation() + transform.getUpper3x3() * pointOnBox;
		sphereOut.m_contactNormal = transform.getUpper3x3() * normal;
		sphereOut.m_contactFlag = true;
		sphereOut.m_variable = tmpVariable;
		sphereOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		
		return true;
	}
	
	return false;
}

} //namespace pfxv4
} //namespace sce
