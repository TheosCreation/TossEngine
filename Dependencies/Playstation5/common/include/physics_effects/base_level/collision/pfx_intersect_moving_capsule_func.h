/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_INTERSECT_MOVING_CAPSULE_FUNC_H
#define _SCE_PFX_INTERSECT_MOVING_CAPSULE_FUNC_H

#include "pfx_ray.h"
#include "pfx_shape.h"

namespace sce {
namespace pfxv4 {

typedef PfxBool (*PfxIntersectMovingCapsuleFunc)(
				const PfxCapsuleInputInternal &capsuleIn,PfxCapsuleOutputInternal &capsuleOut,
				const PfxShape &shape,const PfxTransform3 &transform, 
				pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userDataForDiscardingTriangle);

SCE_PFX_API PfxIntersectMovingCapsuleFunc pfxGetIntersectMovingCapsuleFunc(PfxUInt8 shapeType);

SCE_PFX_API PfxInt32 pfxSetIntersectMovingCapsuleFunc(PfxUInt8 shapeType,PfxIntersectMovingCapsuleFunc func);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_INTERSECT_MOVING_CAPSULE_FUNC_H */
