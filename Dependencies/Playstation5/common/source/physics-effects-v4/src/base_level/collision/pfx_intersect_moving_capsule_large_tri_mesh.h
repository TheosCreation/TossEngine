/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_INTERSECT_MOVING_CAPSULE_LARGE_TRI_MESH_H
#define _SCE_PFX_INTERSECT_MOVING_CAPSULE_LARGE_TRI_MESH_H

#include "../../../include/physics_effects/base_level/collision/pfx_ray.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectMovingCapsuleLargeTriMesh(
	const PfxCapsuleInputInternal &capsuleIn,
	PfxCapsuleOutputInternal &capsuleOut,
	const void *shape,
	const PfxTransform3 &transform,
	PfxBool flipTriangle,
	PfxFloat radiusLocal,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle);

PfxBool pfxIntersectMovingCapsuleIslandsOfLargeTriMeshBvh(
	const PfxCapsuleInputInternal &capsuleIn,
	PfxCapsuleOutputInternal &capsuleOut,
	const void *largeTriMesh,
	const PfxUInt32 islandIds[],
	PfxUInt32 numIslands,
	const PfxTransform3 &localToWorldTransform,
	PfxBool flipTriangle,
	PfxFloat radiusLocal,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle);

PfxBool pfxIntersectMovingCapsuleIslandsOfLargeTriMeshArray(
	const PfxCapsuleInputInternal &capsuleIn,
	PfxCapsuleOutputInternal &capsuleOut,
	const void *largeTriMesh,
	const PfxUInt32 islandIds[],
	PfxUInt32 numIslands,
	const PfxTransform3 &localToWorldTransform,
	PfxBool flipTriangle,
	PfxFloat radiusLocal,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_INTERSECT_MOVING_CAPSULE_LARGE_TRI_MESH_H
