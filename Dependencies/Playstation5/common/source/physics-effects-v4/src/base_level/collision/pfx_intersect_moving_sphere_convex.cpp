/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "pfx_intersect_common.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_mesh_common.h"
#include "pfx_intersect_moving_sphere_convex.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectMovingSphereTriangle(const PfxVector3 &rayStartPosition,
										 const PfxVector3 &rayDirection,
										 PfxFloat rayRadius,
										 const PfxTriangle &triangle,
										 PfxFloat &variable,
										 PfxFloat &squaredDistance) // 
{
	const PfxFloat squaredRadius = rayRadius * rayRadius; // 

	const PfxVector3& firstTrianglePoint = triangle.points[ 0 ];

	// [SMS_CHANGE] start: calculating the hit in triangle space to improve precision when the triangle is far from the origin
	const PfxTriangle recenteredTriangle( PfxVector3( 0.f, 0.f, 0.f ),
										  triangle.points[ 1 ] - firstTrianglePoint,
										  triangle.points[ 2 ] - firstTrianglePoint );
	const PfxVector3 receneterdRayStartPosition = rayStartPosition - firstTrianglePoint;
	//  [SMS_CHANGE] end

	// 1. Check if the sphere intersects the triangle at the start position.
	{
		PfxVector3 s;
		pfxClosestPointTriangle(receneterdRayStartPosition, recenteredTriangle,s);
		
		PfxFloat sqrDistance = lengthSqr(s-receneterdRayStartPosition); 	// 
		if(sqrDistance <= squaredRadius + SCE_PFX_INTERSECT_COMMON_EPSILON) {						// 
			variable = 0.0f;
			squaredDistance = sqrDistance;		//  the sphere is already in contact and might be penetrating
			return true;
		}
	}

	// 2. Check if the segment intersects the triangle shifted by sphere's radius along the triangle's normal vector.
	{
		PfxVector3 normal = recenteredTriangle.calcNormal();
		PfxVector3 frontVec = rayRadius * normal;
		PfxTriangle frontTriangle(recenteredTriangle.points[0] + frontVec, recenteredTriangle.points[1] + frontVec, recenteredTriangle.points[2] + frontVec);

		PfxFloat var;
		if(pfxIntersectRayTriangleWithoutBackFace(receneterdRayStartPosition,rayDirection,frontTriangle,var)) {
			variable = var;
			squaredDistance = squaredRadius;	//  the sphere is touching the triangle with no penetration
			return true;
		}
	}

	// 3. Check if the sphere intersects the x3 capsules placed on each edge of the triangle.
	{
		PfxFloat var;
		PfxFloat varMin = 999.0f;
		if(pfxIntersectRayCapsule(receneterdRayStartPosition,rayDirection, recenteredTriangle.points[0], recenteredTriangle.points[1],rayRadius,var) && var < varMin) {
			varMin = var;
		}
		if(pfxIntersectRayCapsule(receneterdRayStartPosition,rayDirection, recenteredTriangle.points[1], recenteredTriangle.points[2],rayRadius,var) && var < varMin) {
			varMin = var;
		}
		if(pfxIntersectRayCapsule(receneterdRayStartPosition,rayDirection, recenteredTriangle.points[2], recenteredTriangle.points[0],rayRadius,var) && var < varMin) {
			varMin = var;
		}

		if(varMin < 1.0f) {
			variable = varMin;
			squaredDistance = squaredRadius;	//  the sphere is touching the triangle with no penetration
			return true;
		}
	}

	return false;
}

