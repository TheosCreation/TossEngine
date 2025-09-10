/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_DETECT_COLLISION_FUNC_H
#define _SCE_PFX_DETECT_COLLISION_FUNC_H

#include "pfx_contact_cache.h"
#include "pfx_shape.h"

namespace sce {
namespace pfxv4 {

typedef void (*pfx_detect_collision_func)(
				PfxContactCache &contacts,
				const PfxShape & shapeA,const PfxTransform3 &offsetTransformA,const PfxTransform3 &worldTransformA0,const PfxTransform3 &worldTransformA1,int shapeIdA,
				const PfxShape & shapeB,const PfxTransform3 &offsetTransformB,const PfxTransform3 &worldTransformB0,const PfxTransform3 &worldTransformB1,int shapeIdB,
				PfxFloat contactThreshold);

SCE_PFX_API pfx_detect_collision_func pfxGetDetectCollisionFunc(PfxUInt8 shapeTypeA,PfxUInt8 shapeTypeB);

SCE_PFX_API int pfxSetDetectCollisionFunc(PfxUInt8 shapeTypeA,PfxUInt8 shapeTypeB,pfx_detect_collision_func func);

} //namespace pfxv4
} //namespace sce
#endif // _SCE_PFX_DETECT_COLLISION_FUNC_H
