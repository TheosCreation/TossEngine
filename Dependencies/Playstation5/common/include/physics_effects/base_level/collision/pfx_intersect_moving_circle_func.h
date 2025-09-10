/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
*                Copyright (C) 2020 Sony Interactive Entertainment Inc.
*                                                
*/

#ifndef _SCE_PFX_INTERSECT_MOVING_CIRCLE_FUNC_H
#define _SCE_PFX_INTERSECT_MOVING_CIRCLE_FUNC_H

#include "pfx_ray.h"
#include "pfx_shape.h"

namespace sce {
namespace pfxv4 {

typedef PfxBool(*PfxIntersectMovingCircleFunc)(
	const PfxCircleInputInternal &circleIn, PfxCircleOutputInternal &circleOut,
	const PfxShape &shape, const PfxTransform3 &transform,
	pfxRayHitDiscardTriangleCallback discardTriangleCallback, void *userData);

SCE_PFX_API PfxIntersectMovingCircleFunc pfxGetIntersectMovingCircleFunc(PfxUInt8 shapeType);

SCE_PFX_API PfxInt32 pfxSetIntersectMovingCircleFunc(PfxUInt8 shapeType, PfxIntersectMovingCircleFunc func);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_INTERSECT_MOVING_CIRCLE_FUNC_H */

