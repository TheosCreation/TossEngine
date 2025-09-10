/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_shape.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_moving_capsule_func.h"
#include "../../../include/physics_effects/base_level/collision/pfx_capsule.h"
#include "../../../include/physics_effects/base_level/collision/pfx_sphere.h"
#include "pfx_intersect_common.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"
#include "pfx_intersect_moving_capsule_large_tri_mesh.h"
#include "pfx_mesh_common.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Moving Capsule Intersection Function Table

PfxBool intersectMovingCapsuleFuncDummy(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback,
				void *userDataForDiscardingTriangle)
{
	(void)capsuleIn,(void)capsuleOut,(void)shape,(void)transform;
	return false;
}

PfxBool intersectMovingCapsuleFuncSphere(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback,
				void *userDataForDiscardingTriangle)
{
	PfxFloat lamda = 1.0f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxSphere sphereA = shape.getSphere();
	PfxCapsule capsuleB(capsuleIn.m_halfLength, capsuleIn.m_radius);

	PfxGjkSweep<PfxSphere, PfxCapsule> gjkSweep(&sphereA, &capsuleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(capsuleIn.m_orientation, capsuleIn.m_startPosition);
	PfxVector3 translationB = capsuleIn.m_direction;

	PfxInt32 err = gjkSweep.sweep(lamda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (err == kPfxGjkResultOk && lamda < capsuleOut.m_variable) {
		capsuleOut.m_contactNormal = -calcNormalOnCapsuleLocal(PfxVector3(pB), capsuleIn);
		capsuleOut.m_contactPoint = PfxVector3(capsuleIn.m_startPosition + lamda * capsuleIn.m_direction + rotate(capsuleIn.m_orientation, PfxVector3(pB)));
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = lamda;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (err == kPfxGjkResultIntersect){
		PfxVector3 normal = rotate(capsuleIn.m_orientation, PfxVector3::zAxis());
		PfxVector3 point = capsuleIn.m_startPosition;
		
		// 1. get the closest point on a sphere 
		PfxVector3 dir = point - transform.getTranslation();
		PfxVector3 closestPoint = point;
		if (lengthSqr(dir) > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			closestPoint = transform.getTranslation() + shape.getScale() * sphereA.m_radius * normalize(dir);
		}
		else
			closestPoint = transform.getTranslation() + shape.getScale() * sphereA.m_radius * PfxVector3::zAxis();
		
		// 2. check if the closest point is inside of a capsule and calculate a normal vector
		PfxFloat distanceSqr = lengthSqr(closestPoint - point);
		if (distanceSqr > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			calcPointAndNormalOnCapsule(closestPoint, normal, capsuleIn);
			normal = -normal;
		}
		
		capsuleOut.m_contactNormal = normal;
		capsuleOut.m_contactPoint = closestPoint;
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = 0.0f;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}

	return false;
}

PfxBool intersectMovingCapsuleFuncBox(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback,
				void *userDataForDiscardingTriangle)
{
	PfxFloat lamda = 1.0f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxBox boxA = shape.getBox();
	PfxCapsule capsuleB(capsuleIn.m_halfLength, capsuleIn.m_radius);

	PfxGjkSweep<PfxBox, PfxCapsule> gjkSweep(&boxA, &capsuleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(capsuleIn.m_orientation, capsuleIn.m_startPosition);
	PfxVector3 translationB = capsuleIn.m_direction;

	PfxInt32 err = gjkSweep.sweep(lamda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (err == kPfxGjkResultOk && lamda < capsuleOut.m_variable) {
		capsuleOut.m_contactNormal = -calcNormalOnCapsuleLocal(PfxVector3(pB), capsuleIn);
		capsuleOut.m_contactPoint = PfxVector3(capsuleIn.m_startPosition + lamda * capsuleIn.m_direction + rotate(capsuleIn.m_orientation, PfxVector3(pB)));
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = lamda;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (err == kPfxGjkResultIntersect) {
		PfxVector3 point = capsuleIn.m_startPosition;
		
		// 1. get the closest point on a box
		PfxTransform3 transformAInv = inverse(transformA);
		PfxVector3 pointBoxLocal = transformAInv.getTranslation() + transformAInv.getUpper3x3() * point;
		PfxVector3 closestPoint;
		pfxClosestPointOnAABBSurface(pointBoxLocal, boxA.m_half, closestPoint);
		closestPoint = transformA.getTranslation() + transformA.getUpper3x3() * closestPoint;
		
		// 2. check if the closest point is inside of a box and calculate a normal vector
		PfxVector3 capB1 = transformB.getTranslation() + transformB.getUpper3x3() * PfxVector3(-capsuleB.m_halfLen, 0.f, 0.f);
		PfxVector3 capB2 = transformB.getTranslation() + transformB.getUpper3x3() * PfxVector3(capsuleB.m_halfLen, 0.f, 0.f);
		PfxFloat capRadB = capsuleB.m_radius;
		PfxVector3 closestPointOnRayAxis;
		pfxClosestPointLine(closestPoint, capB1, capB2 - capB1, closestPointOnRayAxis);

		// Normalize the normal vector
		PfxVector3 normal = closestPointOnRayAxis - closestPoint;
		PfxFloat distanceSqr = lengthSqr(normal);
		if (distanceSqr > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON)
			normal /= sqrtf(distanceSqr);
		else
			normal = PfxVector3(0.f, 0.f, 1.f);

		if (distanceSqr > capRadB * capRadB) {
			closestPoint = closestPointOnRayAxis - capRadB * normal;
		}
		
		capsuleOut.m_contactNormal = normal;
		capsuleOut.m_contactPoint = closestPoint;
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = 0.0f;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	
	return false;
}

PfxBool intersectMovingCapsuleFuncCapsule(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback,
				void *userDataForDiscardingTriangle)
{
	PfxFloat lamda = 1.0f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxCapsule capsuleA = shape.getCapsule();
	PfxCapsule capsuleB(capsuleIn.m_halfLength, capsuleIn.m_radius);

	PfxGjkSweep<PfxCapsule, PfxCapsule> gjkSweep(&capsuleA, &capsuleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(capsuleIn.m_orientation, capsuleIn.m_startPosition);
	PfxVector3 translationB = capsuleIn.m_direction;

	PfxInt32 err = gjkSweep.sweep(lamda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (err == kPfxGjkResultOk && lamda < capsuleOut.m_variable) {
		capsuleOut.m_contactNormal = -calcNormalOnCapsuleLocal(PfxVector3(pB), capsuleIn);
		capsuleOut.m_contactPoint = PfxVector3(capsuleIn.m_startPosition + lamda * capsuleIn.m_direction + rotate(capsuleIn.m_orientation, PfxVector3(pB)));
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = lamda;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (err == kPfxGjkResultIntersect) {
		PfxVector3 point = capsuleIn.m_startPosition;

		// 1. Detect the closest point on capsule A
		PfxVector3 closestPoint;
		PfxVector3 capA1 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3(-capsuleA.m_halfLen,0.0f,0.0f);
		PfxVector3 capA2 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3( capsuleA.m_halfLen,0.0f,0.0f);
		PfxFloat capRadA = shape.getScale() * capsuleA.m_radius;
		pfxClosestPointLine(point, capA1, capA2 - capA1, closestPoint);
		if(lengthSqr(point - closestPoint) > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			closestPoint += capRadA * normalize(point - closestPoint);
		}
		else { 
			closestPoint += capRadA * normalize(transformA.getUpper3x3() * PfxVector3(0.f, 0.f, 1.f));
		}
		
		// 2. Check if the closest point is inside of the capsule B and calculate the normal vector
		PfxVector3 capB1 = transformB.getTranslation() + transformB.getUpper3x3() * PfxVector3(-capsuleB.m_halfLen, 0.f, 0.f);
		PfxVector3 capB2 = transformB.getTranslation() + transformB.getUpper3x3() * PfxVector3(capsuleB.m_halfLen, 0.f, 0.f);
		PfxFloat capRadB = capsuleB.m_radius;
		PfxVector3 closestPointOnRayAxis;
		pfxClosestPointLine(closestPoint, capB1, capB2 - capB1, closestPointOnRayAxis);
		
		// Normalize the normal vector
		PfxVector3 normal = closestPointOnRayAxis - closestPoint;
		PfxFloat distanceSqr = lengthSqr(normal);
		if (distanceSqr > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON)
			normal /= sqrtf(distanceSqr);

		else
			normal = PfxVector3(0.f, 0.f, 1.f);

		if (distanceSqr > capRadB * capRadB) {
			closestPoint = closestPointOnRayAxis - capRadB * normal;
		}
		
		capsuleOut.m_contactNormal = normal;
		capsuleOut.m_contactPoint = closestPoint;
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = 0.0f;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	
	return false;
}

PfxBool intersectMovingCapsuleFuncCylinder(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback,
				void *userDataForDiscardingTriangle)
{
	PfxFloat lamda = 1.0f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxCylinder cylinderA = shape.getCylinder();
	PfxCapsule capsuleB(capsuleIn.m_halfLength, capsuleIn.m_radius);

	PfxGjkSweep<PfxCylinder, PfxCapsule> gjkSweep(&cylinderA, &capsuleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(capsuleIn.m_orientation, capsuleIn.m_startPosition);
	PfxVector3 translationB = capsuleIn.m_direction;

	PfxInt32 err = gjkSweep.sweep(lamda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (err == kPfxGjkResultOk && lamda < capsuleOut.m_variable) {
		capsuleOut.m_contactNormal = -calcNormalOnCapsuleLocal(PfxVector3(pB), capsuleIn);
		capsuleOut.m_contactPoint = PfxVector3(capsuleIn.m_startPosition + lamda * capsuleIn.m_direction + rotate(capsuleIn.m_orientation, PfxVector3(pB)));
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = lamda;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (err == kPfxGjkResultIntersect) {
		PfxVector3 point = capsuleIn.m_startPosition;

		// 1. get the closest point on a cylinder
		PfxVector3 closestPoint = point;
		PfxVector3 cylA1 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3(-cylinderA.m_halfLen,0.0f,0.0f);
		PfxVector3 cylA2 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3( cylinderA.m_halfLen,0.0f,0.0f);
		PfxFloat cylRadA = shape.getScale() * cylinderA.m_radius;
		pfxClosestPointCylinder(point, cylA1, cylA2, cylRadA, closestPoint);
		
		// 2. check if the closest point is inside of a cylinder and calculate a normal vector
		PfxVector3 capB1 = transformB.getTranslation() + transformB.getUpper3x3() * PfxVector3(-capsuleB.m_halfLen, 0.f, 0.f);
		PfxVector3 capB2 = transformB.getTranslation() + transformB.getUpper3x3() * PfxVector3( capsuleB.m_halfLen, 0.f, 0.f);
		PfxFloat capRadB = capsuleB.m_radius;
		PfxVector3 closestPointOnRayAxis;
		pfxClosestPointLine(closestPoint, capB1, capB2 - capB1, closestPointOnRayAxis);

		// Normalize the normal vector
		PfxVector3 normal = closestPointOnRayAxis - closestPoint;
		PfxFloat distanceSqr = lengthSqr(normal);
		if (distanceSqr > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON)
			normal /= sqrtf(distanceSqr);

		else
			normal = PfxVector3(0.f, 0.f, 1.f);

		if (distanceSqr > capRadB * capRadB) {
			closestPoint = closestPointOnRayAxis - capRadB * normal;
		}
		
		capsuleOut.m_contactNormal = normal;
		capsuleOut.m_contactPoint = closestPoint;
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = 0.0f;
		capsuleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	
	return false;
}

PfxBool intersectMovingCapsuleFuncConvex(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback,
				void *userDataForDiscardingTriangle)
{
	PfxFloat lamda = 1.0f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shape.getConvexMesh();
	PfxCapsule capsuleB(capsuleIn.m_halfLength, capsuleIn.m_radius);

	PfxGjkSweep<PfxConvexMeshImpl, PfxCapsule> gjkSweep(convexA, &capsuleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(capsuleIn.m_orientation, capsuleIn.m_startPosition);
	PfxVector3 translationB = capsuleIn.m_direction;

	PfxInt32 err = gjkSweep.sweep(lamda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (err == kPfxGjkResultOk && lamda < capsuleOut.m_variable) {
		capsuleOut.m_contactNormal = -calcNormalOnCapsuleLocal(PfxVector3(pB), capsuleIn);
		capsuleOut.m_contactPoint = PfxVector3(capsuleIn.m_startPosition + lamda * capsuleIn.m_direction + rotate(capsuleIn.m_orientation, PfxVector3(pB)));
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = lamda;
		capsuleOut.m_subData.setType(PfxSubData::MESH_INFO);

		// find the closest facet and set barycentric coordinates.
		PfxUInt32 faceId = 0;
		PfxUInt32 ids[3] = {0,1,2};
		PfxVector3 scale = shape.getScaleXyz();
		PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
		if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
		
		PfxFloat dmin = SCE_PFX_FLT_MAX;
		for(PfxUInt32 f=0;f<(PfxUInt32)convexA->m_numFacets;f++) {
			PfxTriangle triangle(
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[0]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[1]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[2]] * 3));
			
			PfxFloat d = dot(triangle.calcNormal(), pA - PfxPoint3(triangle.points[0]));
			
			if(fabsf(d) < fabsf(dmin)) {
				faceId = f;
				dmin = d;
				if(fabsf(dmin) < SCE_PFX_INTERSECT_COMMON_EPSILON) {
					break;
				}
			}
		}
		
		PfxFloat closestS = 0.0f, closestT = 0.0f;
		{
			PfxTriangle triangle(
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[0]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[1]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId*3+ids[2]] * 3));
			pfxCalcBarycentricCoords(PfxVector3(pA), triangle, closestS, closestT);
		}
		capsuleOut.m_subData.setFacetLocalS(closestS);
		capsuleOut.m_subData.setFacetLocalT(closestT);
		capsuleOut.m_subData.setFacetId(faceId);
		
		return true;
	}
	else if (err == kPfxGjkResultIntersect) {
		PfxVector3 normal = rotate(capsuleIn.m_orientation, PfxVector3::zAxis());
		PfxVector3 point = capsuleIn.m_startPosition;
		
		// find the closest triangle
		PfxUInt32 faceId = (PfxUInt32)-1;
		PfxUInt32 ids[3] = {0,1,2};

		PfxVector3 scale = shape.getScaleXyz();
		PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
		if(flipTriangle) {ids[0] = 2;ids[2] = 0;}
		
		PfxVector3 closestPoint = point;
		PfxFloat closestDistanceSqr = SCE_PFX_FLT_MAX;
		PfxFloat closestS = 0.0f, closestT = 0.0f;
		
		// 1. get the closest point on a triangle
		for(PfxUInt32 f=0;f<(PfxUInt32)convexA->m_numFacets;f++) {
			PfxTriangle triangle(
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[0]] * 3),
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[1]] * 3),
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f*3+ids[2]] * 3));
			
			PfxVector3 normal = triangle.calcNormal();
			PfxFloat centerPositionAlongNormal = dot(normal, point - triangle.points[0]);
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

		// 2. check if the closest point is inside of a capsule and calculate a normal vector
		if (closestDistanceSqr > SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			calcPointAndNormalOnCapsule(closestPoint, normal, capsuleIn);
			normal = -normal;
		}
		
		capsuleOut.m_contactNormal = normal;
		capsuleOut.m_contactPoint = closestPoint;
		capsuleOut.m_contactFlag = true;
		capsuleOut.m_variable = 0.0f;
		capsuleOut.m_subData.setType(PfxSubData::MESH_INFO);
		capsuleOut.m_subData.setFacetLocalS(closestS);
		capsuleOut.m_subData.setFacetLocalT(closestT);
		capsuleOut.m_subData.setFacetId(faceId);
		if(convexA->m_userData) {
			capsuleOut.m_subData.setUserData(convexA->m_userData[faceId]);
		}
		return true;
	}
	
	return false;
}

PfxBool intersectMovingCapsuleFuncLargeTriMesh(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback,
				void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape.getLargeTriMesh();
	
	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	
	PfxFloat radiusLocal = calcRadiusOfSweptCapsule(capsuleIn) / minElem(absPerElem(scale));
	
	PfxBool ret = pfxIntersectMovingCapsuleLargeTriMesh(capsuleIn,capsuleOut,(const void*)largeMesh,transform,flipTriangle,radiusLocal, discardTriangleCallback, userDataForDiscardingTriangle);
	
	return ret;
}

PfxIntersectMovingCapsuleFunc funcTbl_intersectMovingCapsule[kPfxShapeCount] = {
	intersectMovingCapsuleFuncSphere,
	intersectMovingCapsuleFuncBox,
	intersectMovingCapsuleFuncCapsule,
	intersectMovingCapsuleFuncCylinder,
	intersectMovingCapsuleFuncConvex,
	intersectMovingCapsuleFuncLargeTriMesh,
	intersectMovingCapsuleFuncDummy, // Reserved
	intersectMovingCapsuleFuncDummy, // Reserved
	intersectMovingCapsuleFuncDummy, // Reserved
	intersectMovingCapsuleFuncDummy, // Reserved
	intersectMovingCapsuleFuncDummy, // Reserved
};

///////////////////////////////////////////////////////////////////////////////
// Moving Capsule Intersection Function Table Interface

PfxIntersectMovingCapsuleFunc pfxGetIntersectMovingCapsuleFunc(PfxUInt8 shapeType)
{
	return funcTbl_intersectMovingCapsule[shapeType];
}

PfxInt32 pfxSetIntersectCapsuleFunc(PfxUInt8 shapeType,PfxIntersectMovingCapsuleFunc func)
{
	if(shapeType >= kPfxShapeCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	funcTbl_intersectMovingCapsule[shapeType] = func;
	
	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
