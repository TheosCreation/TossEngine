/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_INTERSECT_MOVING_SPHERE_ALL_FACTES_H
#define _SCE_PFX_INTERSECT_MOVING_SPHERE_ALL_FACTES_H

#include "../../../include/physics_effects/base_level/collision/pfx_ray.h"
#include "../../../include/physics_effects/low_level/collision/pfx_sphere_cast.h"

namespace sce {
namespace pfxv4 {

PfxBool pfxIntersectMovingSphereAllFacetsInLargeTriMesh(
	const PfxSphereInputInternal &sphereIn,
	PfxBool(*callback)(const PfxSphereOutput &hit, void *userData),
	void *userData,
	const void *shape,
	const PfxTransform3 &transform,
	PfxBool flipTriangle,
	PfxFloat radiusLocal,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback,
	void *userDataForDiscardingTriangle);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_INTERSECT_MOVING_SPHERE_ALL_FACTES_H
