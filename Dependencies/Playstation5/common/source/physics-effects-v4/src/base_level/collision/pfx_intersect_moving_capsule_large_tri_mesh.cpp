/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "../../../include/physics_effects/base_level/collision/pfx_capsule.h"
#include "pfx_intersect_common.h"
#include "pfx_convex_mesh_impl.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_gjk_solver.h"
#include "pfx_gjk_support_func.h"
#include "pfx_intersect_moving_capsule_large_tri_mesh.h"
#include "../../low_level/broadphase/pfx_bounding_volume.h"
#include "../../low_level/collision/pfx_bounding_volume_vector3.h"
#include <float.h>

namespace sce {
namespace pfxv4 {

static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcCastRadius(const PfxCapsuleInputInternal &capsuleIn) { return calcRadiusOfSweptCapsule(capsuleIn); }
static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcInitialSquaredDistance(const PfxCapsuleInputInternal &capsuleIn) { return (capsuleIn.m_halfLength + capsuleIn.m_radius) * (capsuleIn.m_halfLength + capsuleIn.m_radius); }

#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

static PfxBool pfxIntersectMovingCapsuleOneTriangle(const PfxTriangle &triangle, const PfxCapsuleInputInternal &capsuleIn, PfxClosestFacetInfo &closestFacetInfo)
{
	PfxFloat lambda = 1.0f;
	PfxVector3 normal;
	PfxPoint3 pA, pB;

	PfxVector3 centerA = (triangle.points[0] + triangle.points[1] + triangle.points[2]) / 3.0f;
	PfxTransform3 transformA = PfxTransform3::translation(centerA);

	PfxTriangle triangleA(triangle.points[0] - centerA, triangle.points[1] - centerA, triangle.points[2] - centerA);
	PfxCapsule capsuleB(capsuleIn.m_halfLength, capsuleIn.m_radius);

	PfxGjkSweep<PfxTriangle, PfxCapsule> gjkSweep(&triangleA, &capsuleB);

	PfxVector3 translationA(0.0f);
	PfxTransform3 transformB(capsuleIn.m_orientation, capsuleIn.m_startPosition);
	PfxVector3 translationB = capsuleIn.m_direction;

	PfxInt32 err = gjkSweep.sweep(lambda, normal, pA, pB, transformA, translationA, transformB, translationB);

	if (err == kPfxGjkResultOk) {
		closestFacetInfo.tmin = lambda;
		closestFacetInfo.contactPoint = capsuleIn.m_startPosition + lambda * capsuleIn.m_direction + rotate(capsuleIn.m_orientation, PfxVector3(pB));
		return true;
	}

	else if (err == kPfxGjkResultIntersect) {
		pfxClosestPointTriangle(capsuleIn.m_startPosition, triangle, closestFacetInfo.contactPoint);
		closestFacetInfo.squaredDistance = lengthSqr(closestFacetInfo.contactPoint - capsuleIn.m_startPosition);
		closestFacetInfo.tmin = 0.0f;
		return true;
	}
	return false;
}
PfxBool pfxIntersectMovingCapsuleTriangle(const PfxCapsuleInputInternal &capsuleIn, const PfxTriangle &triangle, PfxVector3 &contactPoint, PfxFloat &variable, PfxFloat &squaredDistance)
{
	PfxClosestFacetInfo closestFacetInfo;
	closestFacetInfo.squaredDistance = squaredDistance;

	if (pfxIntersectMovingCapsuleOneTriangle(triangle, capsuleIn, closestFacetInfo)) {
		contactPoint = closestFacetInfo.contactPoint;
		variable = closestFacetInfo.tmin;
		squaredDistance = closestFacetInfo.squaredDistance;
		return true;
	}
	return false;
}

static PfxBool pfxIntersectMovingCapsuleLargeTriMeshAllTriangles(const PfxCapsuleInputInternal &capsuleIn, PfxCapsuleOutputInternal &capsuleOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;

	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal> bvhDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, transform, flipTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastTriMeshArrayObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal> arrayDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, transform, flipTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
}

// ----------------------------------------------------------------------------
//	Capsule cast with discarding triangle callback

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_DISCARD_TRIANGLE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

static PfxBool pfxIntersectMovingCapsuleLargeTriMeshPartialTriangles(const PfxCapsuleInputInternal &capsuleIn, PfxCapsuleOutputInternal &capsuleOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhPartialObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastTriMeshArrayPartialObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
}

#undef CAST_ENABLE_DISCARD_TRIANGLE

PfxBool pfxIntersectMovingCapsuleLargeTriMesh(const PfxCapsuleInputInternal &capsuleIn, PfxCapsuleOutputInternal &capsuleOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	if (discardTriangleCallback == NULL) {
		return pfxIntersectMovingCapsuleLargeTriMeshAllTriangles(capsuleIn, capsuleOut, shape, transform, flipTriangle, radiusLocal);
	}
	return pfxIntersectMovingCapsuleLargeTriMeshPartialTriangles(capsuleIn, capsuleOut, shape, transform, flipTriangle, radiusLocal, discardTriangleCallback, userDataForDiscardingTriangle);
}

// ----------------------------------------------------------------------------
//	Capsule cast of cached rigidbody 

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_RIGIDBODY_CACHE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

// ----------------------------------------------------------------------------
//	Capsule cast of cached rigidbody with discarding triangle callback

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_DISCARD_TRIANGLE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

PfxBool pfxIntersectMovingCapsuleIslandsOfLargeTriMeshBvh(const PfxCapsuleInputInternal &capsuleIn, PfxCapsuleOutputInternal &capsuleOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandBvhObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal> bvhDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastCachedIslandBvhPartialObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
}

PfxBool pfxIntersectMovingCapsuleIslandsOfLargeTriMeshArray(const PfxCapsuleInputInternal &capsuleIn, PfxCapsuleOutputInternal &capsuleOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandArrayObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal> arrayDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastCachedIslandArrayPartialObj<PfxCapsuleInputInternal, PfxCapsuleOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectMovingCapsuleOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(capsuleIn, capsuleOut, radiusLocal);
	}
}

#undef CAST_ENABLE_RIGIDBODY_CACHE
#undef CAST_ENABLE_DISCARD_TRIANGLE

} //namespace pfxv4
} //namespace sce
