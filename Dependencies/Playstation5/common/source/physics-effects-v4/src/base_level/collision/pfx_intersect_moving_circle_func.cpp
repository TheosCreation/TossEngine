/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/base_level/collision/pfx_shape.h"
#include "../../../include/physics_effects/base_level/collision/pfx_circle.h"
#include "../../../include/physics_effects/base_level/collision/pfx_intersect_moving_circle_func.h"
#include "pfx_intersect_moving_circle_large_tri_mesh.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_intersect_common.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"
#include "pfx_mesh_common.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Moving Circle Intersection Function Table

PfxBool intersectMovingCircleFuncDummy(
				const PfxCircleInputInternal &circleIn,PfxCircleOutputInternal &circleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	(void)circleIn,(void)circleOut,(void)shape,(void)transform;
	return false;
}

static PfxBool intersectStaticCircleSphere(const PfxCircleInputInternal &circleIn, const PfxCircle &circle, PfxCircleOutputInternal &circleOut, const PfxSphere &sphere, const PfxTransform3 &transform)
{
	PfxVector3 spherePos = transform.getTranslation();

	PfxVector3 vecCircleToSphere = spherePos - circleIn.m_startPosition;
	PfxFloat variableToSphereProjection = dot(vecCircleToSphere, circle.m_normal);
	PfxVector3 sphereProjectionOnAxis = circleIn.m_startPosition + circle.m_normal * (variableToSphereProjection);
	PfxVector3 vecSphereToAxis = sphereProjectionOnAxis - spherePos;
	PfxFloat distSphereToAxis = length(vecSphereToAxis);

	PfxFloat distCircleToContactPoint = distSphereToAxis - sqrtf(sphere.m_radius * sphere.m_radius - variableToSphereProjection * variableToSphereProjection);
	if (distCircleToContactPoint <= circleIn.m_radius) {

		// Adjust the vector close to 0
		if (distSphereToAxis < SCE_PFX_INTERSECT_COMMON_EPSILON) {
			vecSphereToAxis = cross(PfxVector3::xAxis(), circle.m_normal);
			distSphereToAxis = length(vecSphereToAxis);
			if (distSphereToAxis < SCE_PFX_INTERSECT_COMMON_EPSILON) {
				vecSphereToAxis = cross(PfxVector3(0.0f, 1.0f, 1.0f), circle.m_normal);
				distSphereToAxis = length(vecSphereToAxis);
			}
		}

		circleOut.m_contactPoint = circleIn.m_startPosition - vecSphereToAxis * (SCE_PFX_CLAMP(distCircleToContactPoint, -circleIn.m_radius, circleIn.m_radius) / distSphereToAxis);
		circleOut.m_contactNormal = -circle.m_normal;
		circleOut.m_contactFlag = true;
		circleOut.m_variable = 0.f;
		circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	return false;
}

PfxBool intersectMovingCircleFuncSphere(
	const PfxCircleInputInternal &circleIn, PfxCircleOutputInternal &circleOut,
	const PfxShape &shape, const PfxTransform3 &transform,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxFloat lambda = 1.0f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxSphere sphereA = shape.getSphere();
	PfxCircle circleB(circleIn.m_radius, rotate(circleIn.m_orientation, normalize(circleIn.m_direction)));

	PfxGjkSweep<PfxSphere, PfxCircle> gjkSweep(&sphereA, &circleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(PfxQuat::identity(), circleIn.m_startPosition);
	PfxVector3 translationB = circleIn.m_direction;

	PfxInt32 res = gjkSweep.sweep(lambda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (res == kPfxGjkResultOk && lambda < circleOut.m_variable) {
		circleOut.m_contactPoint = PfxVector3(circleIn.m_startPosition + lambda * circleIn.m_direction + PfxVector3(pB));
		circleOut.m_contactNormal = -circleB.m_normal;
		circleOut.m_contactFlag = true;
		circleOut.m_variable = lambda;
		circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (res == kPfxGjkResultIntersect && lambda < circleOut.m_variable) {
		return intersectStaticCircleSphere(circleIn, circleB, circleOut, sphereA, pfxRemoveScale(transform, shape.getScale()));
	}
	return false;
}

static PfxBool findClosestPointOnRectangularPlane(
	const PfxVector3 &rayStartPosInA,
	const PfxVector3 &p1,
	const PfxVector3 &p2,
	const PfxVector3 &p3,
	const PfxVector3 &p4,
	const PfxVector3 &normalizedRayDirInA,
	PfxVector3 &closestPoint)
{
	// The intersection between 12 edges and the plane on which the circle is
	PfxVector3 intersectionsBetweenEdgesAndPlane[4];
	PfxVector3 tempIntersect;

	PfxVector3 p[4] = { p1, p2, p3, p4 };
	PfxVector3 d[4] = { p2 - p1, p3 - p2, p4 - p3, p1 - p4 };

	PfxUInt32 numLines = 0u, numPoints = 0u;
	PfxVector3 intersectPoints[4];
	PfxUInt32 intersectLinesId[4];

	for (PfxUInt32 i = 0; i < 4; ++i) {
		PfxUInt8 res = pfxIntersectLineAndPlane(p[i], d[i], rayStartPosInA, normalizedRayDirInA, tempIntersect);

		if (res == SCE_PFX_INTERSECT_RESULT_ONE_LINE) {
			intersectLinesId[numLines++] = i;
		}
		else if (res == SCE_PFX_INTERSECT_RESULT_ONE_POINT) {
			intersectPoints[numPoints++] = tempIntersect;
		}
	}

	// Same plane
	if (numLines == 4)
	{
		// Project the point onto the rectangle plane
		PfxVector3 planeNormal = cross(d[0], -d[3]);
		SCE_PFX_ASSERT(lengthSqr(planeNormal) >= SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON);
		planeNormal /= length(planeNormal);
		PfxFloat t = dot(p1 - rayStartPosInA, planeNormal);
		PfxVector3 projectedPointOnPlane = rayStartPosInA + planeNormal * t;

		// Find the closest point inside the rectangle
		PfxVector3 vecRectangleToProjectedPoint = projectedPointOnPlane - p1;
		PfxFloat rectangleWSqr = lengthSqr(d[0]);
		PfxFloat rectangleHSqr = lengthSqr(d[3]);
		PfxFloat xRootInRectangle =  dot(vecRectangleToProjectedPoint, d[0]) / rectangleWSqr;
		PfxFloat yRootInRectangle = -dot(vecRectangleToProjectedPoint, d[3]) / rectangleHSqr;

		PfxFloat closestXRootInRectangle = SCE_PFX_CLAMP(xRootInRectangle, 0.f, 1.f);
		PfxFloat closestYRootInRectangle = SCE_PFX_CLAMP(yRootInRectangle, 0.f, 1.f);

		closestPoint = p1 + closestXRootInRectangle * d[0] - closestYRootInRectangle * d[3];
		return true;
	}

	// Same edge
	else if (numLines == 1)
	{
		pfxClosestPointLine(rayStartPosInA, p[intersectLinesId[0]], d[intersectLinesId[0]], closestPoint);
		return true;
	}

	// One line or one point
	else if (numPoints == 2)
	{
		if (lengthSqr(intersectPoints[1] - intersectPoints[0]) < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON)
			closestPoint = intersectPoints[0];

		else 
			pfxClosestPointLine(rayStartPosInA, intersectPoints[0], intersectPoints[1] - intersectPoints[0], closestPoint);

		return true;
	}

	return false;
}

PfxBool intersectMovingCircleFuncBox(
				const PfxCircleInputInternal &circleIn,PfxCircleOutputInternal &circleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxFloat lambda = 1.f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxBox boxA = shape.getBox();
	PfxCircle circleB(circleIn.m_radius, rotate(circleIn.m_orientation, normalize(circleIn.m_direction)));

	PfxGjkSweep<PfxBox, PfxCircle> gjkSweep(&boxA, &circleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(PfxQuat::identity(), circleIn.m_startPosition);
	PfxVector3 translationB = circleIn.m_direction;

	PfxInt32 res = gjkSweep.sweep(lambda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (res == kPfxGjkResultOk && lambda < circleOut.m_variable) {
		circleOut.m_contactPoint = PfxVector3(circleIn.m_startPosition + lambda * circleIn.m_direction + PfxVector3(pB));
		circleOut.m_contactNormal = -circleB.m_normal;
		circleOut.m_contactFlag = true;
		circleOut.m_variable = lambda;
		circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (res == kPfxGjkResultIntersect && lambda < circleOut.m_variable) {
		// World coordinate -> A-local coordinate
		PfxTransform3 transformAInv = inverse(transformA);
		PfxVector3 offsetAB = transformAInv.getTranslation();

		// The 8 vertices of AABB surface
		PfxVector3 p1 = mulPerElem(boxA.m_half, PfxVector3( 1.f, -1.f,  1.f));
		PfxVector3 p2 = mulPerElem(boxA.m_half, PfxVector3(-1.f, -1.f,  1.f));
		PfxVector3 p3 = mulPerElem(boxA.m_half, PfxVector3(-1.f, -1.f, -1.f));
		PfxVector3 p4 = mulPerElem(boxA.m_half, PfxVector3( 1.f, -1.f, -1.f));
		PfxVector3 p5 = mulPerElem(boxA.m_half, PfxVector3( 1.f,  1.f,  1.f));
		PfxVector3 p6 = mulPerElem(boxA.m_half, PfxVector3(-1.f,  1.f,  1.f));
		PfxVector3 p7 = mulPerElem(boxA.m_half, PfxVector3(-1.f,  1.f, -1.f));
		PfxVector3 p8 = mulPerElem(boxA.m_half, PfxVector3( 1.f,  1.f, -1.f));

		// Test the 6 planes and find the closest point
		PfxVector3 normalizedRayDirInA = normalize(transformAInv.getUpper3x3() * rotate(circleIn.m_orientation, circleIn.m_direction));
		PfxVector3 rayStartPosInA = (transformAInv.getUpper3x3() * circleIn.m_startPosition) + offsetAB;
		PfxVector3 closestPointInA, closestNormalInA, tempClosestPointInA;
		PfxFloat closestDistanceSqr = SCE_PFX_FLT_MAX, tempDistanceSqr;

		// Bottom plane (y == -1)
		if (findClosestPointOnRectangularPlane(rayStartPosInA, p1, p2, p3, p4, normalizedRayDirInA, tempClosestPointInA)) {
			tempDistanceSqr = lengthSqr(tempClosestPointInA - rayStartPosInA);
			if (closestDistanceSqr > tempDistanceSqr) {
				closestPointInA = tempClosestPointInA;
				closestNormalInA = -PfxVector3::yAxis();
				closestDistanceSqr = tempDistanceSqr;
			}
		}

		// Left plane (x == -1)
		if (findClosestPointOnRectangularPlane(rayStartPosInA, p6, p2, p3, p7, normalizedRayDirInA, tempClosestPointInA)) {
			tempDistanceSqr = lengthSqr(tempClosestPointInA - rayStartPosInA);
			if (closestDistanceSqr > tempDistanceSqr) {
				closestPointInA = tempClosestPointInA;
				closestNormalInA = -PfxVector3::xAxis();
				closestDistanceSqr = tempDistanceSqr;
			}
		}

		// Front plane (z == -1)
		if (findClosestPointOnRectangularPlane(rayStartPosInA, p8, p7, p3, p4, normalizedRayDirInA, tempClosestPointInA)) {
			tempDistanceSqr = lengthSqr(tempClosestPointInA - rayStartPosInA);
			if (closestDistanceSqr > tempDistanceSqr) {
				closestPointInA = tempClosestPointInA;
				closestNormalInA = -PfxVector3::zAxis();
				closestDistanceSqr = tempDistanceSqr;
			}
		}

		// Right plane (x == 1)
		if (findClosestPointOnRectangularPlane(rayStartPosInA, p5, p1, p4, p8, normalizedRayDirInA, tempClosestPointInA)) {
			tempDistanceSqr = lengthSqr(tempClosestPointInA - rayStartPosInA);
			if (closestDistanceSqr > tempDistanceSqr) {
				closestPointInA = tempClosestPointInA;
				closestNormalInA = PfxVector3::xAxis();
				closestDistanceSqr = tempDistanceSqr;
			}
		}

		// Back plane (z == 1)
		if (findClosestPointOnRectangularPlane(rayStartPosInA, p5, p6, p2, p1, normalizedRayDirInA, tempClosestPointInA)) {
			tempDistanceSqr = lengthSqr(tempClosestPointInA - rayStartPosInA);
			if (closestDistanceSqr > tempDistanceSqr) {
				closestPointInA = tempClosestPointInA;
				closestNormalInA = PfxVector3::zAxis();
				closestDistanceSqr = tempDistanceSqr;
			}
		}

		// Top plane (y == 1)
		if (findClosestPointOnRectangularPlane(rayStartPosInA, p5, p6, p7, p8, normalizedRayDirInA, tempClosestPointInA)) {
			tempDistanceSqr = lengthSqr(tempClosestPointInA - rayStartPosInA);
			if (closestDistanceSqr > tempDistanceSqr) {
				closestPointInA = tempClosestPointInA;
				closestNormalInA = PfxVector3::yAxis();
				closestDistanceSqr = tempDistanceSqr;
			}
		}

		// If the box intersects with the circle plane
		if (closestDistanceSqr < SCE_PFX_FLT_MAX) {

			// Transform to world coordinage
			PfxVector3 closestPoint = (transformA.getUpper3x3() * closestPointInA) + transformA.getTranslation();

			// Adjust the position
			PfxVector3 normal = closestPoint - circleIn.m_startPosition;
			PfxFloat lenSqrNormal = lengthSqr(normal);
			if (lenSqrNormal > circleIn.m_radius * circleIn.m_radius){
				normal /= -sqrtf(lenSqrNormal);
				closestPoint = circleIn.m_startPosition - normal * circleIn.m_radius;
			}

			circleOut.m_contactPoint = closestPoint;
			circleOut.m_contactNormal = -circleB.m_normal;
			circleOut.m_contactFlag = true;
			circleOut.m_variable = lambda;
			circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);

			return true;
		}
	}

	return false;
}

static PfxBool findClosestPointOnClippedSemiSphere(
	const PfxVector3 &rayStartPoint,
	const PfxVector3 &normalizedRayDir,
	const PfxVector3 &semiSphereCenter,
	const PfxVector3 &normalizedSemiSphereDir,
	PfxFloat semiSphereRadius,
	PfxVector3 &closestPoint)
{
	PfxVector3 intersectCircleCenter;
	PfxCircle intersectCircle;

	PfxUInt8 res = pfxIntersectSphereAndPlane(semiSphereCenter, semiSphereRadius, rayStartPoint, normalizedRayDir, intersectCircleCenter, intersectCircle);
	if (res == SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE) {

		// The vector from ray start posion to intersect
		PfxVector3 vecToClosestPoint = cross(normalizedRayDir, normalizedSemiSphereDir);
		PfxFloat lenSqrVecToClosestPoint = lengthSqr(vecToClosestPoint);
		if (lenSqrVecToClosestPoint >= SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
			vecToClosestPoint = cross(normalizedRayDir, vecToClosestPoint);
			lenSqrVecToClosestPoint = lengthSqr(vecToClosestPoint);
		}
		else {
			vecToClosestPoint = cross(normalizedRayDir, PfxVector3::yAxis());
			lenSqrVecToClosestPoint = lengthSqr(vecToClosestPoint);
			if (lenSqrVecToClosestPoint < SCE_PFX_INTERSECT_COMMON_EPSILON * SCE_PFX_INTERSECT_COMMON_EPSILON) {
				vecToClosestPoint = cross(normalizedRayDir, PfxVector3::zAxis());
				lenSqrVecToClosestPoint = lengthSqr(vecToClosestPoint);
			}
		}
		vecToClosestPoint /= sqrtf(lenSqrVecToClosestPoint);

		closestPoint = intersectCircleCenter + vecToClosestPoint * intersectCircle.m_radius;

		// Detect whether the closest point is on the correct side of the sphere
		if (dot(closestPoint - semiSphereCenter, normalizedSemiSphereDir) < 0.f) {
			closestPoint = intersectCircleCenter - vecToClosestPoint * intersectCircle.m_radius;
		}

		return (dot(closestPoint - semiSphereCenter, normalizedSemiSphereDir) >= 0.f);
	}
	else if (res == SCE_PFX_INTERSECT_RESULT_ONE_POINT) {

		closestPoint = intersectCircleCenter;

		// Detect whether the point is on the semi-sphere
		return (dot(closestPoint - semiSphereCenter, normalizedSemiSphereDir) >= 0.f);
	}

	return false;	
}

static PfxBool findClosestPointOnClippedTube(
	const PfxVector3 &rayStartPoint,
	const PfxVector3 &normalizedRayDir,
	const PfxVector3 &tubeP1,
	const PfxVector3 &tubeP2,
	PfxFloat tubeRadius,
	PfxVector3 &closestPoint)
{
	PfxVector3 intersectCenter;
	PfxEllipse intersectEllipse;
	PfxLineSegment intersectLineSegment1, intersectLineSegment2;

	PfxUInt8 res = pfxIntersectTubeAndPlane(tubeP1, tubeP2, tubeRadius, rayStartPoint, normalizedRayDir, intersectCenter, intersectEllipse, intersectLineSegment1, intersectLineSegment2);
	if (res == SCE_PFX_INTERSECT_RESULT_ONE_LINE) {
		pfxClosestPointLine(rayStartPoint, intersectLineSegment1.m_point1, intersectLineSegment1.m_point2 - intersectLineSegment1.m_point1, closestPoint);
		return true;
	}

	else if (res == SCE_PFX_INTERSECT_RESULT_TWO_LINES) {
		PfxVector3 closestPoint1, closestPoint2;
		pfxClosestPointLine(rayStartPoint, intersectLineSegment1.m_point1, intersectLineSegment1.m_point2 - intersectLineSegment1.m_point1, closestPoint1);
		pfxClosestPointLine(rayStartPoint, intersectLineSegment2.m_point1, intersectLineSegment2.m_point2 - intersectLineSegment2.m_point1, closestPoint2);

		if (lengthSqr(rayStartPoint - closestPoint1) < lengthSqr(rayStartPoint - closestPoint2))
			closestPoint = closestPoint1;

		else
			closestPoint = closestPoint2;

		return true;
	}
	
	else if (res == SCE_PFX_INTERSECT_RESULT_ONE_CIRCLE)
	{
		PfxVector3 vecCenterToRay = rayStartPoint - intersectCenter;
		closestPoint = intersectCenter + vecCenterToRay * (tubeRadius / length(vecCenterToRay));
		return true;
	}

	else if (res == SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE) 
	{
		PfxVector3 vecCenterToRay = rayStartPoint - intersectCenter;
		PfxFloat x = dot(vecCenterToRay, intersectEllipse.m_majorAxis);
		PfxFloat y = dot(vecCenterToRay, intersectEllipse.m_minorAxis);
		PfxFloat a = intersectEllipse.m_majorRadius;
		PfxFloat b = intersectEllipse.m_minorRadius;

		PfxFloat t = sqrtf(1.f / ((x*x) / (a * a) + (y*y) / (b*b)));

		closestPoint = intersectCenter + vecCenterToRay * t;
		return true;
	}

	else if (res == SCE_PFX_INTERSECT_RESULT_ONE_ELLIPSE_PARTIALLY_OUTSIDE)
	{
		PfxVector3 vecCenterToRay = rayStartPoint - intersectCenter;
		PfxFloat x = dot(vecCenterToRay, intersectEllipse.m_majorAxis);
		PfxFloat y = dot(vecCenterToRay, intersectEllipse.m_minorAxis);
		PfxFloat a = intersectEllipse.m_majorRadius;
		PfxFloat b = intersectEllipse.m_minorRadius;

		PfxFloat t = sqrtf(1.f / ((x*x) / (a * a) + (y*y) / (b*b)));

		PfxVector3 tempClosestPoint = intersectCenter + vecCenterToRay * t;
		PfxVector3 tubeAxis = tubeP2 - tubeP1;
		PfxFloat tempClosestPointPosOnTubeAxis = dot(tempClosestPoint - tubeP1, tubeAxis) / lengthSqr(tubeAxis);
		if (tempClosestPointPosOnTubeAxis >= 0.f && 
			tempClosestPointPosOnTubeAxis <= 1.f) 
		{
			closestPoint = tempClosestPoint;
			return true;
		}

		// Test the boundary of tube
		PfxVector3 normalizedTubeDir = normalize(tubeAxis);
		PfxVector3 circleP1, circleP2;
		PfxCircle boundaryCircle(tubeRadius, normalizedTubeDir);
		PfxUInt8 res2;
		
		if (lengthSqr(rayStartPoint - tubeP1) < lengthSqr(rayStartPoint - tubeP2))
			res2 = pfxIntersectHollowCircleAndPlane(tubeP1, boundaryCircle, rayStartPoint, normalizedRayDir, circleP1, circleP2);

		else
			res2 = pfxIntersectHollowCircleAndPlane(tubeP2, boundaryCircle, rayStartPoint, normalizedRayDir, circleP1, circleP2);
		
		if (res2 == SCE_PFX_INTERSECT_RESULT_ONE_POINT) {
			closestPoint = circleP1;
			return true;
		}

		else if (res2 == SCE_PFX_INTERSECT_RESULT_TWO_POINTS) {

			if (lengthSqr(rayStartPoint - circleP1) < lengthSqr(rayStartPoint - circleP2))
				closestPoint = circleP1;

			else
				closestPoint = circleP2;

			return true;
		}
	}

	return false;
}

PfxBool intersectMovingCircleFuncCapsule(
				const PfxCircleInputInternal &circleIn,PfxCircleOutputInternal &circleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxFloat lambda = 1.f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxCapsule capsuleA = shape.getCapsule();
	PfxCircle circleB(circleIn.m_radius, rotate(circleIn.m_orientation, normalize(circleIn.m_direction)));

	PfxGjkSweep<PfxCapsule, PfxCircle> gjkSweep(&capsuleA, &circleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(PfxQuat::identity(), circleIn.m_startPosition);
	PfxVector3 translationB = circleIn.m_direction;

	PfxInt32 res = gjkSweep.sweep(lambda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (res == kPfxGjkResultOk && lambda < circleOut.m_variable) {
		circleOut.m_contactPoint = PfxVector3(circleIn.m_startPosition + lambda * circleIn.m_direction + PfxVector3(pB));
		circleOut.m_contactNormal = -circleB.m_normal;
		circleOut.m_contactFlag = true;
		circleOut.m_variable = lambda;
		circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (res == kPfxGjkResultIntersect && lambda < circleOut.m_variable) {
		PfxVector3 closestPoint, tempClosestPoint, closestNormal;
		PfxFloat distanceSqrToClosestPoint = SCE_PFX_FLT_MAX, tempDistanceSqrToClosestPoint;

		PfxVector3 capP1 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3(-capsuleA.m_halfLen, 0.f, 0.f);
		PfxVector3 capP2 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3( capsuleA.m_halfLen, 0.f, 0.f);
		PfxFloat capRadA = shape.getScale() * capsuleA.m_radius;
		PfxVector3 capAxis = capP2 - capP1;
		if (lengthSqr(capAxis) <= SCE_PFX_INTERSECT_COMMON_EPSILON) {
			PfxSphere sphere(capRadA);
//			return pfxIntersectMovingCircleSphere(circleIn, circleOut, sphere, pfxRemoveScale(transform, shape.getScale()));
			return intersectStaticCircleSphere(circleIn, circleB, circleOut, sphere, pfxRemoveScale(transform, shape.getScale()));
		}
		PfxVector3 normalizedCapDir = capAxis / length(capAxis);
		PfxVector3 normalizedRayDir = circleB.m_normal;

		// Detect collision with the 2 semi-spheres
		if (findClosestPointOnClippedSemiSphere(circleIn.m_startPosition, normalizedRayDir, capP1, -normalizedCapDir, capRadA, tempClosestPoint)) {
			tempDistanceSqrToClosestPoint = lengthSqr(tempClosestPoint - circleIn.m_startPosition);
			if (distanceSqrToClosestPoint > tempDistanceSqrToClosestPoint) {
				closestPoint = tempClosestPoint;
				closestNormal = normalize(tempClosestPoint - capP1);
				distanceSqrToClosestPoint = tempDistanceSqrToClosestPoint;
			}
		}

		if (findClosestPointOnClippedSemiSphere(circleIn.m_startPosition, normalizedRayDir, capP2, normalizedCapDir, capRadA, tempClosestPoint)) {
			tempDistanceSqrToClosestPoint = lengthSqr(tempClosestPoint - circleIn.m_startPosition);
			if (distanceSqrToClosestPoint > tempDistanceSqrToClosestPoint) {
				closestPoint = tempClosestPoint;
				closestNormal = normalize(tempClosestPoint - capP2);
				distanceSqrToClosestPoint = tempDistanceSqrToClosestPoint;
			}
		}

		// Detect collistion with the tube
		if (findClosestPointOnClippedTube(circleIn.m_startPosition, normalizedRayDir, capP1, capP2, capRadA, tempClosestPoint)) {
			tempDistanceSqrToClosestPoint = lengthSqr(tempClosestPoint - circleIn.m_startPosition);
			if (distanceSqrToClosestPoint > tempDistanceSqrToClosestPoint) {
				PfxVector3 closestPointOnTubeAxis;
				pfxClosestPointLine(tempClosestPoint, capP1, normalizedCapDir, closestPointOnTubeAxis);

				closestPoint = tempClosestPoint;
				closestNormal = normalize(tempClosestPoint - closestPointOnTubeAxis);
				distanceSqrToClosestPoint = tempDistanceSqrToClosestPoint;
			}
		}

		if (distanceSqrToClosestPoint < SCE_PFX_FLT_MAX) {

			// Adjust the position
			PfxVector3 normal = closestPoint - circleIn.m_startPosition;
			PfxFloat lenSqrNormal = lengthSqr(normal);
			if (lenSqrNormal > circleIn.m_radius * circleIn.m_radius){
				normal /= -sqrtf(lenSqrNormal);
				closestPoint = circleIn.m_startPosition - normal * circleIn.m_radius;
			}

			circleOut.m_contactPoint = closestPoint;
			circleOut.m_contactNormal = -circleB.m_normal;
			circleOut.m_contactFlag = true;
			circleOut.m_variable = lambda;
			circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);

			return true;
		}
	}

	return false;
}

static PfxBool findClosestPointOnFilledCircle(
	const PfxVector3 &rayStartPoint,
	const PfxVector3 &normalizedRayDir,
	const PfxVector3 &circleCenter,
	const PfxCircle &circle,
	PfxVector3 &closestPoint)
{
	PfxVector3 intersectPoint;
	PfxLineSegment intersectLineSegment;

	PfxUInt8 res = pfxIntersectFilledCircleAndPlane(circleCenter, circle, rayStartPoint, normalizedRayDir, intersectPoint, intersectLineSegment);
	if (res == SCE_PFX_INTERSECT_RESULT_ONE_PLANE) {

		// The vector from circle center to ray start posion
		PfxVector3 vecCircleToRay = rayStartPoint - circleCenter;
		PfxFloat distSqrVecCircleToRay = lengthSqr(vecCircleToRay);
		if (distSqrVecCircleToRay > circle.m_radius) {
			closestPoint = circleCenter + vecCircleToRay * (circle.m_radius / sqrtf(distSqrVecCircleToRay));
		}

		else {
			closestPoint = rayStartPoint;
		}
		return true;
	}
	else if (res == SCE_PFX_INTERSECT_RESULT_ONE_LINE) {

		pfxClosestPointLine(rayStartPoint, intersectLineSegment.m_point1, intersectLineSegment.m_point2 - intersectLineSegment.m_point1, closestPoint);
		return true;
	}

	else if (res == SCE_PFX_INTERSECT_RESULT_ONE_POINT) {

		closestPoint = intersectPoint;
		return true;
	}

	return false;
}

PfxBool intersectMovingCircleFuncCylinder(
				const PfxCircleInputInternal &circleIn,PfxCircleOutputInternal &circleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxFloat lambda = 1.f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxCylinder cylinderA = shape.getCylinder();
	PfxCircle circleB(circleIn.m_radius, rotate(circleIn.m_orientation, normalize(circleIn.m_direction)));

	PfxGjkSweep<PfxCylinder, PfxCircle> gjkSweep(&cylinderA, &circleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.f);
	PfxTransform3 transformB(PfxQuat::identity(), circleIn.m_startPosition);
	PfxVector3 translationB = circleIn.m_direction;

	PfxInt32 res = gjkSweep.sweep(lambda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (res == kPfxGjkResultOk && lambda < circleOut.m_variable) {
		circleOut.m_contactPoint = PfxVector3(circleIn.m_startPosition + lambda * circleIn.m_direction + PfxVector3(pB));
		circleOut.m_contactNormal = -circleB.m_normal;
		circleOut.m_contactFlag = true;
		circleOut.m_variable = lambda;
		circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		return true;
	}
	else if (res == kPfxGjkResultIntersect && lambda < circleOut.m_variable) {
		PfxVector3 closestPoint, tempClosestPoint, closestNormal;
		PfxFloat distanceSqrToClosestPoint = SCE_PFX_FLT_MAX, tempDistanceSqrToClosestPoint;

		PfxVector3 cylinderP1 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3(-cylinderA.m_halfLen, 0.f, 0.f);
		PfxVector3 cylinderP2 = transformA.getTranslation() + transformA.getUpper3x3() * PfxVector3(cylinderA.m_halfLen, 0.f, 0.f);
		PfxFloat cylinderRadA = shape.getScale() * cylinderA.m_radius;
		PfxVector3 cylinderAxis = cylinderP2 - cylinderP1;
		PfxVector3 normalizedCylinderDir;
		if (lengthSqr(cylinderAxis) <= SCE_PFX_INTERSECT_COMMON_EPSILON) {
			normalizedCylinderDir = PfxVector3::xAxis();
		}
		normalizedCylinderDir = cylinderAxis / length(cylinderAxis);
		PfxVector3 normalizedRayDir = circleB.m_normal;
		
		PfxCircle circleP1(cylinderA.m_radius, -normalizedCylinderDir);
		PfxCircle circleP2(cylinderA.m_radius, normalizedCylinderDir);

		// Detect collision with the 2 circles
		if (findClosestPointOnFilledCircle(circleIn.m_startPosition, normalizedRayDir, cylinderP1, circleP1, tempClosestPoint)) {
			tempDistanceSqrToClosestPoint = lengthSqr(tempClosestPoint - circleIn.m_startPosition);
			if (distanceSqrToClosestPoint > tempDistanceSqrToClosestPoint) {
				closestPoint = tempClosestPoint;
				closestNormal = -normalizedCylinderDir;
				distanceSqrToClosestPoint = tempDistanceSqrToClosestPoint;
			}
		}

		if (findClosestPointOnFilledCircle(circleIn.m_startPosition, normalizedRayDir, cylinderP2, circleP2, tempClosestPoint)) {
			tempDistanceSqrToClosestPoint = lengthSqr(tempClosestPoint - circleIn.m_startPosition);
			if (distanceSqrToClosestPoint > tempDistanceSqrToClosestPoint) {
				closestPoint = tempClosestPoint;
				closestNormal = normalizedCylinderDir;
				distanceSqrToClosestPoint = tempDistanceSqrToClosestPoint;
			}
		}

		// Detect collision with the tube
		if (findClosestPointOnClippedTube(circleIn.m_startPosition, normalizedRayDir, cylinderP1, cylinderP2, cylinderRadA, tempClosestPoint)) {
			tempDistanceSqrToClosestPoint = lengthSqr(tempClosestPoint - circleIn.m_startPosition);
			if (distanceSqrToClosestPoint > tempDistanceSqrToClosestPoint) {
				PfxVector3 closestPointOnTubeAxis;
				pfxClosestPointLine(tempClosestPoint, cylinderP1, normalizedCylinderDir, closestPointOnTubeAxis);

				closestPoint = tempClosestPoint;
				closestNormal = normalize(tempClosestPoint - closestPointOnTubeAxis);
				distanceSqrToClosestPoint = tempDistanceSqrToClosestPoint;
			}
		}

		if (distanceSqrToClosestPoint < SCE_PFX_FLT_MAX) {

			// Adjust the position
			PfxVector3 normal = closestPoint - circleIn.m_startPosition;
			PfxFloat lenSqrNormal = lengthSqr(normal);
			if (lenSqrNormal > circleIn.m_radius * circleIn.m_radius) {
				normal /= -sqrtf(lenSqrNormal);
				closestPoint = circleIn.m_startPosition - normal * circleIn.m_radius;
			}

			circleOut.m_contactPoint = closestPoint;
			circleOut.m_contactNormal = -circleB.m_normal;
			circleOut.m_contactFlag = true;
			circleOut.m_variable = lambda;
			circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);

			return true;
		}
	}

	return false;
}

PfxBool intersectMovingCircleFuncConvex(
				const PfxCircleInputInternal &circleIn,PfxCircleOutputInternal &circleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	PfxFloat lambda = 1.f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	const PfxConvexMeshImpl *convexA = (PfxConvexMeshImpl*)shape.getConvexMesh();
	PfxCircle circleB(circleIn.m_radius, rotate(circleIn.m_orientation, normalize(circleIn.m_direction)));

	PfxGjkSweep<PfxConvexMeshImpl, PfxCircle> gjkSweep(convexA, &circleB);

	PfxTransform3 transformA = transform;
	PfxVector3 translationA(0.f);
	PfxTransform3 transformB(PfxQuat::identity(), circleIn.m_startPosition);
	PfxVector3 translationB = circleIn.m_direction;

	PfxInt32 res = gjkSweep.sweep(lambda, nml, pA, pB, transformA, translationA, transformB, translationB);
	
	if (res == kPfxGjkResultOk && lambda < circleOut.m_variable) {
		circleOut.m_contactPoint = PfxVector3(circleIn.m_startPosition + lambda * circleIn.m_direction + PfxVector3(pB));
		circleOut.m_contactNormal = -circleB.m_normal;
		circleOut.m_contactFlag = true;
		circleOut.m_variable = lambda;
		circleOut.m_subData.setType(PfxSubData::SHAPE_INFO);
		
		// find the closest facet and set barycentric coordinates.
		PfxUInt32 faceId = 0;
		PfxUInt32 ids[3] = { 0, 1, 2 };
		PfxVector3 scale = shape.getScaleXyz();
		PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
		if (flipTriangle) { ids[0] = 2; ids[2] = 0; }

		PfxFloat dmin = SCE_PFX_FLT_MAX;
		for (PfxUInt32 f = 0; f<(PfxUInt32)convexA->m_numFacets; f++) {
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

		PfxFloat closestS = 0.0f, closestT = 0.0f;
		{
			PfxTriangle triangle(
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId * 3 + ids[0]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId * 3 + ids[1]] * 3),
				pfxReadPoint3(convexA->m_verts + convexA->m_indices[faceId * 3 + ids[2]] * 3));
			pfxCalcBarycentricCoords(PfxVector3(pA), triangle, closestS, closestT);
		}
		circleOut.m_subData.setFacetLocalS(closestS);
		circleOut.m_subData.setFacetLocalT(closestT);
		circleOut.m_subData.setFacetId(faceId);

		return true;
	}
	else if (res == kPfxGjkResultIntersect && lambda < circleOut.m_variable) {

		PfxVector3 closestNormal;
		PfxVector3 point = circleIn.m_startPosition;
		PfxVector3 normalizedRayDir = circleB.m_normal;

		// Find the closest triangle
		PfxUInt32 faceId = (PfxUInt32)-1;
		PfxUInt32 ids[3] = { 0, 1, 2 };

		PfxVector3 scale = shape.getScaleXyz();
		PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.f;
		if (flipTriangle) { ids[0] = 2; ids[2] = 0; }

		PfxVector3 closestPoint = point;
		PfxFloat closestDistacneSqr = SCE_PFX_FLT_MAX;
		PfxFloat closestS = 0.f, closestT = 0.f;
		
		// Get the closest point on a triangle
		for (PfxUInt32 f = 0u; f < (PfxUInt32)convexA->m_numFacets; ++f){
			PfxTriangle triangle(
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f * 3 + ids[0]] * 3),
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f * 3 + ids[1]] * 3),
				transformA * pfxReadPoint3(convexA->m_verts + convexA->m_indices[f * 3 + ids[2]] * 3));

			PfxVector3 triangleNormal = triangle.calcNormal();
			PfxFloat centerPositionAlongNormal = dot(triangleNormal, point - triangle.points[0]);
			if (centerPositionAlongNormal < 0.f) {
				continue;
			}

			PfxVector3 s;
			if (!pfxClosestPointOnClippedTriangle(point, normalizedRayDir, triangle, s)) {
				continue;
			}

			PfxFloat distanceSqr = lengthSqr(s - point);
			if (distanceSqr > closestDistacneSqr) {
				continue;
			}

			faceId = f;
			closestPoint = s;
			closestNormal = triangleNormal;
			closestDistacneSqr = distanceSqr;
			pfxCalcBarycentricCoords(closestPoint, triangle, closestS, closestT);
		}

		if (faceId == (PfxUInt32)-1) return false;

		// 2. check if the closest point is inside of the circle and calculate a normal vector
		PfxVector3 normal = closestPoint - circleIn.m_startPosition;
		PfxFloat lenSqrNormal = lengthSqr(normal);
		if (lenSqrNormal > circleIn.m_radius * circleIn.m_radius) {
			normal /= -sqrtf(lenSqrNormal);
			closestPoint = circleIn.m_startPosition - normal * circleIn.m_radius;
		}

		circleOut.m_contactNormal = -circleB.m_normal;
		circleOut.m_contactPoint = closestPoint;
		circleOut.m_contactFlag = true;
		circleOut.m_variable = lambda;
		circleOut.m_subData.setType(PfxSubData::MESH_INFO);
		circleOut.m_subData.setFacetLocalS(closestS);
		circleOut.m_subData.setFacetLocalT(closestT);
		circleOut.m_subData.setFacetId(faceId);
		if (convexA->m_userData) {
			circleOut.m_subData.setUserData(convexA->m_userData[faceId]);
		}
		return true;
	}

	return false;
}

PfxBool intersectMovingCircleFuncLargeTriMesh(
				const PfxCircleInputInternal &circleIn,PfxCircleOutputInternal &circleOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape.getLargeTriMesh();
	
	PfxVector3 scale = shape.getScaleXyz();
	PfxBool flipTriangle = (scale[0] * scale[1] * scale[2]) < 0.0f;
	PfxFloat radiusLocal = circleIn.m_radius / minElem(absPerElem(scale));

	PfxBool ret = pfxIntersectMovingCircleLargeTriMesh(circleIn,circleOut,(const void*)largeMesh,transform,flipTriangle,radiusLocal, discardTriangleCallback, userDataForDiscardingTriangles);
	
	return ret;
}

PfxIntersectMovingCircleFunc funcTbl_intersectMovingCircle[kPfxShapeCount] = {
	intersectMovingCircleFuncSphere,
	intersectMovingCircleFuncBox,
	intersectMovingCircleFuncCapsule,
	intersectMovingCircleFuncCylinder,
	intersectMovingCircleFuncConvex,
	intersectMovingCircleFuncLargeTriMesh,
	intersectMovingCircleFuncDummy,
	intersectMovingCircleFuncDummy, // Reserved
	intersectMovingCircleFuncDummy, // Reserved
	intersectMovingCircleFuncDummy, // Reserved
	intersectMovingCircleFuncDummy, // Reserved
	intersectMovingCircleFuncDummy, // Reserved
};

///////////////////////////////////////////////////////////////////////////////
// Moving Circle Intersection Function Table Interface

PfxIntersectMovingCircleFunc pfxGetIntersectMovingCircleFunc(PfxUInt8 shapeType)
{
	return funcTbl_intersectMovingCircle[shapeType];
}

PfxInt32 pfxSetIntersectCircleFunc(PfxUInt8 shapeType,PfxIntersectMovingCircleFunc func)
{
	if(shapeType >= kPfxShapeCount) {
		return SCE_PFX_ERR_OUT_OF_RANGE;
	}
	
	funcTbl_intersectMovingCircle[shapeType] = func;
	
	return SCE_PFX_OK;
}

} //namespace pfxv4
} //namespace sce