PfxBool pfxIntersectMovingSphereConvex(const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,const void *shape,const PfxTransform3 &transform,PfxBool flipTriangle)
{
	const PfxConvexMeshImpl *convexA = (const PfxConvexMeshImpl*)shape;
	
	PfxVector3 startPos = sphereIn.m_startPosition;
	PfxFloat rayRadius = sphereIn.m_radius;
	
#if 1
	PfxSphere sphereB(rayRadius);

	PfxGjkSweep<PfxConvexMeshImpl, PfxSphere> gjkSweep(convexA, &sphereB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB = PfxTransform3::translation(sphereIn.m_startPosition);
	PfxVector3 translationB = sphereIn.m_direction;
	
	PfxFloat lamda = 1.0f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;
	
	PfxInt32 err = gjkSweep.sweep(lamda, nml, pA, pB, transformA, translationA, transformB, translationB);
	
	if (err == kPfxGjkResultOk && lamda < sphereOut.m_variable) {
		sphereOut.m_contactNormal = -normalize(PfxVector3(pB));
		sphereOut.m_contactPoint = PfxVector3(sphereIn.m_startPosition + lamda * sphereIn.m_direction + PfxVector3(pB));
		sphereOut.m_contactFlag = true;
		sphereOut.m_variable = lamda;
		sphereOut.m_subData.setType(PfxSubData::MESH_INFO);
		
		// find the closest facet and set barycentric coordinates.
		PfxUInt32 faceId = 0;
		PfxUInt32 ids[3] = {0,1,2};
		if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
		
		PfxFloat dmin = SCE_PFX_FLT_MAX;

		for (PfxUInt32 f = 0; f < (PfxUInt32)convexA->m_numFacets; f++) {
			PfxTriangle triangle(
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[f * 3 + ids[0]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[f * 3 + ids[1]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[f * 3 + ids[2]] * 3));

			PfxFloat d = dot(triangle.calcNormal(), pA - PfxPoint3(triangle.points[0]));

			if (fabsf(d) < fabsf(dmin)) {
				faceId = f;
				dmin = d;
				if (fabsf(dmin) < SCE_PFX_INTERSECT_COMMON_EPSILON) {
					break;
				}
			}
		}
		
		PfxFloat closestS = 0.0f,closestT = 0.0f;
		{
			PfxTriangle triangle(
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[0]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[1]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[2]] * 3));
			pfxCalcBarycentricCoords(PfxVector3(pA), triangle, closestS, closestT);
		}
		sphereOut.m_subData.setFacetLocalS(closestS);
		sphereOut.m_subData.setFacetLocalT(closestT);
		sphereOut.m_subData.setFacetId(faceId);
		if(convexA->m_userData) {
			sphereOut.m_subData.setUserData(convexA->m_userData[faceId]);
		}		
		return true;
	}
	else if (err == kPfxGjkResultIntersect) {
		PfxVector3 normal(1.0f, 0.0f, 0.0f);
		PfxVector3 point = sphereIn.m_startPosition;
		
		// find the closest triangle
		PfxUInt32 faceId = -1;
		PfxUInt32 ids[3] = {0,1,2};
		
		if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
		
		PfxVector3 closestPoint = point;
		PfxFloat closestDistanceSqr = SCE_PFX_FLT_MAX;
		PfxFloat closestS = 0.0f,closestT = 0.0f;
		
		// 1. get the closest point on a triangle
		for(PfxUInt32 f=0;f<(PfxUInt32)convexA->m_numFacets;f++) {
			PfxTriangle triangle(
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[0]] * 3),
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[1]] * 3),
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[2]] * 3));
			
			PfxVector3 normal = triangle.calcNormal();
			PfxFloat centerPositionAlongNormal = dot(normal,startPos-triangle.points[ 0 ]);
			if(centerPositionAlongNormal < 0.0f) {
				continue;
			}
			
			PfxVector3 s;
			pfxClosestPointTriangle(point, triangle, s);
			
			PfxFloat distanceSqr = lengthSqr(s - point);
			if(distanceSqr > closestDistanceSqr) {
				continue;
			}
			
			faceId = f;
			closestPoint = s;
			closestDistanceSqr = distanceSqr;
			pfxCalcBarycentricCoords(closestPoint, triangle, closestS, closestT);
		}

		if (faceId == (PfxUInt32)-1) return false;

		// 2. check if the closest point is inside of a triangle and calculate a normal vector
		if (closestDistanceSqr > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			normal = -normalize(closestPoint - point);
		}
		
		sphereOut.m_contactNormal = normal;
		sphereOut.m_contactPoint = closestPoint;
		sphereOut.m_contactFlag = true;
		sphereOut.m_variable = 0.0f;
		sphereOut.m_subData.setType(PfxSubData::MESH_INFO);
		sphereOut.m_subData.setFacetLocalS(closestS);
		sphereOut.m_subData.setFacetLocalT(closestT);
		sphereOut.m_subData.setFacetId(faceId);
		if(convexA->m_userData) {
			sphereOut.m_subData.setUserData(convexA->m_userData[faceId]);
		}
		return true;
	}
	
	return false;
