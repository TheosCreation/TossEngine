/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2020 Sony Interactive Entertainment Inc.
*                                                
*/

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "pfx_intersect_common.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"
#include "pfx_intersect_moving_circle_large_tri_mesh.h"
#include "../../low_level/broadphase/pfx_bounding_volume.h"
#include "../../low_level/collision/pfx_bounding_volume_vector3.h"
#include <float.h>

namespace sce {
namespace pfxv4 {

static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcCastRadius(const PfxCircleInputInternal &circleIn) { return (circleIn.m_radius); }
static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcInitialSquaredDistance(const PfxCircleInputInternal &circleIn) { return (circleIn.m_radius) * (circleIn.m_radius); }

#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

static PfxBool pfxIntersectMovingCircleOneTriangle(const PfxTriangle &triangle, const PfxCircleInputInternal &circleIn, PfxClosestFacetInfo &closestFacetInfo)
{
	PfxFloat lambda = 1.f;
	PfxVector3 nml;
	PfxPoint3 pA, pB;

	PfxVector3 centerA = (triangle.points[0] + triangle.points[1] + triangle.points[2]) / 3.f;
	PfxTransform3 transformA = PfxTransform3::translation(centerA);
	PfxVector3 facetPoints[3] = { triangle.points[0] - centerA, triangle.points[1] - centerA, triangle.points[2] - centerA };

	PfxTriangle triangle_(facetPoints[0], facetPoints[1], facetPoints[2]);
	PfxCircle circleB(circleIn.m_radius, normalize(circleIn.m_direction));

	PfxGjkSweep<PfxTriangle, PfxCircle> gjkSweep(&triangle_, &circleB);

	PfxVector3 translationA(0.f);
	PfxTransform3 transformB(PfxQuat::identity(), circleIn.m_startPosition);
	PfxVector3 translationB = circleIn.m_direction;

	PfxInt32 res = gjkSweep.sweep(lambda, nml, pA, pB, transformA, translationA, transformB, translationB);

	if (res == kPfxGjkResultOk) {
		closestFacetInfo.tmin = lambda;
		closestFacetInfo.contactPoint = PfxVector3(circleIn.m_startPosition + lambda * circleIn.m_direction + PfxVector3(pB));
		closestFacetInfo.squaredDistance = lengthSqr(closestFacetInfo.contactPoint - circleIn.m_startPosition);
		return true;
	}
	else if (res == kPfxGjkResultIntersect) {
		pfxClosestPointOnClippedTriangle(circleIn.m_startPosition, normalize(circleIn.m_direction), triangle, closestFacetInfo.contactPoint);
		closestFacetInfo.squaredDistance = lengthSqr(closestFacetInfo.contactPoint - circleIn.m_startPosition);
		closestFacetInfo.tmin = 0.f;
		return true;
	}

	return false;
}

PfxBool pfxIntersectMovingCircleTriangle(const PfxCircleInputInternal &circleIn, const PfxTriangle &triangle, PfxVector3 &contactPoint, PfxFloat &variable, PfxFloat &squaredDistance)
{
	PfxClosestFacetInfo closestFacetInfo;
	closestFacetInfo.squaredDistance = squaredDistance;

	if (pfxIntersectMovingCircleOneTriangle(triangle, circleIn, closestFacetInfo)) {
		contactPoint = closestFacetInfo.contactPoint;
		variable = closestFacetInfo.tmin;
		squaredDistance = closestFacetInfo.squaredDistance;
		return true;
	}
	return false;
}

static PfxBool pfxIntersectMovingCircleLargeTriMeshAllTriangles(const PfxCircleInputInternal &circleIn, PfxCircleOutputInternal &circleOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;

	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhObj<PfxCircleInputInternal, PfxCircleOutputInternal> bvhDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, transform, flipTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastTriMeshArrayObj<PfxCircleInputInternal, PfxCircleOutputInternal> arrayDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, transform, flipTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
}

// ----------------------------------------------------------------------------
//	Sphere cast with discarding triangle callback

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_DISCARD_TRIANGLE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

static PfxBool pfxIntersectMovingCircleLargeTriMeshPartialTriangles(const PfxCircleInputInternal &circleIn, PfxCircleOutputInternal &circleOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles) 
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhPartialObj<PfxCircleInputInternal, PfxCircleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangles);
		return bvhDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastTriMeshArrayPartialObj<PfxCircleInputInternal, PfxCircleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangles);
		return arrayDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
}

#undef CAST_ENABLE_DISCARD_TRIANGLE

PfxBool pfxIntersectMovingCircleLargeTriMesh(const PfxCircleInputInternal &circleIn, PfxCircleOutputInternal &circleOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangles)
{
	if (discardTriangleCallback == NULL) {
		return pfxIntersectMovingCircleLargeTriMeshAllTriangles(circleIn, circleOut, shape, transform, flipTriangle, radiusLocal);
	}
	return pfxIntersectMovingCircleLargeTriMeshPartialTriangles(circleIn, circleOut, shape, transform, flipTriangle, radiusLocal, discardTriangleCallback, userDataForDiscardingTriangles);
}

// ----------------------------------------------------------------------------
//	CircleCast of cached rigidbody 

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_RIGIDBODY_CACHE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

// ----------------------------------------------------------------------------
//	Ray cast of cached rigidbody with discarding triangle callback

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_DISCARD_TRIANGLE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

PfxBool pfxIntersectMovingCircleIslandsOfLargeTriMeshBvh(const PfxCircleInputInternal &circleIn, PfxCircleOutputInternal &circleOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &localToWorldTransform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandBvhObj<PfxCircleInputInternal, PfxCircleOutputInternal> bvhDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, islandIds, numIslands, localToWorldTransform, flipTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastCachedIslandBvhPartialObj<PfxCircleInputInternal, PfxCircleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, islandIds, numIslands, localToWorldTransform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
}

PfxBool pfxIntersectMovingCircleIslandsOfLargeTriMeshArray(const PfxCircleInputInternal &circleIn, PfxCircleOutputInternal &circleOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &localToWorldTransform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandArrayObj<PfxCircleInputInternal, PfxCircleOutputInternal> arrayDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, islandIds, numIslands, localToWorldTransform, flipTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastCachedIslandArrayPartialObj<PfxCircleInputInternal, PfxCircleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectMovingCircleOneTriangle, largeMesh, islandIds, numIslands, localToWorldTransform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(circleIn, circleOut, radiusLocal);
	}
}

#undef CAST_ENABLE_RIGIDBODY_CACHE
#undef CAST_ENABLE_DISCARD_TRIANGLE

}
}

