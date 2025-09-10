/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_INTERSECT_MOVING_SPHERE_FUNC_H
#define _SCE_PFX_INTERSECT_MOVING_SPHERE_FUNC_H

#include "pfx_ray.h"
#include "pfx_shape.h"
#include "../../low_level/collision/pfx_sphere_cast.h"

namespace sce {
namespace pfxv4 {

typedef PfxBool (*PfxIntersectMovingSphereFunc)(
				const PfxSphereInputInternal &sphereIn,PfxSphereOutputInternal &sphereOut,
				const PfxShape &shape,const PfxTransform3 &transform,
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle);

PfxIntersectMovingSphereFunc pfxGetIntersectMovingSphereFunc(PfxUInt8 shapeType);

SCE_PFX_API PfxInt32 pfxSetIntersectMovingSphereFunc(PfxUInt8 shapeType,PfxIntersectMovingSphereFunc func);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_INTERSECT_MOVING_SPHERE_FUNC_H */