#else
	// レイとConvexの交差判定
	PfxUInt32 faceId = 0;
	PfxUInt32 ids[3] = {0,1,2};

	if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
	
	bool ret = false;
	PfxFloat tmpVariable(0.0f);
	PfxFloat tmpAngle(1.0f);
	PfxFloat minSquaredDistance(rayRadius*rayRadius);	// 
	for(PfxUInt32 f=0;f<(PfxUInt32)convexA->m_numIndices/3;f++) {
		PfxTriangle triangle(
			transform * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[0]] * 3),
			transform * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[1]] * 3),
			transform * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[2]] * 3));
		
		PfxVector3 normal = triangle.calcNormal();
		PfxFloat centerPositionAlongNormal = dot(normal,startPos-triangle.points[ 0 ]); // only checking the ray direction against the
		if(centerPositionAlongNormal < 0.0f) {                                          // normal will make test fail if the sphere collides with the triangle
			continue;                                                                   // at the start position and the center of the sphere is on the side of the normal
		}
		
		PfxVector3 aabbMin(triangle.points[0]),aabbMax(triangle.points[0]);
		aabbMin = minPerElem(minPerElem(aabbMin,triangle.points[1]),triangle.points[2]) - PfxVector3(rayRadius);
		aabbMax = maxPerElem(maxPerElem(aabbMax,triangle.points[1]),triangle.points[2]) + PfxVector3(rayRadius);
		
		if(!pfxIntersectRayAABBFast(startPos,rayDir,0.5f * (aabbMax + aabbMin),0.5f * (aabbMax - aabbMin),tmpVariable)) {
			continue;
		}
		else if(tmpVariable >= sphereOut.m_variable) {
			continue;
		}
		
		PfxFloat angle = dot(normal,rayDir); // moved here since the code above does not need to compute this anymore
			
		//  The distance between sphere and triangle will be equal to radius if the sphere touches the triangle and between [0,radius] if the sphere penetrates the triangle (which
		// is only possible if the sphere intersects at t=0). If we intersect at t=0 we want to report the contact with deepest penetration.
		PfxFloat tmpSquaredDistance(rayRadius*rayRadius);																// 
		if(pfxIntersectMovingSphereTriangle(startPos,rayDir,rayRadius,triangle,tmpVariable,tmpSquaredDistance)) {		// 
			//  if we already found a contact with more penetration then discard the new one
			if( tmpSquaredDistance > minSquaredDistance )	// 
				continue;												//  note: this implies that closestFacetInfo.tmin == 0

			if( tmpSquaredDistance < minSquaredDistance || //  if there's more penetration we just accept the contact (note that this implies that cur_t is zero)
				tmpVariable < sphereOut.m_variable - SCE_PFX_INTERSECT_COMMON_EPSILON || //  if it happens earlier just accept the contact 
				( tmpVariable < sphereOut.m_variable + SCE_PFX_INTERSECT_COMMON_EPSILON &&  //  if it happens approximately at the same time only accept if it has a more incident angle
				angle <= tmpAngle)) {
				tmpAngle = angle;
				minSquaredDistance = tmpSquaredDistance;	// 
				sphereOut.m_variable = tmpVariable;
				faceId = f;
				ret = true;
			}
		}
	}
	
	if(ret) {
		sphereOut.m_contactFlag = true;
		
		PfxTriangle triangle(
			transform * pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[0]] * 3),
			transform * pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[1]] * 3),
			transform * pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[2]] * 3));
		
		PfxVector3 stopPoint = startPos+sphereOut.m_variable*rayDir;
		PfxVector3 contactPoint;
		
		pfxClosestPointTriangle(stopPoint,triangle,contactPoint);

		sphereOut.m_contactPoint = contactPoint;

		PfxVector3 normal = stopPoint - contactPoint;
		PfxFloat distanceSqr = lengthSqr(normal);
		if (distanceSqr < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON)
			normal = triangle.calcNormal();
		else
			normal = normal / sqrtf(distanceSqr);
		
		sphereOut.m_contactNormal = normal;
		
		PfxFloat s=0.0f,t=0.0f;
		pfxCalcBarycentricCoords(contactPoint,triangle,s,t);
		sphereOut.m_subData.setType(PfxSubData::MESH_INFO);
		sphereOut.m_subData.setFacetLocalS(s);
		sphereOut.m_subData.setFacetLocalT(t);
		sphereOut.m_subData.setFacetId(faceId);
		if(convexA->m_userData) {
			sphereOut.m_subData.setUserData(convexA->m_userData[faceId]);
		}
	}

	return ret;
#endif
}

} //namespace pfxv4
} //namespace sce
