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
#include "pfx_intersect_moving_sphere_large_tri_mesh.h"
#include "../../low_level/collision/pfx_rigid_body_cache_manager.h"
#include "../../low_level/broadphase/pfx_bounding_volume.h"
#include "../../low_level/collision/pfx_bounding_volume_vector3.h"
#include <float.h>

namespace sce {
namespace pfxv4 {

static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcCastRadius(const PfxSphereInputInternal &sphereIn) { return (sphereIn.m_radius); }
static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcInitialSquaredDistance(const PfxSphereInputInternal &sphereIn) { return (sphereIn.m_radius) * (sphereIn.m_radius); }

// ----------------------------------------------------------------------------
//	General sphere cast

#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

// defined in pfx_intersect_moving_sphere_convex.cpp
extern PfxBool pfxIntersectMovingSphereTriangle(const PfxVector3 &rayStartPosition,const PfxVector3 &rayDirection,PfxFloat rayRadius,const PfxTriangle &triangle,PfxFloat &variable,PfxFloat &squaredDistance);
static PfxBool pfxIntersectMovingSphereOneTriangle(const PfxTriangle &triangle, const PfxSphereInputInternal &sphereIn, PfxClosestFacetInfo &closestFacetInfo)
{
	if (pfxIntersectMovingSphereTriangle(sphereIn.m_startPosition, sphereIn.m_direction, sphereIn.m_radius, triangle, closestFacetInfo.tmin, closestFacetInfo.squaredDistance))
	{
		PfxVector3 stopPoint = sphereIn.m_startPosition + sphereIn.m_direction * closestFacetInfo.tmin;
		pfxClosestPointTriangle(stopPoint, triangle, closestFacetInfo.contactPoint);
		return true;
	}
	return false;
}
static PfxBool pfxIntersectMovingSphereLargeTriMeshAllTriangles(const PfxSphereInputInternal &sphereIn, PfxSphereOutputInternal &sphereOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhObj<PfxSphereInputInternal, PfxSphereOutputInternal> bvhDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, transform, flipTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastTriMeshArrayObj<PfxSphereInputInternal, PfxSphereOutputInternal> arrayDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, transform, flipTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
}

// ----------------------------------------------------------------------------
//	Sphere cast with discarding triangle callback

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_DISCARD_TRIANGLE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

static PfxBool pfxIntersectMovingSphereLargeTriMeshPartialTriangles(const PfxSphereInputInternal &sphereIn, PfxSphereOutputInternal &sphereOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhPartialObj<PfxSphereInputInternal, PfxSphereOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastTriMeshArrayPartialObj<PfxSphereInputInternal, PfxSphereOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
}

#undef CAST_ENABLE_DISCARD_TRIANGLE

PfxBool pfxIntersectMovingSphereLargeTriMesh(const PfxSphereInputInternal &sphereIn, PfxSphereOutputInternal &sphereOut, const void *shape, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	if (discardTriangleCallback == NULL) {
		return pfxIntersectMovingSphereLargeTriMeshAllTriangles(sphereIn, sphereOut, shape, transform, flipTriangle, radiusLocal);
	}
	return pfxIntersectMovingSphereLargeTriMeshPartialTriangles(sphereIn, sphereOut, shape, transform, flipTriangle, radiusLocal, discardTriangleCallback, userDataForDiscardingTriangle);
}

// ----------------------------------------------------------------------------
//	Ray cast of cached rigidbody 

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_RIGIDBODY_CACHE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

// ----------------------------------------------------------------------------
//	Ray cast of cached rigidbody with discarding triangle callback

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_DISCARD_TRIANGLE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

PfxBool pfxIntersectMovingSphereIslandsOfLargeTriMeshBvh(const PfxSphereInputInternal &sphereIn, PfxSphereOutputInternal &sphereOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandBvhObj<PfxSphereInputInternal, PfxSphereOutputInternal> bvhDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastCachedIslandBvhPartialObj<PfxSphereInputInternal, PfxSphereOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
}

PfxBool pfxIntersectMovingSphereIslandsOfLargeTriMeshArray(const PfxSphereInputInternal &sphereIn, PfxSphereOutputInternal &sphereOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandArrayObj<PfxSphereInputInternal, PfxSphereOutputInternal> arrayDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
	else {
		PfxIntersectAllCastCachedIslandArrayPartialObj<PfxSphereInputInternal, PfxSphereOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectMovingSphereOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle, discardTriangleCallback, userDataForDiscardingTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(sphereIn, sphereOut, radiusLocal);
	}
	return false;
}

/*PfxBool pfxIntersectMovingSphereIslandsOfLargeTriMeshArray(const PfxSphereInputInternal &sphereIn, PfxSphereOutputInternal &sphereOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &transform, PfxBool flipTriangle, PfxFloat radiusLocal, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
//		return pfxIntersectAllCastCachedIslandArray(sphereIn, sphereOut, &pfxIntersectMovingSphereOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle, radiusLocal);
	}
//	return pfxIntersectAllCastCachedIslandArrayPartial(sphereIn, sphereOut, &pfxIntersectMovingSphereOneTriangle, largeMesh, islandIds, numIslands, transform, flipTriangle, radiusLocal, discardTriangleCallback, userDataForDiscardingTriangle);
	return false;
}*/

//template PfxBool pfxIntersectAllCastCachedIslandArray(const PfxSphereInputInternal &, PfxSphereOutputInternal &, pfxIntersectAllCastTriangleFuncPtr<PfxSphereInputInternal>, const PfxLargeTriMeshImpl *, const PfxUInt32[], PfxUInt32, const PfxTransform3 &, PfxBool, PfxFloat);
//template PfxBool pfxIntersectAllCastCachedIslandArrayPartial(const PfxSphereInputInternal &, PfxSphereOutputInternal &, pfxIntersectAllCastTriangleFuncPtr<PfxSphereInputInternal>, const PfxLargeTriMeshImpl *, const PfxUInt32[], PfxUInt32, const PfxTransform3 &, PfxBool, PfxFloat, pfxIntersectAllCastDiscardTriangleCallbackFuncPtr<PfxRayHitDiscardTriangleCallbackTriData>, void*);

#undef CAST_ENABLE_RIGIDBODY_CACHE
#undef CAST_ENABLE_DISCARD_TRIANGLE


} //namespace pfxv4
} //namespace sce
