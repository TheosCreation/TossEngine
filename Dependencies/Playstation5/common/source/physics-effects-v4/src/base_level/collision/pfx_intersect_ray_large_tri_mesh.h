/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_INTERSECT_RAYLARGETRIMESH_H
#define _SCE_PFX_INTERSECT_RAYLARGETRIMESH_H

#include "../../../include/physics_effects/base_level/collision/pfx_ray.h"
#include "../../../include/physics_effects/low_level/collision/pfx_ray_cast.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectRayLargeTriMesh(const PfxRayInputInternal &ray,PfxRayOutputInternal &out,const void *shape,const PfxTransform3 &transform, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle);

PfxBool pfxIntersectRayIslandsOfLargeTriMeshBvh(const PfxRayInputInternal &rayIn, PfxRayOutputInternal &rayOut, const void *largeTriMesh, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &localToWorldTransform, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle);
PfxBool pfxIntersectRayIslandsOfLargeTriMeshArray(const PfxRayInputInternal &rayIn, PfxRayOutputInternal &rayOut, const void *largeTriMesh, const PfxUInt32 islandIds[], PfxUInt32 numIslands, const PfxTransform3 &localToWorldTransform, pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle);


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_INTERSECT_RAYLARGETRIMESH_H
