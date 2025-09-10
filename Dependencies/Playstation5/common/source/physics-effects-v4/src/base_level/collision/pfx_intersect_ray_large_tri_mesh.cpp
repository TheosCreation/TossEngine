/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"
#include "../../../include/physics_effects/util/pfx_static_array.h"
#include "pfx_large_tri_mesh_impl.h"
#include "pfx_intersect_common.h"
#include "pfx_mesh_common.h"
#include "pfx_decoded_tri_mesh.h"
#include "pfx_intersect_ray_large_tri_mesh.h"
#include "../../low_level/collision/pfx_rigid_body_cache_manager.h"
#include "../../low_level/collision/pfx_bounding_volume_vector3.h"
#include "../../../include/physics_effects/base_level/sort/pfx_sort.h"

namespace sce {
namespace pfxv4 {

static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcCastRadius(const PfxRayInputInternal &rayIn) { return 0.0f; }
static SCE_PFX_FORCE_INLINE PfxFloat pfxCalcInitialSquaredDistance(const PfxRayInputInternal &rayIn) { return 0.0f; }

#define CAST_NO_RADIUS

// ----------------------------------------------------------------------------
//	General ray cast

#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

static PfxBool pfxIntersectRayOneTriangle(const PfxTriangle &triangle, const PfxRayInputInternal &rayIn, PfxClosestFacetInfo &closestFacetInfo)
{
	if (pfxIntersectRayTriangle(rayIn.m_startPosition, rayIn.m_direction, rayIn.m_facetMode, triangle, closestFacetInfo.tmin)) {
		closestFacetInfo.squaredDistance = 0.0f;
		closestFacetInfo.contactPoint = rayIn.m_startPosition + rayIn.m_direction * closestFacetInfo.tmin;
		return true;
	}
	return false;
}

static PfxBool pfxIntersectRayLargeTriMeshAllTriangles(const PfxRayInputInternal &rayIn, PfxRayOutputInternal &rayOut, const void *shape, const PfxTransform3 &transform)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;

	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhObj<PfxRayInputInternal, PfxRayOutputInternal> bvhDetector(&pfxIntersectRayOneTriangle, largeMesh, transform, false);
		return bvhDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
	else {
		PfxIntersectAllCastTriMeshArrayObj<PfxRayInputInternal, PfxRayOutputInternal> arrayDetector(&pfxIntersectRayOneTriangle, largeMesh, transform, false);
		return arrayDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
}

// ----------------------------------------------------------------------------
//	Ray cast with discarding triangle callback

#undef _SCE_PFX_INTERSECT_ALL_CAST_LARGE_TRI_MESH_OBJ_TEMPLATE_H
#define CAST_ENABLE_DISCARD_TRIANGLE
#include "pfx_intersect_all_cast_large_tri_mesh_obj_template.h"

static PfxBool pfxIntersectRayLargeTriMeshPartialTriangles(const PfxRayInputInternal &rayIn, PfxRayOutputInternal &rayOut, const void *shape, const PfxTransform3 &transform, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (largeMesh->isUsingBvh()) {
		PfxIntersectAllCastTriMeshBvhPartialObj<PfxRayInputInternal, PfxRayOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectRayOneTriangle, largeMesh, transform, false, discardTriangleCallback, userDataForDiscardingTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
	else {
		PfxIntersectAllCastTriMeshArrayPartialObj<PfxRayInputInternal, PfxRayOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectRayOneTriangle, largeMesh, transform, false, discardTriangleCallback, userDataForDiscardingTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
}

#undef CAST_ENABLE_DISCARD_TRIANGLE

PfxBool pfxIntersectRayLargeTriMesh(const PfxRayInputInternal &rayIn, PfxRayOutputInternal &rayOut, const void *shape, const PfxTransform3 &transform, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	if (discardTriangleCallback == NULL) {
		return pfxIntersectRayLargeTriMeshAllTriangles(rayIn, rayOut, shape, transform);
	}
	return pfxIntersectRayLargeTriMeshPartialTriangles(rayIn, rayOut, shape, transform, discardTriangleCallback, userDataForDiscardingTriangle);
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

PfxBool pfxIntersectRayIslandsOfLargeTriMeshBvh(const PfxRayInputInternal &rayIn, PfxRayOutputInternal &rayOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &transform, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandBvhObj<PfxRayInputInternal, PfxRayOutputInternal> bvhDetector(&pfxIntersectRayOneTriangle, largeMesh, islandIds, numIslands, transform, false);
		return bvhDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
	else {
		PfxIntersectAllCastCachedIslandBvhPartialObj<PfxRayInputInternal, PfxRayOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> bvhDetector(&pfxIntersectRayOneTriangle, largeMesh, islandIds, numIslands, transform, false, discardTriangleCallback, userDataForDiscardingTriangle);
		return bvhDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
}

PfxBool pfxIntersectRayIslandsOfLargeTriMeshArray(const PfxRayInputInternal &rayIn, PfxRayOutputInternal &rayOut, const void *shape, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &transform, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle)
{
	const PfxLargeTriMeshImpl *largeMesh = (PfxLargeTriMeshImpl*)shape;
	if (discardTriangleCallback == NULL) {
		PfxIntersectAllCastCachedIslandArrayObj<PfxRayInputInternal, PfxRayOutputInternal> arrayDetector(&pfxIntersectRayOneTriangle, largeMesh, islandIds, numIslands, transform, false);
		return arrayDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
	else {
		PfxIntersectAllCastCachedIslandArrayPartialObj<PfxRayInputInternal, PfxRayOutputInternal, PfxRayHitDiscardTriangleCallbackTriData> arrayDetector(&pfxIntersectRayOneTriangle, largeMesh, islandIds, numIslands, transform, false, discardTriangleCallback, userDataForDiscardingTriangle);
		return arrayDetector.intersectAllCastLargeTriMesh(rayIn, rayOut, 0.0f);
	}
}

#undef CAST_ENABLE_RIGIDBODY_CACHE
#undef CAST_ENABLE_DISCARD_TRIANGLE

} //namespace pfxv4
} //namespace sce
